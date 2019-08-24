/*
 * Invader (c) 2018 Kavawuvi
 *
 * This program is free software under the GNU General Public License v3.0 or later. See LICENSE for more information.
 */

#ifndef INVADER__CRC__HEK__CRC_HPP
#define INVADER__CRC__HEK__CRC_HPP

#include <cstdint>
#include <cstddef>

namespace Invader {
    /**
     * Calculate the CRC32 of a map
     * @param  data        pointer to data
     * @param  size        size of data
     * @param  new_crc     new CRC32 of the map
     * @param  new_random  new random number of the map (if forging a CRC32)
     * @return             CRC32 of the map
     */
    std::uint32_t calculate_map_crc(const std::byte *data, std::size_t size, const std::uint32_t *new_crc = nullptr, std::uint32_t *new_random = nullptr);
}

#endif
