// SPDX-License-Identifier: GPL-3.0-only

#ifndef INVADER__BITMAP__BITMAP_PROCESSOR_HPP
#define INVADER__BITMAP__BITMAP_PROCESSOR_HPP

#include "color_plate_scanner.hpp"

namespace Invader {
    struct BitmapProcessorSpriteParameters {
        BitmapSpriteUsage sprite_usage;
        std::uint32_t sprite_budget;
        std::uint32_t sprite_budget_count;
        std::uint32_t sprite_spacing;
        bool force_square_sprite_sheets;
    };
    
    class BitmapProcessor {
    public:
        /**
         * Process the bitmap data
         * @param  generated_bitmap   generated bitmap data from the color plate scanner
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
        static void process_bitmap_data(
            GeneratedBitmapData &generated_bitmap,
            BitmapType type,
            BitmapUsage usage,
            float bump_height,
            std::optional<BitmapProcessorSpriteParameters> &sprite_parameters,
            std::int16_t mipmaps,
            BitmapMipmapScaleType mipmap_type,
            std::optional<float> mipmap_fade_factor,
            std::optional<float> sharpen,
            std::optional<float> blur
        );
        
    private:
        bool power_of_two;
        
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
        static void generate_mipmaps(GeneratedBitmapData &generated_bitmap, std::int16_t mipmaps, BitmapMipmapScaleType mipmap_type, std::optional<float> mipmap_fade_factor, const std::optional<BitmapProcessorSpriteParameters> &sprite_parameters, std::optional<float> sharpen, std::optional<float> blur, BitmapUsage usage);

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
        static void process_sprites(GeneratedBitmapData &generated_bitmap, BitmapProcessorSpriteParameters &parameters, std::int16_t &mipmaps);
    };
};

#endif
