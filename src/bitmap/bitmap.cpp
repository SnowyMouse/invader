/*
 * Invader (c) 2018 Kavawuvi
 *
 * This program is free software under the GNU General Public License v3.0 or later. See LICENSE for more information.
 */

#include <getopt.h>
#include <zlib.h>
#include <filesystem>
#include <optional>
#include <tiffio.h>

#define STB_DXT_USE_ROUNDING_BIAS
#include "stb/stb_dxt.h"
#include "stb/stb_image.h"
#include "../eprintf.hpp"
#include "../version.hpp"
#include "../tag/hek/class/bitmap.hpp"
#include "color_plate_scanner.hpp"

enum SUPPORTED_FORMATS_INT {
    SUPPORTED_FORMATS_TIF = 0,
    SUPPORTED_FORMATS_TIFF,
    SUPPORTED_FORMATS_PNG,
    SUPPORTED_FORMATS_TGA,
    SUPPORTED_FORMATS_BMP,

    SUPPORTED_FORMATS_INT_COUNT
};

static const char *SUPPORTED_FORMATS[] = {
    ".tif",
    ".tiff",
    ".png",
    ".tga",
    ".bmp"
};

static_assert(sizeof(SUPPORTED_FORMATS) / sizeof(*SUPPORTED_FORMATS) == SUPPORTED_FORMATS_INT_COUNT);

static Invader::ColorPlatePixel *load_tiff(const char *path, std::uint32_t &image_width, std::uint32_t &image_height, std::size_t &image_size);
static Invader::ColorPlatePixel *load_image(const char *path, std::uint32_t &image_width, std::uint32_t &image_height, std::size_t &image_size);

