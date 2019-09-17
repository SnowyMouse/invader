/*
 * Invader (c) 2019 Kavawuvi
 *
 * This program is free software under the GNU General Public License v3.0. See LICENSE for more information.
 */

#ifndef INVADER__BITMAP__BITMAP_DATA_WRITER_HPP
#define INVADER__BITMAP__BITMAP_DATA_WRITER_HPP

#include "color_plate_scanner.hpp"

namespace Invader {
    using BitmapFormat = HEK::BitmapFormat;

    void write_bitmap_data(const GeneratedBitmapData &scanned_color_plate, std::vector<std::byte> &bitmap_data_pixels, std::vector<HEK::BitmapData<HEK::BigEndian>> &bitmap_data, BitmapUsage usage, BitmapFormat format, BitmapType bitmap_type, bool palettize, bool dither_alpha, bool dither_red, bool dither_green, bool dither_blue);
}

#endif
