// SPDX-License-Identifier: GPL-3.0-only

#ifndef INVADER__COMPRESS__COMPRESSION_HPP
#define INVADER__COMPRESS__COMPRESSION_HPP

#include <vector>

namespace Invader::Compression {
    /**
     * Compress the map data
     * @param data              data pointer
     * @param data_size         size of the data
     * @param output            data output
     * @param output_size       output buffer size
     * @param compression_level compression level to use
     * @return                  actual size of the output
     */
    std::size_t compress_map_data(const std::byte *data, std::size_t data_size, std::byte *output, std::size_t output_size, int compression_level = 3);

    /**
     * Decompress the map data
     * @param data              data pointer
     * @param data_size         size of the data
     * @param output            data output
     * @param output_size       output buffer size
     * @return                  actual size of the output
     */
    std::size_t decompress_map_data(const std::byte *data, std::size_t data_size, std::byte *output, std::size_t output_size);

    /**
     * Compress the map data
     * @param data              data pointer
     * @param data_size         size of the data
     * @param compression_level compression level to use
     * @return                  vector of compressed data
     */
    std::vector<std::byte> compress_map_data(const std::byte *data, std::size_t data_size, int compression_level = 3);

    /**
     * Decompress the map data
     * @param data              data pointer
     * @param data_size         size of the data
     * @return                  vector of decompressed data
     */
    std::vector<std::byte> decompress_map_data(const std::byte *data, std::size_t data_size);

    /**
     * Decompress one file to another file, using significantly less memory but also significantly more disk I/O
     * @param input  path to the compressed file
     * @param output path to the decompressed file
     */
    void decompress_map_file(const char *input, const char *output);

    /**
     * Decompress one file to a buffer, using significantly less memory but also significantly more disk I/O
     * @param input       path to the compressed file
     * @param output      buffer to output to
     * @param output_size size of buffer
     */
    void decompress_map_file(const char *input, std::byte *output, std::size_t output_size);
}

#endif
