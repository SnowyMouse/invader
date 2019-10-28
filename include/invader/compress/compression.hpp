// SPDX-License-Identifier: GPL-3.0-only

#ifndef INVADER__COMPRESS__COMPRESSION_HPP
#define INVADER__COMPRESS__COMPRESSION_HPP

#include <vector>

namespace Invader::Compression {
    /**
     * Compress the map data
     * @param data              data pointer
     * @param data_size         size of the data
     * @param compression_level compression level to use
     */
    std::vector<std::byte> compress_map_data(const std::byte *data, std::size_t data_size, int compression_level = 3);

    /**
     * Decompress the map data
     * @param data              data pointer
     * @param data_size         size of the data
     */
    std::vector<std::byte> decompress_map_data(const std::byte *data, std::size_t data_size);
}

#endif
