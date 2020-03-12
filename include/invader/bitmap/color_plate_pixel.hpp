// SPDX-License-Identifier: GPL-3.0-only

#ifndef INVADER__BITMAP__COLOR_PLATE_PIXEL_HPP
#define INVADER__BITMAP__COLOR_PLATE_PIXEL_HPP

#include <cstdint>

namespace Invader {
    struct ColorPlatePixel {
        std::uint8_t blue;
        std::uint8_t green;
        std::uint8_t red;
        std::uint8_t alpha;

        /**
         * Compare with another color and return if the color is the same
         * @param  source color to compare with
         * @return        true if the colors are equal
         */
        bool operator==(const ColorPlatePixel &other) const {
            return this->blue == other.blue && this->green == other.green && this->red == other.red && this->alpha == other.alpha;
        }

        /**
         * Compare with another color and return if the color is not the same
         * @param  source color to compare with
         * @return        true if the colors are not equal
         */
        bool operator!=(const ColorPlatePixel &other) const {
            return !(*this == other);
        }

        /**
         * Return the source color
         * @param  source source color
         * @return        source color
         */
        ColorPlatePixel replace(const ColorPlatePixel &source) const {
            return source;
        };

        /**
         * Alpha-blend the color with another color
         * @param  source color to alpha blend with
         * @return        alpha-blended color
         */
        ColorPlatePixel alpha_blend(const ColorPlatePixel &source) const {
            ColorPlatePixel output;

            if(source.alpha == 0.0F) {
                return *this;
            }

            float destination_alpha_float = this->alpha / 255.0F;
            float source_alpha_float = source.alpha / 255.0F;

            float blend = destination_alpha_float * (1.0F - source_alpha_float);
            float output_alpha_float = source_alpha_float + blend;

            output.alpha = static_cast<std::uint8_t>(output_alpha_float * UINT8_MAX);

            #define ALPHA_BLEND_CHANNEL(channel) output.channel = static_cast<std::uint8_t>(((source.channel * source_alpha_float) + (this->channel * blend)) / output_alpha_float);

            ALPHA_BLEND_CHANNEL(red);
            ALPHA_BLEND_CHANNEL(green);
            ALPHA_BLEND_CHANNEL(blue);

            #undef ALPHA_BLEND_CHANNEL

            return output;
        }

        /**
         * Convert the color to 8-bit grayscale
         * @return 8-bit grayscale representation of the color
         */
        std::uint8_t convert_to_y8() {
            // Based on Luma
            static const std::uint8_t RED_WEIGHT = 54;
            static const std::uint8_t GREEN_WEIGHT = 182;
            static const std::uint8_t BLUE_WEIGHT = 19;
            static_assert(RED_WEIGHT + GREEN_WEIGHT + BLUE_WEIGHT == UINT8_MAX, "red + green + blue weights (grayscale) must equal 255");

            std::uint8_t grayscale_output = 0;
            #define COMPOSITE_BITMAP_GRAYSCALE_SET_CHANNEL_VALUE_FOR_COLOR(channel, weight) \
                grayscale_output += (this->channel * weight + (UINT8_MAX + 1) / 2) / UINT8_MAX;

            COMPOSITE_BITMAP_GRAYSCALE_SET_CHANNEL_VALUE_FOR_COLOR(red, RED_WEIGHT)
            COMPOSITE_BITMAP_GRAYSCALE_SET_CHANNEL_VALUE_FOR_COLOR(green, GREEN_WEIGHT)
            COMPOSITE_BITMAP_GRAYSCALE_SET_CHANNEL_VALUE_FOR_COLOR(blue, BLUE_WEIGHT)

            #undef COMPOSITE_BITMAP_GRAYSCALE_SET_CHANNEL_VALUE_FOR_COLOR

            return grayscale_output;
        }

        /**
         * Convert the color to p8
         * @return p8 index
         */
        std::uint8_t convert_to_p8() {
            extern std::uint8_t rg_convert_to_p8(std::uint8_t red, std::uint8_t green, std::uint8_t blue, std::uint8_t alpha);
            return rg_convert_to_p8(this->red, this->green, this->blue, this->alpha);
        }

        /**
         * Convert p8 to color
         * @param  p8 p8 index
         * @return    color plate pixel
         */
        static ColorPlatePixel convert_from_p8(std::uint8_t p8) {
            extern void p8_convert_to_rgba(std::uint8_t p8, std::uint8_t &red, std::uint8_t &green, std::uint8_t &blue, std::uint8_t &alpha);
            ColorPlatePixel color;
            p8_convert_to_rgba(p8, color.red, color.green, color.blue, color.alpha);
            return color;
        }

