// SPDX-License-Identifier: GPL-3.0-only

#include "bitmap_data_writer.hpp"
#include <invader/tag/hek/class/bitmap.hpp>
#include <invader/printf.hpp>
#include <invader/bitmap/bitmap_encode.hpp>
#include <algorithm>
#include <squish.h>

namespace Invader {
    void write_bitmap_data(const GeneratedBitmapData &scanned_color_plate, std::vector<std::byte> &bitmap_data_pixels, std::vector<Parser::BitmapData> &bitmap_data, BitmapUsage usage, BitmapFormat format, BitmapType bitmap_type, bool palettize, bool dither_alpha, bool dither_red, bool dither_green, bool dither_blue) {
        using namespace Invader::HEK;

        auto bitmap_count = scanned_color_plate.bitmaps.size();
        for(std::size_t i = 0; i < bitmap_count; i++) {
            // Write all of the fields here
            auto &bitmap = bitmap_data.emplace_back();
            auto &bitmap_color_plate = scanned_color_plate.bitmaps[i];
            bitmap.bitmap_class = TagClassInt::TAG_CLASS_BITMAP;
            bitmap.width = bitmap_color_plate.width;
            bitmap.height = bitmap_color_plate.height;
            switch(bitmap_type) {
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
            bitmap.pixel_data_offset = static_cast<std::uint32_t>(bitmap_data_pixels.size());
            std::uint32_t mipmap_count = bitmap_color_plate.mipmaps.size();

            // Get the data
            std::vector<std::byte> current_bitmap_pixels(reinterpret_cast<const std::byte *>(bitmap_color_plate.pixels.data()), reinterpret_cast<const std::byte *>(bitmap_color_plate.pixels.data() + bitmap_color_plate.pixels.size()));

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

            // If we need to do monochrome, check if the alpha equals luminosity
            if(format == BitmapFormat::BITMAP_FORMAT_MONOCHROME) {
                for(auto *pixel = first_pixel; pixel < last_pixel; pixel++) {
                    std::uint8_t luminosity = pixel->convert_to_y8();

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

            // If we aren't doing monochrome, then we check the bitness of the alpha (1-bit = 0xFF or 0x00, which if we only had that, we can get 5-bit color on 16-bit ARGB instead of 4-bit color)
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
            bool compressed = (format == BitmapFormat::BITMAP_FORMAT_DXT1 || format == BitmapFormat::BITMAP_FORMAT_DXT3 || format == BitmapFormat::BITMAP_FORMAT_DXT3);

            // If the bitmap length or height isn't divisible by 4, use 32-bit color
            if(compressed && ((bitmap.height % 4) != 0 || (bitmap.width % 4) != 0)) {
                format = BitmapFormat::BITMAP_FORMAT_32_BIT;
                compressed = false;
            }

            // Set palettized
            bool palettized = false;

            switch(format) {
                case BitmapFormat::BITMAP_FORMAT_32_BIT:
                    bitmap.format = alpha_present == AlphaType::ALPHA_TYPE_NONE ? BitmapDataFormat::BITMAP_DATA_FORMAT_X8R8G8B8 : BitmapDataFormat::BITMAP_DATA_FORMAT_A8R8G8B8;
                    break;
                case BitmapFormat::BITMAP_FORMAT_16_BIT:
                    switch(alpha_present) {
                        case ALPHA_TYPE_NONE:
                            bitmap.format = BitmapDataFormat::BITMAP_DATA_FORMAT_R5G6B5;
                            break;
                        case ALPHA_TYPE_ONE_BIT:
                            bitmap.format = BitmapDataFormat::BITMAP_DATA_FORMAT_A1R5G5B5;
                            break;
                        case ALPHA_TYPE_MULTI_BIT:
                            bitmap.format = BitmapDataFormat::BITMAP_DATA_FORMAT_A4R4G4B4;
                            break;
                    }
                    break;
                case BitmapFormat::BITMAP_FORMAT_MONOCHROME:
                    if(alpha_equals_luminosity) {
                        bitmap.format = BitmapDataFormat::BITMAP_DATA_FORMAT_AY8;
                    }
                    else if(alpha_present == ALPHA_TYPE_MULTI_BIT) {
                        if(luminosity_set) {
                            bitmap.format = BitmapDataFormat::BITMAP_DATA_FORMAT_A8Y8;
                        }
                        else {
                            bitmap.format = BitmapDataFormat::BITMAP_DATA_FORMAT_A8;
                        }
                    }
                    else {
                        bitmap.format = BitmapDataFormat::BITMAP_DATA_FORMAT_Y8;
                    }
                    break;

                case BitmapFormat::BITMAP_FORMAT_DXT1:
                    bitmap.format = BitmapDataFormat::BITMAP_DATA_FORMAT_DXT1;
                    break;
                case BitmapFormat::BITMAP_FORMAT_DXT3:
                    bitmap.format = alpha_present == AlphaType::ALPHA_TYPE_NONE ? BitmapDataFormat::BITMAP_DATA_FORMAT_DXT1 : BitmapDataFormat::BITMAP_DATA_FORMAT_DXT3;
                    break;
                case BitmapFormat::BITMAP_FORMAT_DXT5:
                    bitmap.format = alpha_present == AlphaType::ALPHA_TYPE_NONE ? BitmapDataFormat::BITMAP_DATA_FORMAT_DXT1 : BitmapDataFormat::BITMAP_DATA_FORMAT_DXT5;
                    break;

                default:
                    eprintf_error("Unsupported bitmap format.");
                    throw InvalidBitmapFormatException();
            }

            // Determine if we should use P8 bump
            bool should_p8 = usage == BitmapUsage::BITMAP_USAGE_HEIGHT_MAP && palettize;
            if(should_p8) {
                compressed = false;
                bitmap.format = BitmapDataFormat::BITMAP_DATA_FORMAT_P8_BUMP;
            }

            // Go through each mipmap; compress
            std::size_t minimum_dimension = 1;
            std::size_t mipmap_width = bitmap.width;
            std::size_t mipmap_height = bitmap.height;
            std::size_t total_size = 0;
            
            for(std::size_t i = 0; i <= mipmap_count; i++) {
                auto new_data = BitmapEncode::encode_bitmap(reinterpret_cast<const std::byte *>(first_pixel), BitmapDataFormat::BITMAP_DATA_FORMAT_A8R8G8B8, bitmap.format, mipmap_width, mipmap_height, dither_alpha, dither_red, dither_green, dither_blue);
                
                bitmap_data_pixels.insert(bitmap_data_pixels.end(), new_data.begin(), new_data.end());
                total_size += new_data.size();
                
                first_pixel += mipmap_width * mipmap_height;
                
                mipmap_width = std::max(mipmap_width / 2, minimum_dimension);
                mipmap_height = std::max(mipmap_height / 2, minimum_dimension);
            }

            bitmap.mipmap_count = mipmap_count;

            BitmapDataFlags flags = {};
            if(compressed) {
                flags |= HEK::BitmapDataFlagsFlag::BITMAP_DATA_FLAGS_FLAG_COMPRESSED;
            }
            if(bitmap_type == BitmapType::BITMAP_TYPE_INTERFACE_BITMAPS) {
                flags |= HEK::BitmapDataFlagsFlag::BITMAP_DATA_FLAGS_FLAG_LINEAR;
            }
            
            if(is_power_of_two(bitmap.width) && is_power_of_two(bitmap.height)) {
                flags |= HEK::BitmapDataFlagsFlag::BITMAP_DATA_FLAGS_FLAG_POWER_OF_TWO_DIMENSIONS;
            }
            if(palettized) {
                flags |= HEK::BitmapDataFlagsFlag::BITMAP_DATA_FLAGS_FLAG_PALETTIZED;
            }
            bitmap.flags = flags;

            bitmap.registration_point.x = bitmap_color_plate.registration_point_x;
            bitmap.registration_point.y = bitmap_color_plate.registration_point_y;

            #define BYTES_TO_MIB(bytes) (bytes / 1024.0F / 1024.0F)

            oprintf("    Bitmap #%zu: %ux%u, %u mipmap%s, %s%s - %.03f MiB\n", i, scanned_color_plate.bitmaps[i].width, scanned_color_plate.bitmaps[i].height, mipmap_count, mipmap_count == 1 ? "" : "s", bitmap_data_format_name(bitmap.format), (bitmap.format == BitmapDataFormat::BITMAP_DATA_FORMAT_DXT1 && alpha_present != AlphaType::ALPHA_TYPE_NONE) ? " with alpha" : "", BYTES_TO_MIB(total_size));
        }
    }
}
