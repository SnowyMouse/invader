// SPDX-License-Identifier: GPL-3.0-only

#include <optional>
#include <algorithm>

#include <invader/hek/data_type.hpp>
#include <invader/bitmap/color_plate_scanner.hpp>
#include <invader/printf.hpp>

namespace Invader {
    static constexpr char ERROR_INVALID_BITMAP_WIDTH[] = "Error: Found a bitmap with an invalid width: %u\n";
    static constexpr char ERROR_INVALID_BITMAP_HEIGHT[] = "Error: Found a bitmap with an invalid height: %u\n";

    template <typename T> inline constexpr T divide_by_two_round(T value) {
        return (value / 2) + (value & 1);
    }

    static inline bool same_color_ignore_opacity(const Pixel &color_a, const Pixel &color_b) {
        return color_a.red == color_b.red && color_a.blue == color_b.blue && color_a.green == color_b.green;
    }

    #define GET_PIXEL(x,y) (pixels[y * width + x])

    GeneratedBitmapData ColorPlateScanner::scan_color_plate(const Pixel *pixels, std::uint32_t width, std::uint32_t height, BitmapType type, BitmapUsage usage) {
        // We don't support this yet
        if(usage == BitmapUsage::BITMAP_USAGE_VECTOR_MAP) {
            eprintf_error("Vector maps are not supported at this time");
            throw std::exception();
        }
        
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
                }
                
                // If there's no sequence divider color, check if we're blue. If not, we're not a valid key (treat as one bitmap)
                else if(!scanner.is_transparency_color(Pixel { 0xFF, 0x00, 0x00, 0xFF } )) {
                    valid_color_plate_key = false;
                }
                
                // Otherwise, it's valid, but we have to look for sequences based on seeing if a horizontal line is fully blue or not
                //
                // Basically, transparency IS the sequence divider
                else {
                    // start_y has a value whenever we're inside a sequence
                    std::optional<std::size_t> start_y;
                    
                    auto break_off_sequence = [&start_y, &generated_bitmap](std::size_t y) {
                        if(start_y.has_value()) {
                            auto &sequences = generated_bitmap.sequences.emplace_back();
                            sequences.y_start = *start_y;
                            sequences.y_end = y;
                            start_y = std::nullopt;
                        }
                        else {
                            start_y = y;
                        }
                    };
                    
                    for(std::size_t y = 1; y < height; y++) {
                        bool all_blue = true;
                        for(std::size_t x = 0; x < width; x++) {
                            if(!scanner.is_transparency_color(GET_PIXEL(x,y))) {
                                all_blue = false;
                                break;
                            }
                        }
                        
                        // If it's all blue and we're in a sequence, then the sequence has ended
                        if(all_blue == start_y.has_value()) {
                            break_off_sequence(y);
                        }
                    }
                    
                    break_off_sequence(height);
                    
                    scanner.spacing_color = Pixel { 0xFF, 0xFF, 0x00, 0xFF };
                    
                    if(!scanner.is_transparency_color(spacing_candidate) && !scanner.is_spacing_color(spacing_candidate)) {
                        eprintf_error("Error: Spacing color, if set, can only be #00FFFF if sequence divider is not set");
                        throw InvalidInputBitmapException();
                    }
                }
            }
            
            // If we still have a valid color plate key and we don't have sequences, find them
            if(valid_color_plate_key && !generated_bitmap.sequences.size()) {
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
            
            // Is it still valid? If so, check spacing
            if(valid_color_plate_key && !scanner.spacing_color.has_value() && !same_color_ignore_opacity(transparency_candidate, spacing_candidate)) {
                scanner.spacing_color = spacing_candidate;
                
                // Make sure it's valid!
                if(same_color_ignore_opacity(separator_candidate, spacing_candidate)) {
                    eprintf_error("Spacing and sequence divider colors must not match");
                    throw InvalidInputBitmapException();
                }
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

        return generated_bitmap;
    }

    void ColorPlateScanner::read_color_plate(GeneratedBitmapData &generated_bitmap, const Pixel *pixels, std::uint32_t width) const {
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
                        if(!HEK::is_power_of_two(bitmap_width)) {
                            eprintf(ERROR_INVALID_BITMAP_WIDTH, bitmap_width);
                            throw InvalidInputBitmapException();
                        }
                        if(!HEK::is_power_of_two(bitmap_height)) {
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
                                bitmap.pixels.push_back(Pixel {});
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

    void ColorPlateScanner::read_unrolled_cubemap(GeneratedBitmapData &generated_bitmap, const Pixel *pixels, std::uint32_t width, std::uint32_t height) const {
        // Make sure the height and width of each face is the same
        std::uint32_t face_width = width / 4;
        std::uint32_t face_height = height / 3;

        if(face_height != face_width || !HEK::is_power_of_two(face_width) || face_width < 1 || face_width * 4 != width || face_height * 3 != height) {
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

    void ColorPlateScanner::read_single_bitmap(GeneratedBitmapData &generated_bitmap, const Pixel *pixels, std::uint32_t width, std::uint32_t height) const {
        if(generated_bitmap.type != BitmapType::BITMAP_TYPE_INTERFACE_BITMAPS) {
            if(!HEK::is_power_of_two(width)) {
                eprintf(ERROR_INVALID_BITMAP_WIDTH, width);
                throw InvalidInputBitmapException();
            }
            if(!HEK::is_power_of_two(height)) {
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

    bool ColorPlateScanner::is_transparency_color(const Pixel &color) const {
        return this->transparency_color.has_value() && same_color_ignore_opacity(*this->transparency_color, color);
    }

    bool ColorPlateScanner::is_sequence_divider_color(const Pixel &color) const {
        return this->sequence_divider_color.has_value() && same_color_ignore_opacity(*this->sequence_divider_color, color);
    }

    bool ColorPlateScanner::is_spacing_color(const Pixel &color) const {
        return this->spacing_color.has_value() && same_color_ignore_opacity(*this->spacing_color, color);
    }

    bool ColorPlateScanner::is_ignored(const Pixel &color) const {
        return this->is_transparency_color(color) || this->is_spacing_color(color) || this->is_sequence_divider_color(color);
    }
}
