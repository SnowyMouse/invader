// SPDX-License-Identifier: GPL-3.0-only

#include <invader/bitmap/bitmap_processor.hpp>

using namespace Invader::Parser;

namespace Invader {
    void BitmapProcessor::process_bitmap_data(
        GeneratedBitmapData &generated_bitmap,
        BitmapType type,
        BitmapUsage usage,
        float bump_height,
        std::optional<BitmapProcessorSpriteParameters> &sprite_parameters,
        std::int16_t mipmaps,
        BitmapMipmapScaleType mipmap_type,
        std::optional<float> mipmap_fade_factor,
        std::optional<float> sharpen,
        std::optional<float> blur) {
        
        BitmapProcessor processor;
        processor.power_of_two = (type != BitmapType::BITMAP_TYPE_SPRITES) && (type != BitmapType::BITMAP_TYPE_INTERFACE_BITMAPS);
        
        // If we have alpha blend, we have to crop the bitmaps
        if(usage == BitmapUsage::BITMAP_USAGE_ALPHA_BLEND) {
            std::vector<std::size_t> bitmaps_to_remove;
            auto bitmap_count = generated_bitmap.bitmaps.size();
            
            for(std::size_t b = 0; b < bitmap_count; b++) {
                auto &bitmap = generated_bitmap.bitmaps[b];
                
                std::size_t width = bitmap.width;
                std::size_t height = bitmap.height;
                
                std::size_t new_start_x;
                std::size_t new_end_x;
                std::size_t new_start_y;
                std::size_t new_end_y;
                
                // Remove from the left
                for(new_start_x = 0; new_start_x < width; new_start_x++) {
                    for(std::size_t y = 0; y < height; y++) {
                        auto &pixel = bitmap.pixels[new_start_x + y * width];
                        if(pixel.alpha != 0) {
                            goto top_check;
                        }
                    }
                }
                
                // Remove from the top
                top_check:
                for(new_start_y = 0; new_start_y < height; new_start_y++) {
                    for(std::size_t x = 0; x < width; x++) {
                        auto &pixel = bitmap.pixels[new_start_y * width + x];
                        if(pixel.alpha != 0) {
                            goto right_check;
                        }
                    }
                }
                
                // Remove from the right
                right_check:
                for(new_end_x = width; new_end_x > new_start_x; new_end_x--) {
                    for(std::size_t y = 0; y < height; y++) {
                        auto &pixel = bitmap.pixels[(new_end_x - 1) + y * width];
                        if(pixel.alpha != 0) {
                            goto bottom_check;
                        }
                    }
                }
                
                // Remove from the bottom
                bottom_check:
                for(new_end_y = height; new_end_y > new_start_y; new_end_y--) {
                    for(std::size_t x = 0; x < width; x++) {
                        auto &pixel = bitmap.pixels[(new_end_y - 1) * width + x];
                        if(pixel.alpha != 0) {
                            goto done_check;
                        }
                    }
                }
                
                // Check dimensions
                done_check:
                if(new_end_x != width || new_end_y != height || new_start_x != 0 || new_start_y != 0) {
                    if(new_end_x == new_start_x) {
                        eprintf_warn("Bitmap #%zu was deleted due to zero alpha (alpha blend usage)", b);
                        bitmaps_to_remove.emplace_back(b);
                    }
                    else {
                        std::size_t new_width = new_end_x - new_start_x;
                        std::size_t new_height = new_end_y - new_start_y;
                        eprintf_warn("Bitmap #%zu was resized to %zux%zu due to zero alpha on edge (alpha blend usage)", b, new_width, new_height);
                        
                        // Overwrite pixels
                        std::vector<Pixel> new_pixels(new_width * new_height);
                        for(std::size_t y = new_start_y; y < new_end_y; y++) {
                            const auto *first_pixel = bitmap.pixels.data() + y * width + new_start_x;
                            const auto *last_pixel = first_pixel + new_width;
                            std::copy(first_pixel, last_pixel, new_pixels.data() + (y - new_start_y) * new_width);
                        }
                        
                        bitmap.width = new_width;
                        bitmap.height = new_height;
                        bitmap.pixels = new_pixels;
                        
                        // Check if power-of-two
                        if(processor.power_of_two && (!is_power_of_two(new_width) || !is_power_of_two(new_height))) {
                            eprintf_error("This is non-power-of-two, but the bitmap type requires power-of-two");
                            throw InvalidInputBitmapException();
                        }
                    }
                }
            }
            
            // Remove any empty bitmaps
            auto bitmap_removal_count = bitmaps_to_remove.size();
            if(bitmap_removal_count) {
                eprintf_warn("%zu bitmaps were deleted due to no alpha (usage is set to alpha blend)", bitmap_removal_count);
                
                for(std::size_t i = 0; i < bitmap_removal_count; i++) {
                    // Get the index
                    auto bitmap_index = bitmaps_to_remove[i];
                    
                    // We're removing the bitmap, so decrement all indices after this
                    for(std::size_t j = i + 1; j < bitmap_removal_count; j++) {
                        bitmaps_to_remove[j]--;
                    }
                    
                    // Fix indices
                    for(auto &s : generated_bitmap.sequences) {
                        auto range_length = s.bitmap_count;
                        auto range_start = s.first_bitmap;
                        auto range_end = range_start + range_length;
                        
                        if(range_start > bitmap_index) {
                            s.first_bitmap--;
                        }
                        else if(bitmap_index >= range_start && bitmap_index < range_end) {
                            s.bitmap_count--;
                        }
                    }
                    
                    // Delete the bitmap now
                    generated_bitmap.bitmaps.erase(generated_bitmap.bitmaps.begin() + bitmap_index);
                }
            }
        }
        
        // If we have zero bitmaps, error
        if(generated_bitmap.bitmaps.empty()) {
            eprintf_error("Error: No bitmaps were found in the color plate");
            throw InvalidInputBitmapException();
        }

        // If the last sequence is empty, purge it
        if(generated_bitmap.sequences.size() > 0 && generated_bitmap.sequences[generated_bitmap.sequences.size() - 1].bitmap_count == 0) {
            generated_bitmap.sequences.erase(generated_bitmap.sequences.end() - 1);
        }

        // If we are doing sprites, we need to handle those now
        if(type == BitmapType::BITMAP_TYPE_SPRITES) {
            if(mipmaps > 2) {
                mipmaps = 2;
            }
            process_sprites(generated_bitmap, sprite_parameters.value(), mipmaps);
        }

        // If we're doing height maps, do this
        if(usage == BitmapUsage::BITMAP_USAGE_HEIGHT_MAP) {
            process_height_maps(generated_bitmap, bump_height);
        }

        // If we aren't making interface bitmaps, generate mipmaps when needed
        if(type != BitmapType::BITMAP_TYPE_INTERFACE_BITMAPS && usage != BitmapUsage::BITMAP_USAGE_LIGHT_MAP) {
            generate_mipmaps(generated_bitmap, mipmaps, mipmap_type, mipmap_fade_factor, sprite_parameters, sharpen, blur, usage);
        }

        // If we're making cubemaps, we need to make all sides of each cubemap sequence one cubemap bitmap data. 3D textures work similarly
        if(type == BitmapType::BITMAP_TYPE_CUBE_MAPS || type == BitmapType::BITMAP_TYPE_3D_TEXTURES) {
            consolidate_stacked_bitmaps(generated_bitmap);
        }

        // 3D textures also halve in depth, too
        if(type == BitmapType::BITMAP_TYPE_3D_TEXTURES) {
            merge_3d_texture_mipmaps(generated_bitmap);
        }
    }

