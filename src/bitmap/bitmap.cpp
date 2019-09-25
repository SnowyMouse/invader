/*
 * Invader (c) 2019 Kavawuvi
 *
 * This program is free software under the GNU General Public License v3.0. See LICENSE for more information.
 */

#include <getopt.h>
#include <zlib.h>
#include <filesystem>
#include <optional>

#include "../eprintf.hpp"
#include "../version.hpp"
#include "../tag/hek/class/bitmap.hpp"
#include "image_loader.hpp"
#include "color_plate_scanner.hpp"
#include "bitmap_data_writer.hpp"

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

    // Long options
    int longindex = 0;
    static struct option options[] = {
        {"info",  no_argument, 0, 'i'},
        {"ignore-tag",  no_argument, 0, 'I'},
        {"help",  no_argument, 0, 'h'},
        {"dithering",  required_argument, 0, 'D'},
        {"data",  required_argument, 0, 'd' },
        {"tags",  required_argument, 0, 't' },
        {"format", required_argument, 0, 'F' },
        {"type", required_argument, 0, 'T' },
        {"mipmap-count", required_argument, 0, 'm' },
        {"mipmap-scale", required_argument, 0, 's' },
        {"detail-fade", required_argument, 0, 'f' },
        {"budget", required_argument, 0, 'B' },
        {"budget-count", required_argument, 0, 'C' },
        {"bump-palettize", required_argument, 0, 'p' },
        {"bump-palettise", required_argument, 0, 'p' },
        {"bump-height", required_argument, 0, 'H' },
        {0, 0, 0, 0 }
    };

    // Go through each argument
    while((opt = getopt_long(argc, argv, "D:iIhd:t:f:s:f:F:m:T:B:C:p:h:u:H:", options, &longindex)) != -1) {
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
                dither_alpha = false;
                dither_red = false;
                dither_green = false;
                dither_blue = false;
                for(const char *c = optarg; *c; c++) {
                    switch(*c) {
                        case 'a':
                            dither_alpha = true;
                            break;
                        case 'r':
                            dither_red = true;
                            break;
                        case 'g':
                            dither_green = true;
                            break;
                        case 'b':
                            dither_blue = true;
                            break;
                        default:
                            printf("Unknown channel %c.\n", *c);
                            return EXIT_FAILURE;
                    }
                }
                break;

            case 'p':
                if(strcmp(optarg,"on") == 0) {
                    palettize = true;
                }
                else if(strcmp(optarg,"off") == 0) {
                    palettize = false;
                }
                else {
                    eprintf("Unknown palettize setting %s\n", optarg);
                    return EXIT_FAILURE;
                }
                break;

            case 'u':
                if(strcmp(optarg, "default") == 0) {
                    usage = BitmapUsage::BITMAP_USAGE_DEFAULT;
                }
                else if(strcmp(optarg, "bumpmap") == 0) {
                    usage = BitmapUsage::BITMAP_USAGE_HEIGHT_MAP;
                }
                else if(strcmp(optarg, "detail") == 0) {
                    usage = BitmapUsage::BITMAP_USAGE_DETAIL_MAP;
                }
                else {
                    eprintf("Unknown usage %s\n", optarg);
                    return EXIT_FAILURE;
                }
                break;

            case 'H':
                bump_height = static_cast<float>(std::strtof(optarg, nullptr));
                break;

            case 'm':
                max_mipmap_count = static_cast<std::int32_t>(std::strtol(optarg, nullptr, 10));
                break;

            case 'C':
                sprite_budget_count = static_cast<std::uint32_t>(std::strtoul(optarg, nullptr, 10));
                break;

            case 'B':
                sprite_budget = static_cast<std::uint32_t>(std::strtoul(optarg, nullptr, 10));
                switch(sprite_budget.value()) {
                    case 32:
                    case 64:
                    case 128:
                    case 256:
                    case 512:
                        break;
                    default:
                        eprintf("Invalid sprite budget %u.\n", sprite_budget.value());
                        break;
                }

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
        if(!usage.has_value()) {
            usage = bitmap_tag_header.usage;
        }
        if(!palettize.has_value()) {
            palettize = !bitmap_tag_header.flags.read().disable_height_map_compression;
        }
        if(!bump_height.has_value()) {
            bump_height = bitmap_tag_header.bump_height;
        }
        if(!sharpen.has_value() && bitmap_tag_header.sharpen_amount > 0.0F && bitmap_tag_header.sharpen_amount <= 1.0F) {
            sharpen = bitmap_tag_header.sharpen_amount;
        }
        if(!blur.has_value() && bitmap_tag_header.blur_filter_size > 0.0F) {
            blur = bitmap_tag_header.blur_filter_size;
        }

        auto header_flags = bitmap_tag_header.flags.read();
        if(!mipmap_scale_type.has_value()) {
            if(header_flags.invader_nearest_mipmap_alpha_and_color) {
                mipmap_scale_type = ScannedColorMipmapType::SCANNED_COLOR_MIPMAP_NEAREST_ALPHA_COLOR;
            }
            else if(header_flags.invader_nearest_mipmap_alpha) {
                mipmap_scale_type = ScannedColorMipmapType::SCANNED_COLOR_MIPMAP_NEAREST_ALPHA;
            }
            else {
                mipmap_scale_type = ScannedColorMipmapType::SCANNED_COLOR_MIPMAP_LINEAR;
            }
        }
        if(!dithering) {
            dither_alpha = header_flags.invader_dither_alpha == 1;
            dither_red = header_flags.invader_dither_red == 1;
            dither_green = header_flags.invader_dither_green == 1;
            dither_blue = header_flags.invader_dither_blue == 1;
        }

        std::fclose(tag_read);
    }

    // If these values weren't set, set them
    #define DEFAULT_VALUE(what, default) if(!what.has_value()) { what = default; }

    DEFAULT_VALUE(format,BitmapFormat::BITMAP_FORMAT_32_BIT_COLOR);
    DEFAULT_VALUE(bitmap_type,BitmapType::BITMAP_TYPE_2D_TEXTURES);
    DEFAULT_VALUE(max_mipmap_count,INT16_MAX);
    DEFAULT_VALUE(sprite_usage,BitmapSpriteUsage::BITMAP_SPRITE_USAGE_BLEND_ADD_SUBTRACT_MAX);
    DEFAULT_VALUE(sprite_budget,32);
    DEFAULT_VALUE(sprite_budget_count,0);
    DEFAULT_VALUE(mipmap_scale_type,ScannedColorMipmapType::SCANNED_COLOR_MIPMAP_LINEAR);
    DEFAULT_VALUE(mipmap_fade,0.0F);
    DEFAULT_VALUE(usage,BitmapUsage::BITMAP_USAGE_DEFAULT);
    DEFAULT_VALUE(palettize,false);
    DEFAULT_VALUE(bump_height,0.02F);
    DEFAULT_VALUE(mipmap_fade,0.0F);
    DEFAULT_VALUE(dither_alpha,false);
    DEFAULT_VALUE(dither_red,false);
    DEFAULT_VALUE(dither_green,false);
    DEFAULT_VALUE(dither_blue,false);

    #undef DEFAULT_VALUE

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
        p.sprite_usage = sprite_usage.value();
    }

    // Do it!
    auto scanned_color_plate = ColorPlateScanner::scan_color_plate(reinterpret_cast<const ColorPlatePixel *>(image_pixels), image_width, image_height, bitmap_type.value(), usage.value(), bump_height.value(), sprite_parameters, max_mipmap_count.value(), mipmap_scale_type.value(), usage == BitmapUsage::BITMAP_USAGE_DETAIL_MAP ? mipmap_fade : std::nullopt, sharpen, blur);
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
    printf("Found %zu bitmap%s:\n", bitmap_count, bitmap_count == 1 ? "" : "s");
    write_bitmap_data(scanned_color_plate, bitmap_data_pixels, bitmap_data, usage.value(), format.value(), bitmap_type.value(), palettize.value(), dither_alpha.value(), dither_red.value(), dither_green.value(), dither_blue.value());
    std::printf("Total: %.03f MiB\n", BYTES_TO_MIB(bitmap_data_pixels.size()));

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
    new_tag_header.usage = usage.value();
    new_tag_header.bump_height = bump_height.value();
    new_tag_header.detail_fade_factor = mipmap_fade.value();
    new_tag_header.format = format.value();
    new_tag_header.sharpen_amount = sharpen.value_or(0.0F);
    new_tag_header.blur_filter_size = blur.value_or(0.0F);
    if(max_mipmap_count.value() >= INT16_MAX) {
        new_tag_header.mipmap_count = 0;
    }
    else {
        new_tag_header.mipmap_count = max_mipmap_count.value() + 1;
    }

    BitmapFlags flags = {};
    flags.disable_height_map_compression = !palettize.value();
    switch(mipmap_scale_type.value()) {
        case ScannedColorMipmapType::SCANNED_COLOR_MIPMAP_LINEAR:
            break;
        case ScannedColorMipmapType::SCANNED_COLOR_MIPMAP_NEAREST_ALPHA:
            flags.invader_nearest_mipmap_alpha = 1;
            break;
        case ScannedColorMipmapType::SCANNED_COLOR_MIPMAP_NEAREST_ALPHA_COLOR:
            flags.invader_nearest_mipmap_alpha_and_color = 1;
            break;
    };
    flags.invader_dither_alpha = dither_alpha.value();
    flags.invader_dither_red = dither_red.value();
    flags.invader_dither_green = dither_green.value();
    flags.invader_dither_blue = dither_blue.value();
    new_tag_header.flags = flags;

    new_tag_header.sprite_spacing = sprite_parameters.value_or(ColorPlateScannerSpriteParameters{}).sprite_spacing;
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
