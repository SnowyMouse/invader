// SPDX-License-Identifier: GPL-3.0-only

#ifndef INVADER__TAG__INDEX__INDEX_HPP
#define INVADER__TAG__INDEX__INDEX_HPP

#include <utility>
#include <vector>
#include <string>
#include "../../hek/fourcc.hpp"
#include "../../file/file.hpp"

namespace Invader {
    /**
     * Get the stock indices for the given map for demo
     * @param map_name scenario name
     * @return         indices, if present
     */
    std::optional<std::vector<File::TagFilePath>> demo_indices(const char *map_name);

    /**
     * Get the stock indices for the given map for retail
     * @param map_name scenario name
     * @return         indices, if present
     */
    std::optional<std::vector<File::TagFilePath>> retail_indices(const char *map_name);

    /**
     * Get the stock indices for the given map for custom edition
     * @param map_name scenario name
     * @return         indices, if present
     */
    std::optional<std::vector<File::TagFilePath>> custom_edition_indices(const char *map_name);

    /**
     * Get the stock indices for the given map for CEA (MCC)
     * @param map_name scenario name
     * @return         indices, if present
     */
    std::optional<std::vector<File::TagFilePath>> mcc_cea_indices(const char *map_name);
}

#endif