    void BitmapProcessor::process_height_maps(GeneratedBitmapData &generated_bitmap, float bump_height) {
        if(bump_height <= 0.0F) {
            eprintf_warn("process_height_maps(): No bump height given, so no bump map will be generated");
            return;
        }
        
        if(bump_height > 0.5F) {
            eprintf_warn("process_height_maps(): Bump height was capped to 0.5");
            bump_height = 0.5F;
        }

        for(auto &bitmap : generated_bitmap.bitmaps) {
            std::vector<Pixel> bitmap_pixels_copy = bitmap.pixels;

            auto largest_dimension = bitmap.width > bitmap.height ? bitmap.height : bitmap.width;
            float bump_scale = 1.5F / (largest_dimension / 256.0F);

            for(std::uint32_t y = 0; y < bitmap.height; y++) {
                for(std::uint32_t x = 0; x < bitmap.width; x++) {
                    // from https://stackoverflow.com/a/2368794

                    #define CLAMP_SUBTRACT(a,b) (b > a ? 0 : (a - b))
                    #define CLAMP_ADD(a,b,max) (a + b > (max - 1) ? (max - 1) : (a + b))

                    // Get the surrounding pixels' positions
                    std::uint32_t down = CLAMP_SUBTRACT(y,1);
                    std::uint32_t up = CLAMP_ADD(y,1,bitmap.height);
                    std::uint32_t right = CLAMP_SUBTRACT(x,1);
                    std::uint32_t left = CLAMP_ADD(x,1,bitmap.width);

                    #define GET_COPY_PIXELS_INTENSITY(x,y) (static_cast<float>(bitmap_pixels_copy[x + y * bitmap.width].convert_to_y8() / 255.0F))

                    // Get all of our pixels
                    float left_up_pixel = GET_COPY_PIXELS_INTENSITY(left,up);
                    float up_pixel = GET_COPY_PIXELS_INTENSITY(x,up);
                    float right_up_pixel = GET_COPY_PIXELS_INTENSITY(right,up);

                    float left_pixel = GET_COPY_PIXELS_INTENSITY(left,y);
                    //float center_pixel = GET_COPY_PIXELS_INTENSITY(x,y);
                    float right_pixel = GET_COPY_PIXELS_INTENSITY(right,y);

                    float left_down_pixel = GET_COPY_PIXELS_INTENSITY(left,down);
                    float down_pixel = GET_COPY_PIXELS_INTENSITY(x,down);
                    float right_down_pixel = GET_COPY_PIXELS_INTENSITY(right,down);

                    #undef GET_COPY_PIXELS_INTENSITY

                    auto &mut_pixel = bitmap.pixels[x + y * (bitmap.width)];

                    float x_intensity = (right_up_pixel + 2.0F * right_pixel + right_down_pixel) - (left_up_pixel + 2.0F * left_pixel + left_down_pixel);
                    float y_intensity = (left_down_pixel + 2.0F * down_pixel + right_down_pixel) - (left_up_pixel + 2.0F * up_pixel + right_up_pixel);
                    float z_intensity = bump_scale / (bump_height / 0.02F);
                    Vector3D<NativeEndian> v;
                    v.i = x_intensity;
                    v.j = y_intensity;
                    v.k = z_intensity;
                    v = v.normalize();

                    mut_pixel.red = static_cast<std::uint8_t>((v.i + 1.0F) / 2.0F * 255);
                    mut_pixel.green = static_cast<std::uint8_t>((v.j + 1.0F) / 2.0F * 255);
                    mut_pixel.blue = static_cast<std::uint8_t>((v.k + 1.0F) / 2.0F * 255);
                }
            }
        }
    }

