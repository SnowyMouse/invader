#include <getopt.h>
#include <zlib.h>
#include <filesystem>
#include <tiffio.h>
#include <png.h>

#define STB_DXT_IMPLEMENTATION
#define STB_DXT_USE_ROUNDING_BIAS
#include "stb/stb_dxt.h"
#include "../eprintf.hpp"
#include "../version.hpp"
#include "../tag/hek/class/bitmap.hpp"
#include "composite_bitmap.hpp"

enum SUPPORTED_FORMATS_INT {
    SUPPORTED_FORMATS_TIF = 0,
    SUPPORTED_FORMATS_TIFF,
    SUPPORTED_FORMATS_PNG,

    SUPPORTED_FORMATS_INT_COUNT
};

const char *SUPPORTED_FORMATS[] = {
    ".tif",
    ".tiff",
    ".png"
};

static_assert(sizeof(SUPPORTED_FORMATS) / sizeof(*SUPPORTED_FORMATS) == SUPPORTED_FORMATS_INT_COUNT);

static Invader::CompositeBitmapPixel *load_tiff(const char *path, std::uint32_t &image_width, std::uint32_t &image_height, std::size_t &image_size);
static Invader::CompositeBitmapPixel *load_png(const char *path, std::uint32_t &image_width, std::uint32_t &image_height, std::size_t &image_size);

enum MipmapScaleType {
    /** Guess based on what the tag says */
    MIPMAP_SCALE_TYPE_TAG,

    /** Interpolate colors */
    MIPMAP_SCALE_TYPE_LINEAR,

    /** Interpolate RGB only */
    MIPMAP_SCALE_TYPE_NEAREST_ALPHA,

    /** Do not interpolate colors */
    MIPMAP_SCALE_TYPE_NEAREST,

    /** No mipmap */
    MIPMAP_SCALE_TYPE_NONE
};

