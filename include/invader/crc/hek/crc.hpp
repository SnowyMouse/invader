// SPDX-License-Identifier: GPL-3.0-only

#ifndef INVADER__CRC__HEK__CRC_HPP
#define INVADER__CRC__HEK__CRC_HPP

#include <cstdint>
#include <cstddef>
#include <optional>

namespace Invader {
    /**
     * Calculate the CRC32 of a map
     * @param  data             pointer to data
     * @param  size             size of data
     * @param  new_crc          new CRC32 of the map
     * @param  new_random       new random number of the map (if forging a CRC32)
     * @param  check_dirty      optionally set to false if the cache file is not dirty or true if it is
     * @return                  CRC32 of the map
     */
    std::uint32_t calculate_map_crc(const std::byte *data, std::size_t size, const std::uint32_t *new_crc = nullptr, std::uint32_t *new_random = nullptr, bool *check_dirty = nullptr);
    
    class Map;
    
    /**
     * Calculate the CRC32 of a map
     * @param  map              map to calculate
     * @param  new_crc          new CRC32 of the map
     * @param  new_random       new random number of the map (if forging a CRC32)
     * @param  check_dirty      optionally set to false if the cache file is not dirty or true if it is
     * @return                  CRC32 of the map
     */
    std::uint32_t calculate_map_crc(const Invader::Map &map, const std::uint32_t *new_crc = nullptr, std::uint32_t *new_random = nullptr, bool *check_dirty = nullptr);
}

#endif