int main(int argc, char *argv[]) {
    using namespace Invader::HEK;
    using namespace Invader;
    int opt;

    // Data directory
    const char *data = "data/";

    // Tags directory
    const char *tags = "tags/";

    // Scale type?
    std::optional<ScannedColorMipmapType> mipmap_scale_type;

    // Format?
    std::optional<BitmapFormat> format;

    // Mipmap fade factor
    std::optional<float> mipmap_fade;

    // Bitmap type
    std::optional<BitmapType> bitmap_type;

    // Sprite parameters
    std::optional<BitmapSpriteUsage> sprite_usage;
    std::optional<std::uint32_t> sprite_budget;
    std::optional<std::uint32_t> sprite_budget_count;
    std::optional<std::uint32_t> sprite_spacing;

    // Dithering?
    bool dithering = false;

    // Generate this many mipmaps
    std::optional<std::uint16_t> max_mipmap_count;

    // Ignore the tag data?
    bool ignore_tag_data = false;

    // Long options
    int longindex = 0;
    static struct option options[] = {
        {"info",  no_argument, 0, 'i'},
        {"ignore-tag",  no_argument, 0, 'I'},
        {"help",  no_argument, 0, 'h'},
        {"dithering",  no_argument, 0, 'D'},
        {"data",  required_argument, 0, 'd' },
        {"tags",  required_argument, 0, 't' },
        {"format", required_argument, 0, 'F' },
        {"type", required_argument, 0, 'T' },
        {"mipmap-count", required_argument, 0, 'm' },
        {"mipmap-fade", required_argument, 0, 'f' },
        {"mipmap-scale", required_argument, 0, 's' },
        {"budget", required_argument, 0, 'B' },
        {"budget-count", required_argument, 0, 'C' },
        {"spacing", required_argument, 0, 'S' },
        {0, 0, 0, 0 }
    };

    // Go through each argument
    while((opt = getopt_long(argc, argv, "DiIhd:t:f:s:f:F:m:T:S:B:C:", options, &longindex)) != -1) {
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

            case 'I':
                ignore_tag_data = true;
                break;

            case 'f':
                mipmap_fade = std::strtof(optarg, nullptr);
                if(mipmap_fade < 0.0F || mipmap_fade > 1.0F) {
                    eprintf("Mipmap fade must be between 0-1\n");
                    return EXIT_FAILURE;
                }
                break;

            case 's':
                if(std::strcmp(optarg, "linear") == 0) {
                    mipmap_scale_type = ScannedColorMipmapType::SCANNED_COLOR_MIPMAP_LINEAR;
                }
                else if(std::strcmp(optarg, "nearest") == 0) {
                    mipmap_scale_type = ScannedColorMipmapType::SCANNED_COLOR_MIPMAP_NEAREST_ALPHA_COLOR;
                }
                else if(std::strcmp(optarg, "nearest-alpha") == 0) {
                    mipmap_scale_type = ScannedColorMipmapType::SCANNED_COLOR_MIPMAP_NEAREST_ALPHA;
                }
                else {
                    eprintf("Unknown mipmap scale type %s\n", optarg);
                    return EXIT_FAILURE;
                }
                break;

            case 'F':
                if(std::strcmp(optarg, "32-bit") == 0) {
                    format = BitmapFormat::BITMAP_FORMAT_32_BIT_COLOR;
                }
                else if(std::strcmp(optarg, "16-bit") == 0) {
                    format = BitmapFormat::BITMAP_FORMAT_16_BIT_COLOR;
                }
                else if(std::strcmp(optarg, "monochrome") == 0) {
                    format = BitmapFormat::BITMAP_FORMAT_MONOCHROME;
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
                else {
                    eprintf("Unknown format %s\n", optarg);
                    return EXIT_FAILURE;
                }
                break;

            case 'T':
                if(std::strcmp(optarg, "2d") == 0) {
                    bitmap_type = BitmapType::BITMAP_TYPE_2D_TEXTURES;
                }
                else if(std::strcmp(optarg, "3d") == 0) {
                    bitmap_type = BitmapType::BITMAP_TYPE_3D_TEXTURES;
                }
                else if(std::strcmp(optarg, "cubemap") == 0) {
                    bitmap_type = BitmapType::BITMAP_TYPE_CUBE_MAPS;
                }
                else if(std::strcmp(optarg, "interface") == 0) {
                    bitmap_type = BitmapType::BITMAP_TYPE_INTERFACE_BITMAPS;
                }
                else if(std::strcmp(optarg, "sprite") == 0) {
                    bitmap_type = BitmapType::BITMAP_TYPE_SPRITES;
                }
                else {
                    eprintf("Unknown type %s\n", optarg);
                    return EXIT_FAILURE;
                }
                break;

            case 'D':
                dithering = true;
                break;

            case 'm':
                max_mipmap_count = static_cast<std::int32_t>(std::strtol(optarg, nullptr, 10));
                break;

            case 'C':
                sprite_budget_count = static_cast<std::uint32_t>(std::strtoul(optarg, nullptr, 10));
                break;

            case 'B':
                sprite_budget = static_cast<std::uint32_t>(std::strtoul(optarg, nullptr, 10));
                break;

            case 'S':
                sprite_spacing = static_cast<std::uint32_t>(std::strtoul(optarg, nullptr, 10));
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
                eprintf("    --type,-T                  Set the type of bitmap. Can be: 2d, 3d, cubemap,\n");
                eprintf("                               interface, sprite. Default (new tag): 2d\n");
                eprintf("    --dithering,-D             Apply dithering. Only works on dxtn for now.\n");
                eprintf("    --ignore-tag,-I            Ignore the tag data if the tag exists.\n");
                eprintf("    --format,-F <type>         Pixel format. Can be: 32-bit, 16-bit, monochrome,\n");
                eprintf("                               dxt5, dxt3, or dxt1. Default (new tag): 32-bit\n");
                eprintf("    --mipmap-count,-m <count>  Set maximum mipmaps. Default (new tag): 32767\n");
                eprintf("    --mipmap-fade,-f <factor>  Set detail fade factor. Default (new tag): 0.0\n");
                eprintf("    --mipmap-scale,-s <type>   Mipmap scale type. Can be: linear, nearest-alpha,\n");
                eprintf("                               nearest. Default (new tag): linear\n\n");
                eprintf("Sprite options (only applies to sprite bitmaps):\n");
                eprintf("    --spacing,-S <px>          Set the minimum spacing between sprites in\n");
                eprintf("                               pixels. Default (new tag): 4\n");
                eprintf("    --budget-count,-C <count>  Set maximum number of sprite sheets. Setting this\n");
                eprintf("                               to 0 disables budgeting. Default (new tag): 0\n");
                eprintf("    --budget,-B <length>       Set max length of sprite sheet. Values greater\n");
                eprintf("                               than 512 aren't recorded. Default (new tag): 32\n");

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

    // Get the path
    std::filesystem::path tags_path(tags);
    auto tag_path = tags_path / bitmap_tag;
    auto final_path = tag_path.string() + ".bitmap";

    // See if we can get anything out of this
    std::FILE *tag_read;
    if(!ignore_tag_data && (tag_read = std::fopen(final_path.data(), "rb"))) {
        // Here's in case we do fail. It cleans up and exits.
        auto exit_on_failure = [&tag_read, &final_path]() {
            eprintf("%s could not be read.\n", final_path.data());
            eprintf("Use --ignore-tag or -I to override.\n");
            std::fclose(tag_read);
            exit(EXIT_FAILURE);
        };

        // Attempt to get the header
        TagFileHeader header;
        if(std::fread(&header, sizeof(header), 1, tag_read) != 1) {
            exit_on_failure();
        }

        // Make sure it's valid
        if(header.tag_class_int != TagClassInt::TAG_CLASS_BITMAP) {
            exit_on_failure();
        }

        // Get the main tag struct
        Bitmap<BigEndian> bitmap_tag_header;
        if(std::fread(&bitmap_tag_header, sizeof(bitmap_tag_header), 1, tag_read) != 1) {
            exit_on_failure();
        }

        // Set some default values
        if(!format.has_value()) {
            format = bitmap_tag_header.format;
        }
        if(!mipmap_fade.has_value()) {
            mipmap_fade = bitmap_tag_header.detail_fade_factor;
        }
        if(!bitmap_type.has_value()) {
            bitmap_type = bitmap_tag_header.type;
        }
        if(!max_mipmap_count.has_value()) {
            std::int16_t mipmap_count = bitmap_tag_header.mipmap_count.read();
            if(mipmap_count == 0) {
                max_mipmap_count = INT16_MAX;
            }
            else {
                max_mipmap_count = mipmap_count - 1;
            }
        }
        if(!sprite_usage.has_value()) {
            sprite_usage = bitmap_tag_header.sprite_usage;
        }
        if(!sprite_budget.has_value()) {
            sprite_budget = 32 << bitmap_tag_header.sprite_budget_size;
        }
        if(!sprite_budget_count.has_value()) {
            sprite_budget_count = bitmap_tag_header.sprite_budget_count;
        }
        if(!sprite_spacing.has_value()) {
            sprite_spacing = bitmap_tag_header.sprite_spacing;
        }

        std::fclose(tag_read);
    }

    // If format wasn't set, set it
    if(!format.has_value()) {
        format = BitmapFormat::BITMAP_FORMAT_32_BIT_COLOR;
    }

    // Same with bitmap type
    if(!bitmap_type.has_value()) {
        bitmap_type = BitmapType::BITMAP_TYPE_2D_TEXTURES;
    }

    // And these other things too
    if(!max_mipmap_count.has_value()) {
        max_mipmap_count = INT16_MAX;
    }
    if(!sprite_usage.has_value()) {
        sprite_usage = BitmapSpriteUsage::BITMAP_SPRITE_USAGE_BLEND_ADD_SUBTRACT_MAX;
    }
    if(!sprite_budget.has_value()) {
        sprite_budget = 32;
    }
    if(!sprite_budget_count.has_value()) {
        sprite_budget_count = 0;
    }
    if(!mipmap_scale_type.has_value()) {
        mipmap_scale_type = ScannedColorMipmapType::SCANNED_COLOR_MIPMAP_LINEAR;
    }
    if(!sprite_spacing.has_value()) {
        sprite_spacing = 4;
    }
    if(!mipmap_fade.has_value()) {
        mipmap_fade = 0.0F;
    }

    // Have these variables handy
    std::uint32_t image_width = 0, image_height = 0;
    std::size_t image_size = 0;
    ColorPlatePixel *image_pixels = nullptr;

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
                case SUPPORTED_FORMATS_TGA:
                case SUPPORTED_FORMATS_BMP:
                    image_pixels = load_image(image_path.data(), image_width, image_height, image_size);
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

    // Set up sprite parameters
    std::optional<ColorPlateScannerSpriteParameters> sprite_parameters;
    if(bitmap_type.value() == BitmapType::BITMAP_TYPE_SPRITES) {
        sprite_parameters = ColorPlateScannerSpriteParameters {};
        auto &p = sprite_parameters.value();
        p.sprite_budget = sprite_budget.value();
        p.sprite_budget_count = sprite_budget_count.value();
        p.sprite_spacing = sprite_spacing.value();
        p.sprite_usage = sprite_usage.value();
    }

    // Do it!
    auto scanned_color_plate = ColorPlateScanner::scan_color_plate(reinterpret_cast<const ColorPlatePixel *>(image_pixels), image_width, image_height, bitmap_type.value(), sprite_parameters, max_mipmap_count.value(), mipmap_scale_type.value(), mipmap_fade.value());
    std::size_t bitmap_count = scanned_color_plate.bitmaps.size();

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
    std::vector<BitmapData<BigEndian>> bitmap_data(bitmap_count);

    // Default mipmap parameters
    if(!mipmap_fade.has_value()) {
        mipmap_fade = 0.0F;
    }

    // Add our bitmap data
    printf("Found %zu bitmap%s:\n", bitmap_count, bitmap_count == 1 ? "" : "s");
    for(std::size_t i = 0; i < bitmap_count; i++) {
        // Write all of the fields here
        auto &bitmap = bitmap_data[i];
        auto &bitmap_color_plate = scanned_color_plate.bitmaps[i];
        bitmap.bitmap_class = TagClassInt::TAG_CLASS_BITMAP;
        bitmap.width = bitmap_color_plate.width;
        bitmap.height = bitmap_color_plate.height;
        switch(bitmap_type.value()) {
            case BitmapType::BITMAP_TYPE_CUBE_MAPS:
                bitmap.type = BitmapDataType::BITMAP_DATA_TYPE_CUBE_MAP;
                bitmap.depth = 1;
                break;
            case BitmapType::BITMAP_TYPE_3D_TEXTURES:
                bitmap.type = BitmapDataType::BITMAP_DATA_TYPE_3D_TEXTURE;
                bitmap.depth = bitmap_color_plate.depth;
                break;
            default:
                bitmap.type = BitmapDataType::BITMAP_DATA_TYPE_2D_TEXTURE;
                bitmap.depth = 1;
                break;
        }
        bitmap.flags = BigEndian<BitmapDataFlags> {};
        bitmap.pixels_offset = static_cast<std::uint32_t>(bitmap_data_pixels.size());
        std::uint32_t mipmap_count = bitmap_color_plate.mipmaps.size();

        // Get the data
        std::vector<std::byte> current_bitmap_pixels(reinterpret_cast<std::byte *>(bitmap_color_plate.pixels.data()), reinterpret_cast<std::byte *>(bitmap_color_plate.pixels.data() + bitmap_color_plate.pixels.size()));

        // Determine if there is any alpha present
        enum AlphaType {
            ALPHA_TYPE_NONE,
            ALPHA_TYPE_ONE_BIT,
            ALPHA_TYPE_MULTI_BIT
        };

        bool alpha_equals_luminosity = true;
        bool luminosity_set = false;

        AlphaType alpha_present = ALPHA_TYPE_NONE;
        auto *first_pixel = reinterpret_cast<ColorPlatePixel *>(current_bitmap_pixels.data());
        auto *last_pixel = reinterpret_cast<ColorPlatePixel *>(current_bitmap_pixels.data() + current_bitmap_pixels.size());
        std::size_t pixel_count = last_pixel - first_pixel;

        // If we need to do monochrome, check if the alpha equals luminosity
        if(format == BitmapFormat::BITMAP_FORMAT_MONOCHROME) {
            for(auto *pixel = first_pixel; pixel < last_pixel; pixel++) {
                std::uint8_t luminosity = ColorPlatePixel::convert_to_y8(pixel);

                // First, check if the luminosity is the same as alpha. If not, AY8 is not an option.
                if(luminosity != pixel->alpha) {
                    alpha_equals_luminosity = false;

                    // Next, check if luminosity is not 0xFF. If so, A8 is not an option
                    if(luminosity != 0xFF) {
                        luminosity_set = true;
                    }

                    // Next, check if the alpha is set. If so, A8Y8 or A8 are options. Otherwise, Y8 is what we can do.
                    if(pixel->alpha != 0xFF) {
                        alpha_present = ALPHA_TYPE_MULTI_BIT;
                    }
                }
            }
        }

        // If we aren't doing monochrome, then we the bitness of the alpha
        else {
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
        }

        // Set the format
        auto format_check = format.value();
        bool compressed = (format_check == BitmapFormat::BITMAP_FORMAT_COMPRESSED_WITH_COLOR_KEY_TRANSPARENCY || format_check == BitmapFormat::BITMAP_FORMAT_COMPRESSED_WITH_EXPLICIT_ALPHA || format_check == BitmapFormat::BITMAP_FORMAT_COMPRESSED_WITH_INTERPOLATED_ALPHA);

        // If the bitmap length or height isn't divisible by 4, use 32-bit color
        if(compressed && ((bitmap.height.read() % 4) != 0 || (bitmap.width.read() % 4) != 0)) {
            format_check = BitmapFormat::BITMAP_FORMAT_32_BIT_COLOR;
            compressed = false;
        }

        switch(format_check) {
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
            case BitmapFormat::BITMAP_FORMAT_MONOCHROME:
                if(alpha_equals_luminosity) {
                    bitmap.format = BitmapDataFormat::BITMAP_FORMAT_AY8;
                }
                else if(alpha_present == ALPHA_TYPE_MULTI_BIT) {
                    if(luminosity_set) {
                        bitmap.format = BitmapDataFormat::BITMAP_FORMAT_A8Y8;
                    }
                    else {
                        bitmap.format = BitmapDataFormat::BITMAP_FORMAT_A8;
                    }
                }
                else {
                    bitmap.format = BitmapDataFormat::BITMAP_FORMAT_Y8;
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
                uint16_t (*conversion_function)(const ColorPlatePixel *);

                switch(bitmap_format) {
                    case BitmapDataFormat::BITMAP_FORMAT_A1R5G5B5:
                        conversion_function = ColorPlatePixel::convert_to_16_bit<1,5,5,5>;
                        break;
                    case BitmapDataFormat::BITMAP_FORMAT_A4R4G4B4:
                        conversion_function = ColorPlatePixel::convert_to_16_bit<4,4,4,4>;
                        break;
                    case BitmapDataFormat::BITMAP_FORMAT_R5G6B5:
                        conversion_function = ColorPlatePixel::convert_to_16_bit<0,5,6,5>;
                        break;
                    default:
                        std::terminate();
                        break;
                }

                // Begin
                std::vector<LittleEndian<std::uint16_t>> new_bitmap_pixels(pixel_count);
                auto *pixel_16_bit = reinterpret_cast<std::uint16_t *>(new_bitmap_pixels.data());
                for(ColorPlatePixel *pixel_32_bit = first_pixel; pixel_32_bit < last_pixel; pixel_32_bit++, pixel_16_bit++) {
                    *pixel_16_bit = conversion_function(pixel_32_bit);
                }

                // Replace buffers
                current_bitmap_pixels.clear();
                current_bitmap_pixels.insert(current_bitmap_pixels.end(), reinterpret_cast<std::byte *>(new_bitmap_pixels.begin().base()), reinterpret_cast<std::byte *>(new_bitmap_pixels.end().base()));

                break;
            }

            // If it's monochrome, it depends
            case BitmapDataFormat::BITMAP_FORMAT_A8:
            case BitmapDataFormat::BITMAP_FORMAT_AY8: {
                std::vector<LittleEndian<std::uint8_t>> new_bitmap_pixels(pixel_count);
                auto *pixel_8_bit = reinterpret_cast<std::uint8_t *>(new_bitmap_pixels.data());
                for(auto *pixel = first_pixel; pixel < last_pixel; pixel++, pixel_8_bit++) {
                    *pixel_8_bit = pixel->alpha;
                }
                current_bitmap_pixels.clear();
                current_bitmap_pixels.insert(current_bitmap_pixels.end(), reinterpret_cast<std::byte *>(new_bitmap_pixels.begin().base()), reinterpret_cast<std::byte *>(new_bitmap_pixels.end().base()));
                break;
            }
            case BitmapDataFormat::BITMAP_FORMAT_Y8: {
                std::vector<LittleEndian<std::uint8_t>> new_bitmap_pixels(pixel_count);
                auto *pixel_8_bit = reinterpret_cast<std::uint8_t *>(new_bitmap_pixels.data());
                for(auto *pixel = first_pixel; pixel < last_pixel; pixel++, pixel_8_bit++) {
                    *pixel_8_bit = ColorPlatePixel::convert_to_y8(pixel);
                }
                current_bitmap_pixels.clear();
                current_bitmap_pixels.insert(current_bitmap_pixels.end(), reinterpret_cast<std::byte *>(new_bitmap_pixels.begin().base()), reinterpret_cast<std::byte *>(new_bitmap_pixels.end().base()));
                break;
            }
            case BitmapDataFormat::BITMAP_FORMAT_A8Y8: {
                std::vector<LittleEndian<std::uint16_t>> new_bitmap_pixels(pixel_count);
                auto *pixel_16_bit = reinterpret_cast<std::uint16_t *>(new_bitmap_pixels.data());
                for(auto *pixel = first_pixel; pixel < last_pixel; pixel++, pixel_16_bit++) {
                    *pixel_16_bit = ColorPlatePixel::convert_to_a8y8(pixel);
                }
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
                    std::uint32_t effective_mipmap_height = mipmap_height;
                    if(bitmap.type == BitmapDataType::BITMAP_DATA_TYPE_CUBE_MAP) {
                        effective_mipmap_height *= 6;
                    }

                    if(mipmap_width >= BLOCK_LENGTH && effective_mipmap_height >= BLOCK_LENGTH) {
                        for(std::size_t y = 0; y < effective_mipmap_height; y += BLOCK_LENGTH) {
                            for(std::size_t x = 0; x < mipmap_width; x += BLOCK_LENGTH) {
                                // Let's make the 4x4 block
                                ColorPlatePixel block[BLOCK_LENGTH * BLOCK_LENGTH];

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

                    uncompressed_pixel += mipmap_width * effective_mipmap_height;
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
        flags.compressed = compressed;
        flags.power_of_two_dimensions = 1;
        bitmap.flags = flags;

        bitmap.registration_point.x = bitmap_color_plate.registration_point_x;
        bitmap.registration_point.y = bitmap_color_plate.registration_point_y;

        #define BYTES_TO_MIB(bytes) (bytes / 1024.0F / 1024.0F)

        printf("    Bitmap #%zu: %ux%u, %u mipmap%s, %s - %.03f MiB\n", i, scanned_color_plate.bitmaps[i].width, scanned_color_plate.bitmaps[i].height, mipmap_count, mipmap_count == 1 ? "" : "s", bitmap_data_format_name(bitmap.format), BYTES_TO_MIB(current_bitmap_pixels.size()));
    }
    printf("Total: %.03f MiB\n", BYTES_TO_MIB(bitmap_data_pixels.size()));

    // Add the bitmap pixel data
    bitmap_tag_data.insert(bitmap_tag_data.end(), bitmap_data_pixels.begin(), bitmap_data_pixels.end());
    new_tag_header.processed_pixel_data.size = bitmap_data_pixels.size();

    // Add all sequences
    std::vector<std::byte> sprite_data;
    for(auto &sequence : scanned_color_plate.sequences) {
        BitmapGroupSequence<BigEndian> bgs = {};
        bgs.first_bitmap_index = sequence.first_bitmap;
        bgs.bitmap_count = sequence.bitmap_count;

        bgs.sprites.count = static_cast<std::uint32_t>(sequence.sprites.size());
        for(auto &sprite : sequence.sprites) {
            BitmapGroupSprite<BigEndian> bgss = {};
            auto &bitmap = scanned_color_plate.bitmaps[sprite.bitmap_index];
            bgss.bitmap_index = sprite.bitmap_index;

            bgss.bottom = static_cast<float>(sprite.bottom) / bitmap.height;
            bgss.top = static_cast<float>(sprite.top) / bitmap.height;
            bgss.registration_point.y = static_cast<float>(sprite.registration_point_y) / bitmap.height;

            bgss.left = static_cast<float>(sprite.left) / bitmap.width;
            bgss.right = static_cast<float>(sprite.right) / bitmap.width;
            bgss.registration_point.x = static_cast<float>(sprite.registration_point_x) / bitmap.width;

            sprite_data.insert(sprite_data.end(), reinterpret_cast<const std::byte *>(&bgss), reinterpret_cast<const std::byte *>(&bgss + 1));
        }

        bitmap_tag_data.insert(bitmap_tag_data.end(), reinterpret_cast<const std::byte *>(&bgs), reinterpret_cast<const std::byte *>(&bgs + 1));
    }
    new_tag_header.bitmap_group_sequence.count = scanned_color_plate.sequences.size();
    bitmap_tag_data.insert(bitmap_tag_data.end(), sprite_data.begin(), sprite_data.end());

    // Add the bitmap tag data
    const auto *bitmap_data_start = reinterpret_cast<const std::byte *>(bitmap_data.data());
    const auto *bitmap_data_end = reinterpret_cast<const std::byte *>(bitmap_data.data() + bitmap_data.size());
    bitmap_tag_data.insert(bitmap_tag_data.end(), bitmap_data_start, bitmap_data_end);
    new_tag_header.bitmap_data.count = bitmap_data.size();

    // Set more parameters
    new_tag_header.type = bitmap_type.value();
    new_tag_header.usage = BitmapUsage::BITMAP_USAGE_DEFAULT;
    new_tag_header.detail_fade_factor = mipmap_fade.value();
    new_tag_header.format = format.value();
    if(max_mipmap_count.value() >= INT16_MAX) {
        new_tag_header.mipmap_count = 0;
    }
    else {
        new_tag_header.mipmap_count = max_mipmap_count.value() + 1;
    }

    new_tag_header.sprite_spacing = sprite_spacing.value();
    new_tag_header.sprite_budget_count = sprite_budget_count.value();
    new_tag_header.sprite_usage = sprite_usage.value();
    auto &sprite_budget_value = sprite_budget.value();
    switch(sprite_budget_value) {
        case 32:
            new_tag_header.sprite_budget_size = BitmapSpriteBudgetSize::BITMAP_SPRITE_BUDGET_SIZE_32X32;
            break;
        case 64:
            new_tag_header.sprite_budget_size = BitmapSpriteBudgetSize::BITMAP_SPRITE_BUDGET_SIZE_64X64;
            break;
        case 128:
            new_tag_header.sprite_budget_size = BitmapSpriteBudgetSize::BITMAP_SPRITE_BUDGET_SIZE_128X128;
            break;
        case 256:
            new_tag_header.sprite_budget_size = BitmapSpriteBudgetSize::BITMAP_SPRITE_BUDGET_SIZE_256X256;
            break;
        case 512:
            new_tag_header.sprite_budget_size = BitmapSpriteBudgetSize::BITMAP_SPRITE_BUDGET_SIZE_512X512;
            break;
        default:
            new_tag_header.sprite_budget_size = BitmapSpriteBudgetSize::BITMAP_SPRITE_BUDGET_SIZE_32X32;
            break;
    }

    // Add the struct in
    *reinterpret_cast<Bitmap<BigEndian> *>(bitmap_tag_data.data() + sizeof(TagFileHeader)) = new_tag_header;

    // Write it all
    std::filesystem::create_directories(tag_path.parent_path());
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

#define ALLOCATE_PIXELS(count) reinterpret_cast<Invader::ColorPlatePixel *>(calloc(sizeof(Invader::ColorPlatePixel) * pixel_count, 1))

static Invader::ColorPlatePixel *rgba_to_pixel(const std::uint8_t *data, std::size_t pixel_count) {
    auto *pixel_data = ALLOCATE_PIXELS(pixel_count);

    for(std::size_t i = 0; i < pixel_count; i++) {
        pixel_data[i].alpha = data[3];
        pixel_data[i].red = data[0];
        pixel_data[i].green = data[1];
        pixel_data[i].blue = data[2];
        data += 4;
    }

    return pixel_data;
}

#undef ALLOCATE_PIXELS

static Invader::ColorPlatePixel *load_image(const char *path, std::uint32_t &image_width, std::uint32_t &image_height, std::size_t &image_size) {
    // Load it
    int x = 0, y = 0, channels = 0;
    auto *image_buffer = stbi_load(path, &x, &y, &channels, 4);
    if(!image_buffer) {
        eprintf("Failed to load %s. Error was: %s\n", path, stbi_failure_reason());
        exit(EXIT_FAILURE);
    }

    // Get the width and height
    image_width = static_cast<std::uint32_t>(x);
    image_height = static_cast<std::uint32_t>(y);
    image_size = image_width * image_height * sizeof(Invader::ColorPlatePixel);

    // Do the thing
    Invader::ColorPlatePixel *return_value = rgba_to_pixel(reinterpret_cast<std::uint8_t *>(image_buffer), image_width * image_height);

    // Free the buffer
    stbi_image_free(image_buffer);

    return return_value;
}

static Invader::ColorPlatePixel *load_tiff(const char *path, std::uint32_t &image_width, std::uint32_t &image_height, std::size_t &image_size) {
    TIFF *image_tiff = TIFFOpen(path, "r");
    if(!image_tiff) {
        eprintf("Cannot open %s\n", path);
        exit(EXIT_FAILURE);
    }
    TIFFGetField(image_tiff, TIFFTAG_IMAGEWIDTH, &image_width);
    TIFFGetField(image_tiff, TIFFTAG_IMAGELENGTH, &image_height);

    // Force associated alpha so alpha doesn't get multiplied in TIFFReadRGBAImageOriented
    uint16_t ua[] = { EXTRASAMPLE_ASSOCALPHA };
    TIFFSetField(image_tiff, TIFFTAG_EXTRASAMPLES, 1, ua);

    // Read it all
    image_size = image_width * image_height * sizeof(Invader::ColorPlatePixel);
    auto *image_pixels = reinterpret_cast<Invader::ColorPlatePixel *>(std::calloc(image_size, 1));
    TIFFReadRGBAImageOriented(image_tiff, image_width, image_height, reinterpret_cast<std::uint32_t *>(image_pixels), ORIENTATION_TOPLEFT);

    // Close the TIFF
    TIFFClose(image_tiff);

    // Swap red and blue channels
    for(std::size_t i = 0; i < image_size / 4; i++) {
        Invader::ColorPlatePixel swapped = image_pixels[i];
        swapped.red = image_pixels[i].blue;
        swapped.blue = image_pixels[i].red;
        image_pixels[i] = swapped;
    }

    return image_pixels;
}