        /**
         * Convert the color from alpha and luminosity
         * @param  color alpha and luminosity value
         * @return       color
         */
        static ColorPlatePixel convert_from_ay8(std::uint8_t color) {
            return ColorPlatePixel { color, color, color, color };
        }

        /**
         * Convert the color from luminosity
         * @param  color luminosity value
         * @return       color
         */
        static ColorPlatePixel convert_from_y8(std::uint8_t color) {
            return ColorPlatePixel { 0xFF, color, color, color };
        }

        /**
         * Conver the color to alpha
         * @return alpha of the color
         */
        std::uint8_t convert_to_a8() {
            return this->alpha;
        }

        /**
         * Convert the color from alpha
         * @param  color alpha value
         * @return       color
         */
        static ColorPlatePixel convert_from_a8(std::uint8_t color) {
            return ColorPlatePixel { color, 0xFF, 0xFF, 0xFF };
        }

        /**
         * Convert the color to grayscale with alpha.
         * @return A8Y8 representation of the color
         */
        std::uint16_t convert_to_a8y8() {
            return (this->alpha << 8) | this->convert_to_y8();
        }

        /**
         * Convert from A8Y8
         * @param  color A8Y8 color pixel
         * @return       color
         */
        static ColorPlatePixel convert_from_a8y8(std::uint16_t color) {
            auto luminosity = static_cast<std::uint8_t>(color);
            return ColorPlatePixel { static_cast<std::uint8_t>(color >> 8), luminosity, luminosity, luminosity };
        }

        /**
         * Convert the color to a 16-bit integer.
         * @return 16-bit representation of the color
         */
        template<std::uint8_t alpha, std::uint8_t red, std::uint8_t green, std::uint8_t blue>
        std::uint16_t convert_to_16_bit() {
            static_assert(alpha + red + green + blue == 16, "alpha + red + green + blue must equal 16");

            std::uint16_t color_output = 0;

            #define COMPOSITE_BITMAP_COLOR_SET_CHANNEL_VALUE_FOR_COLOR(channel) if(channel) { \
                color_output <<= channel; \
                color_output |= (static_cast<std::uint16_t>(this->channel) * ((1 << (channel)) - 1) + (UINT8_MAX + 1) / 2) / UINT8_MAX; \
            }

            COMPOSITE_BITMAP_COLOR_SET_CHANNEL_VALUE_FOR_COLOR(alpha);
            COMPOSITE_BITMAP_COLOR_SET_CHANNEL_VALUE_FOR_COLOR(red);
            COMPOSITE_BITMAP_COLOR_SET_CHANNEL_VALUE_FOR_COLOR(green);
            COMPOSITE_BITMAP_COLOR_SET_CHANNEL_VALUE_FOR_COLOR(blue);

            #undef COMPOSITE_BITMAP_COLOR_SET_CHANNEL_VALUE_FOR_COLOR

            return color_output;
        }

        /**
         * Convert the color from a 16-bit integer.
         * @param color 16-bit color pixel
         * @return      color
         */
        template<std::uint8_t alpha, std::uint8_t red, std::uint8_t green, std::uint8_t blue>
        static ColorPlatePixel convert_from_16_bit(std::uint16_t color) {
            static_assert(alpha + red + green + blue == 16, "alpha + red + green + blue must equal 16");

            ColorPlatePixel color_output;

            int shift_amount = 0;

            #define COMPOSITE_BITMAP_COLOR_GET_CHANNEL_VALUE_FOR_COLOR(channel) if(channel) { \
                color_output.channel = static_cast<std::uint16_t>(UINT8_MAX * ((color >> shift_amount) & ((1 << channel) - 1))) / ((1 << channel) - 1); \
                shift_amount += channel; \
            } \
            else { \
                color_output.channel = 0xFF; \
            }

            COMPOSITE_BITMAP_COLOR_GET_CHANNEL_VALUE_FOR_COLOR(blue);
            COMPOSITE_BITMAP_COLOR_GET_CHANNEL_VALUE_FOR_COLOR(green);
            COMPOSITE_BITMAP_COLOR_GET_CHANNEL_VALUE_FOR_COLOR(red);
            COMPOSITE_BITMAP_COLOR_GET_CHANNEL_VALUE_FOR_COLOR(alpha);

            #undef COMPOSITE_BITMAP_COLOR_GET_CHANNEL_VALUE_FOR_COLOR

            return color_output;
        }
    };
    static_assert(sizeof(ColorPlatePixel) == 4);
}

#endif
