#include <getopt.h>
#include <zlib.h>
#include <filesystem>
#include <tiffio.h>
#include <png.h>

#include "../eprintf.hpp"
#include "../version.hpp"
#include "../tag/hek/class/bitmap.hpp"
#include "composite_bitmap.hpp"

static Invader::CompositeBitmapPixel *load_tiff(const char *path, std::uint32_t &image_width, std::uint32_t &image_height, std::size_t &image_size);
static Invader::CompositeBitmapPixel *load_png(const char *path, std::uint32_t &image_width, std::uint32_t &image_height, std::size_t &image_size);

int main(int argc, char *argv[]) {
    using namespace Invader::HEK;
    using namespace Invader;
    int opt;

    // Data directory
    const char *data = "data/";

    // Tags directory
    const char *tags = "tags/";

    // Input type
    const char *input_type = "tif";

    // Long options
    int longindex = 0;
    static struct option options[] = {
        {"info",  no_argument, 0, 'i'},
        {"help",  no_argument, 0, 'h'},
        {"data",  required_argument, 0, 'd' },
        {"tags",  required_argument, 0, 't' },
        {"format", required_argument, 0, 'f' },
        {"input-format", required_argument, 0, 'I' },
        {0, 0, 0, 0 }
    };

    // Go through each argument
    while((opt = getopt_long(argc, argv, "ihd:t:f:I:", options, &longindex)) != -1) {
        switch(opt) {
            case 'd':
                data = optarg;
                break;

            case 't':
                tags = optarg;
                break;

            case 'I':
                input_type = optarg;
                break;

            case 'i':
                INVADER_SHOW_INFO
                return EXIT_FAILURE;

            default:
                eprintf("Usage: %s [options] <bitmap-tag>\n\n", *argv);
                eprintf("Create or modify a bitmap tag. If the format is \"tag\", the tag must exist in the\n");
                eprintf("tags folder. Otherwise, the image must exist in the data folder.\n\n");
                eprintf("Options:\n");
                eprintf("    --info,-i                  Show license and credits.\n");
                eprintf("    --help,-h                  Show help\n\n");
                eprintf("Directory options:\n");
                eprintf("    --data,-d <path>           Set the data directory.\n");
                eprintf("    --tags,-t <path>           Set the tags directory.\n\n");
                eprintf("Bitmap options:\n");
                //eprintf("    --format,-f <type>         Format used in tag. Can be: dxt, 32bit, 16bit,\n");
                //eprintf("                               p8, or monochrome. Default (new tag): 32bit\n");
                eprintf("    --input-format,-I <type>   Input format. Can be: tif or png. Default: tif\n");
                return EXIT_FAILURE;
        }
    }

    // Make sure we have the bitmap tag path
    if(optind != argc - 1) {
        eprintf("%s: Expected a bitmap tag path. Use -h for help.\n", argv[0]);
        return EXIT_FAILURE;
    }
    std::string bitmap_tag = argv[argc - 1];
    std::filesystem::path data_path = data;

    // Have these variables handy
    std::uint32_t image_width, image_height;
    std::size_t image_size;
    CompositeBitmapPixel *image_pixels;

    // Load the bitmap file
    std::string image_path = (data_path / (bitmap_tag + "." + input_type)).string();

    if(std::strcmp(input_type, "tif") == 0) {
        image_pixels = load_tiff(image_path.data(), image_width, image_height, image_size);
    }
    else if(std::strcmp(input_type, "png") == 0) {
        image_pixels = load_png(image_path.data(), image_width, image_height, image_size);
    }
    else {
        eprintf("Unrecognized input-type %s\n", input_type);
        return EXIT_FAILURE;
    }

    // Generate the thing
    CompositeBitmap bitmaps(image_pixels, image_width, image_height);
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
        deflate_stream.avail_in = image_size;
        deflate_stream.next_in = const_cast<Bytef *>(reinterpret_cast<const Bytef *>(image_pixels));
        std::vector<std::byte> compressed_data(image_size * 4);
        deflate_stream.avail_out = compressed_data.size();
        deflate_stream.next_out = reinterpret_cast<Bytef *>(compressed_data.data());

        // Do it
        deflateInit(&deflate_stream, Z_BEST_COMPRESSION);
        deflate(&deflate_stream, Z_FINISH);
        deflateEnd(&deflate_stream);

        // Set fields
        new_tag_header.color_plate_width = image_width;
        new_tag_header.color_plate_height = image_height;

        // Set decompressed size
        BigEndian<std::uint32_t> decompressed_size;
        decompressed_size = static_cast<std::uint32_t>(image_size);

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

    // Get the path
    std::filesystem::path tags_path(tags);
    auto tag_path = tags_path / bitmap_tag;

    // Write it all
    std::filesystem::create_directories(tag_path.parent_path());
    auto final_path = tag_path.string() + ".bitmap";
    std::FILE *tag_write = std::fopen(final_path.data(), "wb");
    if(!tag_write) {
        eprintf("Error: Failed to open %s for writing.\n", final_path.data());;
        return EXIT_FAILURE;
    }

    if(std::fwrite(bitmap_tag_data.data(), bitmap_tag_data.size(), 1, tag_write) != 1) {
        eprintf("Error: Failed to write to %s.\n", final_path.data());
        std::fclose(tag_write);
        return EXIT_FAILURE;
    }

    std::fclose(tag_write);

    return 0;
}

