// SPDX-License-Identifier: GPL-3.0-only

#include <optional>
#include <algorithm>

#include <invader/hek/data_type.hpp>
#include "color_plate_scanner.hpp"
#include <invader/printf.hpp>

namespace Invader {
    static constexpr char ERROR_INVALID_BITMAP_WIDTH[] = "Error: Found a bitmap with an invalid width: %u\n";
    static constexpr char ERROR_INVALID_BITMAP_HEIGHT[] = "Error: Found a bitmap with an invalid height: %u\n";

    template <typename T> inline constexpr T divide_by_two_round(T value) {
        return (value / 2) + (value & 1);
    }

    template<typename T> static constexpr inline T log2_int(T number) {
        T log2_value = 0;
        while(number != 1) {
            number >>= 1;
            log2_value++;
        }
        return log2_value;
    }

    static inline bool same_color_ignore_opacity(const ColorPlatePixel &color_a, const ColorPlatePixel &color_b) {
        return color_a.red == color_b.red && color_a.blue == color_b.blue && color_a.green == color_b.green;
    }

    static inline bool is_power_of_two(std::uint32_t number) {
        std::uint32_t ones = 0;
        while(number > 0) {
            ones += number & 1;
            number >>= 1;
        }
        return ones <= 1;
    }

    #define GET_PIXEL(x,y) (pixels[y * width + x])

