/*
 * Invader (c) 2018 Kavawuvi
 *
 * This program is free software under the GNU General Public License v3.0 or later. See LICENSE for more information.
 */

#ifndef INVADER__BITMAP__COLOR_PLATE_SCANNER_HPP
#define INVADER__BITMAP__COLOR_PLATE_SCANNER_HPP

#include <cstdint>
#include <vector>
#include <optional>
#include "../tag/hek/class/bitmap.hpp"

namespace Invader {
    using BitmapType = HEK::BitmapType;
    using BitmapSpriteUsage = HEK::BitmapSpriteUsage;

    struct ColorPlatePixel {
        std::uint8_t blue;
        std::uint8_t green;
        std::uint8_t red;
        std::uint8_t alpha;

        bool operator==(const ColorPlatePixel &other) const {
            return this->blue == other.blue && this->green == other.green && this->red == other.red && this->alpha == other.alpha;
        }

        bool operator!=(const ColorPlatePixel &other) const {
            return !(*this == other);
        }

        template<std::uint8_t alpha, std::uint8_t red, std::uint8_t green, std::uint8_t blue>
        static std::uint16_t convert_to_16_bit(const ColorPlatePixel *color) {
            static_assert(alpha + red + green + blue == 16, "alpha + red + green + blue must equal 16");

            std::uint16_t color_output = 0;

            #define COMPOSITE_BITMAP_COLOR_SET_CHANNEL_VALUE_FOR_COLOR(channel) if(channel) { \
                color_output <<= channel; \
                color_output |= (static_cast<std::uint16_t>(color->channel) * ((1 << (channel)) - 1) + (UINT8_MAX + 1) / 2) / UINT8_MAX; \
            }

            COMPOSITE_BITMAP_COLOR_SET_CHANNEL_VALUE_FOR_COLOR(alpha);
            COMPOSITE_BITMAP_COLOR_SET_CHANNEL_VALUE_FOR_COLOR(red);
            COMPOSITE_BITMAP_COLOR_SET_CHANNEL_VALUE_FOR_COLOR(green);
            COMPOSITE_BITMAP_COLOR_SET_CHANNEL_VALUE_FOR_COLOR(blue);

            #undef COMPOSITE_BITMAP_COLOR_SET_CHANNEL_VALUE_FOR_COLOR

            return color_output;
        }

        static std::uint8_t convert_to_y8(const ColorPlatePixel *color) {
            // Based on Luma
            static const std::uint8_t RED_WEIGHT = 54;
            static const std::uint8_t GREEN_WEIGHT = 182;
            static const std::uint8_t BLUE_WEIGHT = 19;
            static_assert(RED_WEIGHT + GREEN_WEIGHT + BLUE_WEIGHT == UINT8_MAX, "red + green + blue weights (grayscale) must equal 255");

            std::uint8_t grayscale_output = 0;
            #define COMPOSITE_BITMAP_GRAYSCALE_SET_CHANNEL_VALUE_FOR_COLOR(channel, weight) \
                grayscale_output += (color->channel * weight + (UINT8_MAX + 1) / 2) / UINT8_MAX;

            COMPOSITE_BITMAP_GRAYSCALE_SET_CHANNEL_VALUE_FOR_COLOR(red, RED_WEIGHT)
            COMPOSITE_BITMAP_GRAYSCALE_SET_CHANNEL_VALUE_FOR_COLOR(green, GREEN_WEIGHT)
            COMPOSITE_BITMAP_GRAYSCALE_SET_CHANNEL_VALUE_FOR_COLOR(blue, BLUE_WEIGHT)

            #undef COMPOSITE_BITMAP_GRAYSCALE_SET_CHANNEL_VALUE_FOR_COLOR

            return grayscale_output;
        }

        static std::uint8_t convert_to_a8(const ColorPlatePixel *color) {
            return color->alpha;
        }

