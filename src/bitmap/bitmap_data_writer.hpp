// SPDX-License-Identifier: GPL-3.0-only

#ifndef INVADER__BITMAP__BITMAP_DATA_WRITER_HPP
#define INVADER__BITMAP__BITMAP_DATA_WRITER_HPP

#include "color_plate_scanner.hpp"
#include <invader/tag/parser/definition/bitmap.hpp>

namespace Invader {
    using BitmapFormat = Parser::BitmapFormat;

    void write_bitmap_data(const GeneratedBitmapData &scanned_color_plate, std::vector<std::byte> &bitmap_data_pixels, std::vector<Parser::BitmapData> &bitmap_data, BitmapUsage usage, BitmapFormat format, BitmapType bitmap_type, bool palettize, bool dither_alpha, bool dither_red, bool dither_green, bool dither_blue);
}

#endif
