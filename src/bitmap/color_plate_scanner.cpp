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

    static inline bool power_of_two(std::uint32_t number) {
        std::uint32_t ones = 0;
        while(number > 0) {
            ones += number & 1;
            number >>= 1;
        }
        return ones <= 1;
    }

    ScannedColorPlate ColorPlateScanner::scan_color_plate(const ColorPlatePixel *pixels, std::uint32_t width, std::uint32_t height, bool sprites) {
        ColorPlateScanner scanner;
        ScannedColorPlate color_plate;

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

        // If we're getting sprites, make sure we're getting valid stuff
        if(sprites && scanner.valid_color_plate == false) {
            eprintf("Sprites require valid color plate data.\n");
            return color_plate;
        }

        // Go through each sequence
        for(auto &sequence : color_plate.sequences) {
            sequence.first_bitmap = color_plate.bitmaps.size();
            sequence.bitmap_count = 0;

            // Go through each pixel to find a bitmap
            const std::uint32_t X_END = width;
            const std::uint32_t Y_END = sequence.y_end;
            for(std::uint32_t y = sequence.y_start; y < Y_END; y++) {
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

                        // Looks like we don't need to do that
                        eprintf("Found a pixel! (%u, %u)\n", x, y);

                        auto get_bitmap_mipmaps = [&scanner, &get_pixel, &X_END, &Y_END](std::uint32_t x, std::uint32_t y, std::uint32_t &width, std::uint32_t &height, auto &get_bitmap_mipmaps_recursion, std::uint32_t *expected_width = nullptr, std::uint32_t *expected_height = nullptr) -> std::int32_t {
                            // Find the width
                            std::uint32_t x2;
                            for(x2 = x; x2 < X_END && !scanner.is_ignored(get_pixel(x2,y)); x2++);
                            width = x2 - x;
                            if(width < 1 || !power_of_two(width)) {
                                eprintf("Error: Found a bitmap with an invalid width: %u; Must be a non-zero power of two\n", width);
                                return -1;
                            }

                            // Find the height
                            std::uint32_t y2;
                            for(y2 = y; y2 < Y_END && !scanner.is_ignored(get_pixel(x2 - 1,y2)); y2++);
                            height = y2 - y;
                            if(height < 1 || !power_of_two(height)) {
                                eprintf("Error: Found a bitmap with an invalid height: %u; Must be a non-zero power of two\n", height);
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

                            // Next, we should check if there's an explicit mipmap
                            bool potential_mipmap;
                            std::uint32_t xm;
                            if(scanner.is_ignored(get_pixel(x, y2))) {
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

                        eprintf("Dimensions: %u x %u, %i mips\n", bitmap_width, bitmap_height, mipmap_count);

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

        // Output debugging info
        eprintf("Dimensions:        %u x %u\n", width, height);
        eprintf("Valid color plate: %s\n", scanner.valid_color_plate ? "true" : "false");
        eprintf("Sequences:         %zu\n", color_plate.sequences.size());
        for(auto &sequence : color_plate.sequences) {
            eprintf("    Sequence #%zu\n", &sequence - color_plate.sequences.data());
            eprintf("        Y:       %u-%u\n", sequence.y_start, sequence.y_end);
            std::uint32_t bitmap_end = sequence.first_bitmap + sequence.bitmap_count;
            eprintf("        Bitmaps: %u-%u\n", sequence.first_bitmap, bitmap_end);
            for(std::uint32_t b = sequence.first_bitmap; b < bitmap_end; b++) {
                eprintf("            Bitmap #%u\n", b);
                auto &bitmap = color_plate.bitmaps[b];
                eprintf("                Dimensions: %u x %u\n", bitmap.width, bitmap.height);
                eprintf("                Position:   %u , %u\n", bitmap.color_plate_x, bitmap.color_plate_y);
                eprintf("                Mipmaps:    %u\n", bitmap.mipmaps);
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
