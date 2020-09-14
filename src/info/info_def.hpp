// SPDX-License-Identifier: GPL-3.0-only

#ifndef INVADER__INFO__INFO_DEF_HPP
#define INVADER__INFO__INFO_DEF_HPP

#include <vector>
#include <optional>

namespace Invader {
    class Map;
}

namespace Invader::Info {
    /**
     * Check if the indices are valid for stock Halo Custom Edition
     * @param map map to check
     * @return    true if valid
     */
    bool check_if_valid_indexed_tags_for_stock_custom_edition(const Invader::Map &map);
    
    /**
     * List all indices of external tags with the given parameters
     * @param map         map to check
     * @param data_type   resource map type
     * @param by_index    check indexed tags
     * @param by_resource check tags with external pointers to resource maps
     * @return            vector of all external tags with the parameters given
     */
    std::vector<std::size_t> find_external_tags_indices(const Invader::Map &map, Map::DataMapType data_type, bool by_index, bool by_resource);
    
    /**
     * Calculate the number of stubbed tags in the map
     * @param map map to check
     * @return    number of stubs
     */
    std::size_t calculate_stub_count(const Invader::Map &map) noexcept;
    
    /**
     * Find all languages that match the given map
     * @param map to check
     * @param all boolean output that gets set to true if it matches every language
     * @return    vector of all languages matched
     */
    std::vector<std::string> find_languages_for_map(const Invader::Map &map, bool &all);
    
    void overview(const Invader::Map &);
    void build(const Invader::Map &);
    void compressed(const Invader::Map &);
    void crc32(const Invader::Map &);
    void crc32_mismatched(const Invader::Map &);
    void dirty(const Invader::Map &);
    void engine(const Invader::Map &);
    void external_bitmap_indices(const Invader::Map &);
    void external_bitmaps(const Invader::Map &);
    void external_indices(const Invader::Map &);
    void external_loc_indices(const Invader::Map &);
    void external_pointers(const Invader::Map &);
    void external_sound_indices(const Invader::Map &);
    void external_sounds(const Invader::Map &);
    void external_tags(const Invader::Map &);
    void languages(const Invader::Map &);
    void map_type(const Invader::Map &);
    void protection(const Invader::Map &);
    void scenario(const Invader::Map &);
    void scenario_path(const Invader::Map &);
    void tag_count(const Invader::Map &);
    void stub_count(const Invader::Map &);
    void tags(const Invader::Map &);
    void tags_external_bitmap_indices(const Invader::Map &);
    void tags_external_loc_indices(const Invader::Map &);
    void tags_external_pointers(const Invader::Map &);
    void tags_external_sound_indices(const Invader::Map &);
    void tags_external_indices(const Invader::Map &);
    void uncompressed_size(const Invader::Map &);
}

#endif