int main(int argc, char *argv[]) {
    using namespace Invader::HEK;
    using namespace Invader;
    int opt;

    // Data directory
    const char *data = "data/";

    // Tags directory
    const char *tags = "tags/";

    // Scale type?
    MipmapScaleType mipmap_scale_type = MipmapScaleType::MIPMAP_SCALE_TYPE_TAG;

    // Format?
    BitmapFormat format = BitmapFormat::BITMAP_FORMAT_32_BIT_COLOR;

    // Mipmap fade factor
    float mipmap_fade = -1.0F;

    // Dithering?
    bool dithering = false;

    // Generate this many mipmaps
    std::int16_t max_mipmap_count = INT16_MAX;

    // Long options
    int longindex = 0;
    static struct option options[] = {
        {"info",  no_argument, 0, 'i'},
        {"help",  no_argument, 0, 'h'},
        {"dithering",  no_argument, 0, 'D'},
        {"data",  required_argument, 0, 'd' },
        {"tags",  required_argument, 0, 't' },
        {"format", required_argument, 0, 'f' },
        {"input-format", required_argument, 0, 'I' },
        {"output-format", required_argument, 0, 'O' },
        {"mipmap-count", required_argument, 0, 'm' },
        {"mipmap-fade", required_argument, 0, 'f' },
        {"mipmap-scale", required_argument, 0, 's' },
        {0, 0, 0, 0 }
    };

    // Go through each argument
    while((opt = getopt_long(argc, argv, "Dihd:t:f:s:f:O:m:", options, &longindex)) != -1) {
        switch(opt) {
            case 'd':
                data = optarg;
                break;

            case 't':
                tags = optarg;
                break;

            case 'i':
                INVADER_SHOW_INFO
                return EXIT_FAILURE;

            case 'f':
                mipmap_fade = std::strtof(optarg, nullptr);
                if(mipmap_fade < 0.0F || mipmap_fade > 1.0F) {
                    eprintf("Mipmap fade must be between 0-1\n");
                    return EXIT_FAILURE;
                }
                break;

            case 's':
                if(std::strcmp(optarg, "linear") == 0) {
                    mipmap_scale_type = MipmapScaleType::MIPMAP_SCALE_TYPE_LINEAR;
                }
                else if(std::strcmp(optarg, "nearest") == 0) {
                    mipmap_scale_type = MipmapScaleType::MIPMAP_SCALE_TYPE_NEAREST;
                }
                else if(std::strcmp(optarg, "nearest-alpha") == 0) {
                    mipmap_scale_type = MipmapScaleType::MIPMAP_SCALE_TYPE_NEAREST_ALPHA;
                }
                else if(std::strcmp(optarg, "none") == 0) {
                    mipmap_scale_type = MipmapScaleType::MIPMAP_SCALE_TYPE_NONE;
                }
                else {
                    eprintf("Unknown mipmap scale type %s\n", optarg);
                    return EXIT_FAILURE;
                }
                break;

            case 'O':
                if(std::strcmp(optarg, "32-bit") == 0) {
                    format = BitmapFormat::BITMAP_FORMAT_32_BIT_COLOR;
                }
                else if(std::strcmp(optarg, "16-bit") == 0) {
                    format = BitmapFormat::BITMAP_FORMAT_16_BIT_COLOR;
                }
                else if(std::strcmp(optarg, "dxt5") == 0) {
                    format = BitmapFormat::BITMAP_FORMAT_COMPRESSED_WITH_INTERPOLATED_ALPHA;
                }
                else if(std::strcmp(optarg, "dxt3") == 0) {
                    format = BitmapFormat::BITMAP_FORMAT_COMPRESSED_WITH_EXPLICIT_ALPHA;
                }
                else if(std::strcmp(optarg, "dxt1") == 0) {
                    format = BitmapFormat::BITMAP_FORMAT_COMPRESSED_WITH_COLOR_KEY_TRANSPARENCY;
                }
                break;

            case 'D':
                dithering = true;
                break;

            case 'm':
                max_mipmap_count = static_cast<std::int32_t>(std::strtol(optarg, nullptr, 10));
                break;

            default:
                eprintf("Usage: %s [options] <bitmap-tag>\n\n", *argv);
                eprintf("Create or modify a bitmap tag.\n\n");
                eprintf("Options:\n");
                eprintf("    --info,-i                  Show license and credits.\n");
                eprintf("    --help,-h                  Show help\n\n");
                eprintf("Directory options:\n");
                eprintf("    --data,-d <path>           Set the data directory.\n");
                eprintf("    --tags,-t <path>           Set the tags directory.\n\n");
                eprintf("Bitmap options:\n");
                eprintf("    --dithering,-D             Apply dithering. Only works on dxt5/dxt1 for now.\n");
                eprintf("    --output-format,-O <type>  Output format. Can be: 32-bit, 16-bit, dxt5,\n");
                eprintf("                               dxt3, or dxt1. Default (new tag): 32-bit\n");
                eprintf("    --mipmap-count,-m <count>  Set maximum mipmaps. Negative numbers discard\n");
                eprintf("                               <count> mipmaps, instead.\n");
                eprintf("    --mipmap-fade,-f <factor>  Set detail fade factor. Default (new tag): 0.0\n");
                eprintf("    --mipmap-scale,-s <type>   Mipmap scale type. Can be: linear, nearest-alpha,\n");
                eprintf("                               nearest, none. Default (new tag): linear\n");

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
    std::uint32_t image_width = 0, image_height = 0;
    std::size_t image_size = 0;
    CompositeBitmapPixel *image_pixels = nullptr;

    // Load the bitmap file
    for(auto i = SUPPORTED_FORMATS_TIF; i < SUPPORTED_FORMATS_INT_COUNT; i = static_cast<SUPPORTED_FORMATS_INT>(i + 1)) {
        std::string image_path = (data_path / (bitmap_tag + SUPPORTED_FORMATS[i])).string();
        if(std::filesystem::exists(image_path)) {
            switch(i) {
                case SUPPORTED_FORMATS_TIF:
                case SUPPORTED_FORMATS_TIFF:
                    image_pixels = load_tiff(image_path.data(), image_width, image_height, image_size);
                    break;
                case SUPPORTED_FORMATS_PNG:
                    image_pixels = load_png(image_path.data(), image_width, image_height, image_size);
                    break;
                case SUPPORTED_FORMATS_INT_COUNT:
                    std::terminate();
                    break;
            }
            break;
        }
    }

    if(image_pixels == nullptr) {
        eprintf("Failed to find %s in %s\nValid formats are:\n", bitmap_tag.data(), data);
        for(auto *format : SUPPORTED_FORMATS) {
            eprintf("    %s\n", format);
        }
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

    // Default mipmap parameters
    if(mipmap_fade == -1.0F) {
        mipmap_fade = 0.0F;
    }
    if(mipmap_scale_type == MipmapScaleType::MIPMAP_SCALE_TYPE_TAG) {
        mipmap_scale_type = MipmapScaleType::MIPMAP_SCALE_TYPE_LINEAR;
    }

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
        bitmap.flags = BigEndian<BitmapDataFlags> {};
        bitmap.registration_point = Point2DInt<BigEndian> {};

        // Calculate how big the bitmap is
        std::size_t total_size = 0;
        std::size_t mipmap_width = bitmap_pixels.get_width();
        std::size_t mipmap_height = bitmap_pixels.get_height();
        std::size_t *smaller_dimension = mipmap_width > mipmap_height ? &mipmap_height : &mipmap_width;

        // Calculate how many mipmaps we can make
        std::size_t bitmap_maximum_mipmap_count = 0;
        for(std::size_t s = *smaller_dimension >> 1; s > 0; s >>= 1) {
            bitmap_maximum_mipmap_count++;
        }

        // If we're removing mipmaps, let's see how many
        if(max_mipmap_count < 0) {
            std::size_t mipmaps_discarded = -max_mipmap_count;
            if(bitmap_maximum_mipmap_count > mipmaps_discarded) {
                bitmap_maximum_mipmap_count -= mipmaps_discarded;
            }
            else {
                bitmap_maximum_mipmap_count = 0; // more mipmaps than we have, so we'll just remove all of them
            }
        }
        else if(static_cast<std::size_t>(max_mipmap_count) < bitmap_maximum_mipmap_count) {
            bitmap_maximum_mipmap_count = static_cast<std::size_t>(max_mipmap_count);
        }

        // Calculate the lowest-resolution mipmap we have
        std::size_t mipmap_count = bitmap_pixels.get_mipmap_count();
        for(std::size_t i = 0; i <= mipmap_count && i <= bitmap_maximum_mipmap_count; i++) {
            total_size += mipmap_width * mipmap_height * 4;
            mipmap_height /= 2;
            mipmap_width /= 2;
        }

        if(mipmap_count > bitmap_maximum_mipmap_count) {
            mipmap_count = bitmap_maximum_mipmap_count;
        }

        // Generate mipmaps?
        const auto *pixels_start = reinterpret_cast<const std::byte *>(bitmap_pixels.get_pixels());
        std::vector<std::byte> current_bitmap_pixels(pixels_start, pixels_start + total_size);

        // Process mipmaps
        if(mipmap_scale_type != MipmapScaleType::MIPMAP_SCALE_TYPE_NONE) {
            while(*smaller_dimension > 0 && mipmap_count < bitmap_maximum_mipmap_count) {
                // Get the last mipmap
                const auto *last_mipmap = reinterpret_cast<const CompositeBitmapPixel *>(current_bitmap_pixels.data() + current_bitmap_pixels.size() - (mipmap_height * 2) * (mipmap_width * 2) * sizeof(CompositeBitmapPixel));

                // Allocate data to hold the new mipmap data
                std::vector<std::byte> this_mipmap_v(mipmap_height * mipmap_width * sizeof(CompositeBitmapPixel));
                auto *this_mipmap = reinterpret_cast<CompositeBitmapPixel *>(this_mipmap_v.data());
                for(std::size_t y = 0; y < mipmap_height; y++) {
                    for(std::size_t x = 0; x < mipmap_width; x++) {
                        // Get the pixels
                        CompositeBitmapPixel pixels[4] = {
                            last_mipmap[x * 2 + y * 2 * mipmap_width * 2],
                            last_mipmap[x * 2 + 1 + y * 2 * mipmap_width * 2],
                            last_mipmap[x * 2 + (y * 2 + 1) * mipmap_width * 2],
                            last_mipmap[x * 2 + 1 + (y * 2 + 1) * mipmap_width * 2]
                        };

                        // Get this mipmap pixel
                        CompositeBitmapPixel &pixel = this_mipmap[y * mipmap_width + x];

                        // Average the pixels
                        switch(mipmap_scale_type) {
                            case MipmapScaleType::MIPMAP_SCALE_TYPE_LINEAR:
                            case MipmapScaleType::MIPMAP_SCALE_TYPE_NEAREST_ALPHA:
                                #define AVERAGE_CHANNEL_VALUE(channel) static_cast<std::uint8_t>(static_cast<std::size_t>(pixels[0].channel + pixels[1].channel + pixels[2].channel + pixels[3].channel) / 4)

                                // Determine whether or not to interpolate the alpha
                                if(mipmap_scale_type == MipmapScaleType::MIPMAP_SCALE_TYPE_NEAREST_ALPHA) {
                                    pixel.alpha = pixels[0].alpha;
                                }
                                else {
                                    pixel.alpha = AVERAGE_CHANNEL_VALUE(alpha);
                                }

                                // Interpolate RGB
                                pixel.red = AVERAGE_CHANNEL_VALUE(red);
                                pixel.green = AVERAGE_CHANNEL_VALUE(green);
                                pixel.blue = AVERAGE_CHANNEL_VALUE(blue);
                                break;

                            case MipmapScaleType::MIPMAP_SCALE_TYPE_NEAREST:
                                pixel = pixels[0];
                                break;

                            default:
                                break;
                        }

                        // Fade to gray?
                        if(mipmap_fade > 0.0F) {
                            // Alpha -> white
                            std::uint32_t alpha_delta = pixel.alpha * mipmap_fade + 1;
                            if(static_cast<std::uint32_t>(0xFF - pixel.alpha) < alpha_delta) {
                                pixel.alpha = 0xFF;
                            }
                            else {
                                pixel.alpha += alpha_delta;
                            }

                            // RGB -> gray
                            pixel.red -= (static_cast<int>(pixel.red) - 0x7F) * mipmap_fade;
                            pixel.green -= (static_cast<int>(pixel.green) - 0x7F) * mipmap_fade;
                            pixel.blue -= (static_cast<int>(pixel.blue) - 0x7F) * mipmap_fade;
                        }
                    }
                }

                // Handle compression
                bitmap.format = BitmapDataFormat::BITMAP_FORMAT_A8R8G8B8;

                // Add the new mipmap
                total_size += this_mipmap_v.size();
                current_bitmap_pixels.insert(current_bitmap_pixels.end(), this_mipmap_v.begin(), this_mipmap_v.end());

                // Halve both dimensions and add mipmap count
                mipmap_height /= 2;
                mipmap_width /= 2;
                mipmap_count++;
            }
        }

        // Determine if there is any alpha present
        enum AlphaType {
            ALPHA_TYPE_NONE,
            ALPHA_TYPE_ONE_BIT,
            ALPHA_TYPE_MULTI_BIT
        };
        AlphaType alpha_present = ALPHA_TYPE_NONE;
        auto *first_pixel = reinterpret_cast<CompositeBitmapPixel *>(current_bitmap_pixels.data());
        auto *last_pixel = reinterpret_cast<CompositeBitmapPixel *>(current_bitmap_pixels.data() + current_bitmap_pixels.size());
        std::size_t pixel_count = last_pixel - first_pixel;

        for(auto *pixel = first_pixel; pixel < last_pixel; pixel++) {
            if(pixel->alpha != 0xFF) {
                if(pixel->alpha == 0) {
                    alpha_present = ALPHA_TYPE_ONE_BIT;
                }
                else {
                    alpha_present = ALPHA_TYPE_MULTI_BIT;
                    break;
                }
            }
        }

        // Set the format
        switch(format) {
            case BitmapFormat::BITMAP_FORMAT_32_BIT_COLOR:
                bitmap.format = alpha_present == AlphaType::ALPHA_TYPE_NONE ? BitmapDataFormat::BITMAP_FORMAT_X8R8G8B8 : BitmapDataFormat::BITMAP_FORMAT_A8R8G8B8;
                break;
            case BitmapFormat::BITMAP_FORMAT_16_BIT_COLOR:
                switch(alpha_present) {
                    case ALPHA_TYPE_NONE:
                        bitmap.format = BitmapDataFormat::BITMAP_FORMAT_R5G6B5;
                        break;
                    case ALPHA_TYPE_ONE_BIT:
                        bitmap.format = BitmapDataFormat::BITMAP_FORMAT_A1R5G5B5;
                        break;
                    case ALPHA_TYPE_MULTI_BIT:
                        bitmap.format = BitmapDataFormat::BITMAP_FORMAT_A4R4G4B4;
                        break;
                }
                break;
            case BitmapFormat::BITMAP_FORMAT_COMPRESSED_WITH_COLOR_KEY_TRANSPARENCY:
                bitmap.format = BitmapDataFormat::BITMAP_FORMAT_DXT1;
                break;
            case BitmapFormat::BITMAP_FORMAT_COMPRESSED_WITH_EXPLICIT_ALPHA:
                bitmap.format = alpha_present == AlphaType::ALPHA_TYPE_NONE ? BitmapDataFormat::BITMAP_FORMAT_DXT1 : BitmapDataFormat::BITMAP_FORMAT_DXT3;
                break;
            case BitmapFormat::BITMAP_FORMAT_COMPRESSED_WITH_INTERPOLATED_ALPHA:
                bitmap.format = alpha_present == AlphaType::ALPHA_TYPE_NONE ? BitmapDataFormat::BITMAP_FORMAT_DXT1 : BitmapDataFormat::BITMAP_FORMAT_DXT5;
                break;

            default:
                eprintf("Unsupported bitmap format.\n");
                return EXIT_FAILURE;
        }

        // Depending on the format, do something
        auto bitmap_format = bitmap.format.read();
        switch(bitmap_format) {
            // If it's 32-bit, this is a no-op.
            case BitmapDataFormat::BITMAP_FORMAT_A8R8G8B8:
            case BitmapDataFormat::BITMAP_FORMAT_X8R8G8B8:
                break;

            // If it's 16-bit, there is stuff we will need to do
            case BitmapDataFormat::BITMAP_FORMAT_A1R5G5B5:
            case BitmapDataFormat::BITMAP_FORMAT_A4R4G4B4:
            case BitmapDataFormat::BITMAP_FORMAT_R5G6B5: {
                // Figure out what we'll be doing
                uint16_t (*conversion_function)(const CompositeBitmapPixel *);

                switch(bitmap_format) {
                    case BitmapDataFormat::BITMAP_FORMAT_A1R5G5B5:
                        conversion_function = CompositeBitmapPixel::convert_to_16_bit<1,5,5,5>;
                        break;
                    case BitmapDataFormat::BITMAP_FORMAT_A4R4G4B4:
                        conversion_function = CompositeBitmapPixel::convert_to_16_bit<4,4,4,4>;
                        break;
                    case BitmapDataFormat::BITMAP_FORMAT_R5G6B5:
                        conversion_function = CompositeBitmapPixel::convert_to_16_bit<0,5,6,5>;
                        break;
                    default:
                        std::terminate();
                        break;
                }

                // Begin
                std::vector<LittleEndian<std::uint16_t>> new_bitmap_pixels(pixel_count * sizeof(std::uint16_t));
                auto *pixel_16_bit = reinterpret_cast<std::uint16_t *>(new_bitmap_pixels.data());
                for(CompositeBitmapPixel *pixel_32_bit = first_pixel; pixel_32_bit < last_pixel; pixel_32_bit++, pixel_16_bit++) {
                    *pixel_16_bit = conversion_function(pixel_32_bit);
                }

                // Replace buffers
                current_bitmap_pixels.clear();
                current_bitmap_pixels.insert(current_bitmap_pixels.end(), reinterpret_cast<std::byte *>(new_bitmap_pixels.begin().base()), reinterpret_cast<std::byte *>(new_bitmap_pixels.end().base()));

                break;
            }

            // Do DisguXTing compression
            case BitmapDataFormat::BITMAP_FORMAT_DXT1:
            case BitmapDataFormat::BITMAP_FORMAT_DXT3:
            case BitmapDataFormat::BITMAP_FORMAT_DXT5: {
                // Begin
                bool use_dxt1 = bitmap.format == BitmapDataFormat::BITMAP_FORMAT_DXT1;
                bool use_dxt3 = bitmap.format == BitmapDataFormat::BITMAP_FORMAT_DXT3;
                bool use_dxt5 = bitmap.format == BitmapDataFormat::BITMAP_FORMAT_DXT5;
                std::size_t pixel_size = (!use_dxt1) ? 2 : 1;
                std::vector<std::byte> new_bitmap_pixels(pixel_count * pixel_size / 2);
                auto *compressed_pixel = new_bitmap_pixels.data();

                std::size_t mipmap_width = bitmap.width;
                std::size_t mipmap_height = bitmap.height;

                auto *uncompressed_pixel = first_pixel;

                #define BLOCK_LENGTH 4

                std::size_t mipmaps_reduced = 0;

                std::size_t pixel_increment = BLOCK_LENGTH * (use_dxt3 ? 1 : pixel_size) * 2;

                // Go through each 4x4 block and make them compressed
                for(std::size_t i = 0; i <= mipmap_count; i++) {
                    if(mipmap_width >= BLOCK_LENGTH && mipmap_height >= BLOCK_LENGTH) {
                        for(std::size_t y = 0; y < mipmap_height; y += BLOCK_LENGTH) {
                            for(std::size_t x = 0; x < mipmap_width; x += BLOCK_LENGTH) {
                                // Let's make the 4x4 block
                                CompositeBitmapPixel block[BLOCK_LENGTH * BLOCK_LENGTH];

                                // Get the block
                                for(int i = 0; i < BLOCK_LENGTH; i++) {
                                    std::size_t offset = ((y + i) * mipmap_width + x);
                                    auto *first_block_pixel = block + i * BLOCK_LENGTH;
                                    auto *first_uncompressed_pixel = uncompressed_pixel + offset;

                                    for(std::size_t j = 0; j < 4; j++) {
                                        first_block_pixel[j].alpha = first_uncompressed_pixel[j].alpha;
                                        first_block_pixel[j].red = first_uncompressed_pixel[j].blue;
                                        first_block_pixel[j].green = first_uncompressed_pixel[j].green;
                                        first_block_pixel[j].blue = first_uncompressed_pixel[j].red;
                                    }
                                }

                                // If we're using DXT3, put the alpha in here
                                if(use_dxt3) {
                                    std::uint64_t dxt3_alpha = 0;

                                    // Alpha is stored in order from the first ones being the least significant bytes, and the last ones being the most significant bytes
                                    for(int i = 0; i < BLOCK_LENGTH * BLOCK_LENGTH; i++) {
                                        dxt3_alpha <<= 4;
                                        dxt3_alpha |= (block[BLOCK_LENGTH * BLOCK_LENGTH - i - 1].alpha * 15 + UINT8_MAX + 1) / UINT8_MAX / 2;
                                    }

                                    auto &compressed_alpha = *reinterpret_cast<LittleEndian<std::uint64_t> *>(compressed_pixel);
                                    compressed_alpha = dxt3_alpha;
                                    compressed_pixel += sizeof(compressed_alpha);
                                }

                                // Compress
                                stb_compress_dxt_block(reinterpret_cast<unsigned char *>(compressed_pixel), reinterpret_cast<unsigned char *>(block), use_dxt5, STB_DXT_HIGHQUAL | (dithering ? STB_DXT_DITHER : 0));
                                compressed_pixel += pixel_increment;
                            }
                        }
                    }
                    else {
                        mipmaps_reduced++;
                    }

                    uncompressed_pixel += mipmap_width * mipmap_height;
                    mipmap_width /= 2;
                    mipmap_height /= 2;
                }

                current_bitmap_pixels.clear();
                current_bitmap_pixels.insert(current_bitmap_pixels.end(), new_bitmap_pixels.data(), reinterpret_cast<std::byte *>(compressed_pixel));

                // If we had to cut out mipmaps due to them being less than 4x4, here we go
                mipmap_count -= mipmaps_reduced;

                #undef BLOCK_LENGTH

                break;
            }

            default:
                bitmap.format = alpha_present == AlphaType::ALPHA_TYPE_NONE ? BitmapDataFormat::BITMAP_FORMAT_X8R8G8B8 : BitmapDataFormat::BITMAP_FORMAT_A8R8G8B8;
                break;
        }

        // Add pixel data to the end
        bitmap_data_pixels.insert(bitmap_data_pixels.end(), current_bitmap_pixels.begin(), current_bitmap_pixels.end());

        bitmap.mipmap_count = mipmap_count;
        bitmap.pixels_count = current_bitmap_pixels.size();

        BitmapDataFlags flags = {};
        flags.compressed = (bitmap.format == BitmapDataFormat::BITMAP_FORMAT_DXT1) || (bitmap.format == BitmapDataFormat::BITMAP_FORMAT_DXT3) || (bitmap.format == BitmapDataFormat::BITMAP_FORMAT_DXT5);
        flags.power_of_two_dimensions = 1;
        bitmap.flags = flags;

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

    // Set more parameters
    new_tag_header.usage = BitmapUsage::BITMAP_USAGE_DEFAULT;
    new_tag_header.detail_fade_factor = mipmap_fade;
    new_tag_header.format = static_cast<BitmapFormat>(format);
    if(max_mipmap_count == INT16_MAX) {
        new_tag_header.mipmap_count = 0;
    }
    else if(max_mipmap_count >= 0) {
        new_tag_header.mipmap_count = max_mipmap_count + 1;
    }
    else {
        new_tag_header.mipmap_count = max_mipmap_count;
    }

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
