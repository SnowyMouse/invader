// SPDX-License-Identifier: GPL-3.0-only

#include <zlib.h>
#include <filesystem>
#include <optional>

#include <invader/printf.hpp>
#include <invader/version.hpp>
#include <invader/tag/hek/definition.hpp>
#include <invader/tag/hek/header.hpp>
#include <invader/bitmap/image_loader.hpp>
#include <invader/bitmap/color_plate_scanner.hpp>
#include <invader/bitmap/bitmap_data_writer.hpp>
#include <invader/command_line_option.hpp>
#include <invader/file/file.hpp>
#include <invader/tag/parser/parser.hpp>

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
    EXIT_IF_INVADER_EXTRACT_HIDDEN_VALUES

    using namespace Invader::HEK;
    using namespace Invader;

    struct BitmapOptions {
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

    std::vector<CommandLineOption> options;
    options.emplace_back("info", 'i', 0, "Show license and credits.");
    options.emplace_back("ignore-tag", 'I', 0, "Ignore the tag data if the tag exists.");
    options.emplace_back("dithering", 'D', 1, "Apply dithering to 16-bit, dxtn, or p8 bitmaps. Specify channels with letters (i.e. argb).", "<channels>");
    options.emplace_back("data <path>", 'd', 1, "Set the data directory.", "<path>");
    options.emplace_back("tags", 't', 1, "Set the data directory.", "<path>");
    options.emplace_back("format", 'F', 1, "Pixel format. Can be: 32-bit, 16-bit, monochrome, dxt5, dxt3, or dxt1. Default (new tag): 32-bit", "<type>");
    options.emplace_back("type", 'T', 1, "Set the type of bitmap. Can be: 2d, 3d, cubemap, interface, or sprite. Default (new tag): 2d", "<type>");
    options.emplace_back("mipmap-count", 'M', 1, "Set maximum mipmaps. Default (new tag): 32767", "<count>");
    options.emplace_back("mipmap-scale", 's', 1, "Mipmap scale type. Can be: linear, nearest-alpha, nearest. Default (new tag): linear", "<type>");
    options.emplace_back("detail-fade", 'f', 1, "Set detail fade factor. Default (new tag): 0.0", "<factor>");
    options.emplace_back("budget", 'B', 1, "Set max length of sprite sheet. Can be 32, 64, 128, 256, 512, 1024, or 2048. Default (new tag): 32", "<length>");
    options.emplace_back("budget-count", 'C', 1, "Set maximum number of sprite sheets. Setting this to 0 disables budgeting. Default (new tag): 0", "<count>");
    options.emplace_back("bump-palettize", 'p', 1, "Set the bumpmap palettization setting. This will not work with stock Halo. Can be: off or on. Default (new tag): off", "<val>");
    options.emplace_back("bump-height", 'H', 1, "Set the apparent bumpmap height from 0 to 1. Default (new tag): 0.026", "<height>");
    options.emplace_back("fs-path", 'P', 0, "Use a filesystem path for the data.");

    static constexpr char DESCRIPTION[] = "Create or modify a bitmap tag.";
    static constexpr char USAGE[] = "[options] <bitmap-tag>";

    // Go through each argument
    auto remaining_arguments = CommandLineOption::parse_arguments<BitmapOptions &>(argc, argv, options, USAGE, DESCRIPTION, 1, 1, bitmap_options, [](char opt, const std::vector<const char *> &arguments, auto &bitmap_options) {
        switch(opt) {
            case 'd':
                bitmap_options.data = arguments[0];
                break;

            case 't':
                bitmap_options.tags = arguments[0];
                break;

            case 'i':
                show_version_info();
                std::exit(EXIT_SUCCESS);

            case 'I':
                bitmap_options.ignore_tag_data = true;
                break;

            case 'f':
                bitmap_options.mipmap_fade = std::strtof(arguments[0], nullptr);
                if(bitmap_options.mipmap_fade < 0.0F || bitmap_options.mipmap_fade > 1.0F) {
                    eprintf_error("Mipmap fade must be between 0-1");
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
                    eprintf_error("Unknown mipmap scale type %s", arguments[0]);
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
                    eprintf_error("Unknown format %s", arguments[0]);
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
                    eprintf_error("Unknown type %s", arguments[0]);
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
                            eprintf_error("Unknown channel %c.", *c);
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
                    eprintf_error("Unknown palettize setting %s", arguments[0]);
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
                    eprintf_error("Unknown usage %s", arguments[0]);
                    std::exit(EXIT_FAILURE);
                }
                break;

            case 'H':
                bitmap_options.bump_height = static_cast<float>(std::strtof(arguments[0], nullptr));
                break;

            case 'M':
                bitmap_options.max_mipmap_count = static_cast<std::uint32_t>(std::strtol(arguments[0], nullptr, 10));
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
                    case 1024:
                    case 2048:
                        break;
                    default:
                        eprintf_error("Invalid sprite budget %u.", bitmap_options.sprite_budget.value());
                        std::exit(EXIT_FAILURE);
                }

                break;

            case 'P':
                bitmap_options.filesystem_path = true;
                break;
        }
    });

    // See if we can figure out the bitmap tag using extensions
    std::string bitmap_tag = remaining_arguments[0];
    SupportedFormatsInt found_format = static_cast<SupportedFormatsInt>(0);

    if(bitmap_options.filesystem_path) {
        std::vector<std::string> data_v(&bitmap_options.data, &bitmap_options.data + 1);
        SupportedFormatsInt i;
        for(i = found_format; i < SupportedFormatsInt::SUPPORTED_FORMATS_INT_COUNT; i = static_cast<SupportedFormatsInt>(i + 1)) {
            auto bitmap_tag_maybe = Invader::File::file_path_to_tag_path_with_extension(bitmap_tag, data_v, SUPPORTED_FORMATS[i]);
            if(bitmap_tag_maybe.has_value()) {
                bitmap_tag = bitmap_tag_maybe.value();
                found_format = i;
                break;
            }
        }
        if(i == SupportedFormatsInt::SUPPORTED_FORMATS_INT_COUNT) {
            eprintf_error("Failed to find a valid bitmap %s in the data directory.", remaining_arguments[0]);
            return EXIT_FAILURE;
        }
    }

    // Ensure it's lowercase
    for(char &c : bitmap_tag) {
        if(c >= 'A' && c <= 'Z') {
            eprintf_error("Invalid tag path %s. Tag paths must be lowercase.", bitmap_tag.c_str());
            return EXIT_FAILURE;
        }
    }

    std::filesystem::path data_path = bitmap_options.data;

    // Check if the tags directory exists
    std::filesystem::path tags_path(bitmap_options.tags);
    if(!std::filesystem::is_directory(tags_path)) {
        if(std::strcmp(bitmap_options.tags, "tags") == 0) {
            eprintf_error("No tags directory was given, and \"tags\" was not found or is not a directory.");
        }
        else {
            eprintf_error("Directory %s was not found or is not a directory", bitmap_options.tags);
        }
        return EXIT_FAILURE;
    }

    auto tag_path = tags_path / bitmap_tag;
    auto final_path = tag_path.string() + ".bitmap";

    // See if we can get anything out of this
    std::FILE *tag_read;
    if(!bitmap_options.ignore_tag_data && (tag_read = std::fopen(final_path.c_str(), "rb"))) {
        // Here's in case we do fail. It cleans up and exits.
        auto exit_on_failure = [&tag_read, &final_path]() {
            eprintf_error("%s could not be read.", final_path.c_str());
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
    DEFAULT_VALUE(bitmap_options.bump_height,0.026F);
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
    auto bitmap_data_path = (data_path / bitmap_tag).string();
    for(auto i = found_format; i < SUPPORTED_FORMATS_INT_COUNT; i = static_cast<SupportedFormatsInt>(i + 1)) {
        std::string image_path = bitmap_data_path + SUPPORTED_FORMATS[i];
        if(std::filesystem::exists(image_path)) {
            switch(i) {
                case SUPPORTED_FORMATS_TIF:
                case SUPPORTED_FORMATS_TIFF:
                    image_pixels = load_tiff(image_path.c_str(), image_width, image_height, image_size);
                    break;
                case SUPPORTED_FORMATS_PNG:
                case SUPPORTED_FORMATS_TGA:
                case SUPPORTED_FORMATS_BMP:
                    image_pixels = load_image(image_path.c_str(), image_width, image_height, image_size);
                    break;
                default:
                    std::terminate();
                    break;
            }
            break;
        }
    }

    if(image_pixels == nullptr) {
        eprintf_error("Failed to find %s in %s", bitmap_tag.c_str(), bitmap_options.data);
        eprintf("Valid formats are:\n");
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
            eprintf_error("Failed to process the image: %s", e.what());
            std::exit(1);
        };
    };

    auto scanned_color_plate = try_to_scan_color_plate();
    std::size_t bitmap_count = scanned_color_plate.bitmaps.size();

    // Start building the bitmap tag
    Parser::Bitmap bitmap_tag_data = {};

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
        bitmap_tag_data.color_plate_width = image_width;
        bitmap_tag_data.color_plate_height = image_height;

        // Set decompressed sizeblue_dither_alphadithering
        BigEndian<std::uint32_t> decompressed_size;
        decompressed_size = static_cast<std::uint32_t>(image_size);

        // Set compressed size
        bitmap_tag_data.compressed_color_plate_data.resize(sizeof(decompressed_size));
        *reinterpret_cast<BigEndian<std::uint32_t> *>(bitmap_tag_data.compressed_color_plate_data.data()) = decompressed_size;

        // Add it to the end now
        bitmap_tag_data.compressed_color_plate_data.insert(bitmap_tag_data.compressed_color_plate_data.end(), compressed_data.data(), compressed_data.data() + deflate_stream.total_out);
    }

    // Now let's add the actual bitmap data
    #define BYTES_TO_MIB(bytes) (bytes / 1024.0F / 1024.0F)

    // Add our bitmap data
    oprintf("Found %zu bitmap%s:\n", bitmap_count, bitmap_count == 1 ? "" : "s");
    try {
        write_bitmap_data(scanned_color_plate, bitmap_tag_data.processed_pixel_data, bitmap_tag_data.bitmap_data, bitmap_options.usage.value(), bitmap_options.format.value(), bitmap_options.bitmap_type.value(), bitmap_options.palettize.value(), bitmap_options.dither_alpha.value(), bitmap_options.dither_red.value(), bitmap_options.dither_green.value(), bitmap_options.dither_blue.value());
    }
    catch (std::exception &e) {
        eprintf_error("Failed to generate bitmap data: %s", e.what());
        std::exit(1);
    }
    oprintf("Total: %.03f MiB\n", BYTES_TO_MIB(bitmap_tag_data.processed_pixel_data.size()));

    // Add all sequences
    for(auto &sequence : scanned_color_plate.sequences) {
        auto &bgs = bitmap_tag_data.bitmap_group_sequence.emplace_back();

        bgs.first_bitmap_index = sequence.first_bitmap;
        bgs.bitmap_count = sequence.bitmap_count;

        // Add the sprites in the sequence
        for(auto &sprite : sequence.sprites) {
            auto &bgss = bgs.sprites.emplace_back();
            auto &bitmap = scanned_color_plate.bitmaps[sprite.bitmap_index];
            bgss.bitmap_index = sprite.bitmap_index;

            bgss.bottom = static_cast<float>(sprite.bottom) / bitmap.height;
            bgss.top = static_cast<float>(sprite.top) / bitmap.height;
            bgss.registration_point.y = static_cast<float>(sprite.registration_point_y) / bitmap.height;

            bgss.left = static_cast<float>(sprite.left) / bitmap.width;
            bgss.right = static_cast<float>(sprite.right) / bitmap.width;
            bgss.registration_point.x = static_cast<float>(sprite.registration_point_x) / bitmap.width;
        }
    }

    // Set more parameters
    bitmap_tag_data.type = bitmap_options.bitmap_type.value();
    bitmap_tag_data.usage = bitmap_options.usage.value();
    bitmap_tag_data.bump_height = bitmap_options.bump_height.value();
    bitmap_tag_data.detail_fade_factor = bitmap_options.mipmap_fade.value();
    bitmap_tag_data.format = bitmap_options.format.value();
    bitmap_tag_data.sharpen_amount = bitmap_options.sharpen.value_or(0.0F);
    bitmap_tag_data.blur_filter_size = bitmap_options.blur.value_or(0.0F);
    if(bitmap_options.max_mipmap_count.value() >= INT16_MAX) {
        bitmap_tag_data.mipmap_count = 0;
    }
    else {
        bitmap_tag_data.mipmap_count = bitmap_options.max_mipmap_count.value() + 1;
    }

    // Set flags
    auto &flags = bitmap_tag_data.flags;
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

    // Set sprite stuff
    bitmap_tag_data.sprite_spacing = sprite_parameters.value_or(ColorPlateScannerSpriteParameters{}).sprite_spacing;
    bitmap_tag_data.sprite_budget_count = bitmap_options.sprite_budget_count.value();
    bitmap_tag_data.sprite_usage = bitmap_options.sprite_usage.value();
    auto &sprite_budget_value = bitmap_options.sprite_budget.value();
    switch(sprite_budget_value) {
        case 32:
            bitmap_tag_data.sprite_budget_size = BitmapSpriteBudgetSize::BITMAP_SPRITE_BUDGET_SIZE_32X32;
            break;
        case 64:
            bitmap_tag_data.sprite_budget_size = BitmapSpriteBudgetSize::BITMAP_SPRITE_BUDGET_SIZE_64X64;
            break;
        case 128:
            bitmap_tag_data.sprite_budget_size = BitmapSpriteBudgetSize::BITMAP_SPRITE_BUDGET_SIZE_128X128;
            break;
        case 256:
            bitmap_tag_data.sprite_budget_size = BitmapSpriteBudgetSize::BITMAP_SPRITE_BUDGET_SIZE_256X256;
            break;
        case 512:
            bitmap_tag_data.sprite_budget_size = BitmapSpriteBudgetSize::BITMAP_SPRITE_BUDGET_SIZE_512X512;
            break;
        case 1024:
            bitmap_tag_data.sprite_budget_size = BitmapSpriteBudgetSize::BITMAP_SPRITE_BUDGET_SIZE_1024X1024;
            break;
        case 2048:
            bitmap_tag_data.sprite_budget_size = BitmapSpriteBudgetSize::BITMAP_SPRITE_BUDGET_SIZE_2048X2048;
            break;
        default:
            bitmap_tag_data.sprite_budget_size = BitmapSpriteBudgetSize::BITMAP_SPRITE_BUDGET_SIZE_32X32;
            break;
    }

    // Write it all
    try {
        if(!std::filesystem::exists(tag_path.parent_path())) {
            std::filesystem::create_directories(tag_path.parent_path());
        }
    }
    catch(std::exception &e) {
        eprintf_error("Error: Failed to create a directory: %s\n", e.what());
        return EXIT_FAILURE;
    }
    if(!File::save_file(final_path.c_str(), bitmap_tag_data.generate_hek_tag_data(TagClassInt::TAG_CLASS_BITMAP, true))) {
        eprintf_error("Error: Failed to write to %s.", final_path.c_str());
        return EXIT_FAILURE;
    }
}
