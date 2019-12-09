// SPDX-License-Identifier: GPL-3.0-only

#ifndef INVADER__INFO__LANGUAGE_HPP
#define INVADER__INFO__LANGUAGE_HPP

#include <vector>
#include <string>

namespace Invader {
    std::vector<std::string> get_languages_for_resources(const std::size_t *bitmaps_offsets, const std::size_t *bitmaps_sizes, std::size_t bitmap_count, const std::size_t *sounds_offsets, const std::size_t *sounds_sizes, std::size_t sounds_count, bool &all_languages);
}

#endif
