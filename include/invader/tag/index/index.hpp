// SPDX-License-Identifier: GPL-3.0-only

#ifndef INVADER__TAG__INDEX__INDEX_HPP
#define INVADER__TAG__INDEX__INDEX_HPP

#include <utility>
#include <vector>
#include <string>
#include "../../hek/class_int.hpp"

namespace Invader {
    /**
     * Get the stock indices for the given map for demo
     * @param map_name scenario name
     */
    std::vector<std::pair<HEK::TagClassInt, std::string>> demo_indices(const char *map_name);

    /**
     * Get the stock indices for the given map for retail
     * @param map_name scenario name
     */
    std::vector<std::pair<HEK::TagClassInt, std::string>> retail_indices(const char *map_name);

    /**
     * Get the stock indices for the given map for custom edition
     * @param map_name scenario name
     */
    std::vector<std::pair<HEK::TagClassInt, std::string>> custom_edition_indices(const char *map_name);
}

#endif
