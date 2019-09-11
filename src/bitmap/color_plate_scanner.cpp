/*
 * Invader (c) 2018 Kavawuvi
 *
 * This program is free software under the GNU General Public License v3.0 or later. See LICENSE for more information.
 */

#include <optional>

#include "color_plate_scanner.hpp"
#include "../eprintf.hpp"

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

    GeneratedBitmapData ColorPlateScanner::scan_color_plate(const ColorPlatePixel *pixels, std::uint32_t width, std::uint32_t height, BitmapType type, const std::optional<ColorPlateScannerSpriteParameters> &sprite_parameters, std::int16_t mipmaps, ScannedColorMipmapType mipmap_type, float mipmap_fade_factor) {
        ColorPlateScanner scanner;
        GeneratedBitmapData generated_bitmap;

        generated_bitmap.type = type;
        scanner.power_of_two = (type != BitmapType::BITMAP_TYPE_SPRITES) && (type != BitmapType::BITMAP_TYPE_INTERFACE_BITMAPS);

        if(width == 0 || height == 0) {
            return generated_bitmap;
        }

        // Check to see if we have valid color plate data. If so, look for sequences
        if(width >= 4 && height >= 2) {
            // Get the candidate pixels
            const auto &blue_candidate = pixels[0];
            const auto &magenta_candidate = pixels[1];
            const auto &cyan_candidate = pixels[2];

            scanner.valid_color_plate = true;

            // Make sure they aren't the same
            if(!same_color_ignore_opacity(blue_candidate, magenta_candidate) && !same_color_ignore_opacity(magenta_candidate, cyan_candidate)) {
                // Let's assume for a moment they are. Is everything after the cyan pixel blue?
                for(std::uint32_t x = 3; x < width; x++) {
                    if(!same_color_ignore_opacity(GET_PIXEL(x, 0), blue_candidate)) {
                        scanner.valid_color_plate = false;
                        break;
                    }
                }

                // Is the cyan pixel blue? If it is, that's okay, but we can't use the cyan pixel
                bool ignore_cyan = same_color_ignore_opacity(blue_candidate, cyan_candidate);

                // Next, is there a sequence border immediately below this?
                if(scanner.valid_color_plate) {
                    for(std::uint32_t x = 0; x < width; x++) {
                        if(!same_color_ignore_opacity(GET_PIXEL(x, 1), magenta_candidate)) {
                            scanner.valid_color_plate = false;
                            break;
                        }
                    }
                }

                // Okay, I'm convinced this is a valid color plate
                if(scanner.valid_color_plate) {
                    scanner.blue = blue_candidate;
                    scanner.magenta = magenta_candidate;

                    if(!ignore_cyan) {
                        scanner.cyan = cyan_candidate;
                    }

                    auto *sequence = &generated_bitmap.sequences.emplace_back();
                    sequence->y_start = 2;

                    // Next, we need to find all of the sequences
                    for(std::uint32_t y = 2; y < height; y++) {
                        bool sequence_border = true;
                        for(std::uint32_t x = 0; x < width; x++) {
                            if(!same_color_ignore_opacity(GET_PIXEL(x, y), magenta_candidate)) {
                                sequence_border = false;
                                break;
                            }
                        }

                        // If we got it, create a new sequence, but not before terminating the last one
                        if(sequence_border) {
                            sequence->y_end = y;
                            sequence = &generated_bitmap.sequences.emplace_back();
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
                auto &sequence = generated_bitmap.sequences.emplace_back();
                sequence.y_start = 0;
                sequence.y_end = height;
                scanner.valid_color_plate = false;
            }
        }

        // If we have valid color plate data, use the color plate data
        if(scanner.valid_color_plate) {
            scanner.read_color_plate(generated_bitmap, pixels, width);
        }

        // If it's a cubemap that isn't on a sprite sheet, try parsing it like this
        else if(type == BitmapType::BITMAP_TYPE_CUBE_MAPS) {
            std::uint32_t w = 4;
            std::uint32_t h = 3;

            while(w < width && h < height) {
                w <<= 1;
                h <<= 1;
            }

            if(w == width && h == height) {
                scanner.read_unrolled_cubemap(generated_bitmap, pixels, width, height);
            }
            else {
                scanner.read_non_color_plate(generated_bitmap, pixels, width, height);
            }
        }

        // Read a single bitmap if the first pixel is not blue
        else if(!scanner.is_blue(*pixels)) {
            scanner.read_single_bitmap(generated_bitmap, pixels, width, height);
        }

        // Otherwise, look for bitmaps up, down, left, and right
        else {
            scanner.read_non_color_plate(generated_bitmap, pixels, width, height);
        }

        // If the last sequence is empty, purge it
        if(generated_bitmap.sequences.size() > 0 && generated_bitmap.sequences[generated_bitmap.sequences.size() - 1].bitmap_count == 0) {
            generated_bitmap.sequences.erase(generated_bitmap.sequences.end() - 1);
        }

        // If we are doing sprites, we need to handle those now
        if(type == BitmapType::BITMAP_TYPE_SPRITES) {
            process_sprites(generated_bitmap, sprite_parameters.value());
        }

        // If we aren't making interface bitmaps, generate mipmaps when needed
        if(type != BitmapType::BITMAP_TYPE_INTERFACE_BITMAPS) {
            generate_mipmaps(generated_bitmap, mipmaps, mipmap_type, mipmap_fade_factor);
        }

        // If we're making cubemaps, we need to make all sides of each cubemap sequence one cubemap bitmap data. 3D textures work similarly
        if(type == BitmapType::BITMAP_TYPE_CUBE_MAPS || type == BitmapType::BITMAP_TYPE_3D_TEXTURES) {
            consolidate_stacked_bitmaps(generated_bitmap);
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

            std::optional<std::uint32_t> bitmap_x_start;

            for(std::uint32_t x = 0; x < X_END; x++) {
                for(std::uint32_t y = Y_START; y < Y_END; y++) {
                    auto &pixel = GET_PIXEL(x,y);
                    // Basically, if it's not blue and not magenta, it's the start of a bitmap. Otherwise, it's the end of one if there is a bitmap
                    if(this->is_blue(pixel) || this->is_magenta(pixel)) {
                        // If we're in a bitmap, let's add it
                        if(y + 1 == Y_END && bitmap_x_start.has_value()) {
                            std::optional<std::uint32_t> min_x;
                            std::optional<std::uint32_t> max_x;
                            std::optional<std::uint32_t> min_y;
                            std::optional<std::uint32_t> max_y;

                            // Find the minimum x, y, max x, and max y stuff
                            std::uint32_t xb;
                            for(xb = bitmap_x_start.value(); xb < x; xb++) {
                                for(std::uint32_t yb = Y_START; yb < Y_END; yb++) {
                                    auto &pixel = GET_PIXEL(xb, yb);

                                    // This is for anything that's not a cyan/magenta/blue pixel or zero-alpha if we're looking for sprites
                                    if(!this->is_ignored(pixel) && (generated_bitmap.type != BitmapType::BITMAP_TYPE_SPRITES || pixel.alpha > 0)) {
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
                            std::uint32_t bitmap_width = max_x.value() - min_x.value() + 1;
                            std::uint32_t bitmap_height = max_y.value() - min_y.value() + 1;

                            // If we require power-of-two, check
                            if(power_of_two) {
                                if(!is_power_of_two(bitmap_width)) {
                                    eprintf(ERROR_INVALID_BITMAP_WIDTH, bitmap_width);
                                    std::terminate();
                                }
                                if(!is_power_of_two(bitmap_height)) {
                                    eprintf(ERROR_INVALID_BITMAP_HEIGHT, bitmap_height);
                                    std::terminate();
                                }
                            }

                            // Add the bitmap
                            auto &bitmap = generated_bitmap.bitmaps.emplace_back();
                            bitmap.width = bitmap_width;
                            bitmap.height = bitmap_height;
                            bitmap.color_plate_x = min_x.value();
                            bitmap.color_plate_y = min_y.value();

                            // Calculate registration point.
                            const std::int32_t MID_X = divide_by_two_round(static_cast<std::int32_t>(xb + bitmap_x_start.value()));

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

    void ColorPlateScanner::read_unrolled_cubemap(GeneratedBitmapData &generated_bitmap, const ColorPlatePixel *pixels, std::uint32_t width, std::uint32_t height) const {
        // Make sure the height and width of each face is the same
        std::uint32_t face_width = width / 4;
        std::uint32_t face_height = height / 3;

        if(face_height != face_width || !is_power_of_two(face_width) || face_width < 1 || face_width * 4 != width || face_height * 3 != height) {
            eprintf("Invalid cubemap input dimensions %ux%u.\n", face_width, face_height);
            std::terminate();
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

    void ColorPlateScanner::read_non_color_plate(GeneratedBitmapData &generated_bitmap, const ColorPlatePixel *pixels, std::uint32_t width, std::uint32_t height) const {
        auto &sequence = generated_bitmap.sequences[0];
        sequence.first_bitmap = generated_bitmap.bitmaps.size();
        sequence.bitmap_count = 0;

        // Go through each pixel to find a bitmap
        const std::uint32_t Y_END = height;

        auto pixel_is_blocked = [&generated_bitmap](std::uint32_t x, std::uint32_t y, const GeneratedBitmapDataSequence &sequence) -> std::optional<std::uint32_t> {
            // Okay, let's see if we already got this pixel as a bitmap
            if(sequence.bitmap_count > 0) {
                // Go through each bitmap in the sequence
                const std::uint32_t FIRST_BITMAP = sequence.first_bitmap;
                const std::uint32_t END_BITMAP = FIRST_BITMAP + sequence.bitmap_count;
                for(std::uint32_t b = FIRST_BITMAP; b < END_BITMAP; b++) {
                    auto &bitmap = generated_bitmap.bitmaps[b];

                    // If we are outside the x bounds or, somehow, our y is less than the bitmap, we obviously cannot be in this bitmap
                    const std::uint32_t BITMAP_START_X = bitmap.color_plate_x;
                    const std::uint32_t BITMAP_WIDTH = bitmap.width;
                    const std::uint32_t BITMAP_END_X = BITMAP_START_X + BITMAP_WIDTH;
                    const std::uint32_t BITMAP_START_Y = bitmap.color_plate_y;
                    if(x < BITMAP_START_X || x >= BITMAP_END_X || y < BITMAP_START_Y) {
                        continue;
                    }

                    // Go through each mipmap
                    const std::uint32_t BITMAP_MIPMAP_COUNT = bitmap.mipmaps.size();
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

        for(std::uint32_t y = 0; y < Y_END; y++) {
            for(std::uint32_t x = 0; x < width; x++) {
                auto &pixel = GET_PIXEL(x,y);

                // Found?
                if(!this->is_ignored(pixel)) {
                    auto blocked = pixel_is_blocked(x, y, sequence);

                    // If we should ignore this, set the x to the end of this bitmap (minus one for the iterator)
                    if(blocked.has_value()) {
                        x = blocked.value() - 1;
                        continue;
                    }

                    auto &power_of_two = this->power_of_two;
                    auto &scanner = *this;

                    auto get_bitmap_mipmaps = [&power_of_two, &scanner, &pixels, &width, &Y_END](std::uint32_t x, std::uint32_t y, std::uint32_t &mip_width, std::uint32_t &mip_height, auto &get_bitmap_mipmaps_recursion, std::uint32_t *expected_width = nullptr, std::uint32_t *expected_height = nullptr) -> std::uint32_t {
                        // Find the width
                        std::uint32_t x2;
                        for(x2 = x; x2 < width && !scanner.is_ignored(GET_PIXEL(x2,y)); x2++);
                        mip_width = x2 - x;
                        if(mip_width < 1 || (power_of_two && !is_power_of_two(mip_width))) {
                            eprintf(ERROR_INVALID_BITMAP_WIDTH, mip_width);
                            std::terminate();
                        }

                        // Find the height
                        std::uint32_t y2;
                        for(y2 = y; y2 < Y_END && !scanner.is_ignored(GET_PIXEL(x2 - 1,y2)); y2++);
                        mip_height = y2 - y;
                        if(mip_height < 1 || (power_of_two && !is_power_of_two(mip_height))) {
                            eprintf(ERROR_INVALID_BITMAP_HEIGHT, mip_height);
                            std::terminate();
                        }

                        // Make sure it's correct
                        if(expected_height && *expected_height != mip_height) {
                            eprintf("Error: Found a bitmap with an invalid mipmap height: %u; Expected %u\n", mip_height, *expected_height);
                            std::terminate();
                        }

                        // Make sure it's correct
                        if(expected_width && *expected_width != mip_width) {
                            eprintf("Error: Found a bitmap with an invalid mipmap width: %u; Expected %u\n", mip_width, *expected_width);
                            std::terminate();
                        }

                        // Make sure there's nothing along the right edge
                        static constexpr char TOO_CLOSE_ERROR[] = "Error: Found a bitmap that is too close to another one.\n";
                        if(x2 < width) {
                            for(std::uint32_t ym = y; ym < y2; ym++) {
                                if(!scanner.is_ignored(GET_PIXEL(x2, ym))) {
                                    eprintf(TOO_CLOSE_ERROR);
                                    std::terminate();
                                }
                            }
                        }

                        // If width or height are 1 or y2 is the edge, there are no more mipmaps
                        if(mip_width == 1 || mip_height == 1 || y2 == Y_END) {
                            return 0;
                        }

                        // Next, we should check if there's an explicit mipmap. If we aren't looking for power of two, then there cannot be any explicit mipmap
                        bool potential_mipmap;
                        std::uint32_t xm;
                        if(!power_of_two || scanner.is_ignored(GET_PIXEL(x, y2))) {
                            potential_mipmap = false;
                            xm = x + 1;
                        }
                        else {
                            potential_mipmap = true;
                            xm = x + width / 2;
                        }

                        // Check the bottom edge
                        for(; xm < x2; xm++) {
                            if(!scanner.is_ignored(GET_PIXEL(xm, y2))) {
                                eprintf(TOO_CLOSE_ERROR);
                                return {};
                            }
                        }

                        // Lastly, check for mipmaps if needed
                        if(potential_mipmap) {
                            std::uint32_t w,h;
                            std::uint32_t ew = mip_width / 2;
                            std::uint32_t eh = mip_height / 2;
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
                    std::uint32_t bitmap_width = 0, bitmap_height = 0;
                    auto mipmap_count = get_bitmap_mipmaps(x, y, bitmap_width, bitmap_height, get_bitmap_mipmaps);

                    // Store that in a bitmap
                    auto &bitmap = generated_bitmap.bitmaps.emplace_back();
                    bitmap.color_plate_x = x;
                    bitmap.color_plate_y = y;
                    bitmap.mipmaps = std::vector<GeneratedBitmapDataBitmapMipmap>();
                    bitmap.width = bitmap_width;
                    bitmap.height = bitmap_height;
                    bitmap.registration_point_x = divide_by_two_round(width);
                    bitmap.registration_point_y = divide_by_two_round(height);

                    // Store bitmap data, too
                    std::uint32_t my = y;
                    std::uint32_t mw = bitmap_width;
                    std::uint32_t mh = bitmap_height;
                    for(std::uint32_t m = 0; m <= mipmap_count; m++) {
                        if(m != 0) {
                            GeneratedBitmapDataBitmapMipmap mipmap = {};
                            mipmap.first_pixel = bitmap.pixels.size();
                            mipmap.mipmap_height = mh;
                            mipmap.mipmap_width = mw;
                            mipmap.pixel_count = mw * mh;
                            bitmap.mipmaps.push_back(mipmap);
                        }

                        const std::uint32_t m_end_y = my + mh;
                        for(; my < m_end_y; my++) {
                            auto *pixel = &GET_PIXEL(x, my);
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

    void ColorPlateScanner::read_single_bitmap(GeneratedBitmapData &generated_bitmap, const ColorPlatePixel *pixels, std::uint32_t width, std::uint32_t height) const {
        if(generated_bitmap.type != BitmapType::BITMAP_TYPE_INTERFACE_BITMAPS) {
            if(!is_power_of_two(width)) {
                eprintf(ERROR_INVALID_BITMAP_WIDTH, width);
                std::terminate();
            }
            if(!is_power_of_two(height)) {
                eprintf(ERROR_INVALID_BITMAP_WIDTH, height);
                std::terminate();
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

    bool ColorPlateScanner::is_blue(const ColorPlatePixel &color) const {
        return same_color_ignore_opacity(this->blue, color);
    }

    bool ColorPlateScanner::is_magenta(const ColorPlatePixel &color) const {
        return same_color_ignore_opacity(this->magenta, color);
    }

    bool ColorPlateScanner::is_cyan(const ColorPlatePixel &color) const {
        return this->cyan.has_value() && same_color_ignore_opacity(this->cyan.value(), color);
    }

    bool ColorPlateScanner::is_ignored(const ColorPlatePixel &color) const {
        return this->is_blue(color) || (this->valid_color_plate && (this->is_cyan(color) || this->is_magenta(color)));
    }

    void ColorPlateScanner::generate_mipmaps(GeneratedBitmapData &generated_bitmap, std::int16_t mipmaps, ScannedColorMipmapType mipmap_type, float mipmap_fade_factor) {
        auto mipmaps_unsigned = static_cast<std::uint32_t>(mipmaps);
        for(auto &bitmap : generated_bitmap.bitmaps) {
            std::uint32_t mipmap_width = bitmap.width;
            std::uint32_t mipmap_height = bitmap.height;
            std::uint32_t max_mipmap_count = mipmap_width > mipmap_height ? log2_int(mipmap_height) : log2_int(mipmap_width);
            if(max_mipmap_count > mipmaps_unsigned) {
                max_mipmap_count = mipmaps_unsigned;
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

            mipmap_height /= 2;
            mipmap_width /= 2;

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
                        auto &last_a = last_mipmap_data[x * 2 + y * 2 * mipmap_width * 2];
                        auto &last_b = last_mipmap_data[x * 2 + y * 2 * mipmap_width * 2 + 1];
                        auto &last_c = last_mipmap_data[x * 2 + (y * 2 + 1) * mipmap_width * 2];
                        auto &last_d = last_mipmap_data[x * 2 + (y * 2 + 1) * mipmap_width * 2 + 1];
                        pixel = last_a; // Nearest-neighbor first

                        #define INTERPOLATE_CHANNEL(channel) pixel.channel = static_cast<std::uint8_t>((static_cast<std::uint16_t>(last_a.channel) + static_cast<std::uint16_t>(last_b.channel) + static_cast<std::uint16_t>(last_c.channel) + static_cast<std::uint16_t>(last_d.channel)) / 4)

                        // Interpolate color?
                        if(mipmap_type == ScannedColorMipmapType::SCANNED_COLOR_MIPMAP_LINEAR || mipmap_type == ScannedColorMipmapType::SCANNED_COLOR_MIPMAP_NEAREST_ALPHA) {
                            INTERPOLATE_CHANNEL(red);
                            INTERPOLATE_CHANNEL(green);
                            INTERPOLATE_CHANNEL(blue);
                        }

                        // Interpolate alpha?
                        if(mipmap_type == ScannedColorMipmapType::SCANNED_COLOR_MIPMAP_LINEAR) {
                            INTERPOLATE_CHANNEL(alpha);
                        }

                        #undef INTERPOLATE_CHANNEL

                        // Fade to gray?
                        if(mipmap_fade_factor > 0.0F) {
                            // Alpha -> white
                            std::uint32_t alpha_delta = pixel.alpha * mipmap_fade_factor + 1;
                            if(static_cast<std::uint32_t>(0xFF - pixel.alpha) < alpha_delta) {
                                pixel.alpha = 0xFF;
                            }
                            else {
                                pixel.alpha += alpha_delta;
                            }

                            // RGB -> gray
                            pixel.red -= (static_cast<int>(pixel.red) - 0x7F) * mipmap_fade_factor;
                            pixel.green -= (static_cast<int>(pixel.green) - 0x7F) * mipmap_fade_factor;
                            pixel.blue -= (static_cast<int>(pixel.blue) - 0x7F) * mipmap_fade_factor;
                        }
                    }
                }

                // Set the values for the next mipmap
                mipmap_height /= 2;
                mipmap_width /= 2;
                last_mipmap_offset = this_mipmap_offset;
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
                eprintf("Error: Stacked bitmaps must have at least one bitmap. %u found.\n", FACES);
                std::terminate();
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
                    eprintf("Error: Cubemaps must have six bitmaps per cubemap. %u found.\n", FACES);
                    std::terminate();
                }
                if(BITMAP_WIDTH != BITMAP_HEIGHT) {
                    eprintf("Error: Cubemap length must equal width and height. %ux%u found.\n", BITMAP_WIDTH, BITMAP_HEIGHT);
                    std::terminate();
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
                    eprintf("Error: Stacked bitmaps must be the same dimensions. Expected %ux%u. %ux%u found\n", BITMAP_WIDTH, BITMAP_WIDTH, bitmap.width, bitmap.height);
                    std::terminate();
                }

                // Also ensure it has the same # of mipmaps. I don't know how it wouldn't, but you never know
                if(bitmap.mipmaps.size() != MIPMAP_COUNT) {
                    eprintf("Error: Stacked bitmaps must have the same number of mipmaps. Expected %u. %zu found\n", MIPMAP_COUNT, bitmap.mipmaps.size());
                    std::terminate();
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

    static std::pair<std::uint32_t, std::uint32_t> dimensions_of_sprite_sheet(const std::vector<GeneratedBitmapDataSequence> &sequences, std::uint32_t bitmap_index) {
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

        return std::pair(max_width, max_height);
    }

    static std::size_t number_of_pixels(const std::vector<GeneratedBitmapDataSequence> &sequences) {
        std::uint32_t sprite_sheet_count = number_of_sprite_sheets(sequences);
        std::size_t sprite_sheet_pixels = 0;
        for(std::uint32_t i = 0; i < sprite_sheet_count; i++) {
            auto dimensions = dimensions_of_sprite_sheet(sequences, i);
            sprite_sheet_pixels += dimensions.first * dimensions.second;
        }
        return sprite_sheet_pixels;
    }

    static std::optional<std::vector<GeneratedBitmapDataSequence>> fit_sprites_into_sprite_sheet(std::uint32_t width, std::uint32_t height, const GeneratedBitmapData &generated_bitmap, std::uint32_t sprite_spacing, std::uint32_t maximum_sprite_sheets) {
        // Effectively, all sprites are this many pixels apart
        std::uint32_t effective_sprite_spacing = sprite_spacing * 2;

        // First see if all sprites can even fit by themselves. If not, there is no point in continuing.
        std::size_t total_pixels = 0;
        for(auto &bitmap : generated_bitmap.bitmaps) {
            if(bitmap.height > height || bitmap.width > width) {
                return std::nullopt;
            }
            total_pixels += (bitmap.height + effective_sprite_spacing) * (bitmap.width + effective_sprite_spacing);
        }

        // Also, if the number of pixels is greater than height * width, there is no way we could fit everything in here
        if(total_pixels > height * width * maximum_sprite_sheets) {
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

        // Sort each sprite by height or width depending on if height > width (use the lower dimension). The first element of the pair is the bitmap index and the second element is the length
        std::vector<SortedSprite> sprites_sorted;
        const auto *bitmaps = generated_bitmap.bitmaps.data();
        const bool SORT_BY_HEIGHT = width >= height;
        for(std::uint32_t s = 0; s < generated_bitmap.sequences.size(); s++) {
            // Go through each bitmap in the sequence
            auto &sequence = generated_bitmap.sequences[s];
            const auto FIRST_BITMAP = sequence.first_bitmap;
            const auto END_BITMAP = FIRST_BITMAP + sequence.bitmap_count;
            for(std::uint32_t b = FIRST_BITMAP; b < END_BITMAP; b++) {
                auto &bitmap = bitmaps[b];
                std::uint32_t length;
                if(SORT_BY_HEIGHT) {
                    length = bitmap.height;
                }
                else {
                    length = bitmap.width;
                }

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
                auto fits = [&sprite_fitting, &sprite, &sprites_sorted, &sheet, &width, &height](std::uint32_t x, std::uint32_t y) -> bool {
                    std::uint32_t potential_top = y;
                    std::uint32_t potential_left = x;
                    std::uint32_t potential_bottom = potential_top + sprite_fitting.height - 1;
                    std::uint32_t potential_right = potential_left + sprite_fitting.width - 1;

                    // If we're outside the bitmap, fail
                    if(width <= potential_right || height <= potential_bottom) {
                        return false;
                    }

                    for(std::uint32_t sprite_test = 0; sprite_test < sprite; sprite_test++) {
                        auto sprite_test_value = sprites_sorted[sprite_test];

                        // If we aren't even on the same bitmap, ignore
                        if(sheet != sprite_test_value.sheet_index) {
                            continue;
                        }

                        // Get the value for comparing, spacing included
                        std::uint32_t compare_top = sprite_test_value.top;
                        std::uint32_t compare_left = sprite_test_value.left;
                        std::uint32_t compare_bottom = sprite_test_value.bottom;
                        std::uint32_t compare_right = sprite_test_value.right;

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

                // If it fits, set the coordinates
                #define CHECK_IF_FITS if(fits(x,y)) { \
                    coordinates = std::pair<std::uint32_t,std::uint32_t>(x,y); \
                }

                std::uint32_t max_x = width - sprite_fitting.width;
                std::uint32_t max_y = height - sprite_fitting.height;

                if(SORT_BY_HEIGHT) {
                    for(std::uint32_t x = 0; x <= max_x && !coordinates.has_value(); x++) {
                        for(std::uint32_t y = 0; y <= max_y && !coordinates.has_value(); y++) {
                            CHECK_IF_FITS
                        }
                    }
                }
                else {
                    for(std::uint32_t y = 0; y <= max_y && !coordinates.has_value(); y++) {
                        for(std::uint32_t x = 0; x <= max_x && !coordinates.has_value(); x++) {
                            CHECK_IF_FITS
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
                    sprite_fitting.bottom = y + sprite_fitting.height + effective_sprite_spacing;
                    sprite_fitting.right = x + sprite_fitting.width + effective_sprite_spacing;

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
                        new_sprite.bottom = sprite.bottom;
                        new_sprite.top = sprite.top;
                        new_sprite.left = sprite.left;
                        new_sprite.right = sprite.right;
                        new_sprite.registration_point_x = sprite.registration_point_x + sprite_spacing;
                        new_sprite.registration_point_y = sprite.registration_point_y + sprite_spacing;
                        break;
                    }
                }
            }
        }

        // Done!
        return new_sequences;
    }

    // Go through each possible height/width until we get something. The maximum number of operations this can do is log2(width * height)
    static std::optional<std::vector<GeneratedBitmapDataSequence>> fit_sprites_into_maximum_sprite_sheet(std::uint32_t width, std::uint32_t height, const GeneratedBitmapData &generated_bitmap, std::uint32_t sprite_spacing, std::uint32_t maximum_sprite_sheets, bool sprite_budget_optimize) {
        auto fit_sprites = fit_sprites_into_sprite_sheet(width, height, generated_bitmap, sprite_spacing, maximum_sprite_sheets);
        if(!fit_sprites.has_value()) {
            return std::nullopt;
        }

        if(!sprite_budget_optimize) {
            return fit_sprites;
        }

        auto fit_sprites_pixels = number_of_pixels(fit_sprites.value());

        auto fit_sprites_half_height = fit_sprites_into_maximum_sprite_sheet(width, height / 2, generated_bitmap, sprite_spacing, maximum_sprite_sheets, sprite_budget_optimize);
        auto fit_sprites_half_width = fit_sprites_into_maximum_sprite_sheet(width / 2, height, generated_bitmap, sprite_spacing, maximum_sprite_sheets, sprite_budget_optimize);
        auto fit_sprites_half_height_pixels = fit_sprites_half_height.has_value() ? number_of_pixels(fit_sprites_half_height.value()) : ~0;
        auto fit_sprites_half_width_pixels = fit_sprites_half_width.has_value() ? number_of_pixels(fit_sprites_half_width.value()) : ~0;

        if(fit_sprites_pixels > fit_sprites_half_height_pixels) {
            if(fit_sprites_half_height_pixels > fit_sprites_half_width_pixels) {
                return fit_sprites_half_width;
            }
            else {
                return fit_sprites_half_height;
            }
        }
        else if(fit_sprites_pixels > fit_sprites_half_width_pixels) {
            return fit_sprites_half_width;
        }
        else {
            return fit_sprites;
        }
    }

    void ColorPlateScanner::process_sprites(GeneratedBitmapData &generated_bitmap, const ColorPlateScannerSpriteParameters &parameters) {
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
        }

        // Get the max budget of the sprite sheet. If none is given, automatically choose a large budget
        std::uint32_t max_budget = parameters.sprite_budget;
        std::uint32_t max_sheet_count = parameters.sprite_budget_count;
        if(max_sheet_count == 0) {
            max_budget = 2048;
            max_sheet_count = 1;
        }

        // First see if we can even fit things into this
        std::uint32_t sprite_spacing = parameters.sprite_spacing;
        auto fit_sprites = fit_sprites_into_maximum_sprite_sheet(max_budget, max_budget, generated_bitmap, sprite_spacing, max_sheet_count, parameters.sprite_budget_optimize);
        if(!fit_sprites.has_value()) {
            eprintf("Error: Unable to fit sprites into %u %ux%u sprite sheet%s.\n", max_sheet_count, max_budget, max_budget, max_sheet_count == 1 ? "" : "s");
            std::terminate();
        }

        auto &sprites_fit = fit_sprites.value();
        std::uint32_t sheet_count = number_of_sprite_sheets(sprites_fit);

        std::vector<GeneratedBitmapDataBitmap> new_bitmaps;

        for(std::uint32_t s = 0; s < sheet_count; s++) {
            auto sheet_dimensions = dimensions_of_sprite_sheet(sprites_fit, s);

            // Initialize the new bitmap
            const std::uint32_t SHEET_WIDTH = sheet_dimensions.first;
            const std::uint32_t SHEET_HEIGHT = sheet_dimensions.second;
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
                    eprintf("Error: Color plate sequence bitmap count (%u) doesn't match up with sprite sequence (%zu).\n", color_plate_sequence.bitmap_count, sprite_sequence.sprites.size());
                    std::terminate();
                }

                // Go through each sprite and bake it in
                for(auto &sprite : sprite_sequence.sprites) {
                    if(sprite.bitmap_index == s) {
                        auto &bitmap = generated_bitmap.bitmaps[sprite.original_bitmap_index];
                        const std::uint32_t SPRITE_WIDTH = bitmap.width;
                        const std::uint32_t SPRITE_HEIGHT = bitmap.height;
                        for(std::uint32_t y = 0; y < SPRITE_HEIGHT; y++) {
                            for(std::uint32_t x = 0; x < SPRITE_WIDTH; x++) {
                                const auto &input = bitmap.pixels[y * SPRITE_WIDTH + x];
                                auto &output = new_bitmap.pixels[(y + sprite.top + sprite_spacing) * SHEET_WIDTH + (x + sprite.left + sprite_spacing)];
                                output = output.alpha_blend(input);
                            }
                        }
                    }
                }
            }
        }

        generated_bitmap.bitmaps = std::move(new_bitmaps);
        generated_bitmap.sequences = std::move(sprites_fit);
    }
}
