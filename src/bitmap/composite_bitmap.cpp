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

        // Check if it's a power of two
        bitmap_height = bottom - y;
        if(!is_power_of_two(bitmap_height)) {
            eprintf("Error: Non power-of-two height %zu\n", bitmap_height);
            return -1;
        }

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
                this->bitmaps.emplace_back(pixels, width, x, y, bitmap_width, bitmap_height);
                ignore_rectangles.emplace_back(Rectangle { x, x + bitmap_width, y, y + bitmap_height });

                x += bitmap_width;
            }
        }
    }
}
