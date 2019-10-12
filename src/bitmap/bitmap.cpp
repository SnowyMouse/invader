// SPDX-License-Identifier: GPL-3.0-only

#include <zlib.h>
#include <filesystem>
#include <optional>

#include "../printf.hpp"
#include "../version.hpp"
#include "../tag/hek/class/bitmap.hpp"
#include "image_loader.hpp"
#include "color_plate_scanner.hpp"
#include "bitmap_data_writer.hpp"
#include "../command_line_option.hpp"
#include "../file/file.hpp"

enum SupportedFormatsInt {
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

int main(int argc, char *argv[]) {
    using namespace Invader::HEK;
    using namespace Invader;

    struct BitmapOptions {
        // Program path
        const char *path;

        // Data directory
        const char *data = "data/";

        // Tags directory
        const char *tags = "tags/";

        // Scale type?
        std::optional<ScannedColorMipmapType> mipmap_scale_type;

        // Format?
        std::optional<BitmapFormat> format;

        // Usage?
        std::optional<BitmapUsage> usage;

        // Bump stuff
        std::optional<float> bump_height;

        // Palettize to p8 bump?
        std::optional<bool> palettize;

        // Mipmap fade factor
        std::optional<float> mipmap_fade;

        // Bitmap type
        std::optional<BitmapType> bitmap_type;

        // Sprite parameters
        std::optional<BitmapSpriteUsage> sprite_usage;
        std::optional<std::uint32_t> sprite_budget;
        std::optional<std::uint32_t> sprite_budget_count;

        // Dithering?
        std::optional<bool> dither_alpha, dither_red, dither_green, dither_blue;
        bool dithering = false;

        // Sharpen and blur; legacy support for older tags and should not be used in newer ones
        std::optional<float> sharpen;
        std::optional<float> blur;

        // Generate this many mipmaps
        std::optional<std::uint16_t> max_mipmap_count;

        // Ignore the tag data?
        bool ignore_tag_data = false;

        // Use a filesystem path?
        bool filesystem_path = false;
    } bitmap_options;
    bitmap_options.path = argv[0];

    std::vector<CommandLineOption> options;
    options.emplace_back("info", 'i', 0);
    options.emplace_back("ignore-tag", 'I', 0);
    options.emplace_back("help", 'h', 0);
    options.emplace_back("dithering", 'D', 1);
    options.emplace_back("data", 'd', 1);
    options.emplace_back("tags", 't', 1);
    options.emplace_back("format", 'F', 1);
    options.emplace_back("type", 'T', 1);
    options.emplace_back("mipmap-count", 'm', 1);
    options.emplace_back("mipmap-scale", 's', 1);
    options.emplace_back("detail-fade", 'f', 1);
    options.emplace_back("budget", 'B', 1);
    options.emplace_back("budget-count", 'C', 1);
    options.emplace_back("bump-palettize", 'p', 1);
    options.emplace_back("bump-palettise", 'p', 1);
    options.emplace_back("bump-height", 'H', 1);
    options.emplace_back("fs-path", 'P', 0);

    // Go through each argument
    auto remaining_arguments = CommandLineOption::parse_arguments<BitmapOptions &>(argc, argv, options, 'h', bitmap_options, [](char opt, const std::vector<const char *> &arguments, BitmapOptions &bitmap_options) {
        switch(opt) {
            case 'd':
                bitmap_options.data = arguments[0];
                break;

            case 't':
                bitmap_options.tags = arguments[0];
                break;

            case 'i':
                INVADER_SHOW_INFO
                std::exit(EXIT_FAILURE);

            case 'I':
                bitmap_options.ignore_tag_data = true;
                break;

            case 'f':
                bitmap_options.mipmap_fade = std::strtof(arguments[0], nullptr);
                if(bitmap_options.mipmap_fade < 0.0F || bitmap_options.mipmap_fade > 1.0F) {
                    eprintf("Mipmap fade must be between 0-1\n");
                    std::exit(EXIT_FAILURE);
                }
                break;

            case 's':
                if(std::strcmp(arguments[0], "linear") == 0) {
                    bitmap_options.mipmap_scale_type = ScannedColorMipmapType::SCANNED_COLOR_MIPMAP_LINEAR;
                }
                else if(std::strcmp(arguments[0], "nearest") == 0) {
                    bitmap_options.mipmap_scale_type = ScannedColorMipmapType::SCANNED_COLOR_MIPMAP_NEAREST_ALPHA_COLOR;
                }
                else if(std::strcmp(arguments[0], "nearest-alpha") == 0) {
                    bitmap_options.mipmap_scale_type = ScannedColorMipmapType::SCANNED_COLOR_MIPMAP_NEAREST_ALPHA;
                }
                else {
                    eprintf("Unknown mipmap scale type %s\n", arguments[0]);
                    std::exit(EXIT_FAILURE);
                }
                break;

            case 'F':
                if(std::strcmp(arguments[0], "32-bit") == 0) {
                    bitmap_options.format = BitmapFormat::BITMAP_FORMAT_32_BIT_COLOR;
                }
                else if(std::strcmp(arguments[0], "16-bit") == 0) {
                    bitmap_options.format = BitmapFormat::BITMAP_FORMAT_16_BIT_COLOR;
                }
                else if(std::strcmp(arguments[0], "monochrome") == 0) {
                    bitmap_options.format = BitmapFormat::BITMAP_FORMAT_MONOCHROME;
                }
                else if(std::strcmp(arguments[0], "dxt5") == 0) {
                    bitmap_options.format = BitmapFormat::BITMAP_FORMAT_COMPRESSED_WITH_INTERPOLATED_ALPHA;
                }
                else if(std::strcmp(arguments[0], "dxt3") == 0) {
                    bitmap_options.format = BitmapFormat::BITMAP_FORMAT_COMPRESSED_WITH_EXPLICIT_ALPHA;
                }
                else if(std::strcmp(arguments[0], "dxt1") == 0) {
                    bitmap_options.format = BitmapFormat::BITMAP_FORMAT_COMPRESSED_WITH_COLOR_KEY_TRANSPARENCY;
                }
                else {
                    eprintf("Unknown format %s\n", arguments[0]);
                    std::exit(EXIT_FAILURE);
                }
                break;

            case 'T':
                if(std::strcmp(arguments[0], "2d") == 0) {
                    bitmap_options.bitmap_type = BitmapType::BITMAP_TYPE_2D_TEXTURES;
                }
                else if(std::strcmp(arguments[0], "3d") == 0) {
                    bitmap_options.bitmap_type = BitmapType::BITMAP_TYPE_3D_TEXTURES;
                }
                else if(std::strcmp(arguments[0], "cubemap") == 0) {
                    bitmap_options.bitmap_type = BitmapType::BITMAP_TYPE_CUBE_MAPS;
                }
                else if(std::strcmp(arguments[0], "interface") == 0) {
                    bitmap_options.bitmap_type = BitmapType::BITMAP_TYPE_INTERFACE_BITMAPS;
                }
                else if(std::strcmp(arguments[0], "sprite") == 0) {
                    bitmap_options.bitmap_type = BitmapType::BITMAP_TYPE_SPRITES;
                }
                else {
                    eprintf("Unknown type %s\n", arguments[0]);
                    std::exit(EXIT_FAILURE);
                }
                break;

            case 'D':
                bitmap_options.dithering = true;
                bitmap_options.dither_alpha = false;
                bitmap_options.dither_red = false;
                bitmap_options.dither_green = false;
                bitmap_options.dither_blue = false;
                for(const char *c = arguments[0]; *c; c++) {
                    switch(*c) {
                        case 'a':
                            bitmap_options.dither_alpha = true;
                            break;
                        case 'r':
                            bitmap_options.dither_red = true;
                            break;
                        case 'g':
                            bitmap_options.dither_green = true;
                            break;
                        case 'b':
                            bitmap_options.dither_blue = true;
                            break;
                        default:
                            eprintf("Unknown channel %c.\n", *c);
                            std::exit(EXIT_FAILURE);
                    }
                }
                break;

            case 'p':
                if(strcmp(arguments[0],"on") == 0) {
                    bitmap_options.palettize = true;
                }
                else if(strcmp(arguments[0],"off") == 0) {
                    bitmap_options.palettize = false;
                }
                else {
                    eprintf("Unknown palettize setting %s\n", arguments[0]);
                    std::exit(EXIT_FAILURE);
                }
                break;

            case 'u':
                if(strcmp(arguments[0], "default") == 0) {
                    bitmap_options.usage = BitmapUsage::BITMAP_USAGE_DEFAULT;
                }
                else if(strcmp(arguments[0], "bumpmap") == 0) {
                    bitmap_options.usage = BitmapUsage::BITMAP_USAGE_HEIGHT_MAP;
                }
                else if(strcmp(arguments[0], "detail") == 0) {
                    bitmap_options.usage = BitmapUsage::BITMAP_USAGE_DETAIL_MAP;
                }
                else {
                    eprintf("Unknown usage %s\n", arguments[0]);
                    std::exit(EXIT_FAILURE);
                }
                break;

            case 'H':
                bitmap_options.bump_height = static_cast<float>(std::strtof(arguments[0], nullptr));
                break;

            case 'm':
                bitmap_options.max_mipmap_count = static_cast<std::int32_t>(std::strtol(arguments[0], nullptr, 10));
                break;

            case 'C':
                bitmap_options.sprite_budget_count = static_cast<std::uint32_t>(std::strtoul(arguments[0], nullptr, 10));
                break;

            case 'B':
                bitmap_options.sprite_budget = static_cast<std::uint32_t>(std::strtoul(arguments[0], nullptr, 10));
                switch(bitmap_options.sprite_budget.value()) {
                    case 32:
                    case 64:
                    case 128:
                    case 256:
                    case 512:
                        break;
                    default:
                        eprintf("Invalid sprite budget %u.\n", bitmap_options.sprite_budget.value());
                        std::exit(EXIT_FAILURE);
                }

                break;

            case 'P':
                bitmap_options.filesystem_path = true;
                break;

            default:
                eprintf("Usage: %s [options] <bitmap-tag>\n\n", bitmap_options.path);
                eprintf("Create or modify a bitmap tag.\n\n");
                eprintf("Options:\n");
                eprintf("    --info,-i                  Show license and credits.\n");
                eprintf("    --help,-h                  Show help\n\n");
                eprintf("Directory options:\n");
                eprintf("    --fs-path,-P               Use a filesystem path for the input bitmap.\n");
                eprintf("    --data,-d <path>           Set the data directory.\n");
                eprintf("    --tags,-t <path>           Set the tags directory.\n\n");
                eprintf("Bitmap options:\n");
                eprintf("    --type,-T <type>           Set the type of bitmap. Can be: 2d, 3d, cubemap,\n");
                eprintf("                               interface, or sprite. Default (new tag): 2d\n");
                eprintf("    --usage,-u <usage>         Set the bitmap usage. Can be: default, bumpmap,\n");
                eprintf("                               or detail. Default (new tag): default\n");
                eprintf("    --dithering,-D <channels>  Apply dithering to 16-bit, dxtn, or p8 bitmaps.\n");
                eprintf("                               Specify channels with letters (i.e. argb).\n");
                eprintf("    --ignore-tag,-I            Ignore the tag data if the tag exists.\n");
                eprintf("    --format,-F <type>         Pixel format. Can be: 32-bit, 16-bit, monochrome,\n");
                eprintf("                               dxt5, dxt3, or dxt1. Default (new tag): 32-bit\n\n");
                eprintf("Mipmap options:\n");
                eprintf("    --mipmap-count,-m <count>  Set maximum mipmaps. Default (new tag): 32767\n");
                eprintf("    --mipmap-scale,-s <type>   Mipmap scale type. Can be: linear, nearest-alpha,\n");
                eprintf("                               nearest. Default (new tag): linear\n\n");
                eprintf("Bumpmap options (only applies to bumpmap bitmaps):\n");
                eprintf("    --bump-height,-H <height>  Set the apparent bumpmap height from 0 to 1.\n");
                eprintf("                               Default (new tag): 0.02\n");
                eprintf("    --bump-palettize,-p <type> Set the bumpmap palettization setting. This will\n");
                eprintf("                               not work with stock Halo. Can be: off or on.\n");
                eprintf("                               Default (new tag): off\n\n");
                eprintf("Detail map options (only applies to detail bitmaps):\n");
                eprintf("    --detail-fade,-f <factor>  Set detail fade factor. Default (new tag): 0.0\n\n");
                eprintf("Sprite options (only applies to sprite bitmaps):\n");
                eprintf("    --budget-count,-C <count>  Set maximum number of sprite sheets. Setting this\n");
                eprintf("                               to 0 disables budgeting. Default (new tag): 0\n");
                eprintf("    --budget,-B <length>       Set max length of sprite sheet. Can be 32, 64,\n");
                eprintf("                               128, 256, or 512. Default (new tag): 32\n\n");

                std::exit(EXIT_FAILURE);
        }
    });

    // Make sure we have the bitmap tag path
    if(remaining_arguments.size() == 0) {
        eprintf("Expected a bitmap tag path. Use -h for help.\n");
        return EXIT_FAILURE;
    }
    else if(remaining_arguments.size() > 1) {
        eprintf("Unexpected argument %s\n", remaining_arguments[1]);
        return EXIT_FAILURE;
    }

    // See if we can figure out the bitmap tag using extensions
    std::string bitmap_tag = remaining_arguments[0];
    SupportedFormatsInt found_format = static_cast<SupportedFormatsInt>(0);

    if(bitmap_options.filesystem_path) {
        std::vector<std::string> data_v(&bitmap_options.data, &bitmap_options.data + 1);
        for(SupportedFormatsInt i = found_format; i < SupportedFormatsInt::SUPPORTED_FORMATS_INT_COUNT; i = static_cast<SupportedFormatsInt>(i + 1)) {
            auto bitmap_tag_maybe = Invader::File::file_path_to_tag_path_with_extension(bitmap_tag, data_v, SUPPORTED_FORMATS[i]);
            if(bitmap_tag_maybe.has_value()) {
                bitmap_tag = bitmap_tag_maybe.value();
                found_format = i;
                break;
            }
            else {
                eprintf("Failed to find a valid bitmap %s in the data directory.\n", remaining_arguments[0]);
                return EXIT_FAILURE;
            }
        }
    }

    // Ensure it's lowercase
    for(char &c : bitmap_tag) {
        if(c >= 'A' && c <= 'Z') {
            eprintf("Invalid tag path %s. Tag paths must be lowercase.\n", bitmap_tag.data());
            return EXIT_FAILURE;
        }
    }

    std::filesystem::path data_path = bitmap_options.data;

    // Get the path
    std::filesystem::path tags_path(bitmap_options.tags);
    auto tag_path = tags_path / bitmap_tag;
    auto final_path = tag_path.string() + ".bitmap";

    // See if we can get anything out of this
    std::FILE *tag_read;
    if(!bitmap_options.ignore_tag_data && (tag_read = std::fopen(final_path.data(), "rb"))) {
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
        if(!bitmap_options.format.has_value()) {
            bitmap_options.format = bitmap_tag_header.format;
        }
        if(!bitmap_options.mipmap_fade.has_value()) {
            bitmap_options.mipmap_fade = bitmap_tag_header.detail_fade_factor;
        }
        if(!bitmap_options.bitmap_type.has_value()) {
            bitmap_options.bitmap_type = bitmap_tag_header.type;
        }
        if(!bitmap_options.max_mipmap_count.has_value()) {
            std::int16_t mipmap_count = bitmap_tag_header.mipmap_count.read();
            if(mipmap_count == 0) {
                bitmap_options.max_mipmap_count = INT16_MAX;
            }
            else {
                bitmap_options.max_mipmap_count = mipmap_count - 1;
            }
        }
        if(!bitmap_options.sprite_usage.has_value()) {
            bitmap_options.sprite_usage = bitmap_tag_header.sprite_usage;
        }
        if(!bitmap_options.sprite_budget.has_value()) {
            bitmap_options.sprite_budget = 32 << bitmap_tag_header.sprite_budget_size;
        }
        if(!bitmap_options.sprite_budget_count.has_value()) {
            bitmap_options.sprite_budget_count = bitmap_tag_header.sprite_budget_count;
        }
        if(!bitmap_options.usage.has_value()) {
            bitmap_options.usage = bitmap_tag_header.usage;
        }
        if(!bitmap_options.palettize.has_value()) {
            bitmap_options.palettize = !bitmap_tag_header.flags.read().disable_height_map_compression;
        }
        if(!bitmap_options.bump_height.has_value()) {
            bitmap_options.bump_height = bitmap_tag_header.bump_height;
        }
        if(!bitmap_options.sharpen.has_value() && bitmap_tag_header.sharpen_amount > 0.0F && bitmap_tag_header.sharpen_amount <= 1.0F) {
            bitmap_options.sharpen = bitmap_tag_header.sharpen_amount;
        }
        if(!bitmap_options.blur.has_value() && bitmap_tag_header.blur_filter_size > 0.0F) {
            bitmap_options.blur = bitmap_tag_header.blur_filter_size;
        }

        auto header_flags = bitmap_tag_header.flags.read();
        if(!bitmap_options.mipmap_scale_type.has_value()) {
            if(header_flags.invader_nearest_mipmap_alpha_and_color) {
                bitmap_options.mipmap_scale_type = ScannedColorMipmapType::SCANNED_COLOR_MIPMAP_NEAREST_ALPHA_COLOR;
            }
            else if(header_flags.invader_nearest_mipmap_alpha) {
                bitmap_options.mipmap_scale_type = ScannedColorMipmapType::SCANNED_COLOR_MIPMAP_NEAREST_ALPHA;
            }
            else {
                bitmap_options.mipmap_scale_type = ScannedColorMipmapType::SCANNED_COLOR_MIPMAP_LINEAR;
            }
        }
        if(!bitmap_options.dithering) {
            bitmap_options.dither_alpha = header_flags.invader_dither_alpha == 1;
            bitmap_options.dither_red = header_flags.invader_dither_red == 1;
            bitmap_options.dither_green = header_flags.invader_dither_green == 1;
            bitmap_options.dither_blue = header_flags.invader_dither_blue == 1;
        }

        std::fclose(tag_read);
    }

    // If these values weren't set, set them
    #define DEFAULT_VALUE(what, default) if(!what.has_value()) { what = default; }

    DEFAULT_VALUE(bitmap_options.format,BitmapFormat::BITMAP_FORMAT_32_BIT_COLOR);
    DEFAULT_VALUE(bitmap_options.bitmap_type,BitmapType::BITMAP_TYPE_2D_TEXTURES);
    DEFAULT_VALUE(bitmap_options.max_mipmap_count,INT16_MAX);
    DEFAULT_VALUE(bitmap_options.sprite_usage,BitmapSpriteUsage::BITMAP_SPRITE_USAGE_BLEND_ADD_SUBTRACT_MAX);
    DEFAULT_VALUE(bitmap_options.sprite_budget,32);
    DEFAULT_VALUE(bitmap_options.sprite_budget_count,0);
    DEFAULT_VALUE(bitmap_options.mipmap_scale_type,ScannedColorMipmapType::SCANNED_COLOR_MIPMAP_LINEAR);
    DEFAULT_VALUE(bitmap_options.mipmap_fade,0.0F);
    DEFAULT_VALUE(bitmap_options.usage,BitmapUsage::BITMAP_USAGE_DEFAULT);
    DEFAULT_VALUE(bitmap_options.palettize,false);
    DEFAULT_VALUE(bitmap_options.bump_height,0.02F);
    DEFAULT_VALUE(bitmap_options.mipmap_fade,0.0F);
    DEFAULT_VALUE(bitmap_options.dither_alpha,false);
    DEFAULT_VALUE(bitmap_options.dither_red,false);
    DEFAULT_VALUE(bitmap_options.dither_green,false);
    DEFAULT_VALUE(bitmap_options.dither_blue,false);

    #undef DEFAULT_VALUE

    // Have these variables handy
    std::uint32_t image_width = 0, image_height = 0;
    std::size_t image_size = 0;
    ColorPlatePixel *image_pixels = nullptr;

    // Try to figure out the extension
    for(auto i = found_format; i < SUPPORTED_FORMATS_INT_COUNT; i = static_cast<SupportedFormatsInt>(i + 1)) {
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
                default:
                    std::terminate();
                    break;
            }
            break;
        }
    }

    if(image_pixels == nullptr) {
        eprintf("Failed to find %s in %s\nValid formats are:\n", bitmap_tag.data(), bitmap_options.data);
        for(auto *format : SUPPORTED_FORMATS) {
            eprintf("    %s\n", format);
        }
        return EXIT_FAILURE;
    }

    // Set up sprite parameters
    std::optional<ColorPlateScannerSpriteParameters> sprite_parameters;
    if(bitmap_options.bitmap_type.value() == BitmapType::BITMAP_TYPE_SPRITES) {
        sprite_parameters = ColorPlateScannerSpriteParameters {};
        auto &p = sprite_parameters.value();
        p.sprite_budget = bitmap_options.sprite_budget.value();
        p.sprite_budget_count = bitmap_options.sprite_budget_count.value();
        p.sprite_usage = bitmap_options.sprite_usage.value();
    }

    // Do it!
    auto try_to_scan_color_plate = [&image_pixels, &image_width, &image_height, &bitmap_options, &sprite_parameters]() {
        try {
            return ColorPlateScanner::scan_color_plate(reinterpret_cast<const ColorPlatePixel *>(image_pixels), image_width, image_height, bitmap_options.bitmap_type.value(), bitmap_options.usage.value(), bitmap_options.bump_height.value(), sprite_parameters, bitmap_options.max_mipmap_count.value(), bitmap_options.mipmap_scale_type.value(), bitmap_options.usage == BitmapUsage::BITMAP_USAGE_DETAIL_MAP ? bitmap_options.mipmap_fade : std::nullopt, bitmap_options.sharpen, bitmap_options.blur);
        }
        catch (std::exception &e) {
            oprintf("Failed to process the image: %s\n", e.what());
            std::exit(1);
        };
    };

    auto scanned_color_plate = try_to_scan_color_plate();
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

        // Set decompressed sizeblue_dither_alphadithering
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

    #define BYTES_TO_MIB(bytes) (bytes / 1024.0F / 1024.0F)

    // Add our bitmap data
    oprintf("Found %zu bitmap%s:\n", bitmap_count, bitmap_count == 1 ? "" : "s");
    try {
        write_bitmap_data(scanned_color_plate, bitmap_data_pixels, bitmap_data, bitmap_options.usage.value(), bitmap_options.format.value(), bitmap_options.bitmap_type.value(), bitmap_options.palettize.value(), bitmap_options.dither_alpha.value(), bitmap_options.dither_red.value(), bitmap_options.dither_green.value(), bitmap_options.dither_blue.value());
    }
    catch (std::exception &e) {
        eprintf("Failed to generate bitmap data: %s\n", e.what());
        std::exit(1);
    }
    oprintf("Total: %.03f MiB\n", BYTES_TO_MIB(bitmap_data_pixels.size()));

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
    new_tag_header.type = bitmap_options.bitmap_type.value();
    new_tag_header.usage = bitmap_options.usage.value();
    new_tag_header.bump_height = bitmap_options.bump_height.value();
    new_tag_header.detail_fade_factor = bitmap_options.mipmap_fade.value();
    new_tag_header.format = bitmap_options.format.value();
    new_tag_header.sharpen_amount = bitmap_options.sharpen.value_or(0.0F);
    new_tag_header.blur_filter_size = bitmap_options.blur.value_or(0.0F);
    if(bitmap_options.max_mipmap_count.value() >= INT16_MAX) {
        new_tag_header.mipmap_count = 0;
    }
    else {
        new_tag_header.mipmap_count = bitmap_options.max_mipmap_count.value() + 1;
    }

    BitmapFlags flags = {};
    flags.disable_height_map_compression = !bitmap_options.palettize.value();
    switch(bitmap_options.mipmap_scale_type.value()) {
        case ScannedColorMipmapType::SCANNED_COLOR_MIPMAP_LINEAR:
            break;
        case ScannedColorMipmapType::SCANNED_COLOR_MIPMAP_NEAREST_ALPHA:
            flags.invader_nearest_mipmap_alpha = 1;
            break;
        case ScannedColorMipmapType::SCANNED_COLOR_MIPMAP_NEAREST_ALPHA_COLOR:
            flags.invader_nearest_mipmap_alpha_and_color = 1;
            break;
    };
    flags.invader_dither_alpha = bitmap_options.dither_alpha.value();
    flags.invader_dither_red = bitmap_options.dither_red.value();
    flags.invader_dither_green = bitmap_options.dither_green.value();
    flags.invader_dither_blue = bitmap_options.dither_blue.value();
    new_tag_header.flags = flags;

    new_tag_header.sprite_spacing = sprite_parameters.value_or(ColorPlateScannerSpriteParameters{}).sprite_spacing;
    new_tag_header.sprite_budget_count = bitmap_options.sprite_budget_count.value();
    new_tag_header.sprite_usage = bitmap_options.sprite_usage.value();
    auto &sprite_budget_value = bitmap_options.sprite_budget.value();
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
