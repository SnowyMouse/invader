// SPDX-License-Identifier: GPL-3.0-only

#include "libtxc_dxtn/txc_dxtn.h"

#include "bitmap_data_writer.hpp"
#include <invader/tag/hek/class/bitmap.hpp>
#include <invader/printf.hpp>
#include <algorithm>

namespace Invader {
    void write_bitmap_data(const GeneratedBitmapData &scanned_color_plate, std::vector<std::byte> &bitmap_data_pixels, std::vector<Parser::BitmapData> &bitmap_data, BitmapUsage usage, BitmapFormat format, BitmapType bitmap_type, bool palettize, bool dither_alpha, bool dither_red, bool dither_green, bool dither_blue) {
        using namespace Invader::HEK;

        bool dithering = dither_alpha || dither_red || dither_green || dither_blue;

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
            std::size_t pixel_count = last_pixel - first_pixel;

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

            // Do dithering based on https://en.wikipedia.org/wiki/Floyd–Steinberg_dithering
            auto dither_do = [&dither_alpha, &dither_red, &dither_green, &dither_blue](auto to_palette_fn, auto from_palette_fn, auto *from_pixels, auto *to_pixels, std::uint32_t width, std::uint32_t height, std::uint32_t mipmap_count) {
                std::uint32_t mip_width = width;
                std::uint32_t mip_height = height;
                auto *mip_pixel = from_pixels;
                auto *mip_pixel_output = to_pixels;
                for(std::uint32_t m = 0; m <= mipmap_count; m++) {
                    for(std::uint32_t y = 0; y < mip_height; y++) {
                        for(std::uint32_t x = 0; x < mip_width; x++) {
                            // Get our pixels
                            auto &pixel = mip_pixel[x + y * mip_width];
                            auto &pixel_output = mip_pixel_output[x + y * mip_width];

                            // Convert
                            pixel_output = (pixel.*to_palette_fn)();

                            // Get the error
                            ColorPlatePixel p8_return = from_palette_fn(pixel_output);
                            float alpha_error = static_cast<std::int16_t>(pixel.alpha) - p8_return.alpha;
                            float red_error = static_cast<std::int16_t>(pixel.red) - p8_return.red;
                            float green_error = static_cast<std::int16_t>(pixel.green) - p8_return.green;
                            float blue_error = static_cast<std::int16_t>(pixel.blue) - p8_return.blue;

                            if(x > 0 && x < mip_width - 1 && y < mip_height - 1) {
                                // Apply the error
                                #define APPLY_ERROR(pixel, channel, error, multiply) {\
                                    float delta = (error * multiply / 16);\
                                    std::int16_t value = static_cast<std::int16_t>(pixel.channel) + delta;\
                                    if(value < 0) {\
                                        value = 0;\
                                    }\
                                    else if(value > UINT8_MAX) {\
                                        value = UINT8_MAX;\
                                    }\
                                    pixel.channel = static_cast<std::uint8_t>(value);\
                                }

                                auto &pixel_right = mip_pixel[x + y * mip_width + 1];
                                auto &pixel_below_left = mip_pixel[x + (y + 1) * mip_width - 1];
                                auto &pixel_below_middle = mip_pixel[x + (y + 1) * mip_width];
                                auto &pixel_below_right = mip_pixel[x + (y + 1) * mip_width + 1];

                                if(dither_alpha) {
                                    APPLY_ERROR(pixel_right, alpha, alpha_error, 7);
                                    APPLY_ERROR(pixel_below_left, alpha, alpha_error, 3);
                                    APPLY_ERROR(pixel_below_middle, alpha, alpha_error, 5);
                                    APPLY_ERROR(pixel_below_right, alpha, alpha_error, 1);
                                }

                                if(dither_red) {
                                    APPLY_ERROR(pixel_right, red, red_error, 7);
                                    APPLY_ERROR(pixel_below_left, red, red_error, 3);
                                    APPLY_ERROR(pixel_below_middle, red, red_error, 5);
                                    APPLY_ERROR(pixel_below_right, red, red_error, 1);
                                }

                                if(dither_green) {
                                    APPLY_ERROR(pixel_right, green, green_error, 7);
                                    APPLY_ERROR(pixel_below_left, green, green_error, 3);
                                    APPLY_ERROR(pixel_below_middle, green, green_error, 5);
                                    APPLY_ERROR(pixel_below_right, green, green_error, 1);
                                }

                                if(dither_blue) {
                                    APPLY_ERROR(pixel_right, blue, blue_error, 7);
                                    APPLY_ERROR(pixel_below_left, blue, blue_error, 3);
                                    APPLY_ERROR(pixel_below_middle, blue, blue_error, 5);
                                    APPLY_ERROR(pixel_below_right, blue, blue_error, 1);
                                }

                                #undef APPLY_ERROR
                            }
                        }
                    }

                    mip_pixel_output += mip_width * mip_height;
                    mip_pixel += mip_width * mip_height;
                    mip_height /= 2;
                    mip_width /= 2;
                }
            };