    void BitmapProcessor::generate_mipmaps(GeneratedBitmapData &generated_bitmap, std::int16_t mipmaps, BitmapMipmapScaleType mipmap_type, std::optional<float> mipmap_fade_factor, const std::optional<BitmapProcessorSpriteParameters> &sprite_parameters, std::optional<float> sharpen, std::optional<float> blur, BitmapUsage usage) {
        auto mipmaps_unsigned = static_cast<std::uint32_t>(mipmaps);
        float fade = mipmap_fade_factor.value_or(0.0F);
        
        bool warn_on_zero_alpha = false;

        for(auto &bitmap : generated_bitmap.bitmaps) {
            std::uint32_t mipmap_width = bitmap.width;
            std::uint32_t mipmap_height = bitmap.height;
            std::uint32_t max_mipmap_count = mipmap_width > mipmap_height ? log2_int(mipmap_width) : log2_int(mipmap_height);
            if(max_mipmap_count > mipmaps_unsigned) {
                max_mipmap_count = mipmaps_unsigned;
            }

            // Only generate up to log2(spacing) mipmaps for sprites
            if(generated_bitmap.type == BitmapType::BITMAP_TYPE_SPRITES) {
                auto sprite_spacing = sprite_parameters.value().sprite_spacing;
                if(sprite_spacing == 0) {
                    max_mipmap_count = 0;
                }
                else {
                    auto max_mipmaps_sprites = log2_int(sprite_parameters.value().sprite_spacing);
                    if(max_mipmap_count > max_mipmaps_sprites) {
                        max_mipmap_count = max_mipmaps_sprites;
                    }
                }
            }

            // Delete mipmaps if needed
            while(bitmap.mipmaps.size() > max_mipmap_count) {
                auto mipmap_to_remove = bitmap.mipmaps.begin() + (bitmap.mipmaps.size() - 1);
                auto first_pixel = bitmap.pixels.begin() + mipmap_to_remove->first_pixel;
                auto last_pixel = first_pixel + mipmap_to_remove->pixel_count;
                bitmap.pixels.erase(first_pixel, last_pixel);
                bitmap.mipmaps.erase(mipmap_to_remove);
            }

            // If we don't need to generate mipmaps, bail
            if(bitmap.mipmaps.size() == max_mipmap_count) {
                return;
            }

            // Now generate mipmaps
            std::uint32_t last_mipmap_offset = 0;
            if(bitmap.mipmaps.size() > 0) {
                auto &last_mipmap = bitmap.mipmaps[bitmap.mipmaps.size() - 1];
                last_mipmap_offset = last_mipmap.first_pixel;
                mipmap_width = last_mipmap.mipmap_width;
                mipmap_height = last_mipmap.mipmap_height;
            }

            // Get blur radius
            std::uint32_t blur_pixels = static_cast<std::uint32_t>(blur.value_or(0.0F) + 0.5F);
            if(blur_pixels > 0) {
                auto *pixel_data = bitmap.pixels.data();
                std::vector<Pixel> unblurred(pixel_data, pixel_data + mipmap_width * mipmap_height);

                std::uint32_t blur_size = (blur_pixels * 2);

                // Allocate a filter of the correct size
                std::vector<Pixel *> pixel_filter(blur_size * blur_size);

                for(std::int64_t y = 0; y < mipmap_height; y++) {
                    for(std::int64_t x = 0; x < mipmap_width; x++) {
                        // Generate the filter
                        for(std::uint32_t yf = 0; yf < blur_size; yf++) {
                            for(std::uint32_t xf = 0; xf < blur_size; xf++) {
                                std::int64_t blur_x = static_cast<std::int64_t>(xf) - blur_pixels + x;
                                std::int64_t blur_y = static_cast<std::int64_t>(yf) - blur_pixels + y;

                                std::uint32_t pixel_x = (blur_x < 0) ? 0 : (blur_x >= mipmap_width) ? (mipmap_width - 1) : static_cast<std::uint32_t>(blur_x);
                                std::uint32_t pixel_y = (blur_y < 0) ? 0 : (blur_y >= mipmap_height) ? (mipmap_height - 1) : static_cast<std::uint32_t>(blur_y);

                                pixel_filter[xf + yf * blur_size] = unblurred.data() + pixel_x + pixel_y * mipmap_width;
                            }
                        }

                        // Do it
                        #define BLUR_CHANNEL(channel) { \
                            std::uint32_t channel_value = 0; \
                            for(auto *color : pixel_filter) { \
                                channel_value += color->channel; \
                            } \
                            channel_value /= pixel_filter.size(); \
                            if(channel_value > 0xFF) { \
                                channel_value = 0xFF; \
                            } \
                            pixel_data[x + y * mipmap_width].channel = static_cast<std::uint8_t>(channel_value); \
                        }

                        BLUR_CHANNEL(red);
                        BLUR_CHANNEL(green);
                        BLUR_CHANNEL(blue);

                        #undef BLUR_CHANNEL
                    }
                }
            }

            auto last_mipmap_height = mipmap_height;
            auto last_mipmap_width = mipmap_width;
            
            auto sharpen_pixels = [&mipmap_height, &mipmap_width, &sharpen, &bitmap](Pixel *pixel_data) {
                // Apply a sharpen filter? https://en.wikipedia.org/wiki/Unsharp_masking
                if(sharpen.has_value() && sharpen.value() > 0.0F) {
                    auto sharpen_value = sharpen.value() / (2.0F * (bitmap.mipmaps.size() + 1));

                    // Make a copy of the mipmap to work off of
                    std::vector<Pixel> unsharpened_pixels(pixel_data, pixel_data + mipmap_width * mipmap_height);

                    // Go through each pixel and apply the sharpening filter
                    for(std::uint32_t y = 0; y < mipmap_height; y++) {
                        for(std::uint32_t x = 0; x < mipmap_width; x++) {
                            auto &center = unsharpened_pixels[x + y * mipmap_width];
                            auto &left = (x == 0) ? center : unsharpened_pixels[x + y * mipmap_width - 1];
                            auto &right = (x + 1 == mipmap_width) ? center : unsharpened_pixels[x + y * mipmap_width + 1];
                            auto &top = (y == 0) ? center : unsharpened_pixels[x + (y - 1) * mipmap_width];
                            auto &bottom = (y + 1 == mipmap_height) ? center : unsharpened_pixels[x + (y + 1) * mipmap_width];
                            auto &this_pixel = pixel_data[x + y * mipmap_width];

                            #define APPLY_SHARPEN(channel) { \
                                std::int32_t modification = static_cast<std::int32_t>(center.channel) * (1.0 + 4.0F * sharpen_value) - (static_cast<std::int32_t>(top.channel) + left.channel + bottom.channel + right.channel) * sharpen_value; \
                                if(modification > 0xFF) { \
                                    this_pixel.channel = 0xFF; \
                                } \
                                else if(modification < 0x00) { \
                                    this_pixel.channel = 0x00; \
                                } \
                                else { \
                                    this_pixel.channel = static_cast<std::uint8_t>(modification); \
                                } \
                            }

                            APPLY_SHARPEN(red);
                            APPLY_SHARPEN(green);
                            APPLY_SHARPEN(blue);

                            #undef APPLY_SHARPEN
                        }
                    }
                }
            };
            
