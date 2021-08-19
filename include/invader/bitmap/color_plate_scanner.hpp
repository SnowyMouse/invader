// SPDX-License-Identifier: GPL-3.0-only

#ifndef INVADER__BITMAP__COLOR_PLATE_SCANNER_HPP
#define INVADER__BITMAP__COLOR_PLATE_SCANNER_HPP

#include <cstdint>
#include <vector>
#include <optional>
#include <invader/tag/hek/definition.hpp>
#include <invader/bitmap/pixel.hpp>

namespace Invader {
    using BitmapType = HEK::BitmapType;
    using BitmapSpriteUsage = HEK::BitmapSpriteUsage;
    using BitmapUsage = HEK::BitmapUsage;
    
    enum BitmapMipmapScaleType {
        BITMAP_MIPMAP_SCALE_TYPE_LINEAR,
        BITMAP_MIPMAP_SCALE_TYPE_NEAREST_ALPHA,
        BITMAP_MIPMAP_SCALE_TYPE_NEAREST
    };

    struct GeneratedBitmapDataBitmapMipmap {
        std::uint32_t first_pixel;
        std::uint32_t pixel_count;
        std::uint32_t mipmap_width;
        std::uint32_t mipmap_height;
        std::uint32_t mipmap_depth = 1;
    };

    struct GeneratedBitmapDataBitmap {
        std::uint32_t height;
        std::uint32_t width;
        std::uint32_t color_plate_x;
        std::uint32_t color_plate_y;
        std::int32_t registration_point_x;
        std::int32_t registration_point_y;
        std::uint32_t depth = 1;
        std::vector<Pixel> pixels;
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

    class ColorPlateScanner {
    public:
        /**
         * Scan the color plate for bitmaps
         * @param  pixels             pointer to first pixel
         * @param  width              width of color plate
         * @param  height             height of color plate
         * @param  type               type of bitmap
         * @param  usage              usage value for bitmap
         */
        static GeneratedBitmapData scan_color_plate(
            const Pixel *pixels,
            std::uint32_t width,
            std::uint32_t height,
            BitmapType type,
            BitmapUsage usage
        );

    private:
        /** Is power of two required */
        bool power_of_two = true;

        /** Transparency color */
        std::optional<Pixel> transparency_color;

        /** Sequence divider color */
        std::optional<Pixel> sequence_divider_color;

        /** Spacing color */
        std::optional<Pixel> spacing_color;

        /**
         * Check if the color is blue
         * @param  color color to check
         * @return       true if blue
         */
        bool is_transparency_color(const Pixel &color) const;

        /**
         * Check if the color is magenta
         * @param  color color to check
         * @return       true if magenta
         */
        bool is_sequence_divider_color(const Pixel &color) const;

        /**
         * Check if the color is cyan
         * @param  color color to check
         * @return       true if cyan
         */
        bool is_spacing_color(const Pixel &color) const;

        /**
         * Check if the color is ignored
         * @param  color color to check
         * @return       true if ignored
         */
        bool is_ignored(const Pixel &color) const;

        /**
         * Read the color plate data bitmap data
         * @param generated_bitmap bitmap data to write to (output)
         * @param pixels           pixel input
         * @param width            width of input
         */
        void read_color_plate(GeneratedBitmapData &generated_bitmap, const Pixel *pixels, std::uint32_t width) const;

        /**
         * Read an unrolled cubemap
         * @param generated_bitmap bitmap data to write to (output)
         * @param pixels           pixel input
         * @param width            width of input
         * @param height           height of input
         */
        void read_unrolled_cubemap(GeneratedBitmapData &generated_bitmap, const Pixel *pixels, std::uint32_t width, std::uint32_t height) const;

        /**
         * Read a single bitmap.
         * @param generated_bitmap bitmap data to write to (output)
         * @param pixels           pixel input
         * @param width            width of input
         * @param height           height of input
         */
        void read_single_bitmap(GeneratedBitmapData &generated_bitmap, const Pixel *pixels, std::uint32_t width, std::uint32_t height) const;

        ColorPlateScanner() = default;
    };
}
#endif
