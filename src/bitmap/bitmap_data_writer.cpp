// SPDX-License-Identifier: GPL-3.0-only

#include "bitmap_data_writer.hpp"
#include <invader/tag/hek/class/bitmap.hpp>
#include <invader/printf.hpp>
#include <invader/bitmap/bitmap_encode.hpp>
#include <algorithm>
#include <squish.h>

namespace Invader {
    void write_bitmap_data(const GeneratedBitmapData &scanned_color_plate, std::vector<std::byte> &bitmap_data_pixels, std::vector<Parser::BitmapData> &bitmap_data, BitmapUsage usage, std::optional<BitmapFormat> &format, BitmapType bitmap_type, bool palettize, bool dither_alpha, bool dither_red, bool dither_green, bool dither_blue) {
        using namespace Invader::HEK;

        auto bitmap_count = scanned_color_plate.bitmaps.size();
        
        // If format is nullopt, automatically determine a format
        bool automatically_determined_format = !format.has_value();
        if(automatically_determined_format) {
            // Height maps and vector maps must always be 32-bit
            if(usage == BitmapUsage::BITMAP_USAGE_HEIGHT_MAP || usage == BitmapUsage::BITMAP_USAGE_VECTOR_MAP) {
                format = BitmapFormat::BITMAP_FORMAT_32_BIT;
            }
            else {
                // Determine if we can make it smaller
                bool is_monochrome = true;
                bool is_16_bit = true;
                for(auto &b : scanned_color_plate.bitmaps) {
                    bool is_a0r5g6b5 = true;
                    bool is_a1r5g5b5 = true;
                    bool is_a4r4g4b4 = true;
                    for(auto &pixel : b.pixels) {
                        is_monochrome = is_monochrome && (pixel.red == pixel.green && pixel.green == pixel.blue);
                        is_a0r5g6b5 = is_a0r5g6b5 && (pixel == Pixel::convert_from_16_bit<0,5,6,5>(pixel.convert_to_16_bit<0,5,6,5>()));
                        is_a1r5g5b5 = is_a1r5g5b5 && (pixel == Pixel::convert_from_16_bit<1,5,5,5>(pixel.convert_to_16_bit<1,5,5,5>()));
                        is_a4r4g4b4 = is_a4r4g4b4 && (pixel == Pixel::convert_from_16_bit<4,4,4,4>(pixel.convert_to_16_bit<4,4,4,4>()));
                    }
                    is_16_bit = is_16_bit && (is_a0r5g6b5 || is_a1r5g5b5 || is_a4r4g4b4);
                    
                    // If we don't fit into the 16-bit or monochrome color space, bail!
                    if(!is_16_bit && !is_monochrome) {
                        break;
                    }
                }
                
                if(is_monochrome) {
                    format = BitmapFormat::BITMAP_FORMAT_MONOCHROME;
                }
                else if(is_16_bit) {
                    format = BitmapFormat::BITMAP_FORMAT_16_BIT;
                }
                else {
                    format = BitmapFormat::BITMAP_FORMAT_32_BIT;
                }
            }
            
            switch(*format) {
                case BitmapFormat::BITMAP_FORMAT_32_BIT:
                    oprintf("Automatically determined format as 32-bit\n");
                    break;
                case BitmapFormat::BITMAP_FORMAT_16_BIT:
                    oprintf("Automatically determined format as 16-bit\n");
                    break;
                case BitmapFormat::BITMAP_FORMAT_MONOCHROME:
                    oprintf("Automatically determined format as monochrome\n");
                    break;
                default:
                    std::terminate();
            }
        }
        
        oprintf("Found %zu bitmap%s:\n", bitmap_count, bitmap_count == 1 ? "" : "s");
        
        for(std::size_t i = 0; i < bitmap_count; i++) {
            // Write all of the fields here
            auto &bitmap = bitmap_data.emplace_back();
            auto &bitmap_color_plate = scanned_color_plate.bitmaps[i];
            bitmap.bitmap_class = TagFourCC::TAG_FOURCC_BITMAP;
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
            auto *first_pixel = reinterpret_cast<Pixel *>(current_bitmap_pixels.data());
            bitmap.format = BitmapEncode::most_efficient_format(current_bitmap_pixels.data(), bitmap.width, bitmap.height, bitmap.depth, *format, bitmap.type, mipmap_count);

            // Set the format
            bool compressed = (format == BitmapFormat::BITMAP_FORMAT_DXT1 || format == BitmapFormat::BITMAP_FORMAT_DXT3 || format == BitmapFormat::BITMAP_FORMAT_DXT5);

            // If the bitmap length or height isn't divisible by 4, use 32-bit color
            if(compressed && ((bitmap.height % 4) != 0 || (bitmap.width % 4) != 0)) {
                format = BitmapFormat::BITMAP_FORMAT_32_BIT;
                compressed = false;
            }

            // Set palettized
            bool palettized = false;

            // Determine if we should use P8 bump
            bool should_p8 = (usage == BitmapUsage::BITMAP_USAGE_HEIGHT_MAP || usage == BitmapUsage::BITMAP_USAGE_VECTOR_MAP) && palettize;
            if(should_p8) {
                compressed = false;
                bitmap.format = BitmapDataFormat::BITMAP_DATA_FORMAT_P8_BUMP;
            }

            // Go through each mipmap; compress
            bitmap.mipmap_count = mipmap_count;
            auto encoded_pixels = BitmapEncode::encode_bitmap(reinterpret_cast<const std::byte *>(first_pixel), BitmapDataFormat::BITMAP_DATA_FORMAT_A8R8G8B8, bitmap.format, bitmap.width, bitmap.height, bitmap.depth, bitmap.type, bitmap.mipmap_count, dither_alpha, dither_red, dither_green, dither_blue);
            
            bitmap_data_pixels.insert(bitmap_data_pixels.end(), encoded_pixels.begin(), encoded_pixels.end());

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

            oprintf("    Bitmap #%zu: %ux%u, %u mipmap%s, %s - %.03f MiB\n", i, scanned_color_plate.bitmaps[i].width, scanned_color_plate.bitmaps[i].height, mipmap_count, mipmap_count == 1 ? "" : "s", bitmap_data_format_name(bitmap.format), BYTES_TO_MIB(encoded_pixels.size()));
        }
    }
}
