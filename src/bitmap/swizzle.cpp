// SPDX-License-Identifier: GPL-3.0-only

#include <invader/bitmap/swizzle.hpp>
#include <vector>
#include <cstdint>
#include <cstring>

namespace Invader::Swizzle {
    template<typename Pixel> static std::size_t swizzle_block_2x2(const Pixel *values_in, Pixel *values_out, std::size_t stride, std::size_t counter, bool deswizzle) {
        if(!deswizzle) {
            // values_in  = unswizzled
            // values_out = swizzled
            values_out[counter++] = values_in[0];
            values_out[counter++] = values_in[1];
            values_out[counter++] = values_in[0 + stride];
            values_out[counter++] = values_in[1 + stride];
        }
        else {
            // values_in  = swizzled
            // values_out = unswizzled
            values_out[0] = values_in[counter++];
            values_out[1] = values_in[counter++];
            values_out[0 + stride] = values_in[counter++];
            values_out[1 + stride] = values_in[counter++];
        }
        return counter;
    }

    template<typename Pixel> static std::size_t swizzle_block(const Pixel *values_in, Pixel *values_out, std::size_t width, std::size_t stride, std::size_t counter, bool deswizzle) {
        if(width == 2) {
            return swizzle_block_2x2(values_in, values_out, stride, counter, deswizzle);
        }

        std::size_t new_width = width / 2;

        if(!deswizzle) {
            counter = swizzle_block(values_in, values_out                                 , new_width, stride, counter, deswizzle);
            counter = swizzle_block(values_in + new_width                                 , values_out, new_width, stride, counter, deswizzle);
            counter = swizzle_block(values_in + stride * new_width                        , values_out, new_width, stride, counter, deswizzle);
            counter = swizzle_block(values_in + stride * new_width + new_width, values_out, new_width, stride, counter, deswizzle);
        }
        else {
            counter = swizzle_block(values_in, values_out                                 , new_width, stride, counter, deswizzle);
            counter = swizzle_block(values_in, values_out + new_width                     , new_width, stride, counter, deswizzle);
            counter = swizzle_block(values_in, values_out + stride * new_width            , new_width, stride, counter, deswizzle);
            counter = swizzle_block(values_in, values_out + stride * new_width + new_width, new_width, stride, counter, deswizzle);
        }

        return counter;
    }

    template <typename Pixel> static void swizzle(const Pixel *values_in, Pixel *values_out, std::size_t width, std::size_t height, bool deswizzle) {
        // If height is <= 1 or width is <= 2, it's just a straight copy
        if(width <= 2 || height <= 1) {
            std::memcpy(values_out, values_in, width * height * sizeof(*values_in));
            return;
        }

        // Also, if width is less than the height, we can subdivide
        if(width < height) {
            for(std::size_t y = 0; y < height; y += width) {
                swizzle(values_in + y * width, values_out + y * width, width, width, deswizzle);
            }
            return;
        }

        std::size_t counter = 0;
        for(std::size_t x = 0; x < width; x+=height) {
            if(!deswizzle) {
                counter = swizzle_block(values_in + x, values_out, height, width, counter, deswizzle);
            }
            else {
                counter = swizzle_block(values_in, values_out + x, height, width, counter, deswizzle);
            }
        }
    }

    std::vector<std::byte> swizzle(const std::byte *data, std::size_t bits_per_pixel, std::size_t width, std::size_t height, std::size_t depth, bool deswizzle) {
        std::vector<std::byte> output(width*height*depth*(bits_per_pixel/8));

        switch(bits_per_pixel) {
            case 8:
                swizzle(reinterpret_cast<const std::uint8_t *>(data), reinterpret_cast<std::uint8_t *>(output.data()), width, height, deswizzle);
                break;
            case 16:
                swizzle(reinterpret_cast<const std::uint16_t *>(data), reinterpret_cast<std::uint16_t *>(output.data()), width, height, deswizzle);
                break;
            case 32:
                swizzle(reinterpret_cast<const std::uint32_t *>(data), reinterpret_cast<std::uint32_t *>(output.data()), width, height, deswizzle);
                break;
            case 64:
                swizzle(reinterpret_cast<const std::uint64_t *>(data), reinterpret_cast<std::uint64_t *>(output.data()), width, height, deswizzle);
                break;
        }
        return output;
    }
}
