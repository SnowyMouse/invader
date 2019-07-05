/*
 * Invader (c) 2018 Kavawuvi
 *
 * This program is free software under the GNU General Public License v3.0 or later. See LICENSE for more information.
 */

#include "composite_bitmap.hpp"
#include "../eprintf.hpp"

namespace Invader {
    struct Rectangle {
        std::size_t x_from;
        std::size_t x_to;

        std::size_t y_from;
        std::size_t y_to;
    };

    template<typename T> bool is_power_of_two(T value) {
        while(value > 1) {
            if(value & 1) {
                return false;
            }
            value >>= 1;
        }
        return true;
    }

    #define GET_PIXEL(x,y) pixels[width * y + x]

    /**
     * Recursively calculate the number of mipmaps
     * @param  pixels        pointer to pixels in image
     * @param  height        height of image
     * @param  width         width of image
     * @param  x             x position of the image
     * @param  y             y position of the image
     * @param  bitmap_width  width of the main bitmap found
     * @param  bitmap_height height of the main bitmap found
     * @return               -1 if no image was found, 0 if only a main image was found, 1+ if mipmaps are present
     */
    static int mipmaps_at_position(const CompositeBitmapPixel *pixels, std::size_t height, std::size_t width, std::size_t x, std::size_t y, std::size_t &bitmap_width, std::size_t &bitmap_height) {
        std::size_t right;
        for(right = x + 1; right < width; right++) {
            if(GET_PIXEL(right, y).solid_blue()) {
                break;
            }
        }

        // Figure out what the width of this bitmap is
        bitmap_width = right - x;
        if(!is_power_of_two(bitmap_width)) {
            eprintf("Error: Non power-of-two width: %zu\n", bitmap_width);
            return -1;
        }

        // Now figure out the height
        right--;
        std::size_t bottom;
        for(bottom = y + 1; bottom < height; bottom++) {
            if(GET_PIXEL(right, bottom).solid_blue()) {
                break;
            }
        }
        right++;

        // Check if it's a power of two
        bitmap_height = bottom - y;
        if(!is_power_of_two(bitmap_height)) {
            eprintf("Error: Non power-of-two height %zu\n", bitmap_height);
            return -1;
        }

        // Make sure the left, right, and bottom borders are blue. We don't need to check the top because, if there was something there, this would have errored then.
        if(right < width) {
            // Check along the right edge
            for(std::size_t check_y = y; check_y < bitmap_height; check_y++) {
                if(!GET_PIXEL(right, check_y).solid_blue()) {
                    eprintf("Error: Bitmap does not have a blue border on the right edge\n");
                    return -1;
                }
            }
        }

        if(x > 0) {
            // Check the left edge
            std::size_t left = x - 1;
            for(std::size_t check_y = y; check_y < bitmap_height; check_y++) {
                if(!GET_PIXEL(left, check_y).solid_blue()) {
                    eprintf("Error: Bitmap does not have a blue border on the left edge\n");
                    return -1;
                }
            }
        }

        if(bottom < height) {
            // Check along the right half of the bottom edge. If there is anything here, it is wrong
            for(std::size_t check_x = x + bitmap_width / 2; check_x < right; check_x++) {
                if(!GET_PIXEL(check_x, bottom).solid_blue()) {
                    eprintf("Error: Bitmap does not have a blue border on the bottom edge\n");
                    return -1;
                }
            }

            // Check along the left half of the bottom edge. If there is anything here, then every pixel must not be blue. Otherwise, every pixel must be blue.
            bool mipmap_not_preset = GET_PIXEL(x, bottom).solid_blue();
            for(std::size_t check_x = x; check_x < bitmap_width / 2; check_x++) {
                if(GET_PIXEL(check_x, bottom).solid_blue() != mipmap_not_preset) {
                    eprintf("Error: Bitmap does not have a blue border on the bottom edge or bad mipmap\n");
                    return -1;
                }
            }

            // If there are mipmaps here, check for them
            if(!mipmap_not_preset) {
                std::size_t mip_width;
                std::size_t mip_height;
                int mips_here = mipmaps_at_position(pixels, height, width, x, bottom, mip_width, mip_height);
                if(mips_here == -1) {
                    return -1;
                }
                else if(mip_height != bitmap_height / 2) {
                    eprintf("Error: Bad mipmap. Mipmaps must be half the height of the previous mipmap.\n");
                    return -1;
                }
                else {
                    return 1 + mips_here;
                }
            }
        }

        // No mipmaps? No problem
        return 0;
    }

    CompositeBitmap::CompositeBitmap(const CompositeBitmapPixel *pixels, std::size_t width, std::size_t height) {
        std::vector<Rectangle> ignore_rectangles;

        for(std::size_t y = 0; y < height; y++) {
            for(std::size_t x = 0; x < width; x++) {
                const auto &pixel = GET_PIXEL(x,y);

                // Check if it's solid blue
                if(pixel.solid_blue()) {
                    continue;
                }

                // Check if we need to skip it
                bool ignored = false;
                for(auto &r : ignore_rectangles) {
                    if(x >= r.x_from && x < r.x_to && y >= r.y_from && y < r.y_to) {
                        x = r.x_to;
                        ignored = true;
                        break;
                    }
                }
                if(ignored) {
                    continue;
                }

                // Figure out the number if mipmaps
                std::size_t bitmap_width = 0;
                std::size_t bitmap_height = 0;
                int mipmap_count = mipmaps_at_position(pixels, height, width, x, y, bitmap_width, bitmap_height);
                if(mipmap_count < 0) {
                    this->bitmaps.clear();
                    return;
                }

                // Add it to the list
                this->bitmaps.emplace_back(pixels, width, x, y, bitmap_width, bitmap_height, mipmap_count);

                // Ignore images
                std::size_t mip_y = y;
                std::size_t mip_width = bitmap_width;
                std::size_t mip_height = bitmap_height;
                for(int i = 0; i <= mipmap_count; i++) {
                    ignore_rectangles.emplace_back(Rectangle { x, x + mip_width, mip_y, mip_y + mip_height });
                    mip_y += mip_height;
                    mip_width /= 2;
                    mip_height /= 2;
                }

                x += bitmap_width;
            }
        }
    }

    std::uint16_t CompositeBitmapPixel::to_16_bit(std::uint8_t alpha, std::uint8_t red, std::uint8_t green, std::uint8_t blue) const {
        std::uint16_t color = 0;

        #define SET_CHANNEL_VALUE_FOR_COLOR(channel) \
            if(channel) { \
                color <<= channel; \
                std::uint8_t channel_max = (1 << (channel)) - 1; \
                std::uint16_t color_value = static_cast<std::uint16_t>(this->channel) * channel_max / 255; \
                color |= color_value; \
            }

        SET_CHANNEL_VALUE_FOR_COLOR(alpha);
        SET_CHANNEL_VALUE_FOR_COLOR(red);
        SET_CHANNEL_VALUE_FOR_COLOR(green);
        SET_CHANNEL_VALUE_FOR_COLOR(blue);

        return color;
    }
}
