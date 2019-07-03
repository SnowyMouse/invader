/*
 * Invader (c) 2018 Kavawuvi
 *
 * This program is free software under the GNU General Public License v3.0 or later. See LICENSE for more information.
 */

#ifndef INVADER__BITMAP__COMPOSITE_BITMAP_HPP
#define INVADER__BITMAP__COMPOSITE_BITMAP_HPP

#include <cstdint>
#include <vector>

namespace Invader {
    struct CompositeBitmapPixel {
        std::uint8_t blue;
        std::uint8_t green;
        std::uint8_t red;
        std::uint8_t alpha;

        /**
         * Get if this color is solid blue
         * @return true if the color is solid blue
         */
        bool solid_blue() const {
            return this->blue == 0xFF && this->green == 0x0 && this->red == 0x0 && this->alpha == 0xFF;
        }

        bool operator==(const CompositeBitmapPixel &other) const {
            return this->blue == other.blue && this->green == other.green && this->red == other.red && this->alpha == other.alpha;
        }
    };
    static_assert(sizeof(CompositeBitmapPixel) == 4);

    class CompositeBitmapBitmap {
    public:
        /**
         * Get the height of the bitmap in pixels
         * @return height of bitmap in pixels
         */
        std::size_t get_height() const {
            return this->height;
        }

        /**
         * Get the width of the bitmap in pixels
         * @return width of bitmap in pixels
         */
        std::size_t get_width() const {
            return this->width;
        }

        /**
         * Get the number of mipmaps
         * @return number of mipmaps
         */
        std::size_t get_mipmap_count() const {
            return this->mipmap_count;
        }

        /**
         * Get the pixel given an x and y coordinate
         * @param x x coordinate (column)
         * @param y y coordinate (row)
         * @return reference to the given pixel
         */
        const CompositeBitmapPixel &get_pixel(std::size_t x, std::size_t y) const {
            return this->pixels[this->get_width() * y + x];
        }

        /**
         * Get all of the pixels
         * @return all of the pixels
         */
        const CompositeBitmapPixel *get_pixels() const {
            return this->pixels.data();
        }

        /**
         * Construct a CompositeBitmapBitmap with the given pixels, width, and height of the bitmap
         * @param source_pixels pixel data of source image
         * @param source_width  width of source image
         * @param x             x position of the bitmap
         * @param y             y position of the bitmap
         * @param width         width of the bitmap
         * @param height        height of the bitmap
         * @param mipmap_count  the number of mipmaps
         */
        CompositeBitmapBitmap(const CompositeBitmapPixel *source_pixels, std::size_t source_width, std::size_t x, std::size_t y, std::size_t width, std::size_t height, std::size_t mipmap_count) : height(height), width(width), mipmap_count(mipmap_count) {
            std::size_t mip_height = height;
            std::size_t mip_y = y;
            std::size_t mip_width = width;
            for(std::size_t m = 0; m <= mipmap_count; m++) {
                // Read line by line
                for(std::size_t y_read = mip_y; y_read < mip_y + mip_height; y_read++) {
                    auto *first_pixel = source_pixels + x + y_read * source_width;
                    this->pixels.insert(this->pixels.end(), first_pixel, first_pixel + mip_width);
                }

                mip_y += mip_height;
                mip_height /= 2;
                mip_width /= 2;
            }
        }

    private:
        /**
         * This stores all of the pixel data
         */
        std::vector<CompositeBitmapPixel> pixels;

        /**
         * Height of the bitmap
         */
        std::size_t height;

        /**
         * Width of the bitmap
         */
        std::size_t width;

        /**
         * Number of mipmaps
         */
        std::size_t mipmap_count;
    };

    class CompositeBitmap {
    public:
        /**
         * Get the bitmaps in this composite bitmap
         * @return bitmaps in the composite bitmap
         */
        std::vector<CompositeBitmapBitmap> &get_bitmaps() {
            return this->bitmaps;
        }

        /**
         * Get the bitmaps in this composite bitmap
         * @return bitmaps in the composite bitmap
         */
        const std::vector<CompositeBitmapBitmap> &get_bitmaps() const {
            return this->bitmaps;
        }

        /**
         * Generate a CompositeBitmap with the given dimensions and pixels
         * @param pixels pixel data
         * @param width  number of columns in the image
         * @param height number of rows in the image
         */
        CompositeBitmap(const CompositeBitmapPixel *pixels, std::size_t width, std::size_t height);

    private:
        /**
         * Bitmaps
         */
        std::vector<CompositeBitmapBitmap> bitmaps;
    };
}

#endif
