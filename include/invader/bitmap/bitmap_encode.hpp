
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
     * @param dither_alpha  dither alpha channel
     * @param dither_red    dither red channel
     * @param dither_green  dither green channel
     * @param dither_blue   dither blue channel
     * @output              encoded data
     */
    std::vector<std::byte> encode_bitmap(const std::byte *input_data, HEK::BitmapDataFormat input_format, HEK::BitmapDataFormat output_format, std::size_t width, std::size_t height, bool dither_alpha = false, bool dither_red = false, bool dither_green = false, bool dither_blue = false);
    
    /**
     * Encode the pixel data to another format. Use bitmap_data_size() to determine how big output_data should be.
     * @param input_data    input pixel data
     * @param input_format  input pixel format
     * @param output_data   output pixel data
     * @param output_format output pixel format
     * @param width         width in pixels
     * @param height        height in pixels
     * @param dither_alpha  dither alpha channel
     * @param dither_red    dither red channel
     * @param dither_green  dither green channel
     * @param dither_blue   dither blue channel
     * @output              encoded data
     */
    void encode_bitmap(const std::byte *input_data, HEK::BitmapDataFormat input_format, std::byte *output_data, HEK::BitmapDataFormat output_format, std::size_t width, std::size_t height, bool dither_alpha = false, bool dither_red = false, bool dither_green = false, bool dither_blue = false);
    
    /**
     * Encode the pixel data to another format
     * @param input_data    input pixel data
     * @param input_format  input pixel format
     * @param output_format output pixel format
     * @param width         width in pixels
     * @param height        height in pixels
     * @param depth         depth of the bitmap
     * @param type          type of the bitmap
     * @param mipmap_count  number of mipmaps
     * @param dither_alpha  dither alpha channel
     * @param dither_red    dither red channel
     * @param dither_green  dither green channel
     * @param dither_blue   dither blue channel
     * @output              encoded data
     */
    std::vector<std::byte> encode_bitmap(const std::byte *input_data, HEK::BitmapDataFormat input_format, HEK::BitmapDataFormat output_format, std::size_t width, std::size_t height, std::size_t depth, HEK::BitmapDataType type, std::size_t mipmap_count, bool dither_alpha = false, bool dither_red = false, bool dither_green = false, bool dither_blue = false);
    
    /**
     * Encode the pixel data to another format. Use bitmap_data_size() to determine how big output_data should be.
     * @param input_data    input pixel data
     * @param input_format  input pixel format
     * @param output_data   output pixel data
     * @param output_format output pixel format
     * @param width         width in pixels
     * @param height        height in pixels
     * @param depth         depth of the bitmap
     * @param type          type of the bitmap
     * @param dither_alpha  dither alpha channel
     * @param dither_red    dither red channel
     * @param dither_green  dither green channel
     * @param dither_blue   dither blue channel
     * @output              encoded data
     */
    void encode_bitmap(const std::byte *input_data, HEK::BitmapDataFormat input_format, std::byte *output_data, HEK::BitmapDataFormat output_format, std::size_t width, std::size_t height, std::size_t depth, HEK::BitmapDataType type, std::size_t mipmap_count, bool dither_alpha = false, bool dither_red = false, bool dither_green = false, bool dither_blue = false);
    
    /**
     * Calculate the size of a bitmap
     * @param width        width of the bitmap
     * @param height       height of the bitmap
     * @param depth        depth of the bitmap
     * @param mipmap_count number of mipmaps
     * @param format       format of the bitmap
     * @param type         type of the bitmap
     * @return             size in bytes
     */
    std::size_t bitmap_data_size(std::size_t width, std::size_t height, std::size_t depth, std::size_t mipmap_count, HEK::BitmapDataFormat format, HEK::BitmapDataType type) noexcept;
    
    /**
     * Find the most efficient format without any loss in data. The input bitmap MUST be in 32-bit BGRA (A8R8G8B8) format.
     * @param input_data pixel data
     * @param width      width of the bitmap in pixels
     * @param height     height of the bitmap in pixels
     * @param category   category of formats to use
     */
    HEK::BitmapDataFormat most_efficient_format(const std::byte *input_data, std::size_t width, std::size_t height, HEK::BitmapFormat category) noexcept;
    
    /**
     * Find the most efficient format without any loss in data. The input bitmap MUST be in 32-bit BGRA (A8R8G8B8) format.
     * @param input_data   pixel data
     * @param width        width of the bitmap in pixels
     * @param height       height of the bitmap in pixels
     * @param depth        depth of the bitmap (must be 1 for 2D textures)
     * @param category     category of formats to use
     * @param type         type of bitmap
     * @param mipmap_count number of mipmaps (by default, just check the base bitmap)
     */
    HEK::BitmapDataFormat most_efficient_format(const std::byte *input_data, std::size_t width, std::size_t height, std::size_t depth, HEK::BitmapFormat category, HEK::BitmapDataType type, std::size_t mipmap_count = 0) noexcept;
}

#endif
