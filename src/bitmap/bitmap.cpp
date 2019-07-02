#include <Magick++.h>
#include <getopt.h>
#include <zlib.h>

#ifdef __linux__
#include <sys/stat.h>
#else

#endif

#include "../eprintf.hpp"
#include "../version.hpp"
#include "../tag/hek/class/bitmap.hpp"
#include "composite_bitmap.hpp"

int main(int argc, char *argv[]) {
    using namespace Magick;
    using namespace Invader::HEK;
    using namespace Invader;
    int opt;

    // Data directory
    const char *data = "data/";

    // Tags directory
    const char *tags = "tags/";

    // Should we use LZMA?
    bool use_lzma = false;

    // File format
    const char *input_format = "tif";

    // Long options
    int longindex = 0;
    static struct option options[] = {
        {"about",  no_argument, 0, 'h'},
        {"help",  no_argument, 0, 'a'},
        {"lzma-tiff", no_argument, 0, 'L' },
        {"data",  required_argument, 0, 'd' },
        {"tags",  required_argument, 0, 't' },
        {"format", required_argument, 0, 'f' },
        {"input-type", required_argument, 0, 'i' },
        {0, 0, 0, 0 }
    };

    // Go through each argument
    while((opt = getopt_long(argc, argv, "ahLd:t:i:f:", options, &longindex)) != -1) {
        switch(opt) {
            case 'd':
                data = optarg;
                break;

            case 't':
                tags = optarg;
                break;

            case 'L':
                use_lzma = true;
                break;

            case 'i':
                input_format = optarg;
                break;

            case 'a':
                INVADER_SHOW_INFO
                return EXIT_FAILURE;

            default:
                eprintf("Usage: %s [options] <bitmap-tag>\n\n", *argv);
                eprintf("Create or modify a bitmap tag. If the format is \"tag\", the tag must exist in the\n");
                eprintf("tags folder. Otherwise, the image must exist in the data folder.\n\n");
                eprintf("Options:\n");
                eprintf("    --about,-a                 Show license and credits.\n");
                eprintf("    --help,-h                  Show help\n\n");
                eprintf("Directory options:\n");
                eprintf("    --data,-d <path>           Set the data directory.\n");
                eprintf("    --tags,-t <path>           Set the tags directory.\n\n");
                eprintf("Bitmap options:\n");
                eprintf("    --format,-f <type>         Format used in tag. Can be: dxt, 32bit, 16bit,\n");
                eprintf("                               p8, or monochrome. Default (new tag): 32bit\n");
                eprintf("    --input-type,-i <type>     Input file format. Can be extension or \"tag\".\n");
                eprintf("                               Default: tif\n");
                eprintf("    --lzma-tiff,-L <type>      Compress TIFF data using LZMA. This may not work\n");
                eprintf("                               with the original HEK, but it results in much\n");
                eprintf("                               smaller .bitmap tag files without quality loss.\n");
                return EXIT_FAILURE;
        }
    }

    // Make sure we have the bitmap tag path
    if(optind != argc - 1) {
        eprintf("%s: Expected a bitmap tag path. Use -h for help.\n", argv[0]);
        return EXIT_FAILURE;
    }
    const char *bitmap_tag = argv[argc - 1];

    // Get the data and tag paths to have a directory separator at the end
    std::string data_path = data;
    if(data_path[data_path.length()] != '/') {
        data_path += "/";
    }
    std::string tags_path = tags;
    if(tags_path[tags_path.length()] != '/') {
        tags_path += "/";
    }

    InitializeMagick(*argv);

    // Load the bitmap file
    Image bitmap_file(bitmap_tag);

    // Make a blob
    PixelData bitmap_file_blob(bitmap_file, "BGRA", StorageType::CharPixel);

    // Generate the thing
    CompositeBitmap bitmaps(reinterpret_cast<const CompositeBitmapPixel *>(bitmap_file_blob.data()), bitmap_file.columns(), bitmap_file.rows());
    auto &bitmaps_array = bitmaps.get_bitmaps();
    if(bitmaps_array.size() == 0) {
        eprintf("No bitmaps found in input.\n");
        return EXIT_FAILURE;
    }

    // Start building the bitmap tag
    std::vector<std::byte> bitmap_tag_data(sizeof(TagFileHeader) + sizeof(Bitmap<BigEndian>));
    *reinterpret_cast<TagFileHeader *>(bitmap_tag_data.data()) = TagFileHeader(TagClassInt::TAG_CLASS_BITMAP);
    Bitmap<BigEndian> new_tag_header = {};

    // Compress the original input blob
    {
        z_stream deflate_stream;
        deflate_stream.zalloc = Z_NULL;
        deflate_stream.zfree = Z_NULL;
        deflate_stream.opaque = Z_NULL;
        deflate_stream.avail_in = bitmap_file_blob.length();
        deflate_stream.next_in = const_cast<Bytef *>(reinterpret_cast<const Bytef *>(bitmap_file_blob.data()));
        std::vector<std::byte> compressed_data(bitmap_file_blob.length() * 4);
        deflate_stream.avail_out = compressed_data.size();
        deflate_stream.next_out = reinterpret_cast<Bytef *>(compressed_data.data());

        // Do it
        deflateInit(&deflate_stream, Z_BEST_COMPRESSION);
        deflate(&deflate_stream, Z_FINISH);
        deflateEnd(&deflate_stream);

        // Set fields
        new_tag_header.color_plate_width = bitmap_file.columns();
        new_tag_header.color_plate_height = bitmap_file.rows();

        // Set decompressed size
        BigEndian<std::uint32_t> decompressed_size;
        decompressed_size = static_cast<std::uint32_t>(bitmap_file_blob.length());

        // Set compressed size
        new_tag_header.compressed_color_plate_data.size = sizeof(decompressed_size) + deflate_stream.total_out;

        // Add it to the end now
        bitmap_tag_data.insert(bitmap_tag_data.end(), reinterpret_cast<std::byte *>(&decompressed_size), reinterpret_cast<std::byte *>(&decompressed_size + 1));
        bitmap_tag_data.insert(bitmap_tag_data.end(), compressed_data.data(), compressed_data.data() + deflate_stream.total_out);
    }

    // Now let's add the actual bitmap data
    std::vector<std::byte> bitmap_data_pixels;
    std::vector<BitmapData<BigEndian>> bitmap_data(bitmaps_array.size());

    // Add our bitmap data
    for(std::size_t i = 0; i < bitmaps_array.size(); i++) {
        // Write all of the fields here
        auto &bitmap = bitmap_data[i];
        auto &bitmap_pixels = bitmaps_array[i];
        bitmap.bitmap_class = TagClassInt::TAG_CLASS_BITMAP;
        bitmap.width = bitmap_pixels.get_width();
        bitmap.height = bitmap_pixels.get_height();
        bitmap.depth = 1;
        bitmap.type = BitmapDataType::BITMAP_TYPE__2D_TEXTURE;
        bitmap.format = BitmapDataFormat::BITMAP_FORMAT_A8R8G8B8;
        bitmap.flags = BigEndian<BitmapDataFlags> {};
        bitmap.registration_point = Point2DInt<BigEndian> {};
        bitmap.mipmap_count = bitmap_pixels.get_mipmap_count();

        // Calculate how big the bitmap is
        std::size_t total_size = 0;
        std::size_t mipmap_width = bitmap_pixels.get_width();
        std::size_t mipmap_height = bitmap_pixels.get_height();
        std::size_t mipmap_count = bitmap_pixels.get_mipmap_count();
        for(std::size_t i = 0; i <= mipmap_count; i++) {
            total_size += mipmap_width * mipmap_height * 4;
            mipmap_height /= 2;
            mipmap_width /= 2;
        }
        bitmap.pixels_count = total_size;
        const auto *pixels_start = reinterpret_cast<const std::byte *>(bitmap_pixels.get_pixels());
        bitmap_data_pixels.insert(bitmap_data_pixels.end(), pixels_start, pixels_start + total_size);

        eprintf("Bitmap #%zu: %zux%zu, %zu mipmap%s\n", i, bitmaps_array[i].get_width(), bitmaps_array[i].get_height(), mipmap_count, mipmap_count == 1 ? "" : "s");
    }
    printf("Found %zu bitmap%s total. (%.02f MiB)\n", bitmaps_array.size(), bitmaps_array.size() == 1 ? "" : "s", bitmap_data_pixels.size() / 1024.0F / 1024.0F);

    // Add the bitmap pixel data
    bitmap_tag_data.insert(bitmap_tag_data.end(), bitmap_data_pixels.begin(), bitmap_data_pixels.end());
    new_tag_header.processed_pixel_data.size = bitmap_data_pixels.size();

    // Add the bitmap tag data
    const auto *bitmap_data_start = reinterpret_cast<const std::byte *>(bitmap_data.data());
    const auto *bitmap_data_end = reinterpret_cast<const std::byte *>(bitmap_data.data() + bitmap_data.size());
    bitmap_tag_data.insert(bitmap_tag_data.end(), bitmap_data_start, bitmap_data_end);
    new_tag_header.bitmap_data.count = bitmap_data.size();

    // Add the struct in
    *reinterpret_cast<Bitmap<BigEndian> *>(bitmap_tag_data.data() + sizeof(TagFileHeader)) = new_tag_header;

    // Write it all
    std::FILE *f2 = std::fopen("../haloce/tags/test.bitmap", "wb");
    std::fwrite(bitmap_tag_data.data(), bitmap_tag_data.size(), 1, f2);
    std::fclose(f2);

    return 0;
}