            sharpen_pixels(bitmap.pixels.data());
            
            mipmap_height = std::max(static_cast<std::size_t>(mipmap_height / 2), static_cast<std::size_t>(1));
            mipmap_width = std::max(static_cast<std::size_t>(mipmap_width / 2), static_cast<std::size_t>(1));

            while(bitmap.mipmaps.size() < max_mipmap_count) {
                // Begin creating the mipmap
                auto &next_mipmap = bitmap.mipmaps.emplace_back();
                std::size_t this_mipmap_offset = bitmap.pixels.size();
                next_mipmap.first_pixel = static_cast<std::uint32_t>(this_mipmap_offset);
                next_mipmap.pixel_count = mipmap_height * mipmap_width;
                next_mipmap.mipmap_height = mipmap_height;
                next_mipmap.mipmap_width = mipmap_width;

                // Insert all the pixels needed for the mipmap
                bitmap.pixels.insert(bitmap.pixels.end(), mipmap_height * mipmap_width, Pixel {});
                auto *last_mipmap_data = bitmap.pixels.data() + last_mipmap_offset;
                auto *this_mipmap_data = bitmap.pixels.data() + next_mipmap.first_pixel;
                
                bool has_zero_alpha_and_alpha_blend_usage = usage == BitmapUsage::BITMAP_USAGE_ALPHA_BLEND;

                // Combine each 2x2 block based on the given algorithm
                for(std::uint32_t y = 0; y < mipmap_height; y++) {
                    for(std::uint32_t x = 0; x < mipmap_width; x++) {
                        auto &pixel = this_mipmap_data[x + y * mipmap_width];
                        
                        // Start getting our pixels for mipmaps
                        Pixel last_a, last_b, last_c, last_d;
                        last_a = last_mipmap_data[x * 2 + y * 2 * last_mipmap_width];
                        
                        // If we went down a dimension, use the pixel from the last mipmap. Otherwise, just use last_a so we don't go out-of-bounds
                        bool went_down_both_dimensions = true;
                        
                        // Right pixel
                        if(mipmap_width < last_mipmap_width) {
                            last_b = last_mipmap_data[x * 2 + 1 + y * 2 * last_mipmap_width];
                        }
                        else {
                            last_b = last_a;
                            went_down_both_dimensions = false;
                        }
                        
                        // Bottom pixel
                        if(mipmap_height < last_mipmap_height) {
                            last_c = last_mipmap_data[x * 2     + (y * 2 + 1) * last_mipmap_width];
                        }
                        else {
                            last_c = last_a;
                            went_down_both_dimensions = false;
                        }
                        
                        // Bottom-right pixel - this one's a little tricky
                        if(went_down_both_dimensions) {
                            last_d = last_mipmap_data[x * 2 + 1 + (y * 2 + 1) * last_mipmap_width];
                        }
                        else if(mipmap_height < last_mipmap_height) {
                            last_d = last_c;
                        }
                        else if(mipmap_width < last_mipmap_width) {
                            last_d = last_b;
                        }
                        else {
                            last_d = last_a;
                        }
                        
                        int pixel_count = 4;
                        pixel = last_a;

                        #define INTERPOLATE_CHANNEL(channel) pixel.channel = static_cast<std::uint8_t>((static_cast<std::uint16_t>(last_a.channel) + static_cast<std::uint16_t>(last_b.channel) + static_cast<std::uint16_t>(last_c.channel) + static_cast<std::uint16_t>(last_d.channel)) / 4)
                        #define ZERO_OUT_IF_NO_ALPHA(what) if(what.alpha == 0) { what = {}; pixel_count--; } else { has_zero_alpha_and_alpha_blend_usage = false; }
                        
                        // If alpha blend, discard anything with 0 alpha
                        if(usage == BitmapUsage::BITMAP_USAGE_ALPHA_BLEND) {
                            ZERO_OUT_IF_NO_ALPHA(last_a);
                            ZERO_OUT_IF_NO_ALPHA(last_b);
                            ZERO_OUT_IF_NO_ALPHA(last_c);
                            ZERO_OUT_IF_NO_ALPHA(last_d);
                        }
                        
                        if(pixel_count > 0) {
                            // Interpolate color?
                            if(mipmap_type == BitmapMipmapScaleType::BITMAP_MIPMAP_SCALE_TYPE_LINEAR || mipmap_type == BitmapMipmapScaleType::BITMAP_MIPMAP_SCALE_TYPE_NEAREST_ALPHA) {
                                INTERPOLATE_CHANNEL(red);
                                INTERPOLATE_CHANNEL(green);
                                INTERPOLATE_CHANNEL(blue);
                            }

                            // Interpolate alpha?
                            if(mipmap_type == BitmapMipmapScaleType::BITMAP_MIPMAP_SCALE_TYPE_LINEAR && usage != BitmapUsage::BITMAP_USAGE_VECTOR_MAP) {
                                INTERPOLATE_CHANNEL(alpha);
                            }
                        }
                        else {
                            // Delete if no pixels
                            pixel = {};
                        }
                        
                        #undef ZERO_OUT_IF_NO_ALPHA
                        #undef INTERPOLATE_CHANNEL
                    }
                }
                
                // Sharpen if need be
                sharpen_pixels(this_mipmap_data);

                // Set the values for the next mipmap
                last_mipmap_height = mipmap_height;
                last_mipmap_width = mipmap_width;
                mipmap_height = std::max(static_cast<std::size_t>(mipmap_height / 2), static_cast<std::size_t>(1));
                mipmap_width = std::max(static_cast<std::size_t>(mipmap_width / 2), static_cast<std::size_t>(1));
                last_mipmap_offset = this_mipmap_offset;
                
                warn_on_zero_alpha = warn_on_zero_alpha || has_zero_alpha_and_alpha_blend_usage;
            }

