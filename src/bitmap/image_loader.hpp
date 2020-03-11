// SPDX-License-Identifier: GPL-3.0-only

#ifndef INVADER__BITMAP__IMAGE_LOADER_HPP
#define INVADER__BITMAP__IMAGE_LOADER_HPP

#include "color_plate_scanner.hpp"

namespace Invader {
    ColorPlatePixel *load_tiff(const char *path, std::uint32_t &image_width, std::uint32_t &image_height, std::size_t &image_size);
    ColorPlatePixel *load_image(const char *path, std::uint32_t &image_width, std::uint32_t &image_height, std::size_t &image_size);
}

#endif
