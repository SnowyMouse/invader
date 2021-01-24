// SPDX-License-Identifier: GPL-3.0-only

#include <invader/bitmap/bitmap_encode.hpp>
#include <invader/tag/hek/class/bitmap.hpp>
#include <invader/bitmap/pixel.hpp>
#include <squish.h>

namespace Invader::BitmapEncode {
    static std::vector<Pixel> decode_to_32_bit(const std::byte *input_data, HEK::BitmapDataFormat input_format, std::size_t width, std::size_t height);
    
    void encode_bitmap(const std::byte *input_data, HEK::BitmapDataFormat input_format, std::byte *output_data, HEK::BitmapDataFormat output_format, std::size_t width, std::size_t height, bool dither_alpha, bool dither_red, bool dither_green, bool dither_blue) noexcept {
        auto as_32_bit = decode_to_32_bit(input_data, input_format, width, height);
        auto first_pixel = as_32_bit.data();
        auto last_pixel = first_pixel + as_32_bit.size();
        
        bool dithering = dither_alpha || dither_red || dither_green || dither_blue;

        // Do dithering based on https://en.wikipedia.org/wiki/Floyd–Steinberg_dithering
        auto dither_do = [&dither_alpha, &dither_red, &dither_green, &dither_blue](auto to_palette_fn, auto from_palette_fn, auto *from_pixels, auto *to_pixels, std::uint32_t width, std::uint32_t height) {
            for(std::uint32_t y = 0; y < height; y++) {
                for(std::uint32_t x = 0; x < width; x++) {
                    // Get our pixels
                    auto &pixel = from_pixels[x + y * width];
                    auto &pixel_output = to_pixels[x + y * width];

                    // Convert
                    pixel_output = (pixel.*to_palette_fn)();

                    // Get the error
                    Pixel p8_return = from_palette_fn(pixel_output);
                    float alpha_error = static_cast<std::int16_t>(pixel.alpha) - p8_return.alpha;
                    float red_error = static_cast<std::int16_t>(pixel.red) - p8_return.red;
                    float green_error = static_cast<std::int16_t>(pixel.green) - p8_return.green;
                    float blue_error = static_cast<std::int16_t>(pixel.blue) - p8_return.blue;

                    if(x > 0 && x < width - 1 && y < height - 1) {
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

                        auto &pixel_right = from_pixels[x + y * width + 1];
                        auto &pixel_below_left = from_pixels[x + (y + 1) * width - 1];
                        auto &pixel_below_middle = from_pixels[x + (y + 1) * width];
                        auto &pixel_below_right = from_pixels[x + (y + 1) * width + 1];

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
        };
        
        switch(output_format) {
            // Straight copy
            case HEK::BitmapDataFormat::BITMAP_DATA_FORMAT_A8R8G8B8:
                std::memcpy(output_data, as_32_bit.data(), as_32_bit.size() * sizeof(Pixel));
                return;
            
            // Copy, but then set alpha to 0xFF
            case HEK::BitmapDataFormat::BITMAP_DATA_FORMAT_X8R8G8B8: {
                std::memcpy(output_data, as_32_bit.data(), as_32_bit.size() * sizeof(Pixel));
                auto *start = reinterpret_cast<Pixel *>(output_data);
                auto *end = reinterpret_cast<Pixel *>(output_data + as_32_bit.size());
                for(auto *i = start; i < end; i++) {
                    i->alpha = 0xFF;
                }
                return;
            }
            
            // If it's 16-bit, there is stuff we will need to do
            case HEK::BitmapDataFormat::BITMAP_DATA_FORMAT_A1R5G5B5:
            case HEK::BitmapDataFormat::BITMAP_DATA_FORMAT_A4R4G4B4:
            case HEK::BitmapDataFormat::BITMAP_DATA_FORMAT_R5G6B5: {
                // Figure out what we'll be doing
                std::uint16_t (Pixel::*conversion_function)() const;
                Pixel (*deconversion_function)(std::uint16_t);

                switch(output_format) {
                    case HEK::BitmapDataFormat::BITMAP_DATA_FORMAT_A1R5G5B5:
                        conversion_function = &Pixel::convert_to_16_bit<1,5,5,5>;
                        deconversion_function = Pixel::convert_from_16_bit<1,5,5,5>;
                        break;
                    case HEK::BitmapDataFormat::BITMAP_DATA_FORMAT_A4R4G4B4:
                        conversion_function = &Pixel::convert_to_16_bit<4,4,4,4>;
                        deconversion_function = Pixel::convert_from_16_bit<4,4,4,4>;
                        break;
                    case HEK::BitmapDataFormat::BITMAP_DATA_FORMAT_R5G6B5:
                        conversion_function = &Pixel::convert_to_16_bit<0,5,6,5>;
                        deconversion_function = Pixel::convert_from_16_bit<0,5,6,5>;
                        break;
                    default:
                        std::terminate();
                        break;
                }

                // Begin
                if(dithering) {
                    dither_do(conversion_function, deconversion_function, as_32_bit.data(), reinterpret_cast<HEK::LittleEndian<std::int16_t> *>(output_data), width, height);
                }
                else {
                    auto *pixel_16_bit = reinterpret_cast<HEK::LittleEndian<std::int16_t> *>(output_data);
                    
                    for(Pixel *pixel_32_bit = first_pixel; pixel_32_bit < last_pixel; pixel_32_bit++, pixel_16_bit++) {
                        *pixel_32_bit = deconversion_function((pixel_32_bit->*conversion_function)());
                        *pixel_16_bit = (pixel_32_bit->*conversion_function)();
                    }
                }

                return;
            }

            // If it's monochrome, it depends
            case HEK::BitmapDataFormat::BITMAP_DATA_FORMAT_A8:
            case HEK::BitmapDataFormat::BITMAP_DATA_FORMAT_AY8: {
                auto *pixel_8_bit = reinterpret_cast<std::uint8_t *>(output_data);
                for(auto *pixel = first_pixel; pixel < last_pixel; pixel++, pixel_8_bit++) {
                    *pixel_8_bit = pixel->alpha;
                }
                
                break;
            }
            case HEK::BitmapDataFormat::BITMAP_DATA_FORMAT_Y8: {
                auto *pixel_8_bit = reinterpret_cast<std::uint8_t *>(output_data);
                for(auto *pixel = first_pixel; pixel < last_pixel; pixel++, pixel_8_bit++) {
                    *pixel_8_bit = pixel->convert_to_y8();
                }
                break;
            }
            case HEK::BitmapDataFormat::BITMAP_DATA_FORMAT_A8Y8: {
                auto *pixel_16_bit = reinterpret_cast<HEK::LittleEndian<std::uint16_t> *>(output_data);
                for(auto *pixel = first_pixel; pixel < last_pixel; pixel++, pixel_16_bit++) {
                    *pixel_16_bit = pixel->convert_to_a8y8();
                }
                break;
            }
            case HEK::BitmapDataFormat::BITMAP_DATA_FORMAT_P8_BUMP: {
                auto *pixel_8_bit = reinterpret_cast<std::uint8_t *>(output_data);

                // If we're dithering, do dithering things
                if(dithering) {
                    dither_do(&Pixel::convert_to_p8, Pixel::convert_from_p8, as_32_bit.data(), pixel_8_bit, width, height);
                }
                else {
                    for(auto *pixel = first_pixel; pixel < last_pixel; pixel++, pixel_8_bit++) {
                        *pixel_8_bit = pixel->convert_to_p8();
                    }
                }
                
                break;
            }
            
            // Use libsquish
            case HEK::BitmapDataFormat::BITMAP_DATA_FORMAT_DXT1:
            case HEK::BitmapDataFormat::BITMAP_DATA_FORMAT_DXT3:
            case HEK::BitmapDataFormat::BITMAP_DATA_FORMAT_DXT5: {
                int flags = squish::kColourIterativeClusterFit | squish::kSourceBGRA;
                switch(output_format) {
                    case HEK::BitmapDataFormat::BITMAP_DATA_FORMAT_DXT1:
                        flags |= squish::kDxt1;
                        break;
                    case HEK::BitmapDataFormat::BITMAP_DATA_FORMAT_DXT3:
                        flags |= squish::kDxt3;
                        break;
                    case HEK::BitmapDataFormat::BITMAP_DATA_FORMAT_DXT5:
                        flags |= squish::kDxt5;
                        break;
                    default:
                        std::terminate();
                }
                
                std::vector<Pixel> data_to_compress(as_32_bit.data(), as_32_bit.data() + width * height);
                for(auto &i : data_to_compress) {
                    std::swap(i.blue, i.red);
                }
                squish::CompressImage(reinterpret_cast<const squish::u8 *>(data_to_compress.data()), width, height, output_data, flags);
                
                break;
            }
                
                
            default:
                std::terminate();
        }
    }
    
    std::vector<std::byte> encode_bitmap(const std::byte *input_data, HEK::BitmapDataFormat input_format, HEK::BitmapDataFormat output_format, std::size_t width, std::size_t height, bool dither_alpha, bool dither_red, bool dither_green, bool dither_blue) {
        // Get our output buffer
        std::vector<std::byte> output(bitmap_data_size(output_format, width, height));
        
        // Do it
        encode_bitmap(input_data, input_format, output.data(), output_format, width, height, dither_alpha, dither_red, dither_green, dither_blue);

        // Done
        return output;
    }
    
    std::size_t bitmap_data_size(HEK::BitmapDataFormat format, std::size_t width, std::size_t height) noexcept {
        // Determine the number of bits per pixel
        auto output_bits_per_pixel = HEK::calculate_bits_per_pixel(format);
        
        // DXT stuff is stored in 4x4 blocks
        std::size_t container_width, container_height;
        switch(format) {
            case HEK::BitmapDataFormat::BITMAP_DATA_FORMAT_DXT1:
            case HEK::BitmapDataFormat::BITMAP_DATA_FORMAT_DXT3:
            case HEK::BitmapDataFormat::BITMAP_DATA_FORMAT_DXT5:
                container_width = std::max(width, static_cast<std::size_t>(4));
                container_height = std::max(height, static_cast<std::size_t>(4));
                break;
            default:
                container_width = width;
                container_height = height;
                break;
        }
        
        // Done
        return (container_width * container_height * output_bits_per_pixel) / 8;
    }
    
    static std::vector<Pixel> decode_to_32_bit(const std::byte *input_data, HEK::BitmapDataFormat input_format, std::size_t width, std::size_t height) {
        // First, decode it to 32-bit A8R8G8B8 if necessary
        std::size_t pixel_count = width * height;
        std::vector<Pixel> data(pixel_count);
        
        auto decode_8_bit = [&pixel_count, &input_data, &data](Pixel (*with_what)(std::uint8_t)) {
            auto pixels_left = pixel_count;
            auto *bytes_to_add = input_data;
            auto *bytes_to_write = data.data();
            while(pixels_left) {
                auto from_pixel = *reinterpret_cast<const std::uint8_t *>(bytes_to_add);
                auto to_pixel = with_what(from_pixel);
                *bytes_to_write = to_pixel;

                pixels_left --;
                bytes_to_add += sizeof(from_pixel);
                bytes_to_write ++;
            }
        };

        auto decode_16_bit = [&pixel_count, &input_data, &data](Pixel (*with_what)(std::uint16_t)) {
            auto pixels_left = pixel_count;
            auto *bytes_to_add = input_data;
            auto *bytes_to_write = data.data();
            while(pixels_left) {
                auto from_pixel = *reinterpret_cast<const std::uint16_t *>(bytes_to_add);
                auto to_pixel = with_what(from_pixel);
                *bytes_to_write = to_pixel;

                pixels_left --;
                bytes_to_add += sizeof(from_pixel);
                bytes_to_write ++;
            }
        };
        
        auto decode_dxt = [&width, &height, &input_format, &data, &input_data]() {
            int flags = squish::kSourceBGRA;
            
            switch(input_format) {
                case HEK::BitmapDataFormat::BITMAP_DATA_FORMAT_DXT1:
                    flags |= squish::kDxt1;
                    break;

                case HEK::BitmapDataFormat::BITMAP_DATA_FORMAT_DXT3:
                    flags |= squish::kDxt3;
                    break;

                case HEK::BitmapDataFormat::BITMAP_DATA_FORMAT_DXT5:
                    flags |= squish::kDxt5;
                    break;
                    
                default:
                    std::terminate();
            }
            
            squish::DecompressImage(reinterpret_cast<squish::u8 *>(data.data()), width, height, input_data, flags);
            
            for(auto &color : data) {
                std::swap(color.red, color.blue);
            }
        };
        
        switch(input_format) {
            case HEK::BitmapDataFormat::BITMAP_DATA_FORMAT_DXT1:
            case HEK::BitmapDataFormat::BITMAP_DATA_FORMAT_DXT3:
            case HEK::BitmapDataFormat::BITMAP_DATA_FORMAT_DXT5:
                decode_dxt();
                break;
                
            case HEK::BitmapDataFormat::BITMAP_DATA_FORMAT_A8R8G8B8:
                std::memcpy(reinterpret_cast<std::byte *>(data.data()), input_data, data.size() * sizeof(data[0]));
                break;
            
            case HEK::BitmapDataFormat::BITMAP_DATA_FORMAT_X8R8G8B8:
                std::memcpy(reinterpret_cast<std::byte *>(data.data()), input_data, data.size() * sizeof(data[0]));
                for(std::size_t i = 0; i < pixel_count; i++) {
                    data[i].alpha = 0xFF;
                }
                break;

            // 16-bit color
            case HEK::BitmapDataFormat::BITMAP_DATA_FORMAT_A1R5G5B5:
                decode_16_bit(Pixel::convert_from_16_bit<1,5,5,5>);
                break;
            case HEK::BitmapDataFormat::BITMAP_DATA_FORMAT_R5G6B5:
                decode_16_bit(Pixel::convert_from_16_bit<0,5,6,5>);
                break;
            case HEK::BitmapDataFormat::BITMAP_DATA_FORMAT_A4R4G4B4:
                decode_16_bit(Pixel::convert_from_16_bit<4,4,4,4>);
                break;

            // Monochrome
            case HEK::BitmapDataFormat::BITMAP_DATA_FORMAT_A8Y8:
                decode_16_bit(Pixel::convert_from_a8y8);
                break;
            case HEK::BitmapDataFormat::BITMAP_DATA_FORMAT_A8:
                decode_8_bit(Pixel::convert_from_a8);
                break;
            case HEK::BitmapDataFormat::BITMAP_DATA_FORMAT_Y8:
                decode_8_bit(Pixel::convert_from_y8);
                break;
            case HEK::BitmapDataFormat::BITMAP_DATA_FORMAT_AY8:
                decode_8_bit(Pixel::convert_from_ay8);
                break;

            // p8
            case HEK::BitmapDataFormat::BITMAP_DATA_FORMAT_P8_BUMP:
                decode_8_bit(Pixel::convert_from_p8);
                break;
                
            default:
                throw std::exception();
        }
        
        return data;
    }
    
    static HEK::BitmapDataFormat most_efficient_format(const std::byte *input_data, std::size_t pixel_count, HEK::BitmapFormat category) noexcept {
        // No need to check anything here
        if(category == HEK::BitmapFormat::BITMAP_FORMAT_DXT1) {
            return HEK::BitmapDataFormat::BITMAP_DATA_FORMAT_DXT1;
        }
        
        enum AlphaPresent {
            ALPHA_PRESENT_NONE = 0,
            ALPHA_PRESENT_ONE_BIT = 1,
            ALPHA_PRESENT_MULTI_BIT = 2
        } alpha_present = AlphaPresent::ALPHA_PRESENT_NONE;
        
        bool all_black = true;
        bool luminosity_equals_alpha = true;
        
        // Analyze each pixel
        auto *starting_pixel = reinterpret_cast<const Pixel *>(input_data);
        for(std::size_t i = 0; i < pixel_count; i++, starting_pixel++) {
            if(starting_pixel->alpha == 0x00 && alpha_present == AlphaPresent::ALPHA_PRESENT_NONE) {
                alpha_present = AlphaPresent::ALPHA_PRESENT_ONE_BIT;
            }
            else if(starting_pixel->alpha != 0x00 && starting_pixel->alpha != 0xFF) {
                alpha_present = AlphaPresent::ALPHA_PRESENT_MULTI_BIT;
            }
            
            if(starting_pixel->convert_to_y8() != starting_pixel->alpha) {
                luminosity_equals_alpha = false;
            }
            
            if(starting_pixel->red != 0 || starting_pixel->green != 0 || starting_pixel->blue != 0) {
                all_black = false;
            }
        }
        
        switch(category) {
            case HEK::BitmapFormat::BITMAP_FORMAT_DXT3:
                return alpha_present ? HEK::BitmapDataFormat::BITMAP_DATA_FORMAT_DXT3 : HEK::BitmapDataFormat::BITMAP_DATA_FORMAT_DXT1;
                
            case HEK::BitmapFormat::BITMAP_FORMAT_DXT5:
                return alpha_present ? HEK::BitmapDataFormat::BITMAP_DATA_FORMAT_DXT5 : HEK::BitmapDataFormat::BITMAP_DATA_FORMAT_DXT1;
                
            case HEK::BitmapFormat::BITMAP_FORMAT_16_BIT:
                return alpha_present ? (
                        alpha_present == AlphaPresent::ALPHA_PRESENT_MULTI_BIT ? HEK::BitmapDataFormat::BITMAP_DATA_FORMAT_A4R4G4B4 : HEK::BitmapDataFormat::BITMAP_DATA_FORMAT_A1R5G5B5
                    ) : HEK::BitmapDataFormat::BITMAP_DATA_FORMAT_R5G6B5;
                    
            case HEK::BitmapFormat::BITMAP_FORMAT_32_BIT:
                return alpha_present ? HEK::BitmapDataFormat::BITMAP_DATA_FORMAT_A8R8G8B8 : HEK::BitmapDataFormat::BITMAP_DATA_FORMAT_X8R8G8B8;
                
            case HEK::BitmapFormat::BITMAP_FORMAT_MONOCHROME:
                if(alpha_present == AlphaPresent::ALPHA_PRESENT_NONE) {
                    return HEK::BitmapDataFormat::BITMAP_DATA_FORMAT_Y8;
                }
                else if(all_black) {
                    return HEK::BitmapDataFormat::BITMAP_DATA_FORMAT_A8;
                }
                else if(luminosity_equals_alpha) {
                    return HEK::BitmapDataFormat::BITMAP_DATA_FORMAT_AY8;
                }
                else {
                    return HEK::BitmapDataFormat::BITMAP_DATA_FORMAT_A8Y8;
                }
                
            case HEK::BitmapFormat::BITMAP_FORMAT_ENUM_COUNT:
            case HEK::BitmapFormat::BITMAP_FORMAT_DXT1:
                std::terminate(); // we just checked
        }
        
        std::terminate(); // this shouldn't be reached
    }
    
    HEK::BitmapDataFormat most_efficient_format(const std::byte *input_data, std::size_t width, std::size_t height, HEK::BitmapFormat category) noexcept {
        return most_efficient_format(input_data, width * height, category);
    }
    
    HEK::BitmapDataFormat most_efficient_format(const std::byte *input_data, std::size_t width, std::size_t height, std::size_t depth, HEK::BitmapFormat category, HEK::BitmapDataType type, std::size_t mipmap_count) noexcept {
        return most_efficient_format(input_data, size_of_bitmap(width, height, depth, mipmap_count, HEK::BitmapDataFormat::BITMAP_DATA_FORMAT_A8R8G8B8, type) / sizeof(Pixel), category);
    }
}
