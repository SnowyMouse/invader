/*
 * Invader (c) 2018 Kavawuvi
 *
 * This program is free software under the GNU General Public License v3.0 or later. See LICENSE for more information.
 */

#include <optional>

#include "color_plate_scanner.hpp"
#include "../eprintf.hpp"

namespace Invader {
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

    ScannedColorPlate ColorPlateScanner::scan_color_plate(const ColorPlatePixel *pixels, std::uint32_t width, std::uint32_t height, BitmapType type) {
        ColorPlateScanner scanner;
        ScannedColorPlate color_plate;

        bool power_of_two = (type != BitmapType::BITMAP_TYPE_SPRITES) && (type != BitmapType::BITMAP_TYPE_INTERFACE_BITMAPS);

        auto get_pixel = [&pixels, &width](std::uint32_t x, std::uint32_t y) -> const ColorPlatePixel & {
            return pixels[y * width + x];
        };

        auto pixel_is_blocked = [&color_plate](std::uint32_t x, std::uint32_t y, const ScannedColorPlateSequence &sequence) -> std::optional<std::uint32_t> {
            // Okay, let's see if we already got this pixel as a bitmap
            if(sequence.bitmap_count > 0) {
                // Go through each bitmap in the sequence
                const std::uint32_t FIRST_BITMAP = sequence.first_bitmap;
                const std::uint32_t END_BITMAP = FIRST_BITMAP + sequence.bitmap_count;
                for(std::uint32_t b = FIRST_BITMAP; b < END_BITMAP; b++) {
                    auto &bitmap = color_plate.bitmaps[b];

                    // If we are outside the x bounds or, somehow, our y is less than the bitmap, we obviously cannot be in this bitmap
                    const std::uint32_t BITMAP_START_X = bitmap.color_plate_x;
                    const std::uint32_t BITMAP_WIDTH = bitmap.width;
                    const std::uint32_t BITMAP_END_X = BITMAP_START_X + BITMAP_WIDTH;
                    const std::uint32_t BITMAP_START_Y = bitmap.color_plate_y;
                    if(x < BITMAP_START_X || x >= BITMAP_END_X || y < BITMAP_START_Y) {
                        continue;
                    }

                    // Go through each mipmap
                    const std::uint32_t BITMAP_MIPMAP_COUNT = bitmap.mipmaps;
                    std::uint32_t mip_width = BITMAP_WIDTH;
                    std::uint32_t mip_start_y = BITMAP_START_Y;
                    std::uint32_t mip_height = bitmap.height;
                    std::uint32_t mip_end_y = BITMAP_START_Y + mip_height;
                    for(std::uint32_t m = 0; m <= BITMAP_MIPMAP_COUNT; m++) {
                        const std::uint32_t mip_end_x = BITMAP_START_X + mip_width;
                        if(y < mip_end_y) {
                            if(x < mip_end_x) {
                                return mip_end_x;
                            }
                            else {
                                return std::optional<std::uint32_t>();
                            }
                        }

                        mip_start_y += mip_height;
                        mip_end_y += mip_height;
                        mip_height /= 2;
                        mip_width /= 2;
                    }
                }
            }

            return std::optional<std::uint32_t>();
        };

        // Check to see if we have valid color plate data. If so, look for sequences
        if(width >= 4 && height >= 2) {
            // Get the candidate pixels
            const auto &blue_candidate = pixels[0];
            const auto &magenta_candidate = pixels[1];
            const auto &cyan_candidate = pixels[2];
            scanner.valid_color_plate = true;

            // Make sure they aren't the same
            if(blue_candidate != magenta_candidate && blue_candidate != cyan_candidate && magenta_candidate != cyan_candidate) {
                // Let's assume for a moment they are. Is everything after the cyan pixel blue?
                for(std::uint32_t x = 3; x < width; x++) {
                    if(!same_color_ignore_opacity(get_pixel(x, 0), blue_candidate)) {
                        scanner.valid_color_plate = false;
                        break;
                    }
                }

                // Next, is there a sequence border immediately below this?
                if(scanner.valid_color_plate) {
                    for(std::uint32_t x = 0; x < width; x++) {
                        if(!same_color_ignore_opacity(get_pixel(x, 1), magenta_candidate)) {
                            scanner.valid_color_plate = false;
                            break;
                        }
                    }
                }

                // Okay, I'm convinced this is a valid color plate
                if(scanner.valid_color_plate) {
                    scanner.blue = blue_candidate;
                    scanner.magenta = magenta_candidate;
                    scanner.cyan = cyan_candidate;
                    auto *sequence = &color_plate.sequences.emplace_back();
                    sequence->y_start = 2;

                    // Next, we need to find all of the sequences
                    for(std::uint32_t y = 2; y < height; y++) {
                        bool sequence_border = true;
                        for(std::uint32_t x = 0; x < width; x++) {
                            if(!same_color_ignore_opacity(get_pixel(x, y), magenta_candidate)) {
                                sequence_border = false;
                                break;
                            }
                        }

                        // If we got it, create a new sequence, but not before terminating the last one
                        if(sequence_border) {
                            sequence->y_end = y;
                            sequence = &color_plate.sequences.emplace_back();
                            sequence->y_start = y + 1;
                        }
                    }

                    // Terminate the last sequence index
                    sequence->y_end = height;
                }
            }
            else {
                scanner.valid_color_plate = false;
            }

            if(!scanner.valid_color_plate) {
                // Create a default sequence
                auto &sequence = color_plate.sequences.emplace_back();
                sequence.y_start = 0;
                sequence.y_end = height;
                scanner.valid_color_plate = false;
            }
        }

        // Sprites require a color plate
        if(type == BitmapType::BITMAP_TYPE_SPRITES && !scanner.valid_color_plate) {
            eprintf("Error: Sprites require a valid color plate.\n");
            std::terminate();
        }

        static constexpr char ERROR_INVALID_BITMAP_WIDTH[] = "Error: Found a bitmap with an invalid width: %u\n";
        static constexpr char ERROR_INVALID_BITMAP_HEIGHT[] = "Error: Found a bitmap with an invalid height: %u\n";

        // If we have valid color plate data, use the color plate data
        if(scanner.valid_color_plate) {
            for(auto &sequence : color_plate.sequences) {
                sequence.first_bitmap = color_plate.bitmaps.size();
                sequence.bitmap_count = 0;

                // Search left and right for bitmap borders
                const std::uint32_t X_END = width;
                const std::uint32_t Y_START = sequence.y_start;
                const std::uint32_t Y_END = sequence.y_end;

                std::optional<std::uint32_t> bitmap_x_start;

                for(std::uint32_t x = 0; x < X_END; x++) {
                    for(std::uint32_t y = Y_START; y < Y_END; y++) {
                        auto &pixel = get_pixel(x,y);
                        // Basically, if it's not blue and not magenta, it's the start of a bitmap. Otherwise, it's the end of one if there is a bitmap
                        if(scanner.is_blue(pixel) || scanner.is_magenta(pixel)) {
                            // If we're in a bitmap, let's add it
                            if(y + 1 == Y_END && bitmap_x_start.has_value()) {
                                std::optional<std::uint32_t> min_x;
                                std::optional<std::uint32_t> max_x;
                                std::optional<std::uint32_t> min_y;
                                std::optional<std::uint32_t> max_y;

                                // Find the minimum x, y, max x, and max y stuff
                                for(std::uint32_t xb = bitmap_x_start.value(); xb < x; xb++) {
                                    for(std::uint32_t yb = Y_START; yb < Y_END; yb++) {
                                        auto &pixel = get_pixel(xb, yb);
                                        if(!scanner.is_ignored(pixel)) {
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
                                    }
                                }

                                // If we never got a minimum x, then give up on life
                                if(!min_x.has_value()) {
                                    eprintf("Error: Found a 0x0 bitmap.\n");
                                    std::terminate();
                                }

                                // Get the width and height
                                std::uint32_t width = max_x.value() - min_x.value() + 1;
                                std::uint32_t height = max_y.value() - min_y.value() + 1;

                                // If we require power-of-two, check
                                if(power_of_two) {
                                    if(!is_power_of_two(width)) {
                                        eprintf(ERROR_INVALID_BITMAP_WIDTH, width);
                                        std::terminate();
                                    }
                                    if(!is_power_of_two(height)) {
                                        eprintf(ERROR_INVALID_BITMAP_HEIGHT, height);
                                        std::terminate();
                                    }
                                }

                                // Add the bitmap
                                auto &bitmap = color_plate.bitmaps.emplace_back();
                                bitmap.width = width;
                                bitmap.height = height;
                                bitmap.color_plate_x = min_x.value();
                                bitmap.color_plate_y = min_y.value();
                                for(std::uint32_t by = min_y.value(); by <= max_y.value(); by++) {
                                    for(std::uint32_t bx = min_x.value(); bx <= max_x.value(); bx++) {
                                        auto &pixel = get_pixel(bx, by);
                                        if(scanner.is_ignored(pixel)) {
                                            bitmap.pixels.push_back(ColorPlatePixel {});
                                        }
                                        else {
                                            bitmap.pixels.push_back(pixel);
                                        }
                                    }
                                }

                                bitmap_x_start.reset();
                                sequence.bitmap_count++;
                            }

                            continue;
                        }

                        if(!bitmap_x_start.has_value()) {
                            bitmap_x_start = x;
                        }
                        break;
                    }
                }
            }
        }

        // If it's a cubemap that isn't on a sprite sheet, try parsing it like this
        else if(type == BitmapType::BITMAP_TYPE_CUBE_MAPS) {
            // Make sure the height and width of each face is the same
            std::uint32_t face_width = width / 4;
            std::uint32_t face_height = height / 3;

            if(face_height != face_width || !is_power_of_two(face_width) || face_width < 1 || face_width * 4 != width || face_height * 3 != height) {
                eprintf("Invalid cubemap input dimensions %ux%u.\n", face_width, face_height);
                std::terminate();
            }

            auto get_pixel_transformed = [&get_pixel, &face_width](std::uint32_t left, std::uint32_t top, std::uint32_t relative_x, std::uint32_t relative_y, std::uint32_t rotation) {
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
                return get_pixel(x, y);
            };

            auto add_bitmap = [&color_plate, &get_pixel_transformed, &face_width](std::uint32_t left, std::uint32_t top, std::uint32_t rotation) {
                auto &new_bitmap = color_plate.bitmaps.emplace_back();
                new_bitmap.color_plate_x = left;
                new_bitmap.color_plate_y = top;
                new_bitmap.height = face_width;
                new_bitmap.width = face_width;
                new_bitmap.mipmaps = 0;
                new_bitmap.pixels.reserve(face_width * face_width);

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
            color_plate.sequences[0].first_bitmap = 0;
            color_plate.sequences[0].bitmap_count = 6;
        }

        // Otherwise, look for bitmaps up, down, left, and right
        else {
            auto &sequence = color_plate.sequences[0];
            sequence.first_bitmap = color_plate.bitmaps.size();
            sequence.bitmap_count = 0;

            // Go through each pixel to find a bitmap
            const std::uint32_t X_END = width;
            const std::uint32_t Y_END = height;

            for(std::uint32_t y = 0; y < Y_END; y++) {
                for(std::uint32_t x = 0; x < width; x++) {
                    auto &pixel = get_pixel(x,y);

                    // Found?
                    if(!scanner.is_ignored(pixel)) {
                        auto blocked = pixel_is_blocked(x, y, sequence);

                        // If we should ignore this, set the x to the end of this bitmap (minus one for the iterator)
                        if(blocked.has_value()) {
                            x = blocked.value() - 1;
                            continue;
                        }

                        auto get_bitmap_mipmaps = [&scanner, &get_pixel, &X_END, &Y_END, &power_of_two](std::uint32_t x, std::uint32_t y, std::uint32_t &width, std::uint32_t &height, auto &get_bitmap_mipmaps_recursion, std::uint32_t *expected_width = nullptr, std::uint32_t *expected_height = nullptr) -> std::int32_t {
                            // Find the width
                            std::uint32_t x2;
                            for(x2 = x; x2 < X_END && !scanner.is_ignored(get_pixel(x2,y)); x2++);
                            width = x2 - x;
                            if(width < 1 || (power_of_two && !is_power_of_two(width))) {
                                eprintf(ERROR_INVALID_BITMAP_WIDTH, width);
                                return -1;
                            }

                            // Find the height
                            std::uint32_t y2;
                            for(y2 = y; y2 < Y_END && !scanner.is_ignored(get_pixel(x2 - 1,y2)); y2++);
                            height = y2 - y;
                            if(height < 1 || (power_of_two && !is_power_of_two(height))) {
                                eprintf(ERROR_INVALID_BITMAP_HEIGHT, height);
                                return -1;
                            }

                            // Make sure it's correct
                            if(expected_height && *expected_height != height) {
                                eprintf("Error: Found a bitmap with an invalid mipmap height: %u; Expected %u\n", height, *expected_height);
                                return -1;
                            }

                            // Make sure it's correct
                            if(expected_width && *expected_width != width) {
                                eprintf("Error: Found a bitmap with an invalid mipmap width: %u; Expected %u\n", width, *expected_width);
                                return -1;
                            }

                            // Make sure there's nothing along the right edge
                            static constexpr char TOO_CLOSE_ERROR[] = "Error: Found a bitmap that is too close to another one.\n";
                            if(x2 < X_END) {
                                for(std::uint32_t ym = y; ym < y2; ym++) {
                                    if(!scanner.is_ignored(get_pixel(x2, ym))) {
                                        eprintf(TOO_CLOSE_ERROR);
                                        return -1;
                                    }
                                }
                            }

                            // If width or height are 1 or y2 is the edge, there are no more mipmaps
                            if(width == 1 || height == 1 || y2 == Y_END) {
                                return 0;
                            }

                            // Next, we should check if there's an explicit mipmap. If we aren't looking for power of two, then there cannot be any explicit mipmap
                            bool potential_mipmap;
                            std::uint32_t xm;
                            if(!power_of_two || scanner.is_ignored(get_pixel(x, y2))) {
                                potential_mipmap = false;
                                xm = x + 1;
                            }
                            else {
                                potential_mipmap = true;
                                xm = x + width / 2;
                            }

                            // Check the bottom edge
                            for(; xm < x2; xm++) {
                                if(!scanner.is_ignored(get_pixel(xm, y2))) {
                                    eprintf(TOO_CLOSE_ERROR);
                                    return -1;
                                }
                            }

                            // Lastly, check for mipmaps if needed
                            if(potential_mipmap) {
                                std::uint32_t w,h;
                                std::uint32_t ew = width / 2;
                                std::uint32_t eh = height / 2;
                                std::int32_t mipmap_count = get_bitmap_mipmaps_recursion(x, y2, w, h, get_bitmap_mipmaps_recursion, &ew, &eh);
                                if(mipmap_count == -1) {
                                    return -1;
                                }
                                else {
                                    return 1 + mipmap_count;
                                }
                            }
                            else {
                                return 0;
                            }
                        };

                        // Pull in the mipmap and bitmap stuff
                        std::uint32_t bitmap_width, bitmap_height;
                        std::int32_t mipmap_count = get_bitmap_mipmaps(x, y, bitmap_width, bitmap_height, get_bitmap_mipmaps);
                        if(mipmap_count < 0) {
                            std::terminate();
                        }

                        // Store that in a bitmap
                        auto &bitmap = color_plate.bitmaps.emplace_back();
                        bitmap.color_plate_x = x;
                        bitmap.color_plate_y = y;
                        bitmap.mipmaps = static_cast<std::uint32_t>(mipmap_count);
                        bitmap.width = bitmap_width;
                        bitmap.height = bitmap_height;

                        // Store bitmap data, too
                        std::uint32_t my = y;
                        std::uint32_t mw = bitmap_width;
                        std::uint32_t mh = bitmap_height;
                        for(std::uint32_t m = 0; m <= mipmap_count; m++) {
                            const std::uint32_t m_end_y = my + mh;
                            for(; my < m_end_y; my++) {
                                auto *pixel = &get_pixel(x, my);
                                bitmap.pixels.insert(bitmap.pixels.end(), pixel, pixel + mw);
                            }

                            // Next mipmap - half the resolution
                            mw /= 2;
                            mh /= 2;
                        }

                        sequence.bitmap_count++;
                    }
                }
            }
        }

        // Go through each sequence and check to make sure they have 6 bitmaps (if a cubemap)
        if(type == BitmapType::BITMAP_TYPE_CUBE_MAPS) {
            for(auto &sequence : color_plate.sequences) {
                if(sequence.bitmap_count != 6) {
                    eprintf("Cubemaps must have exactly 6 bitmaps per sequence.\n");
                    std::terminate();
                }
            }
        }

        return color_plate;
    }

    bool ColorPlateScanner::is_blue(const ColorPlatePixel &color) const {
        return same_color_ignore_opacity(this->blue, color);
    }

    bool ColorPlateScanner::is_magenta(const ColorPlatePixel &color) const {
        return same_color_ignore_opacity(this->magenta, color);
    }

    bool ColorPlateScanner::is_cyan(const ColorPlatePixel &color) const {
        return same_color_ignore_opacity(this->cyan, color);
    }

    bool ColorPlateScanner::is_ignored(const ColorPlatePixel &color) const {
        return this->is_blue(color) || (this->valid_color_plate && (this->is_cyan(color) || this->is_magenta(color)));
    }
}
