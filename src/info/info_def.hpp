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
     * @param data_type   resource map type (if std::nullopt, then any type)
     * @param by_index    check indexed tags
     * @param by_resource check tags with external pointers to resource maps
     * @param inverted    invert the check and look for internal tags
     * @return            vector of all external tags with the parameters given
     */
    std::vector<std::size_t> find_external_tags_indices(const Invader::Map &map, std::optional<Map::DataMapType> data_type, bool by_index, bool by_resource, bool inverted = false);
    
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
    
    enum CheckTagOrderResult {
        /** No index stored for this map and cache version */
        CHECK_TAG_ORDER_RESULT_UNKNOWN,
        
        /** Tag order does not match at all */
        CHECK_TAG_ORDER_RESULT_MISMATCHED_TAGS,
        
        /** Tag order does not completely match but is network compatible if the stock map is being hosted and you are joining with the input map */
        CHECK_TAG_ORDER_RESULT_NETWORK_MATCHED_AS_CLIENT,
        
        /** Tag order does not completely match but is network compatible if the input map is being hosted and you are joining with the stock map */
        CHECK_TAG_ORDER_RESULT_NETWORK_MATCHED_AS_HOST,
        
        /** Tag order does not completely match but is network compatible both ways */
        CHECK_TAG_ORDER_RESULT_NETWORK_MATCHED,
        
        /** Tag order completely matches */
        CHECK_TAG_ORDER_RESULT_MATCHED
    };
    
    /**
     * Check if tag order matches
     * @param map to check
     * @return    whether or not the map matches or is at least network compatible
     */
    CheckTagOrderResult check_tag_order(const Invader::Map &map);
    
    void overview(const Invader::Map &);
    void build(const Invader::Map &);
    void crc32(const Invader::Map &);
    void crc32_mismatched(const Invader::Map &);
    void engine(const Invader::Map &);
    void external_bitmap_indices(const Invader::Map &);
    void external_bitmap_indices_count(const Invader::Map &);
    void external_bitmap_pointers(const Invader::Map &);
    void external_bitmap_pointers_count(const Invader::Map &);
    void external_bitmaps(const Invader::Map &);
    void external_bitmaps_count(const Invader::Map &);
    void external_indices(const Invader::Map &);
    void external_indices_count(const Invader::Map &);
    void external_loc_indices(const Invader::Map &);
    void external_loc_indices_count(const Invader::Map &);
    void external_sound_indices(const Invader::Map &);
    void external_sound_indices_count(const Invader::Map &);
    void external_sound_pointers(const Invader::Map &);
    void external_sound_pointers_count(const Invader::Map &);
    void external_sounds(const Invader::Map &);
    void external_sounds_count(const Invader::Map &);
    void external_tags(const Invader::Map &);
    void external_tags_count(const Invader::Map &);
    void internal_bitmaps(const Invader::Map &);
    void internal_bitmaps_count(const Invader::Map &);
    void internal_sounds(const Invader::Map &);
    void internal_sounds_count(const Invader::Map &);
    void is_compressed(const Invader::Map &);
    void is_dirty(const Invader::Map &);
    void is_protected(const Invader::Map &);
    void languages(const Invader::Map &);
    void map_type(const Invader::Map &);
    void protection_issues(const Invader::Map &);
    void scenario(const Invader::Map &);
    void scenario_path(const Invader::Map &);
    void stub_count(const Invader::Map &);
    void tags(const Invader::Map &);
    void tags_count(const Invader::Map &);
    void tag_order_match(const Invader::Map &);
    void uncompressed_size(const Invader::Map &);
    void uses_external_pointers(const Invader::Map &);
}

#endif