static Invader::CompositeBitmapPixel *load_tiff(const char *path, std::uint32_t &image_width, std::uint32_t &image_height, std::size_t &image_size) {
    TIFF *image_tiff = TIFFOpen(path, "r");
    if(!image_tiff) {
        eprintf("Cannot open %s\n", path);
        exit(EXIT_FAILURE);
    }
    TIFFGetField(image_tiff, TIFFTAG_IMAGEWIDTH, &image_width);
    TIFFGetField(image_tiff, TIFFTAG_IMAGELENGTH, &image_height);

    // Read it all
    image_size = image_width * image_height * sizeof(Invader::CompositeBitmapPixel);
    auto *image_pixels = reinterpret_cast<Invader::CompositeBitmapPixel *>(std::calloc(image_size, 1));
    TIFFReadRGBAImageOriented(image_tiff, image_width, image_height, reinterpret_cast<std::uint32_t *>(image_pixels), ORIENTATION_TOPLEFT);

    // Close the TIFF
    TIFFClose(image_tiff);

    // Swap red and blue channels
    for(std::size_t i = 0; i < image_size / 4; i++) {
        Invader::CompositeBitmapPixel swapped = image_pixels[i];
        swapped.red = image_pixels[i].blue;
        swapped.blue = image_pixels[i].red;
        image_pixels[i] = swapped;
    }

    return image_pixels;
}

static Invader::CompositeBitmapPixel *load_png(const char *path, std::uint32_t &image_width, std::uint32_t &image_height, std::size_t &image_size) {
    FILE *image_png_file = fopen(path, "r");
    if(!image_png_file) {
        eprintf("Cannot open %s\n", path);
        exit(EXIT_FAILURE);
    }

    // Check the header
    char header[8] = {};
    std::fread(header, sizeof(header), 1, image_png_file);
    if(png_sig_cmp(reinterpret_cast<png_const_bytep>(header), 0, 8)) {
        eprintf("Invalid PNG file %s\n", path);
        exit(EXIT_FAILURE);
    }

    // Get metadata
    png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    png_init_io(png, image_png_file);
    png_set_sig_bytes(png, 8);
    png_infop png_info = png_create_info_struct(png);
    png_read_info(png, png_info);
    image_width = png_get_image_width(png, png_info);
    image_height = png_get_image_height(png, png_info);

    if(png_get_bit_depth(png, png_info) != 8) {
        eprintf("Unsupported at this time...\n");
        exit(EXIT_FAILURE);
    }

    // Read it
    png_bytep *row_pointers = reinterpret_cast<png_bytep *>(malloc(sizeof(png_bytep) * image_height));
    image_size = 0;
    std::size_t rowbytes_size = png_get_rowbytes(png, png_info);
    for(std::size_t y = 0; y < image_height; y++) {
        row_pointers[y] = reinterpret_cast<png_byte *>(malloc(rowbytes_size));
        image_size += rowbytes_size;
    }
    png_read_image(png, row_pointers);
    fclose(image_png_file);

    // Now allocate and copy stuff
    auto *image_pixels = reinterpret_cast<Invader::CompositeBitmapPixel *>(std::calloc(image_size, 1));
    for(std::size_t y = 0; y < image_height; y++) {
        std::memcpy(image_pixels + y * rowbytes_size / sizeof(*image_pixels), row_pointers[y], rowbytes_size);
        std::free(row_pointers[y]);
    }

    // Free
    std::free(row_pointers);

    // Swap red and blue channels
    for(std::size_t i = 0; i < image_size / 4; i++) {
        Invader::CompositeBitmapPixel swapped = image_pixels[i];
        swapped.red = image_pixels[i].blue;
        swapped.blue = image_pixels[i].red;
        image_pixels[i] = swapped;
    }

    return image_pixels;
}
