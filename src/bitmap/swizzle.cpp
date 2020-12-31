// SPDX-License-Identifier: GPL-3.0-only

#include <invader/bitmap/swizzle.hpp>
#include <vector>
#include <cstdint>
#include <cstring>
#include <cstdlib>

namespace Invader::Swizzle {
    // From https://www.forceflow.be/2013/10/07/morton-encodingdecoding-through-bit-interleaving-implementations/
    static constexpr std::uint64_t morton_encode_3d(unsigned int x, unsigned int y, unsigned int z) {
        std::uint64_t answer = 0;
        for (std::uint64_t i = 0; i < (sizeof(std::uint64_t) * 8)/3; ++i) {
            std::uint64_t bit = static_cast<std::uint64_t>(1) << i;
            answer |= ((x & bit) << 2*i) | ((y & bit) << (2*i + 1)) | ((z & bit) << (2*i + 2));
        }
        return answer;
    }
    
    static constexpr std::uint64_t morton_encode_2d(unsigned int x, unsigned int y) {
        std::uint64_t answer = 0;
        for (std::uint64_t i = 0; i < (sizeof(std::uint64_t) * 8)/3; ++i) {
            std::uint64_t bit = static_cast<std::uint64_t>(1) << i;
            answer |= ((x & bit) << i) | ((y & bit) << (i + 1));
        }
        return answer;
    }
    
    template <typename Pixel> static void perform_swizzle(const Pixel *values_in, Pixel *values_out, std::size_t width, std::size_t height, std::size_t depth, bool deswizzle) {
        // Subdivide
        if(height > width) {
            auto size = width * width * depth;
            for(std::size_t i = 0; i < height / width; i++) {
                perform_swizzle(values_in + size * i, values_out + size * i, width, width, depth, deswizzle);
            }
            return;
        }
        
        auto size = (width * height * depth);
        
        if(size == 0) {
            return;
        }
        
        for(std::size_t z = 0; z < depth; z++) {
            for(std::size_t y = 0; y < height; y++) {
                for(std::size_t x = 0; x < width; x++) {
                    std::uint64_t m;
                    
                    std::size_t tmp_x = x;
                    std::size_t tmp_y = y;
                    
                    // Swizzled textures are a meme, but apparently this is how it's stored -.-
                    
                    // TODODILE: refactor this into something that makes more sense
                    /* 
                        if(tmp_x >= height * 2 && tmp_x < height * 4) {
                            tmp_x -= height * 2;
                            tmp_y += height;
                        }
                        else if(tmp_x >= height * 4 && tmp_x < height * 6) {
                            tmp_x -= height * 2;
                            tmp_y += height * 2;
                        }
                        else if(tmp_x >= height * 6 && tmp_x < height * 8) {
                            tmp_x -= height * 4;
                            tmp_y += height * 3;
                        }
                    */
                    
                    if(tmp_x >= height * 2) {
                        for(std::size_t f = 1; f < 1024; f++) {
                            if(tmp_x >= height * (f * 2) && tmp_x < height * ((f + 1) * 2)) {
                                tmp_x -= height * (((f - 1) / 2 + 1) * 2);
                                tmp_y += height * f;
                                break;
                            }
                        }
                    }
                    
                    if(depth == 1) {
                        m = morton_encode_2d(tmp_x,tmp_y) % size;
                    }
                    else {
                        m = morton_encode_3d(tmp_x,tmp_y,z) % size;
                    }
                    
                    auto offset = x + y * width + z * width * height;
                    
                    if(deswizzle) {
                        values_out[offset] = values_in[m];
                    }
                    else {
                        values_out[m] = values_in[offset];
                    }
                }
            }
        }
    }

    std::vector<std::byte> swizzle(const std::byte *data, std::size_t bits_per_pixel, std::size_t width, std::size_t height, std::size_t depth, bool deswizzle) {
        std::vector<std::byte> output(width*height*depth*(bits_per_pixel/8));

        switch(bits_per_pixel) {
            case 8:
                perform_swizzle(reinterpret_cast<const std::uint8_t *>(data), reinterpret_cast<std::uint8_t *>(output.data()), width, height, depth, deswizzle);
                break;
            case 16:
                perform_swizzle(reinterpret_cast<const std::uint16_t *>(data), reinterpret_cast<std::uint16_t *>(output.data()), width, height, depth, deswizzle);
                break;
            case 32:
                perform_swizzle(reinterpret_cast<const std::uint32_t *>(data), reinterpret_cast<std::uint32_t *>(output.data()), width, height, depth, deswizzle);
                break;
            case 64:
                perform_swizzle(reinterpret_cast<const std::uint64_t *>(data), reinterpret_cast<std::uint64_t *>(output.data()), width, height, depth, deswizzle);
                break;
        }
        return output;
    }
}
