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
        static GeneratedBitmapData scan_color_plate(
            const Pixel *pixels,
            std::uint32_t width,
            std::uint32_t height,
            BitmapType type,
            BitmapUsage usage,
            float bump_height,
            std::optional<ColorPlateScannerSpriteParameters> &sprite_parameters,
            std::int16_t mipmaps,
            HEK::InvaderBitmapMipmapScaling mipmap_type,
            std::optional<float> mipmap_fade_factor,
            std::optional<float> sharpen,
            std::optional<float> blur
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
         * @param usage              bitmap usage value
         */
        static void generate_mipmaps(GeneratedBitmapData &generated_bitmap, std::int16_t mipmaps, HEK::InvaderBitmapMipmapScaling mipmap_type, std::optional<float> mipmap_fade_factor, const std::optional<ColorPlateScannerSpriteParameters> &sprite_parameters, std::optional<float> sharpen, std::optional<float> blur, BitmapUsage usage);

        /**
         * Consolidate the stacked bitmap data (cubemaps and 3d textures)
         * @param generated_bitmap bitmap data to do cubemap stuff with
         */
        static void consolidate_stacked_bitmaps(GeneratedBitmapData &generated_bitmap);

        /**
         * Merge the mipmaps for 3D textures for depth
         */
        static void merge_3d_texture_mipmaps(GeneratedBitmapData &generated_bitmap);

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