            // Do fade-to-gray for each mipmap
            if(mipmap_fade_factor.has_value()) {
                std::size_t mipmap_count = bitmap.mipmaps.size();
                float mipmap_count_plus_one = mipmap_count + 1.0F; // although Guerilla only mentions mipmaps in the fade-to-gray stuff, it includes the first bitmap in the calculation
                float overall_fade_factor = static_cast<float>(mipmap_count_plus_one) - static_cast<float>(fade) * (mipmap_count_plus_one - 1.0F + (1.0F - fade)); // excuse me what the fuck

                for(std::size_t m = 0; m < mipmap_count; m++) {
                    auto &mipmap = bitmap.mipmaps[m];

                    // Iterate through each pixel
                    Pixel *first = bitmap.pixels.data() + mipmap.first_pixel;
                    auto *last = first + mipmap.pixel_count;

                    while(first < last) {
                        std::uint8_t alpha_delta;

                        // If we're fading to gray instantly, do that so we don't divide by 0
                        if(fade >= 1.0F) {
                            alpha_delta = UINT8_MAX;
                        }
                        else {
                            // Basically, a higher mipmap fade factor scales faster
                            float gray_multiplier = static_cast<float>(m + 1) / overall_fade_factor;

                            // If we go over 1, go to 1
                            if(gray_multiplier > 1.0F) {
                                gray_multiplier = 1.0F;
                            }

                            // Round
                            float gray_multiplied = std::floor(UINT8_MAX * gray_multiplier + 0.5F);
                            auto new_gray = static_cast<std::uint32_t>(gray_multiplied);
                            if(new_gray > UINT8_MAX) {
                                alpha_delta = UINT8_MAX;
                            }
                            else {
                                alpha_delta = static_cast<std::uint8_t>(new_gray);
                            }
                        }

                        Pixel FADE_TO_GRAY = { 0x7F, 0x7F, 0x7F, static_cast<std::uint8_t>(alpha_delta) };
                        *first = first->alpha_blend(FADE_TO_GRAY);

                        first++;
                    }
                }
            }
        }
        
        if(warn_on_zero_alpha) {
            eprintf_warn("Usage is alpha blend, and a bitmap has zero alpha; its mipmaps will be black.");
        }
    }

    void BitmapProcessor::consolidate_stacked_bitmaps(GeneratedBitmapData &generated_bitmap) {
        std::vector<GeneratedBitmapDataSequence> new_sequences;
        std::vector<GeneratedBitmapDataBitmap> new_bitmaps;
        for(auto &sequence : generated_bitmap.sequences) {
            auto &new_sequence = new_sequences.emplace_back();
            new_sequence.bitmap_count = 1;
            new_sequence.first_bitmap = new_bitmaps.size();
            new_sequence.y_start = sequence.y_start;
            new_sequence.y_start = sequence.y_end;
            const std::uint32_t FACES = sequence.bitmap_count;

            if(FACES == 0) {
                eprintf_error("Error: Stacked bitmaps must have at least one bitmap. %u found.\n", FACES);
                throw InvalidInputBitmapException();
            }

            auto *bitmaps = generated_bitmap.bitmaps.data() + sequence.first_bitmap;
            auto &new_bitmap = new_bitmaps.emplace_back();
            new_bitmap.height = bitmaps->height;
            new_bitmap.width = bitmaps->width;
            new_bitmap.color_plate_x = bitmaps->color_plate_x;
            new_bitmap.color_plate_y = bitmaps->color_plate_y;
            new_bitmap.registration_point_x = 0;
            new_bitmap.registration_point_y = 0;
            new_bitmap.depth = FACES;

            const std::uint32_t MIPMAP_COUNT = bitmaps->mipmaps.size();
            const std::uint32_t BITMAP_WIDTH = new_bitmap.width;
            const std::uint32_t BITMAP_HEIGHT = new_bitmap.height;

            if(generated_bitmap.type == BitmapType::BITMAP_TYPE_CUBE_MAPS) {
                if(FACES != 6) {
                    eprintf_error("Error: Cubemaps must have six bitmaps per cubemap. %u found.\n", FACES);
                    throw InvalidInputBitmapException();
                }
                if(BITMAP_WIDTH != BITMAP_HEIGHT) {
                    eprintf_error("Error: Cubemap length must equal width and height. %ux%u found.\n", BITMAP_WIDTH, BITMAP_HEIGHT);
                    throw InvalidInputBitmapException();
                }
            }
            else if(generated_bitmap.type == BitmapType::BITMAP_TYPE_3D_TEXTURES) {
                if(!FACES || !is_power_of_two(FACES)) {
                    eprintf_error("Error: 3D texture depth must be a power of two. Got %u\n", FACES);
                    throw InvalidInputBitmapException();
                }
            }

            std::uint32_t mipmap_width = BITMAP_WIDTH;
            std::uint32_t mipmap_height = BITMAP_HEIGHT;

            for(std::uint32_t i = 0; i <= MIPMAP_COUNT; i++) {
                // Calculate the mipmap size in pixels (6 faces x length^2)
                std::uint32_t mipmap_size = mipmap_width * mipmap_height * FACES;

                // Add everything
                if(i) {
                    auto &mipmap = new_bitmap.mipmaps.emplace_back();
                    mipmap.first_pixel = new_bitmap.pixels.size();
                    mipmap.pixel_count = mipmap_size;
                    mipmap.mipmap_height = mipmap_height;
                    mipmap.mipmap_width = mipmap_width;
                    mipmap.mipmap_depth = FACES;
                }

                new_bitmap.pixels.insert(new_bitmap.pixels.end(), mipmap_size, Pixel {});

                mipmap_width /= 2;
                mipmap_height /= 2;
            }

            // Go through each face
            for(std::size_t f = 0; f < FACES; f++) {
                auto &bitmap = bitmaps[f];

                // Ensure it's the same dimensions
                if(bitmap.height != BITMAP_HEIGHT || bitmap.width != BITMAP_WIDTH) {
                    eprintf_error("Error: Stacked bitmaps must be the same dimensions. Expected %ux%u. %ux%u found\n", BITMAP_WIDTH, BITMAP_WIDTH, bitmap.width, bitmap.height);
                    throw InvalidInputBitmapException();
                }

                // Also ensure it has the same # of mipmaps. I don't know how it wouldn't, but you never know
                if(bitmap.mipmaps.size() != MIPMAP_COUNT) {
                    eprintf_error("Error: Stacked bitmaps must have the same number of mipmaps. Expected %u. %zu found\n", MIPMAP_COUNT, bitmap.mipmaps.size());
                    throw InvalidInputBitmapException();
                }

                // One of the only do/while loops I will ever do in Invader while writing it.
                std::optional<std::uint32_t> m;
                do {
                    Pixel *destination_buffer;
                    const Pixel *source_buffer;
                    std::size_t pixel_count;

                    // Determine things
                    if(m.has_value()) {
                        auto &mipmap = new_bitmap.mipmaps[m.value()];
                        pixel_count = mipmap.mipmap_width * mipmap.mipmap_height;
                        destination_buffer = new_bitmap.pixels.data() + mipmap.first_pixel + pixel_count * f;
                        source_buffer = bitmap.pixels.data() + bitmap.mipmaps[m.value()].first_pixel;
                        m = m.value() + 1;
                    }
                    else {
                        pixel_count = BITMAP_WIDTH * BITMAP_HEIGHT;
                        destination_buffer = new_bitmap.pixels.data() + pixel_count * f;
                        source_buffer = bitmap.pixels.data();
                        m = 0;
                    }

                    // Copy!
                    std::copy(source_buffer, source_buffer + pixel_count, destination_buffer);
                }
                while(m.value() < MIPMAP_COUNT);
            }
        }

        generated_bitmap.bitmaps = std::move(new_bitmaps);
        generated_bitmap.sequences = std::move(new_sequences);
    }

    void BitmapProcessor::merge_3d_texture_mipmaps(GeneratedBitmapData &generated_bitmap) {
        for(auto &bitmap : generated_bitmap.bitmaps) {
            std::uint32_t bitmaps_to_merge = 2;
            std::uint32_t bitmap_pixel_count = bitmap.height * bitmap.width;
            std::vector<Pixel> new_pixels(bitmap.pixels.data(), bitmap.pixels.data() + bitmap_pixel_count * bitmap.depth);
            std::vector<GeneratedBitmapDataBitmapMipmap> new_mipmaps;
            for(auto &mipmap : bitmap.mipmaps) {
                if(bitmaps_to_merge > bitmap.depth) {
                    break;
                }

                // Make the new mipmap metadata
                auto &new_mipmap = new_mipmaps.emplace_back();
                new_mipmap.first_pixel = new_pixels.size();
                std::size_t layer_size = mipmap.mipmap_height * mipmap.mipmap_width;
                new_mipmap.mipmap_height = mipmap.mipmap_height;
                new_mipmap.mipmap_width = mipmap.mipmap_width;
                new_mipmap.mipmap_depth = mipmap.mipmap_depth / bitmaps_to_merge;
                new_mipmap.pixel_count = static_cast<std::uint32_t>(layer_size * new_mipmap.mipmap_depth);

                // Go through each pixel and average
                for(std::uint32_t d = 0; d < new_mipmap.mipmap_depth; d++) {
                    for(std::uint32_t y = 0; y < new_mipmap.mipmap_height; y++) {
                        for(std::uint32_t x = 0; x < new_mipmap.mipmap_width; x++) {
                                std::size_t alpha = 0, red = 0, green = 0, blue = 0;
                                for(std::uint32_t d_inner = d * bitmaps_to_merge; d_inner < (d + 1) * bitmaps_to_merge; d_inner++) {
                                    auto &pixel = *(bitmap.pixels.data() + (x + y * new_mipmap.mipmap_height) + layer_size * d_inner + mipmap.first_pixel);
                                    alpha += pixel.alpha;
                                    red += pixel.red;
                                    green += pixel.green;
                                    blue += pixel.blue;
                                }

                                auto &new_pixel = new_pixels.emplace_back();
                                new_pixel.alpha = static_cast<std::uint8_t>(alpha / bitmaps_to_merge);
                                new_pixel.red = static_cast<std::uint8_t>(red / bitmaps_to_merge);
                                new_pixel.green = static_cast<std::uint8_t>(green / bitmaps_to_merge);
                                new_pixel.blue = static_cast<std::uint8_t>(blue / bitmaps_to_merge);
                            }
                        }
                }

                bitmaps_to_merge *= 2;
            }

            bitmap.mipmaps = new_mipmaps;
            bitmap.pixels = new_pixels;
        }
    }
}
