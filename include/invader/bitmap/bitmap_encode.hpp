// SPDX-License-Identifier: GPL-3.0-only

#ifndef INVADER__BITMAP__BITMAP_DECODE_HPP
#define INVADER__BITMAP__BITMAP_DECODE_HPP

#include <cstddef>
#include <vector>
#include "../tag/hek/definition.hpp"

namespace Invader::BitmapEncode {
    /**
     * Encode the pixel data to another format
     * @param input_data    input pixel data
     * @param input_format  input pixel format
     * @param output_format output pixel format
     * @param width         width in pixels
     * @param height        height in pixels
     * @output              encoded data
     */
    std::vector<std::byte> encode_bitmap(const std::byte *input_data, HEK::BitmapDataFormat input_format, HEK::BitmapDataFormat output_format, std::size_t width, std::size_t height);
    
    /**
     * Encode the pixel data to another format. Use bitmap_data_size() to determine how big output_data should be.
     * @param input_data    input pixel data
     * @param input_format  input pixel format
     * @param output_data   output pixel data
     * @param output_format output pixel format
     * @param width         width in pixels
     * @param height        height in pixels
     * @output              encoded data
     */
    void encode_bitmap(const std::byte *input_data, HEK::BitmapDataFormat input_format, std::byte *output_data, HEK::BitmapDataFormat output_format, std::size_t width, std::size_t height);
    
    /**
     * Calculate the number of bytes required to hold the given bitmap
     * @param format format of data
     * @param width  width of bitmap
     * @param height height of bitmap
     * @output       output size
     */
    std::size_t bitmap_data_size(HEK::BitmapDataFormat format, std::size_t width, std::size_t height);
}

#endif
