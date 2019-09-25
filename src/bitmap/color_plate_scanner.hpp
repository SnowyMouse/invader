/*
 * Invader (c) 2019 Kavawuvi
 *
 * This program is free software under the GNU General Public License v3.0. See LICENSE for more information.
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
    using BitmapUsage = HEK::BitmapUsage;

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
        inline bool operator==(const ColorPlatePixel &other) const {
            return this->blue == other.blue && this->green == other.green && this->red == other.red && this->alpha == other.alpha;
        }

        /**
         * Compare with another color and return if the color is not the same
         * @param  source color to compare with
         * @return        true if the colors are not equal
         */
        inline bool operator!=(const ColorPlatePixel &other) const {
            return !(*this == other);
        }

        /**
         * Return the source color
         * @param  source source color
         * @return        source color
         */
        inline ColorPlatePixel replace(const ColorPlatePixel &source) const {
            return source;
        };

        /**
         * Alpha-blend the color with another color
         * @param  source color to alpha blend with
         * @return        alpha-blended color
         */
        ColorPlatePixel alpha_blend(const ColorPlatePixel &source) const;

        /**
         * Convert the color to 8-bit grayscale
         * @return 8-bit grayscale representation of the color
         */
        std::uint8_t convert_to_y8();

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
        static inline ColorPlatePixel p8_to_color(std::uint8_t p8) {
            extern void p8_convert_to_rgba(std::uint8_t p8, std::uint8_t &red, std::uint8_t &green, std::uint8_t &blue, std::uint8_t &alpha);
            ColorPlatePixel color;
            p8_convert_to_rgba(p8, color.red, color.green, color.blue, color.alpha);
            return color;
        }

        /**
         * Conver the color to alpha
         * @return alpha of the color
         */
        inline std::uint8_t convert_to_a8() {
            return this->alpha;
        }

        /**
         * Convert the color to grayscale with alpha.
         * @return A8Y8 representation of the color
         */
        inline std::uint16_t convert_to_a8y8() {
            return (this->alpha << 8) | this->convert_to_y8();
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
         * Convert the color to a 16-bit integer.
         * @return 16-bit representation of the color
         */
        template<std::uint8_t alpha, std::uint8_t red, std::uint8_t green, std::uint8_t blue>
        static ColorPlatePixel convert_from_16_bit(std::uint16_t color) {
            static_assert(alpha + red + green + blue == 16, "alpha + red + green + blue must equal 16");

            ColorPlatePixel color_output;

            int shift_amount = 0;

            #define COMPOSITE_BITMAP_COLOR_GET_CHANNEL_VALUE_FOR_COLOR(channel) if(channel) { \
                color_output.channel = static_cast<std::uint16_t>(UINT8_MAX * ((color >> shift_amount) & ((1 << channel) - 1))) / ((1 << channel) - 1); \
                shift_amount += channel; \
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

    struct GeneratedBitmapDataBitmapMipmap {
        std::uint32_t first_pixel;
        std::uint32_t pixel_count;
        std::uint32_t mipmap_width;
        std::uint32_t mipmap_height;
    };

    struct GeneratedBitmapDataBitmap {
        std::uint32_t height;
        std::uint32_t width;
        std::uint32_t color_plate_x;
        std::uint32_t color_plate_y;
        std::int32_t registration_point_x;
        std::int32_t registration_point_y;
        std::uint32_t depth = 1;
        std::vector<ColorPlatePixel> pixels;
        std::vector<GeneratedBitmapDataBitmapMipmap> mipmaps;
    };

    struct GeneratedBitmapDataSprite {
        std::uint32_t top;
        std::uint32_t left;
        std::uint32_t right;
        std::uint32_t bottom;
        std::uint32_t bitmap_index;
        std::int32_t registration_point_x;
        std::int32_t registration_point_y;
        std::uint32_t original_bitmap_index;
    };

    struct GeneratedBitmapDataSequence {
        std::uint32_t y_start;
        std::uint32_t y_end;
        std::uint32_t first_bitmap;
        std::uint32_t bitmap_count;
        std::vector<GeneratedBitmapDataSprite> sprites;
    };

    struct GeneratedBitmapData {
        BitmapType type;
        std::vector<GeneratedBitmapDataBitmap> bitmaps;
        std::vector<GeneratedBitmapDataSequence> sequences;
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
         * @param  usage              usage value for bitmap
         * @param  bump_height        bump height value
         * @param  sprite_parameters  sprite parameters for sprite generation (only necessary if making sprites)
         * @param  mipmaps            maximum number of mipmaps
         * @param  mipmap_type        type of mipmaps
         * @param  mipmap_fade_factor fade-to-gray factor for mipmaps
         * @param  sharpen            sharpening filter
         * @param  blur               blur filter
         * @return                    scanned color plate data
         */
        static GeneratedBitmapData scan_color_plate(const ColorPlatePixel *pixels, std::uint32_t width, std::uint32_t height, BitmapType type, BitmapUsage usage, float bump_height, std::optional<ColorPlateScannerSpriteParameters> &sprite_parameters, std::int16_t mipmaps, ScannedColorMipmapType mipmap_type, std::optional<float> mipmap_fade_factor, std::optional<float> sharpen, std::optional<float> blur);

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
        std::optional<ColorPlatePixel> cyan;

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
         * Read the color plate data bitmap data
         * @param generated_bitmap bitmap data to write to (output)
         * @param pixels           pixel input
         * @param width            width of input
         */
        void read_color_plate(GeneratedBitmapData &generated_bitmap, const ColorPlatePixel *pixels, std::uint32_t width) const;

        /**
         * Read an unrolled cubemap
         * @param generated_bitmap bitmap data to write to (output)
         * @param pixels           pixel input
         * @param width            width of input
         * @param height           height of input
         */
        void read_unrolled_cubemap(GeneratedBitmapData &generated_bitmap, const ColorPlatePixel *pixels, std::uint32_t width, std::uint32_t height) const;

        /**
         * Read a single bitmap.
         * @param generated_bitmap bitmap data to write to (output)
         * @param pixels           pixel input
         * @param width            width of input
         * @param height           height of input
         */
        void read_single_bitmap(GeneratedBitmapData &generated_bitmap, const ColorPlatePixel *pixels, std::uint32_t width, std::uint32_t height) const;

        /**
         * Process height maps for the bitmap
         * @param generated_bitmap bitmap data to write to (output)
         * @param bump_height      bump height value
         */
        static void process_height_maps(GeneratedBitmapData &generated_bitmap, float bump_height);

        /**
         * Generate mipmaps for the color plate
         * @param generated_bitmap   color plate to generate mipmaps for
         * @param mipmaps            max number of mipmaps
         * @param mipmap_type        scaling filter to use for mipmaps
         * @param mipmap_fade_factor fade-to-gray factor for mipmaps
         * @param sprite_parameters  sprite parameters (if using sprites)
         * @param sharpen            sharpen filter
         */
        static void generate_mipmaps(GeneratedBitmapData &generated_bitmap, std::int16_t mipmaps, ScannedColorMipmapType mipmap_type, std::optional<float> mipmap_fade_factor, const std::optional<ColorPlateScannerSpriteParameters> &sprite_parameters, std::optional<float> sharpen, std::optional<float> blur);

        /**
         * Consolidate the stacked bitmap data (cubemaps and 3d textures)
         * @param generated_bitmap bitmap data to do cubemap stuff with
         */
        static void consolidate_stacked_bitmaps(GeneratedBitmapData &generated_bitmap);

        /**
         * Process sprites
         * @param generated_bitmap bitmap data to do sprite stuff with
         * @param parameters       sprite parameters
         * @param mipmaps          mipmap count
         */
        static void process_sprites(GeneratedBitmapData &generated_bitmap, ColorPlateScannerSpriteParameters &parameters, std::int16_t &mipmaps);

        ColorPlateScanner() = default;
    };
}
#endif
