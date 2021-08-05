// SPDX-License-Identifier: GPL-3.0-only

#ifndef INVADER__TAG__HEK__CLASS__BITMAP_HPP
#define INVADER__TAG__HEK__CLASS__BITMAP_HPP

#include "../../parser/definition/bitmap.hpp"

namespace Invader::Parser {
    /**
     * Get the string name of the bitmap data format
     * @param  format format to get
     * @return        bitmap data format
     */
    const char *bitmap_data_format_name(BitmapDataFormat format) noexcept;
    
    /**
     * Get the number of bits per pixel
     * @param format format to query
     * @return       number of bits per pixel used by the format, if it's valid, or 0 otherwise
     */
    std::size_t calculate_bits_per_pixel(BitmapDataFormat format) noexcept;
    
    /**
     * Calculate the size of a bitmap
     * @param data bitmap data
     * @return     size in bytes
     */
    std::size_t size_of_bitmap(const BitmapData &data) noexcept;
}
#endif