    GeneratedBitmapData ColorPlateScanner::scan_color_plate(const ColorPlatePixel *pixels, std::uint32_t width, std::uint32_t height, BitmapType type, BitmapUsage usage, float bump_height, std::optional<ColorPlateScannerSpriteParameters> &sprite_parameters, std::int16_t mipmaps, HEK::InvaderBitmapMipmapScaling mipmap_type, std::optional<float> mipmap_fade_factor, std::optional<float> sharpen, std::optional<float> blur) {
        ColorPlateScanner scanner;
        GeneratedBitmapData generated_bitmap;

        generated_bitmap.type = type;
        scanner.power_of_two = (type != BitmapType::BITMAP_TYPE_SPRITES) && (type != BitmapType::BITMAP_TYPE_INTERFACE_BITMAPS);

        if(width == 0 || height == 0) {
            return generated_bitmap;
        }

        // Check to see if we have valid color plate data. If so, look for sequences
        bool valid_color_plate_key = true;
        if(width >= 4 && height >= 2) {
            // Get the candidate pixels
            const auto &transparency_candidate = pixels[0];
            const auto &separator_candidate = pixels[1];
            const auto &spacing_candidate = pixels[2];
            
            // First, check to see if everything on the top row except the first three pixels is transparency
            for(std::uint32_t x = 3; x < width; x++) {
                if(!same_color_ignore_opacity(GET_PIXEL(x, 0), transparency_candidate)) {
                    valid_color_plate_key = false;
                    break;
                }
            }
            
            // The key is valid maybe?
            if(valid_color_plate_key) {
                scanner.transparency_color = transparency_candidate;
            
                // What we need to do next is determine if we have a sequence divider
                if(!same_color_ignore_opacity(transparency_candidate, separator_candidate)) {
                    scanner.sequence_divider_color = separator_candidate;
                    
                    // Generate sequences
                    auto *sequence = &generated_bitmap.sequences.emplace_back();
                    
                    auto is_horizontal_bar = [&scanner, &width, &pixels](std::size_t y) {
                        if(scanner.is_sequence_divider_color(GET_PIXEL(0,y))) {
                            for(std::size_t x = 1; x < width; x++) {
                                if(!scanner.is_sequence_divider_color(GET_PIXEL(x,y))) {
                                    eprintf_error("Sequence divider broken at (%zu,%zu)", x, y);
                                    throw InvalidInputBitmapException();
                                }
                            }
                            return true;
                        }
                        return false;
                    };
                    
                    sequence->y_start = is_horizontal_bar(1) ? 2 : 1;

                    // Next, we need to find all of the sequences
                    for(std::uint32_t y = sequence->y_start; y < height; y++) {
                        // If we got it, create a new sequence, but not before terminating the last one
                        if(is_horizontal_bar(y)) {
                            sequence->y_end = y;
                            sequence = &generated_bitmap.sequences.emplace_back();
                            sequence->y_start = y + 1;
                        }
                    }

                    // Terminate the last sequence index
                    sequence->y_end = height;
                }
                
                // Add a default sequence
                else {
                    auto &sequence = generated_bitmap.sequences.emplace_back();
                    sequence.y_start = 1;
                    sequence.y_end = height;
                }
                
                // Lastly, do we have spacing?
                if(!same_color_ignore_opacity(transparency_candidate, spacing_candidate)) {
                    scanner.spacing_color = spacing_candidate;
                    
                    // Make sure it's valid!
                    if(same_color_ignore_opacity(separator_candidate, spacing_candidate)) {
                        eprintf_error("Spacing and sequence divider colors must not match");
                        throw InvalidInputBitmapException();
                    }
                }
            }
            
            // If we don't have a sequence color and the transparency color is NOT blue, then we actually do not have a valid color plate
            if(!scanner.sequence_divider_color.has_value() && !scanner.is_transparency_color(ColorPlatePixel { 0xFF, 0x00, 0x00, 0xFF } )) {
                valid_color_plate_key = false;
                generated_bitmap.sequences.clear();
            }
        }

        // If we have valid color plate data, use the color plate data
        if(valid_color_plate_key) {
            scanner.read_color_plate(generated_bitmap, pixels, width);
        }

        // Otherwise, read as one bitmap
        else {
            auto &new_sequence = generated_bitmap.sequences.emplace_back();
            new_sequence.bitmap_count = 1;
            new_sequence.first_bitmap = 0;
            new_sequence.y_start = 0;
            new_sequence.y_end = height;

            if(type == BitmapType::BITMAP_TYPE_CUBE_MAPS) {
                scanner.read_unrolled_cubemap(generated_bitmap, pixels, width, height);
            }
            else if(type == BitmapType::BITMAP_TYPE_SPRITES) {
                eprintf_error("Error: Sprite color plates must have a color plate key.\n");
                throw InvalidInputBitmapException();
            }
            else {
                scanner.read_single_bitmap(generated_bitmap, pixels, width, height);
            }
        }
        
        // If we have zero bitmaps, error
        if(generated_bitmap.bitmaps.size() == 0) {
            eprintf_error("Error: No bitmaps were found in the color plate");
            throw InvalidInputBitmapException();
        }

        // If the last sequence is empty, purge it
        if(generated_bitmap.sequences.size() > 0 && generated_bitmap.sequences[generated_bitmap.sequences.size() - 1].bitmap_count == 0) {
            generated_bitmap.sequences.erase(generated_bitmap.sequences.end() - 1);
        }

        // If we are doing sprites, we need to handle those now
        if(type == BitmapType::BITMAP_TYPE_SPRITES) {
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

        return generated_bitmap;
    }

    void ColorPlateScanner::read_color_plate(GeneratedBitmapData &generated_bitmap, const ColorPlatePixel *pixels, std::uint32_t width) const {
        for(auto &sequence : generated_bitmap.sequences) {
            sequence.first_bitmap = generated_bitmap.bitmaps.size();
            sequence.bitmap_count = 0;

            // Search left and right for bitmap borders
            const std::uint32_t X_END = width;
            const std::uint32_t Y_START = sequence.y_start;
            const std::uint32_t Y_END = sequence.y_end;

            // This is used for the registration point
            const std::int32_t MID_Y = divide_by_two_round(static_cast<std::int32_t>(Y_START + Y_END));

            // Go through each pixel
            for(std::uint32_t x = 0; x < X_END; x++) {
                for(std::uint32_t y = Y_START; y < Y_END; y++) {
                    auto &pixel = GET_PIXEL(x,y);

                    // Ignore? Okay.
                    if(this->is_transparency_color(pixel) || this->is_sequence_divider_color(pixel)) {
                        continue;
                    }

                    // Begin.
                    std::optional<std::uint32_t> min_x;
                    std::optional<std::uint32_t> max_x;
                    std::optional<std::uint32_t> min_y;
                    std::optional<std::uint32_t> max_y;

                    std::optional<std::uint32_t> virtual_min_x;
                    std::optional<std::uint32_t> virtual_max_x;
                    std::optional<std::uint32_t> virtual_min_y;
                    std::optional<std::uint32_t> virtual_max_y;

                    // Find the minimum x, y, max x, and max y stuff
                    for(std::uint32_t xb = x; xb < X_END; xb++) {
                        // Set this to false if we got something this column
                        bool ignored_this_x = true;

                        for(std::uint32_t yb = Y_START; yb < Y_END; yb++) {
                            auto &pixel = GET_PIXEL(xb, yb);

                            // This is for anything that's not a cyan/magenta/blue pixel
                            if(!this->is_ignored(pixel)) {
                                if(min_x.has_value()) {
                                    if(min_x.value() > xb) {
                                        min_x = xb;
                                    }
                                    if(min_y.value() > yb) {
                                        min_y = yb;
                                    }
                                    if(max_x.value() < xb) {
                                        max_x = xb;
                                    }
                                    if(max_y.value() < yb) {
                                        max_y = yb;
                                    }
                                }
                                else {
                                    min_x = xb;
                                    min_y = yb;
                                    max_x = xb;
                                    max_y = yb;
                                }
                            }

                            // Anything that's not a magenta/blue pixel then
                            if(!this->is_transparency_color(pixel) && !this->is_sequence_divider_color(pixel)) {
                                ignored_this_x = false;

                                if(virtual_min_x.has_value()) {
                                    if(virtual_min_x.value() > xb) {
                                        virtual_min_x = xb;
                                    }
                                    if(virtual_min_y.value() > yb) {
                                        virtual_min_y = yb;
                                    }
                                    if(virtual_max_x.value() < xb) {
                                        virtual_max_x = xb;
                                    }
                                    if(virtual_max_y.value() < yb) {
                                        virtual_max_y = yb;
                                    }
                                }
                                else {
                                    virtual_min_x = xb;
                                    virtual_min_y = yb;
                                    virtual_max_x = xb;
                                    virtual_max_y = yb;
                                }
                            }
                        }

                        if(ignored_this_x) {
                            break;
                        }
                    }

                    // If we never got a minimum x, then continue on
                    if(!min_x.has_value()) {
                        continue;
                    }

                    // Get the width and height
                    std::uint32_t bitmap_width = max_x.value() - min_x.value() + 1;
                    std::uint32_t bitmap_height = max_y.value() - min_y.value() + 1;

                    // If we require power-of-two, check
                    if(power_of_two) {
                        if(!is_power_of_two(bitmap_width)) {
                            eprintf(ERROR_INVALID_BITMAP_WIDTH, bitmap_width);
                            throw InvalidInputBitmapException();
                        }
                        if(!is_power_of_two(bitmap_height)) {
                            eprintf(ERROR_INVALID_BITMAP_HEIGHT, bitmap_height);
                            throw InvalidInputBitmapException();
                        }
                    }

                    // Add the bitmap
                    auto &bitmap = generated_bitmap.bitmaps.emplace_back();
                    bitmap.width = bitmap_width;
                    bitmap.height = bitmap_height;
                    bitmap.color_plate_x = min_x.value();
                    bitmap.color_plate_y = min_y.value();

                    // Calculate registration point.
                    const std::int32_t MID_X = divide_by_two_round(static_cast<std::int32_t>(1 + virtual_max_x.value() + virtual_min_x.value()));

                    // The x point is the midpoint of the width of the bitmap and cyan stuff relative to the left
                    bitmap.registration_point_x = MID_X - static_cast<std::int32_t>(min_x.value());

                    // The x point is the midpoint of the height of the entire sequence relative to the top
                    bitmap.registration_point_y = MID_Y - static_cast<std::int32_t>(min_y.value());

                    // Load the pixels
                    for(std::uint32_t by = min_y.value(); by <= max_y.value(); by++) {
                        for(std::uint32_t bx = min_x.value(); bx <= max_x.value(); bx++) {
                            auto &pixel = GET_PIXEL(bx, by);
                            if(this->is_ignored(pixel)) {
                                bitmap.pixels.push_back(ColorPlatePixel {});
                            }
                            else {
                                bitmap.pixels.push_back(pixel);
                            }
                        }
                    }

                    sequence.bitmap_count++;

                    // Set it to the max value. Add 1 since sprites can't possibly be adjacent to each other. Then, the for loop will add 1 again to get to the minimum possible x value.
                    x = virtual_max_x.value() + 1;
                    break;
                }
            }
        }
    }

    void ColorPlateScanner::read_unrolled_cubemap(GeneratedBitmapData &generated_bitmap, const ColorPlatePixel *pixels, std::uint32_t width, std::uint32_t height) const {
        // Make sure the height and width of each face is the same
        std::uint32_t face_width = width / 4;
        std::uint32_t face_height = height / 3;

        if(face_height != face_width || !is_power_of_two(face_width) || face_width < 1 || face_width * 4 != width || face_height * 3 != height) {
            eprintf_error("Error: Invalid cubemap input dimensions %ux%u.\n", face_width, face_height);
            throw InvalidInputBitmapException();
        }

        auto get_pixel_transformed = [&pixels, &width, &face_width](std::uint32_t left, std::uint32_t top, std::uint32_t relative_x, std::uint32_t relative_y, std::uint32_t rotation) {
            std::uint32_t x, y;
            if(rotation == 0) {
                x = left + relative_x;
                y = top + relative_y;
            }
            else if(rotation == 90) {
                x = left + face_width - relative_y - 1;
                y = top + relative_x;
            }
            else if(rotation == 180) {
                x = left + face_width - relative_x - 1;
                y = top + face_width - relative_y - 1;
            }
            else if(rotation == 270) {
                x = left + relative_y;
                y = top + face_width - relative_x - 1;
            }
            else {
                std::terminate();
            }
            return GET_PIXEL(x, y);
        };

        auto add_bitmap = [&generated_bitmap, &get_pixel_transformed, &face_width](std::uint32_t left, std::uint32_t top, std::uint32_t rotation) {
            auto &new_bitmap = generated_bitmap.bitmaps.emplace_back();
            new_bitmap.color_plate_x = left;
            new_bitmap.color_plate_y = top;
            new_bitmap.height = face_width;
            new_bitmap.width = face_width;
            new_bitmap.pixels.reserve(face_width * face_width);
            new_bitmap.registration_point_x = 0;
            new_bitmap.registration_point_y = 0;

            for(std::uint32_t y = 0; y < face_width; y++) {
                for(std::uint32_t x = 0; x < face_width; x++) {
                    new_bitmap.pixels.push_back(get_pixel_transformed(left, top, x, y, rotation));
                }
            }
        };

        // Add each cubemap face
        add_bitmap(face_width * 0, face_width * 1, 90);
        add_bitmap(face_width * 1, face_width * 1, 180);
        add_bitmap(face_width * 2, face_width * 1, 270);
        add_bitmap(face_width * 3, face_width * 1, 0);
        add_bitmap(face_width * 0, face_width * 0, 90);
        add_bitmap(face_width * 0, face_width * 2, 90);
        generated_bitmap.sequences[0].first_bitmap = 0;
        generated_bitmap.sequences[0].bitmap_count = 6;
    }

    void ColorPlateScanner::read_single_bitmap(GeneratedBitmapData &generated_bitmap, const ColorPlatePixel *pixels, std::uint32_t width, std::uint32_t height) const {
        if(generated_bitmap.type != BitmapType::BITMAP_TYPE_INTERFACE_BITMAPS) {
            if(!is_power_of_two(width)) {
                eprintf(ERROR_INVALID_BITMAP_WIDTH, width);
                throw InvalidInputBitmapException();
            }
            if(!is_power_of_two(height)) {
                eprintf(ERROR_INVALID_BITMAP_WIDTH, height);
                throw InvalidInputBitmapException();
            }
        }

        auto &new_bitmap = generated_bitmap.bitmaps.emplace_back();
        new_bitmap.color_plate_x = 0;
        new_bitmap.color_plate_y = 0;
        new_bitmap.pixels.insert(new_bitmap.pixels.begin(), pixels, pixels + width * height);
        new_bitmap.width = width;
        new_bitmap.height = height;
        new_bitmap.registration_point_x = divide_by_two_round(width);
        new_bitmap.registration_point_y = divide_by_two_round(height);

        generated_bitmap.sequences[0].bitmap_count = 1;
    }

    bool ColorPlateScanner::is_transparency_color(const ColorPlatePixel &color) const {
        return this->transparency_color.has_value() && same_color_ignore_opacity(*this->transparency_color, color);
    }

    bool ColorPlateScanner::is_sequence_divider_color(const ColorPlatePixel &color) const {
        return this->sequence_divider_color.has_value() && same_color_ignore_opacity(*this->sequence_divider_color, color);
    }

    bool ColorPlateScanner::is_spacing_color(const ColorPlatePixel &color) const {
        return this->spacing_color.has_value() && same_color_ignore_opacity(*this->spacing_color, color);
    }

    bool ColorPlateScanner::is_ignored(const ColorPlatePixel &color) const {
        return this->is_transparency_color(color) || this->is_spacing_color(color) || this->is_sequence_divider_color(color);
    }

    void ColorPlateScanner::process_height_maps(GeneratedBitmapData &generated_bitmap, float bump_height) {
        if(bump_height <= 0.0F) {
            eprintf_warn("process_height_maps(): No bump height given, so no bump map will be generated");
            return;
        }
        
        if(bump_height > 0.5F) {
            eprintf_warn("process_height_maps(): Bump height was capped to 0.5");
            bump_height = 0.5F;
        }

        for(auto &bitmap : generated_bitmap.bitmaps) {
            std::vector<ColorPlatePixel> bitmap_pixels_copy = bitmap.pixels;

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
                    HEK::Vector3D<HEK::NativeEndian> v;
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

    void ColorPlateScanner::generate_mipmaps(GeneratedBitmapData &generated_bitmap, std::int16_t mipmaps, HEK::InvaderBitmapMipmapScaling mipmap_type, std::optional<float> mipmap_fade_factor, const std::optional<ColorPlateScannerSpriteParameters> &sprite_parameters, std::optional<float> sharpen, std::optional<float> blur, BitmapUsage usage) {
        auto mipmaps_unsigned = static_cast<std::uint32_t>(mipmaps);
        float fade = mipmap_fade_factor.value_or(0.0F);

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

            // Apply a sharpen filter? https://en.wikipedia.org/wiki/Unsharp_masking
            if(sharpen.has_value() && sharpen.value() > 0.0F) {
                const float &SHARPEN_VALUE = sharpen.value();

                // Make a copy of the mipmap to work off of
                auto *pixel_data = bitmap.pixels.data();
                std::vector<ColorPlatePixel> unsharpened_pixels(pixel_data, pixel_data + mipmap_width * mipmap_height);

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
                            std::int32_t modification = static_cast<std::int32_t>(center.channel) * (1.0 + 4.0F * SHARPEN_VALUE) - (static_cast<std::int32_t>(top.channel) + left.channel + bottom.channel + right.channel) * SHARPEN_VALUE; \
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

            // Get blur radius
            std::uint32_t blur_pixels = static_cast<std::uint32_t>(blur.value_or(0.0F) + 0.5F);
            if(blur_pixels > 0) {
                auto *pixel_data = bitmap.pixels.data();
                std::vector<ColorPlatePixel> unblurred(pixel_data, pixel_data + mipmap_width * mipmap_height);

                std::uint32_t blur_size = blur_pixels * 2 + 1;

                // Allocate a filter of the correct size
                std::vector<ColorPlatePixel *> pixel_filter(blur_size * blur_size);

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
                bitmap.pixels.insert(bitmap.pixels.end(), mipmap_height * mipmap_width, ColorPlatePixel {});
                auto *last_mipmap_data = bitmap.pixels.data() + last_mipmap_offset;
                auto *this_mipmap_data = bitmap.pixels.data() + next_mipmap.first_pixel;

                // Combine each 2x2 block based on the given algorithm
                for(std::uint32_t y = 0; y < mipmap_height; y++) {
                    for(std::uint32_t x = 0; x < mipmap_width; x++) {
                        auto &pixel = this_mipmap_data[x + y * mipmap_width];
                        
                        // Start getting our pixels for mipmaps
                        ColorPlatePixel last_a, last_b, last_c, last_d;
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
                        #define ZERO_OUT_IF_NO_ALPHA(what) if(what.alpha == 0) { what = {}; pixel_count--; }
                        
                        // If alpha blend, discard anything with 0 alpha
                        if(usage == BitmapUsage::BITMAP_USAGE_ALPHA_BLEND) {
                            ZERO_OUT_IF_NO_ALPHA(last_a);
                            ZERO_OUT_IF_NO_ALPHA(last_b);
                            ZERO_OUT_IF_NO_ALPHA(last_c);
                            ZERO_OUT_IF_NO_ALPHA(last_d);
                        }
                        
                        if(pixel_count > 0) {
                            // Interpolate color?
                            if(mipmap_type == HEK::InvaderBitmapMipmapScaling::INVADER_BITMAP_MIPMAP_SCALING_LINEAR || mipmap_type == HEK::InvaderBitmapMipmapScaling::INVADER_BITMAP_MIPMAP_SCALING_NEAREST_ALPHA) {
                                INTERPOLATE_CHANNEL(red);
                                INTERPOLATE_CHANNEL(green);
                                INTERPOLATE_CHANNEL(blue);
                            }

                            // Interpolate alpha?
                            if(mipmap_type == HEK::InvaderBitmapMipmapScaling::INVADER_BITMAP_MIPMAP_SCALING_LINEAR && usage != BitmapUsage::BITMAP_USAGE_VECTOR_MAP) {
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

                // Set the values for the next mipmap
                last_mipmap_height = mipmap_height;
                last_mipmap_width = mipmap_width;
                mipmap_height = std::max(static_cast<std::size_t>(mipmap_height / 2), static_cast<std::size_t>(1));
                mipmap_width = std::max(static_cast<std::size_t>(mipmap_width / 2), static_cast<std::size_t>(1));
                last_mipmap_offset = this_mipmap_offset;
            }

            // Do fade-to-gray for each mipmap
            if(mipmap_fade_factor.has_value()) {
                std::size_t mipmap_count = bitmap.mipmaps.size();
                float mipmap_count_plus_one = mipmap_count + 1.0F; // although Guerilla only mentions mipmaps in the fade-to-gray stuff, it includes the first bitmap in the calculation
                float overall_fade_factor = static_cast<float>(mipmap_count_plus_one) - static_cast<float>(fade) * (mipmap_count_plus_one - 1.0F + (1.0F - fade)); // excuse me what the fuck

                for(std::size_t m = 0; m < mipmap_count; m++) {
                    auto &mipmap = bitmap.mipmaps[m];

                    // Iterate through each pixel
                    ColorPlatePixel *first = bitmap.pixels.data() + mipmap.first_pixel;
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

                        ColorPlatePixel FADE_TO_GRAY = { 0x7F, 0x7F, 0x7F, static_cast<std::uint8_t>(alpha_delta) };
                        *first = first->alpha_blend(FADE_TO_GRAY);

                        first++;
                    }
                }
            }
        }
    }

    void ColorPlateScanner::consolidate_stacked_bitmaps(GeneratedBitmapData &generated_bitmap) {
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

                new_bitmap.pixels.insert(new_bitmap.pixels.end(), mipmap_size, ColorPlatePixel {});

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
                    ColorPlatePixel *destination_buffer;
                    const ColorPlatePixel *source_buffer;
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

    void ColorPlateScanner::merge_3d_texture_mipmaps(GeneratedBitmapData &generated_bitmap) {
        for(auto &bitmap : generated_bitmap.bitmaps) {
            std::uint32_t bitmaps_to_merge = 2;
            std::uint32_t bitmap_pixel_count = bitmap.height * bitmap.width;
            std::vector<ColorPlatePixel> new_pixels(bitmap.pixels.data(), bitmap.pixels.data() + bitmap_pixel_count * bitmap.depth);
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

    static std::uint32_t number_of_sprite_sheets(const std::vector<GeneratedBitmapDataSequence> &sequences) {
        std::uint32_t highest = 0;
        for(auto &sequence : sequences) {
            for(auto &sprite : sequence.sprites) {
                std::uint32_t count = sprite.bitmap_index + 1;
                if(count > highest) {
                    highest = count;
                }
            }
        }
        return highest;
    }

    static std::uint32_t length_of_sprite_sheet(const std::vector<GeneratedBitmapDataSequence> &sequences, std::uint32_t bitmap_index) {
        std::uint32_t max_width = 0;
        std::uint32_t max_height = 0;

        // Go through each sprite of the bitmap index
        for(auto &sequence : sequences) {
            for(auto &sprite : sequence.sprites) {
                if(sprite.bitmap_index != bitmap_index) {
                    continue;
                }
                if(sprite.right > max_width) {
                    max_width = sprite.right;
                }
                if(sprite.bottom > max_height) {
                    max_height = sprite.bottom;
                }
            }
        }

        // Find the lowest power of two that is greater than or equal, doing this for width and height
        auto power_of_twoafy = [](auto number) {
            for(std::uint32_t p = 0; p < sizeof(number)*4-1; p++) {
                std::uint32_t powered = static_cast<std::uint32_t>(1 << p);
                if(powered >= number) {
                    return powered;
                }
            }
            std::terminate();
        };

        max_width = power_of_twoafy(max_width);
        max_height = power_of_twoafy(max_height);

        if(max_width > max_height) {
            return max_width;
        }
        else {
            return max_height;
        }
    }

    static std::optional<std::vector<GeneratedBitmapDataSequence>> fit_sprites_into_sprite_sheet(std::uint32_t length, const GeneratedBitmapData &generated_bitmap, std::uint32_t half_spacing, std::uint32_t maximum_sprite_sheets, bool horizontal) {
        // Effectively, all sprites are this many pixels apart
        std::uint32_t effective_sprite_spacing = half_spacing * 2;

        // If it's impossible to fit even a single pixel, give up
        if(length <= effective_sprite_spacing) {
            return std::nullopt;
        }

        // First see if all sprites can even fit by themselves. If not, there is no point in continuing.
        std::size_t total_pixels = 0;
        for(auto &bitmap : generated_bitmap.bitmaps) {
            if(bitmap.height + effective_sprite_spacing > length || bitmap.width + effective_sprite_spacing > length) {
                return std::nullopt;
            }
            total_pixels += (bitmap.height + effective_sprite_spacing) * (bitmap.width + effective_sprite_spacing);
        }

        // Also, if the number of pixels is greater than length^2, there is no way we could fit everything in here
        if(total_pixels > length * length * maximum_sprite_sheets) {
            return std::nullopt;
        }

        std::uint32_t sheet_count = 1;
        std::vector<GeneratedBitmapDataSequence> new_sequences;

        struct SortedSprite {
            std::uint32_t sheet_index;
            std::uint32_t bitmap_index;
            std::uint32_t sequence_index;
            std::uint32_t sequence_sprite_index;
            std::uint32_t length;

            std::uint32_t width;
            std::uint32_t height;

            std::uint32_t top;
            std::uint32_t left;
            std::uint32_t bottom;
            std::uint32_t right;

            std::uint32_t registration_point_x;
            std::uint32_t registration_point_y;
        };

        // Sort each sprite by height or width depending on if sorting horizontally or vertically. The first element of the pair is the bitmap index and the second element is the length
        std::vector<SortedSprite> sprites_sorted;
        const auto *bitmaps = generated_bitmap.bitmaps.data();
        for(std::uint32_t s = 0; s < generated_bitmap.sequences.size(); s++) {
            // Go through each bitmap in the sequence
            auto &sequence = generated_bitmap.sequences[s];
            const auto FIRST_BITMAP = sequence.first_bitmap;
            const auto END_BITMAP = FIRST_BITMAP + sequence.bitmap_count;
            for(std::uint32_t b = FIRST_BITMAP; b < END_BITMAP; b++) {
                auto &bitmap = bitmaps[b];
                std::uint32_t length;
                length = horizontal ? bitmap.height : bitmap.width;

                // Create it!
                SortedSprite sprite_to_add = {};
                sprite_to_add.bitmap_index = b;
                sprite_to_add.length = length;
                sprite_to_add.sequence_sprite_index = b - FIRST_BITMAP;
                sprite_to_add.sequence_index = s;

                sprite_to_add.width = bitmap.width;
                sprite_to_add.height = bitmap.height;
                sprite_to_add.registration_point_x = bitmap.registration_point_x;
                sprite_to_add.registration_point_y = bitmap.registration_point_y;

                // Find a sprite that's smaller and add it before that, stopping when we reach the end of the array
                auto sprite_iter = sprites_sorted.begin();
                for(; sprite_iter < sprites_sorted.end(); sprite_iter++) {
                    if(sprite_iter->length < length) {
                        break;
                    }
                }
                sprites_sorted.insert(sprite_iter, sprite_to_add);
            }
        }

        // Now that it's all sorted, begin placing things. If sorting by height, then scan vertically. Otherwise scan horizontally
        for(std::size_t sprite = 0; sprite < sprites_sorted.size(); sprite++) {
            auto &sprite_fitting = sprites_sorted[sprite];

            for(std::uint32_t sheet = 0; sheet < sheet_count; sheet++) {
                auto fits = [&sprite_fitting, &sprite, &sprites_sorted, &sheet, &half_spacing, &length](std::uint32_t x, std::uint32_t y) -> bool {
                    // Calculate the top/left/bottom-1/right-1, factoring in spacing
                    std::uint32_t potential_top = y - half_spacing;
                    std::uint32_t potential_left = x - half_spacing;
                    std::uint32_t potential_bottom = y + sprite_fitting.height + half_spacing - 1;
                    std::uint32_t potential_right = x + sprite_fitting.width + half_spacing - 1;

                    // If we're outside the bitmap, fail
                    if(length <= potential_right || length <= potential_bottom) {
                        return false;
                    }

                    for(std::uint32_t sprite_test = 0; sprite_test < sprite; sprite_test++) {
                        auto sprite_test_value = sprites_sorted[sprite_test];

                        // If we aren't even on the same bitmap, ignore
                        if(sheet != sprite_test_value.sheet_index) {
                            continue;
                        }

                        // Get the sprite we're comparing's top/left/bottom-1/right-1, also factoring in spacing
                        std::uint32_t compare_top = sprite_test_value.top - half_spacing;
                        std::uint32_t compare_left = sprite_test_value.left - half_spacing;
                        std::uint32_t compare_bottom = sprite_test_value.bottom + half_spacing - 1;
                        std::uint32_t compare_right = sprite_test_value.right + half_spacing - 1;

                        auto box_intersects_box = [](
                            std::uint32_t box_a_top,
                            std::uint32_t box_a_left,
                            std::uint32_t box_a_bottom,
                            std::uint32_t box_a_right,
                            std::uint32_t box_b_top,
                            std::uint32_t box_b_left,
                            std::uint32_t box_b_bottom,
                            std::uint32_t box_b_right
                        ) {
                            bool top_inside = box_a_top >= box_b_top && box_a_top <= box_b_bottom;
                            bool bottom_inside = box_a_bottom >= box_b_top && box_a_bottom <= box_b_bottom;

                            bool left_inside = box_a_left >= box_b_left && box_a_left <= box_b_right;
                            bool right_inside = box_a_right >= box_b_left && box_a_right <= box_b_right;

                            bool wider = box_a_left <= box_b_left && box_a_right >= box_b_right;
                            bool taller = box_a_top <= box_b_top && box_a_bottom >= box_b_bottom;

                            // If two perpendicular sides are inside, then that means a corner is inside, which means it's banned
                            if((top_inside && left_inside) || (bottom_inside && left_inside) || (top_inside && right_inside) || (bottom_inside && right_inside)) {
                                return true;
                            }

                            // If the box is wider or taller and the adjacent side is inside, then that means it's banned.
                            if((wider && top_inside) || (wider && bottom_inside) || (taller & left_inside) || (taller && right_inside)) {
                                return true;
                            }

                            return false;
                        };

                        if(box_intersects_box(potential_top, potential_left, potential_bottom, potential_right, compare_top, compare_left, compare_bottom, compare_right) || box_intersects_box(compare_top, compare_left, compare_bottom, compare_right, potential_top, potential_left, potential_bottom, potential_right)) {
                            return false;
                        }
                    }

                    return true;
                };

                // Coordinates
                std::optional<std::pair<std::uint32_t,std::uint32_t>> coordinates;

                std::uint32_t max_x = length - sprite_fitting.width - half_spacing;
                std::uint32_t max_y = length - sprite_fitting.height - half_spacing;

                if(horizontal) {
                    for(std::uint32_t y = half_spacing; y <= max_y && !coordinates.has_value(); y++) {
                        for(std::uint32_t x = half_spacing; x <= max_x && !coordinates.has_value(); x++) {
                            if(fits(x,y)) {
                                coordinates = std::pair<std::uint32_t,std::uint32_t>(x,y);
                            }
                        }
                    }
                }
                else {
                    for(std::uint32_t x = half_spacing; x <= max_x && !coordinates.has_value(); x++) {
                        for(std::uint32_t y = half_spacing; y <= max_y && !coordinates.has_value(); y++) {
                            if(fits(x,y)) {
                                coordinates = std::pair<std::uint32_t,std::uint32_t>(x,y);
                            }
                        }
                    }
                }

                // Did we do it?
                if(coordinates.has_value()) {
                    sprite_fitting.sheet_index = sheet;

                    auto &x = coordinates.value().first;
                    auto &y = coordinates.value().second;

                    sprite_fitting.left = x;
                    sprite_fitting.top = y;
                    sprite_fitting.bottom = y + sprite_fitting.height;
                    sprite_fitting.right = x + sprite_fitting.width;

                    break;
                }

                // Try making a new sprite sheet. If we hit the maximum sprite sheets, give up.
                if(sheet + 1 == sheet_count) {
                    if(++sheet_count > maximum_sprite_sheets) {
                        return std::nullopt;
                    }
                }
            }
        }

        // Put it all together
        for(std::size_t s = 0; s < generated_bitmap.sequences.size(); s++) {
            auto &sequence = generated_bitmap.sequences[s];
            auto &new_sequence = new_sequences.emplace_back();
            new_sequence.bitmap_count = 0;
            new_sequence.first_bitmap = 0;
            new_sequence.y_end = sequence.y_end;
            new_sequence.y_start = sequence.y_start;

            // Find the sprite
            for(std::uint32_t b = 0; b < sequence.bitmap_count; b++) {
                for(auto &sprite : sprites_sorted) {
                    if(sprite.sequence_index == s && sprite.sequence_sprite_index == b) {
                        auto &new_sprite = new_sequence.sprites.emplace_back();
                        new_sprite.original_bitmap_index = sprite.bitmap_index;
                        new_sprite.bitmap_index = sprite.sheet_index;
                        new_sprite.top = sprite.top - half_spacing;
                        new_sprite.left = sprite.left - half_spacing;
                        new_sprite.bottom = sprite.bottom + half_spacing;
                        new_sprite.right = sprite.right + half_spacing;
                        new_sprite.registration_point_x = sprite.registration_point_x + half_spacing;
                        new_sprite.registration_point_y = sprite.registration_point_y + half_spacing;

                        break;
                    }
                }
            }
        }

        // Done!
        return new_sequences;
    }

    static std::optional<std::vector<GeneratedBitmapDataSequence>> fit_sprites_into_maximum_sprite_sheet(std::uint32_t length, const GeneratedBitmapData &generated_bitmap, std::uint32_t half_spacing, std::uint32_t maximum_sprite_sheets) {
        auto fit_sprites_vertical = fit_sprites_into_sprite_sheet(length, generated_bitmap, half_spacing, maximum_sprite_sheets, false);
        auto fit_sprites_horizontal = fit_sprites_into_sprite_sheet(length, generated_bitmap, half_spacing, maximum_sprite_sheets, true);
        std::optional<std::vector<GeneratedBitmapDataSequence>> fit_sprites;
        if(fit_sprites_vertical.has_value()) {
            fit_sprites = fit_sprites_vertical;
        }
        else if(fit_sprites_horizontal.has_value()) {
            fit_sprites = fit_sprites_horizontal;
        }
        else {
            return std::nullopt;
        }

        auto fit_sprites_half = fit_sprites_into_maximum_sprite_sheet(length / 2, generated_bitmap, half_spacing, maximum_sprite_sheets);
        if(fit_sprites_half) {
            return fit_sprites_half;
        }
        else {
            return fit_sprites;
        }
    }

    void ColorPlateScanner::process_sprites(GeneratedBitmapData &generated_bitmap, ColorPlateScannerSpriteParameters &parameters, std::int16_t &mipmap) {
        // Pick the background color of the sprite sheet
        ColorPlatePixel background_color;
        switch(parameters.sprite_usage) {
            case BitmapSpriteUsage::BITMAP_SPRITE_USAGE_BLEND_ADD_SUBTRACT_MAX:
                background_color.alpha = 0;
                background_color.red = 0;
                background_color.green = 0;
                background_color.blue = 0;
                break;
            case BitmapSpriteUsage::BITMAP_SPRITE_USAGE_DOUBLE_MULTIPLY:
                background_color.alpha = 255;
                background_color.red = 127;
                background_color.green = 127;
                background_color.blue = 127;
                break;
            case BitmapSpriteUsage::BITMAP_SPRITE_USAGE_MULTIPLY_MIN:
                background_color.alpha = 255;
                background_color.red = 255;
                background_color.green = 255;
                background_color.blue = 255;
                break;
            case BitmapSpriteUsage::BITMAP_SPRITE_USAGE_ENUM_COUNT:
                std::terminate();
        }

        // Get the max budget of the sprite sheet. If none is given, automatically choose a large budget
        std::uint32_t max_budget = parameters.sprite_budget;
        std::uint32_t max_sheet_count = parameters.sprite_budget_count;
        if(max_sheet_count == 0) {
            max_budget = 16384;
            max_sheet_count = 1;
        }

        // Set the spacing based on the mipmap count
        std::uint32_t half_spacing;
        switch(mipmap) {
            case 1:
                half_spacing = 2;
                break;

            case 0:
                half_spacing = 1;
                break;

            default:
                mipmap = 2;
                // fallthrough

            case 2:
                half_spacing = 4;
                break;
        }

        // First see if we can even fit things into this
        auto fit_sprites = fit_sprites_into_maximum_sprite_sheet(max_budget, generated_bitmap, half_spacing, max_sheet_count);
        if(!fit_sprites.has_value()) {
            eprintf_error("Error: Unable to fit sprites into %u %ux%u sprite sheet%s.\n", max_sheet_count, max_budget, max_budget, max_sheet_count == 1 ? "" : "s");
            throw InvalidInputBitmapException();
        }
        parameters.sprite_spacing = half_spacing;

        auto &sprites_fit = fit_sprites.value();
        std::uint32_t sheet_count = number_of_sprite_sheets(sprites_fit);
        std::vector<GeneratedBitmapDataBitmap> new_bitmaps;

        for(std::uint32_t s = 0; s < sheet_count; s++) {
            auto sheet_length = length_of_sprite_sheet(sprites_fit, s);

            // Initialize the new bitmap
            const std::uint32_t SHEET_WIDTH = sheet_length;
            const std::uint32_t SHEET_HEIGHT = sheet_length;
            auto &new_bitmap = new_bitmaps.emplace_back();
            new_bitmap.color_plate_x = 0;
            new_bitmap.color_plate_y = 0;
            new_bitmap.registration_point_x = 0;
            new_bitmap.registration_point_y = 0;
            new_bitmap.width = SHEET_WIDTH;
            new_bitmap.height = SHEET_HEIGHT;
            new_bitmap.pixels.insert(new_bitmap.pixels.begin(), SHEET_WIDTH * SHEET_HEIGHT, background_color);

            // Put the sprites on the bitmap
            for(std::uint32_t sequence_index = 0; sequence_index < generated_bitmap.sequences.size(); sequence_index++) {
                auto &color_plate_sequence = generated_bitmap.sequences[sequence_index];
                auto &sprite_sequence = sprites_fit[sequence_index];

                if(color_plate_sequence.bitmap_count != sprite_sequence.sprites.size()) {
                    eprintf_error("Error: Color plate sequence bitmap count (%u) doesn't match up with sprite sequence (%zu).\n", color_plate_sequence.bitmap_count, sprite_sequence.sprites.size());
                    throw InvalidInputBitmapException();
                }

                // Go through each sprite and bake it in
                #define BAKE_SPRITE(fn) \
                for(auto &sprite : sprite_sequence.sprites) { \
                    if(sprite.bitmap_index == s) { \
                        auto &bitmap = generated_bitmap.bitmaps[sprite.original_bitmap_index]; \
                        const std::uint32_t SPRITE_WIDTH = bitmap.width; \
                        const std::uint32_t SPRITE_HEIGHT = bitmap.height; \
                        for(std::uint32_t y = 0; y < SPRITE_HEIGHT; y++) { \
                            for(std::uint32_t x = 0; x < SPRITE_WIDTH; x++) { \
                                const auto &input = bitmap.pixels[y * SPRITE_WIDTH + x]; \
                                auto &output = new_bitmap.pixels[(y + sprite.top + half_spacing) * SHEET_WIDTH + (x + sprite.left + half_spacing)]; \
                                output = output.fn(input); \
                            } \
                        } \
                    } \
                }

                // If it's multiply, do an alpha blend. Otherwise it just replaces the pixel.
                if(parameters.sprite_usage == BitmapSpriteUsage::BITMAP_SPRITE_USAGE_MULTIPLY_MIN) {
                    BAKE_SPRITE(alpha_blend);
                }
                else {
                    BAKE_SPRITE(replace);
                }
            }
        }

        generated_bitmap.bitmaps = std::move(new_bitmaps);
        generated_bitmap.sequences = std::move(sprites_fit);
    }
}
