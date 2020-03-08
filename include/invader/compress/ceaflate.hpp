// SPDX-License-Identifier: GPL-3.0-only

#ifndef INVADER__COMPRESS__CEAFLATE_HPP
#define INVADER__COMPRESS__CEAFLATE_HPP

#include <cstdlib>
#include <cstddef>

namespace Invader::Compression::Ceaflate {
    /**
     * Calculate the decompressed file size of a compressed map
     * @param  input_data input data
     * @param  input_size input size
     * @return            compression size or 0 if failure
     */
    std::size_t find_decompressed_file_size(const std::byte *input_data, std::size_t input_size) noexcept;

    /**
     * Decompress the file.
     * @param  input_data  input data
     * @param  input_size  input size
     * @param  output_data output data
     * @param  output_size output size - call find_decompressed_file_size() beforehand to find an output size, if needed
     * @return             true if successful, false if not
     */
    bool decompress_file(const std::byte *input_data, std::size_t input_size, std::byte *output_data, std::size_t &output_size);

    /**
     * Compress the file
     * @param  input_data  input data
     * @param  input_size  input size
     * @param  output_data output data
     * @param  output_size output size
     * @return             true if successful, false if not
     */
    bool compress_file(const std::byte *input_data, std::size_t input_size, std::byte *output_data, std::size_t &output_size);
}

#endif
