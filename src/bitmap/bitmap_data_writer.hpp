// SPDX-License-Identifier: GPL-3.0-only

#ifndef INVADER__BITMAP__BITMAP_DATA_WRITER_HPP
#define INVADER__BITMAP__BITMAP_DATA_WRITER_HPP

#include <invader/bitmap/color_plate_scanner.hpp>
#include <invader/tag/parser/parser.hpp>

namespace Invader {
    using BitmapFormat = HEK::BitmapFormat;

    /**
     * if format is nullopt, it will determine one
     */
    void write_bitmap_data(const GeneratedBitmapData &scanned_color_plate, std::vector<std::byte> &bitmap_data_pixels, std::vector<Parser::BitmapData> &bitmap_data, BitmapUsage usage, std::optional<BitmapFormat> &format, BitmapType bitmap_type, bool palettize, bool dither);
}

#endif