            // Determine if we should use P8 bump
            bool should_p8 = usage == BitmapUsage::BITMAP_USAGE_HEIGHT_MAP && palettize;
            if(should_p8) {
                compressed = false;
                bitmap.format = BitmapDataFormat::BITMAP_DATA_FORMAT_P8_BUMP;
            }

            // Depending on the format, do something
            auto bitmap_format = bitmap.format;
            switch(bitmap_format) {
                // If it's 32-bit, this is a no-op.
                case BitmapDataFormat::BITMAP_DATA_FORMAT_A8R8G8B8:
                case BitmapDataFormat::BITMAP_DATA_FORMAT_X8R8G8B8:
                    break;

                // If it's 16-bit, there is stuff we will need to do
                case BitmapDataFormat::BITMAP_DATA_FORMAT_A1R5G5B5:
                case BitmapDataFormat::BITMAP_DATA_FORMAT_A4R4G4B4:
                case BitmapDataFormat::BITMAP_DATA_FORMAT_R5G6B5: {
                    // Figure out what we'll be doing
                    std::uint16_t (ColorPlatePixel::*conversion_function)();
                    ColorPlatePixel (*deconversion_function)(std::uint16_t);

                    switch(bitmap_format) {
                        case BitmapDataFormat::BITMAP_DATA_FORMAT_A1R5G5B5:
                            conversion_function = &ColorPlatePixel::convert_to_16_bit<1,5,5,5>;
                            deconversion_function = ColorPlatePixel::convert_from_16_bit<1,5,5,5>;
                            break;
                        case BitmapDataFormat::BITMAP_DATA_FORMAT_A4R4G4B4:
                            conversion_function = &ColorPlatePixel::convert_to_16_bit<4,4,4,4>;
                            deconversion_function = ColorPlatePixel::convert_from_16_bit<4,4,4,4>;
                            break;
                        case BitmapDataFormat::BITMAP_DATA_FORMAT_R5G6B5:
                            conversion_function = &ColorPlatePixel::convert_to_16_bit<0,5,6,5>;
                            deconversion_function = ColorPlatePixel::convert_from_16_bit<0,5,6,5>;
                            break;
                        default:
                            std::terminate();
                            break;
                    }

                    // Begin
                    std::vector<LittleEndian<std::uint16_t>> new_bitmap_pixels(pixel_count);
                    auto *pixel_16_bit = reinterpret_cast<std::uint16_t *>(new_bitmap_pixels.data());

                    if(dithering) {
                        dither_do(conversion_function, deconversion_function, first_pixel, pixel_16_bit, bitmap.width, bitmap.height, mipmap_count);
                    }
                    else {
                        for(ColorPlatePixel *pixel_32_bit = first_pixel; pixel_32_bit < last_pixel; pixel_32_bit++, pixel_16_bit++) {
                            *pixel_32_bit = deconversion_function((pixel_32_bit->*conversion_function)());
                            *pixel_16_bit = (pixel_32_bit->*conversion_function)();
                        }
                    }

                    // Replace buffers
                    current_bitmap_pixels.clear();
                    current_bitmap_pixels.insert(current_bitmap_pixels.end(), reinterpret_cast<std::byte *>(new_bitmap_pixels.begin().base()), reinterpret_cast<std::byte *>(new_bitmap_pixels.end().base()));

                    break;
                }

                // If it's monochrome, it depends
                case BitmapDataFormat::BITMAP_DATA_FORMAT_A8:
                case BitmapDataFormat::BITMAP_DATA_FORMAT_AY8: {
                    std::vector<LittleEndian<std::uint8_t>> new_bitmap_pixels(pixel_count);
                    auto *pixel_8_bit = reinterpret_cast<std::uint8_t *>(new_bitmap_pixels.data());
                    for(auto *pixel = first_pixel; pixel < last_pixel; pixel++, pixel_8_bit++) {
                        *pixel_8_bit = pixel->alpha;
                    }
                    current_bitmap_pixels.clear();
                    current_bitmap_pixels.insert(current_bitmap_pixels.end(), reinterpret_cast<std::byte *>(new_bitmap_pixels.begin().base()), reinterpret_cast<std::byte *>(new_bitmap_pixels.end().base()));
                    break;
                }
                case BitmapDataFormat::BITMAP_DATA_FORMAT_Y8: {
                    std::vector<LittleEndian<std::uint8_t>> new_bitmap_pixels(pixel_count);
                    auto *pixel_8_bit = reinterpret_cast<std::uint8_t *>(new_bitmap_pixels.data());
                    for(auto *pixel = first_pixel; pixel < last_pixel; pixel++, pixel_8_bit++) {
                        *pixel_8_bit = pixel->convert_to_y8();
                    }
                    current_bitmap_pixels.clear();
                    current_bitmap_pixels.insert(current_bitmap_pixels.end(), reinterpret_cast<std::byte *>(new_bitmap_pixels.begin().base()), reinterpret_cast<std::byte *>(new_bitmap_pixels.end().base()));
                    break;
                }
                case BitmapDataFormat::BITMAP_DATA_FORMAT_A8Y8: {
                    std::vector<LittleEndian<std::uint16_t>> new_bitmap_pixels(pixel_count);
                    auto *pixel_16_bit = reinterpret_cast<std::uint16_t *>(new_bitmap_pixels.data());
                    for(auto *pixel = first_pixel; pixel < last_pixel; pixel++, pixel_16_bit++) {
                        *pixel_16_bit = pixel->convert_to_a8y8();
                    }
                    current_bitmap_pixels.clear();
                    current_bitmap_pixels.insert(current_bitmap_pixels.end(), reinterpret_cast<std::byte *>(new_bitmap_pixels.begin().base()), reinterpret_cast<std::byte *>(new_bitmap_pixels.end().base()));
                    break;
                }
                case BitmapDataFormat::BITMAP_DATA_FORMAT_P8_BUMP: {
                    std::vector<LittleEndian<std::uint8_t>> new_bitmap_pixels(pixel_count);
                    auto *pixel_8_bit = reinterpret_cast<std::uint8_t *>(new_bitmap_pixels.data());

                    // If we're dithering, do dithering things
                    if(dithering) {
                        dither_do(&ColorPlatePixel::convert_to_p8, ColorPlatePixel::convert_from_p8, first_pixel, pixel_8_bit, bitmap.width, bitmap.height, mipmap_count);
                    }
                    else {
                        for(auto *pixel = first_pixel; pixel < last_pixel; pixel++, pixel_8_bit++) {
                            *pixel_8_bit = pixel->convert_to_p8();
                        }
                    }

                    current_bitmap_pixels.clear();
                    current_bitmap_pixels.insert(current_bitmap_pixels.end(), reinterpret_cast<std::byte *>(new_bitmap_pixels.begin().base()), reinterpret_cast<std::byte *>(new_bitmap_pixels.end().base()));
                    palettized = true;
                    break;
                }

                // Do DisguXTing compression
                case BitmapDataFormat::BITMAP_DATA_FORMAT_DXT1:
                case BitmapDataFormat::BITMAP_DATA_FORMAT_DXT3:
                case BitmapDataFormat::BITMAP_DATA_FORMAT_DXT5: {
                    CompressionFormat format;
                    std::size_t block_size;
                    static const constexpr std::size_t block_length = 4;
                    
                    switch(bitmap.format) {
                        case BitmapDataFormat::BITMAP_DATA_FORMAT_DXT1:
                            format = alpha_present ? CompressionFormat::GL_COMPRESSED_RGBA_S3TC_DXT1_EXT : CompressionFormat::GL_COMPRESSED_RGB_S3TC_DXT1_EXT;
                            block_size = sizeof(std::uint64_t);
                            break;
                        case BitmapDataFormat::BITMAP_DATA_FORMAT_DXT3:
                            format = CompressionFormat::GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
                            block_size = sizeof(std::uint64_t) * 2;
                            break;
                        case BitmapDataFormat::BITMAP_DATA_FORMAT_DXT5:
                            format = CompressionFormat::GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
                            block_size = sizeof(std::uint64_t) * 2;
                            break;
                        default:
                            std::terminate();
                    }
                    
                    // Go through each 4x4 block and make them compressed
                    std::size_t mipmap_width = bitmap.width;
                    std::size_t mipmap_height = bitmap.height;
                    std::size_t minimum_dimension = 1;
                    
                    std::vector<std::byte> dxt_data;
                    
                    for(std::size_t i = 0; i <= mipmap_count; i++) {
                        for(std::size_t y = 0; y < mipmap_height; y+=block_length) {
                            for(std::size_t x = 0; x < mipmap_width; x+=block_length) {
                                dxt_data.insert(dxt_data.end(), block_size, std::byte());
                                
                                std::size_t block_width = 4 - (mipmap_width - x) % block_length;
                                std::size_t block_height = 4 - (mipmap_height - y) % block_length;
                                
                                std::uint32_t block_to_compress[block_length*block_length];
                                for(std::size_t yb = 0; yb < block_height; yb++) {
                                    for(std::size_t xb = 0; xb < block_width; xb++) {
                                        std::uint32_t color = first_pixel[x + xb + (y + yb) * mipmap_width].convert_to_32_bit();
                                        
                                        // Swap red and blue channels
                                        color = (color & 0xFF00FF00) | ((color & 0xFF0000) >> 16) | ((color & 0xFF) << 16);
                                        
                                        block_to_compress[xb + yb * block_length] = color;
                                    }
                                }
                                
                                
                                auto *data = dxt_data.data() + dxt_data.size() - block_size;
                                tx_compress_dxtn(4, block_width, block_height, reinterpret_cast<const std::uint8_t *>(block_to_compress), format, reinterpret_cast<std::uint8_t *>(data), block_length);
                            }
                        }
                        
                        first_pixel += mipmap_width * mipmap_height;
                        
                        mipmap_width = std::max(mipmap_width / 2, minimum_dimension);
                        mipmap_height = std::max(mipmap_height / 2, minimum_dimension);
                    }
                    
                    current_bitmap_pixels = std::move(dxt_data);

                    break;
                }

                default:
                    bitmap.format = alpha_present == AlphaType::ALPHA_TYPE_NONE ? BitmapDataFormat::BITMAP_DATA_FORMAT_X8R8G8B8 : BitmapDataFormat::BITMAP_DATA_FORMAT_A8R8G8B8;
                    break;
            }

            // Add pixel data to the end
            bitmap_data_pixels.insert(bitmap_data_pixels.end(), current_bitmap_pixels.begin(), current_bitmap_pixels.end());

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

            oprintf("    Bitmap #%zu: %ux%u, %u mipmap%s, %s%s - %.03f MiB\n", i, scanned_color_plate.bitmaps[i].width, scanned_color_plate.bitmaps[i].height, mipmap_count, mipmap_count == 1 ? "" : "s", bitmap_data_format_name(bitmap.format), (bitmap.format == BitmapDataFormat::BITMAP_DATA_FORMAT_DXT1 && alpha_present != AlphaType::ALPHA_TYPE_NONE) ? "a" : "", BYTES_TO_MIB(current_bitmap_pixels.size()));
        }
    }
}