        static std::uint16_t convert_to_a8y8(const ColorPlatePixel *color) {
            return (color->alpha << 8) | convert_to_y8(color);
        }
    };
    static_assert(sizeof(ColorPlatePixel) == 4);

    struct ScannedColorPlateBitmapMipmap {
        std::uint32_t first_pixel;
        std::uint32_t pixel_count;
        std::uint32_t mipmap_width;
        std::uint32_t mipmap_height;
    };

    struct ScannedColorPlateBitmap {
        std::uint32_t height;
        std::uint32_t width;
        std::uint32_t color_plate_x;
        std::uint32_t color_plate_y;
        std::uint32_t registration_point_x;
        std::uint32_t registration_point_y;
        std::vector<ColorPlatePixel> pixels;
        std::vector<ScannedColorPlateBitmapMipmap> mipmaps;
    };

    struct ScannedColorPlateSprite {
        std::uint32_t top;
        std::uint32_t left;
        std::uint32_t right;
        std::uint32_t bottom;
        std::uint32_t bitmap_index;
        std::uint32_t registration_point_x;
        std::uint32_t registration_point_y;
    };

    struct ScannedColorPlateSequence {
        std::uint32_t y_start;
        std::uint32_t y_end;
        std::uint32_t first_bitmap;
        std::uint32_t bitmap_count;
        std::vector<ScannedColorPlateSprite> sprites;
    };

    struct ScannedColorPlate {
        BitmapType type;
        std::vector<ScannedColorPlateBitmap> bitmaps;
        std::vector<ScannedColorPlateSequence> sequences;
    };

    enum ScannedColorMipmapType {
        SCANNED_COLOR_MIPMAP_LINEAR,
        SCANNED_COLOR_MIPMAP_NEAREST_ALPHA,
        SCANNED_COLOR_MIPMAP_NEAREST_ALPHA_COLOR
    };

    struct ColorPlateScannerSpriteParameters {
        BitmapSpriteUsage sprite_usage;
        std::uint32_t sprite_budget;
        std::uint32_t sprite_budget_count;
        std::uint32_t sprite_spacing;
    };

    class ColorPlateScanner {
    public:
        /**
         * Scan the color plate for bitmaps
         * @param  pixels             pointer to first pixel
         * @param  width              width of color plate
         * @param  height             height of color plate
         * @param  type               type of bitmap
         * @param  mipmaps            maximum number of mipmaps
         * @param  mipmap_type        type of mipmaps
         * @param  mipmap_fade_factor fade-to-gray factor for mipmaps
         * @param  sprite_parameters  sprite parameters for sprite generation (only necessary if making sprites)
         * @return                    scanned color plate data
         */
        static ScannedColorPlate scan_color_plate(const ColorPlatePixel *pixels, std::uint32_t width, std::uint32_t height, BitmapType type, const std::optional<ColorPlateScannerSpriteParameters> &sprite_parameters = std::nullopt, std::int16_t mipmaps = INT16_MAX, ScannedColorMipmapType mipmap_type = ScannedColorMipmapType::SCANNED_COLOR_MIPMAP_LINEAR, float mipmap_fade_factor = 0.0F);

    private:
        /** Was valid color plate data used? If so, we need to check for multiple sequences. */
        bool valid_color_plate = false;

        /** Is power of two required */
        bool power_of_two = true;

        /** Blue color */
        ColorPlatePixel blue = { 0xFF, 0x00, 0x00, 0xFF };

        /** Magenta color */
        ColorPlatePixel magenta = { 0xFF, 0x00, 0xFF, 0xFF };

        /** Cyan color */
        ColorPlatePixel cyan = { 0xFF, 0xFF, 0x00, 0xFF };

        /**
         * Check if the color is blue
         * @param  color color to check
         * @return       true if blue
         */
        bool is_blue(const ColorPlatePixel &color) const;

        /**
         * Check if the color is magenta
         * @param  color color to check
         * @return       true if magenta
         */
        bool is_magenta(const ColorPlatePixel &color) const;

        /**
         * Check if the color is cyan
         * @param  color color to check
         * @return       true if cyan
         */
        bool is_cyan(const ColorPlatePixel &color) const;

        /**
         * Check if the color is ignored
         * @param  color color to check
         * @return       true if ignored
         */
        bool is_ignored(const ColorPlatePixel &color) const;

        /**
         * Read the color plate-valid bitmap data
         * @param color_plate color plate data to write to (output)
         * @param pixels      pixel input
         * @param width       width of input
         */
        void read_color_plate(ScannedColorPlate &color_plate, const ColorPlatePixel *pixels, std::uint32_t width) const;

        /**
         * Read an unrolled cubemap
         * @param color_plate color plate data to write to (output)
         * @param pixels      pixel input
         * @param width       width of input
         * @param height      height of input
         */
        void read_unrolled_cubemap(ScannedColorPlate &color_plate, const ColorPlatePixel *pixels, std::uint32_t width, std::uint32_t height) const;

        /**
         * Read bitmap data that doesn't have a valid color plate
         * @param color_plate color plate data to write to (output)
         * @param pixels      pixel input
         * @param width       width of input
         * @param height      height of input
         */
        void read_non_color_plate(ScannedColorPlate &color_plate, const ColorPlatePixel *pixels, std::uint32_t width, std::uint32_t height) const;

        /**
         * Generate mipmaps for the color plate
         * @param color_plate color plate to generate mipmaps for
         */
        static void generate_mipmaps(ScannedColorPlate &color_plate, std::int16_t mipmaps, ScannedColorMipmapType mipmap_type, float mipmap_fade_factor);

        /**
         * Consolidate the cubemap data
         * @param color_plate color plate data to do cubemap stuff with
         */
        static void consolidate_cubemaps(ScannedColorPlate &color_plate);

        /**
         * Process sprites
         * @param color_plate color plate data to do sprite stuff with
         * @param parameters  sprite parameters
         */
        static void process_sprites(ScannedColorPlate &color_plate, const ColorPlateScannerSpriteParameters &parameters);

        ColorPlateScanner() = default;
    };
}
#endif
