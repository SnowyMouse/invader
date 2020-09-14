// SPDX-License-Identifier: GPL-3.0-only

#include <invader/map/map.hpp>
#include <invader/printf.hpp>
#include <invader/file/file.hpp>
#include <invader/hek/map.hpp>
#include <invader/resource/list/resource_list.hpp>
#include "language/language.hpp"
#include "info_def.hpp"

namespace Invader::Info {
    std::vector<std::pair<std::size_t, std::size_t>> resource_offsets_for_tag(const Invader::Tag &tag) {
        std::vector<std::pair<std::size_t, std::size_t>> offsets;
        
        // Ignore stubbed/indexed tags
        if(!tag.data_is_available()) {
            return {};
        }
        
        // If it's a bitmap tag, iterate through the bitmap data things. Otherwise, iterate through the permutations if it's a sound tag.
        switch(tag.get_tag_class_int()) {
            case TagClassInt::TAG_CLASS_BITMAP: {
                auto &bitmap_header = tag.get_base_struct<HEK::Bitmap>();
                std::size_t bitmap_data_count = bitmap_header.bitmap_data.count;
                auto *bitmap_data = tag.resolve_reflexive(bitmap_header.bitmap_data);
                for(std::size_t b = 0; b < bitmap_data_count; b++) {
                    auto &bd = bitmap_data[b];
                    if(bd.flags.read() & HEK::BitmapDataFlagsFlag::BITMAP_DATA_FLAGS_FLAG_EXTERNAL) {
                        offsets.emplace_back(bd.pixel_data_offset.read(), bd.pixel_data_size.read());
                    }
                }
                break;
            }

            case TagClassInt::TAG_CLASS_SOUND: {
                auto &sound_header = tag.get_base_struct<HEK::Sound>();
                std::size_t pitch_range_count = sound_header.pitch_ranges.count;
                auto *pitch_ranges = tag.resolve_reflexive(sound_header.pitch_ranges);
                for(std::size_t pr = 0; pr < pitch_range_count; pr++) {
                    auto &pitch_range = pitch_ranges[pr];
                    auto *permutations = tag.resolve_reflexive(pitch_range.permutations);
                    std::size_t permutation_count = pitch_range.permutations.count;
                    for(std::size_t pe = 0; pe < permutation_count; pe++) {
                        auto &p = permutations[pe];
                        if(p.samples.external & 1) {
                            offsets.emplace_back(p.samples.file_offset.read(), p.samples.size.read());
                            break;
                        }
                    }
                }
                break;
            }

            default:
                break;
        }
        
        return offsets;
    }
    
    std::vector<std::size_t> find_external_tags_indices(const Invader::Map &map, Map::DataMapType data_type, bool by_index, bool by_resource) {
        std::vector<std::size_t> indices;
        std::size_t tag_count = map.get_tag_count();
        std::vector<HEK::TagClassInt> allowed_classes;
        switch(data_type) {
            case Map::DataMapType::DATA_MAP_BITMAP:
                allowed_classes.push_back(HEK::TagClassInt::TAG_CLASS_BITMAP);
                break;
            case Map::DataMapType::DATA_MAP_SOUND:
                allowed_classes.push_back(HEK::TagClassInt::TAG_CLASS_SOUND);
                break;
            case Map::DataMapType::DATA_MAP_LOC:
                allowed_classes.push_back(HEK::TagClassInt::TAG_CLASS_FONT);
                allowed_classes.push_back(HEK::TagClassInt::TAG_CLASS_HUD_MESSAGE_TEXT);
                allowed_classes.push_back(HEK::TagClassInt::TAG_CLASS_UNICODE_STRING_LIST);
                break;
            default:
                std::terminate();
        }
        indices.reserve(tag_count);
        
        // Find all the tags that are external
        for(std::size_t i = 0; i < tag_count; i++) {
            auto &tag = map.get_tag(i);
            bool found = false;
            
            // Check if we can deal with this tag type
            auto tag_class_int = tag.get_tag_class_int();
            for(auto &c : allowed_classes) {
                if(c == tag_class_int) {
                    found = true;
                    break;
                }
            }
            
            if(!found) {
                continue;
            }
            
            // Add that thing
            if((by_index && tag.is_indexed()) || (by_resource && resource_offsets_for_tag(tag).size() > 0)) {
                indices.push_back(i);
            }
        }
        indices.shrink_to_fit();
        return indices;
    }
    
    std::size_t calculate_stub_count(const Invader::Map &map) noexcept {
        auto tag_count = map.get_tag_count();
        std::size_t stub_count = 0;
        for(std::size_t i = 0; i < tag_count; i++) {
            auto &tag = map.get_tag(i);
            if(!tag.is_stub()) {
                stub_count++;
            }
        }
        return stub_count;
    }
    
    bool check_if_valid_indexed_tags_for_stock_custom_edition(const Invader::Map &map) {
        auto tag_count = map.get_tag_count();
        for(std::size_t i = 0; i < tag_count; i++) {
            auto &tag = map.get_tag(i);
            if(tag.is_indexed()) {
                switch(tag.get_tag_class_int()) {
                    case HEK::TagClassInt::TAG_CLASS_SOUND: {
                        bool found = false;
                        for(const char * const *i = get_default_sound_resources(); *i; i++) {
                            if(tag.get_path() == File::split_tag_class_extension(*i)->path.c_str()) {
                                found = true;
                                break;
                            }
                        }
                        
                        // If it isn't present in default resources, bail
                        if(!found) {
                            return false;
                        }
                        break;
                    }
                        
                    // Check if out of bounds or if the index is not odd (since that's not a thing in default resource maps)
                    case HEK::TagClassInt::TAG_CLASS_BITMAP: {
                        auto resource_index = tag.get_resource_index().value();
                        if(resource_index % 2 != 1 || resource_index > get_default_bitmap_resources_count() * 2) {
                            return false;
                        }
                        break;
                    }
                    
                    // Check if out of bounds
                    default:
                        if(tag.get_resource_index().value() > get_default_bitmap_resources_count()) {
                            return false;
                        }
                        break;
                }
            }
        }
        return true;
    }
    
    std::vector<std::string> find_languages_for_map(const Invader::Map &map, bool &all) {
        auto tag_count = map.get_tag_count();
        std::vector<std::string> languages;
        all = false;
        auto engine = map.get_engine();
        
        // If Custom Edition, check if we have external offsets or invalid indices
        if(engine == HEK::CacheFileEngine::CACHE_FILE_CUSTOM_EDITION) {
            // Do we have invalid indices?
            if(!check_if_valid_indexed_tags_for_stock_custom_edition(map)) {
                return {};
            }
            
            // Now compile a list of offsets
            std::vector<std::size_t> bitmap_offsets, bitmap_sizes, sound_offsets, sound_sizes;
            for(std::size_t i = 0; i < tag_count; i++) {
                auto &tag = map.get_tag(i);
                switch(tag.get_tag_class_int()) {
                    case HEK::TagClassInt::TAG_CLASS_SOUND:
                        for(auto &i : resource_offsets_for_tag(tag)) {
                            sound_offsets.push_back(i.first);
                            sound_sizes.push_back(i.second);
                        }
                        break;
                    
                    case HEK::TagClassInt::TAG_CLASS_BITMAP:
                        for(auto &i : resource_offsets_for_tag(tag)) {
                            bitmap_offsets.push_back(i.first);
                            bitmap_sizes.push_back(i.second);
                        }
                        break;
                    
                    default:
                        break;
                }
            }
            
            // Crunch it
            languages = get_languages_for_resources(bitmap_offsets.data(), bitmap_sizes.data(), bitmap_sizes.size(), sound_offsets.data(), sound_sizes.data(), sound_sizes.size(), all);
        }
        
        return languages;
    }
    
    void build(const Invader::Map &map) {
        oprintf("%s\n", map.get_build());
    }
    
    void compressed(const Invader::Map &map) {
        oprintf("%i\n", map.is_compressed());
    }
    
    void crc32(const Invader::Map &map) {
        oprintf("%08X\n", map.get_crc32());
    }
    void crc32_mismatched(const Invader::Map &map) {
        oprintf("%i\n", map.get_crc32() != map.get_header_crc32());
    }
    
    void dirty(const Invader::Map &map) {
        oprintf("%i\n", !map.is_clean());
    }
    
    void engine(const Invader::Map &map) {
        oprintf("%s\n", engine_name(map.get_engine()));
    }
    
    void external_bitmap_indices(const Invader::Map &map) {
        oprintf("%zu\n", find_external_tags_indices(map, Map::DataMapType::DATA_MAP_BITMAP, true, false).size());
    }
    void external_bitmaps(const Invader::Map &map) {
        oprintf("%zu\n", find_external_tags_indices(map, Map::DataMapType::DATA_MAP_BITMAP, true, true).size());
    }
    
    void external_loc_indices(const Invader::Map &map) {
        oprintf("%zu\n", find_external_tags_indices(map, Map::DataMapType::DATA_MAP_LOC, true, false).size());
    }
    
    void external_sound_indices(const Invader::Map &map) {
        oprintf("%zu\n", find_external_tags_indices(map, Map::DataMapType::DATA_MAP_SOUND, true, false).size());
    }
    void external_sounds(const Invader::Map &map) {
        oprintf("%zu\n", find_external_tags_indices(map, Map::DataMapType::DATA_MAP_SOUND, true, true).size());
    }
    
    void external_tags(const Invader::Map &map) {
        oprintf("%zu\n", find_external_tags_indices(map, Map::DataMapType::DATA_MAP_BITMAP, true, true).size() + find_external_tags_indices(map, Map::DataMapType::DATA_MAP_LOC, true, true).size() + find_external_tags_indices(map, Map::DataMapType::DATA_MAP_SOUND, true, true).size());
    }
    void external_indices(const Invader::Map &map) {
        oprintf("%zu\n", find_external_tags_indices(map, Map::DataMapType::DATA_MAP_BITMAP, true, true).size() + find_external_tags_indices(map, Map::DataMapType::DATA_MAP_LOC, true, false).size() + find_external_tags_indices(map, Map::DataMapType::DATA_MAP_SOUND, true, false).size());
    }
    void external_pointers(const Invader::Map &map) {
        oprintf("%i\n", (find_external_tags_indices(map, Map::DataMapType::DATA_MAP_BITMAP, false, true).size() + find_external_tags_indices(map, Map::DataMapType::DATA_MAP_LOC, false, true).size() + find_external_tags_indices(map, Map::DataMapType::DATA_MAP_SOUND, false, true).size()) > 0);
    }
    
    void languages(const Invader::Map &map) {
        bool all;
        auto languages = find_languages_for_map(map, all);
        if(all) {
            oprintf("all\n");
        }
        else if(languages.size() == 0) {
            oprintf("unknown\n");
        }
        else for(auto &i : languages) {
            oprintf("%s\n", i.c_str());
        }
    }
    
    void map_type(const Invader::Map &map) {
        oprintf("%s\n", type_name(map.get_type()));
    }
    
    void protection(const Invader::Map &map) {
        oprintf("%i\n", map.is_protected());
    }
    
    void scenario(const Invader::Map &map) {
        oprintf("%s\n", map.get_scenario_name());
    }
    
    void scenario_path(const Invader::Map &map) {
        oprintf("%s\n", File::halo_path_to_preferred_path(map.get_tag(map.get_scenario_tag_id()).get_path()).c_str());
    }
    
    void tag_count(const Invader::Map &map) {
        oprintf("%zu\n", map.get_tag_count());
    }
    
    void stub_count(const Invader::Map &map) {
        oprintf("%zu\n", calculate_stub_count(map));
    }
    
    void tags(const Invader::Map &map) {
        auto tag_count = map.get_tag_count();
        for(std::size_t i = 0; i < tag_count; i++) {
            auto &tag = map.get_tag(i);
            oprintf("%s.%s\n", File::halo_path_to_preferred_path(tag.get_path()).c_str(), HEK::tag_class_to_extension(tag.get_tag_class_int()));
        }
    }
    
    static void print_all_indices(const Invader::Map &map, const std::vector<std::size_t> &indices) {
        for(auto i : indices) {
            auto &tag = map.get_tag(i);
            oprintf("%s.%s\n", File::halo_path_to_preferred_path(tag.get_path()).c_str(), HEK::tag_class_to_extension(tag.get_tag_class_int()));
        }
    }
    
    void tags_external_bitmap_indices(const Invader::Map &map) {
        print_all_indices(map, find_external_tags_indices(map, Map::DataMapType::DATA_MAP_BITMAP, true, false));
    }
    void tags_external_loc_indices(const Invader::Map &map) {
        print_all_indices(map, find_external_tags_indices(map, Map::DataMapType::DATA_MAP_LOC, true, false));
    }
    void tags_external_pointers(const Invader::Map &map) {
        print_all_indices(map, find_external_tags_indices(map, Map::DataMapType::DATA_MAP_BITMAP, false, true));
        print_all_indices(map, find_external_tags_indices(map, Map::DataMapType::DATA_MAP_LOC, false, true));
        print_all_indices(map, find_external_tags_indices(map, Map::DataMapType::DATA_MAP_SOUND, false, true));
    }
    void tags_external_sound_indices(const Invader::Map &map) {
        print_all_indices(map, find_external_tags_indices(map, Map::DataMapType::DATA_MAP_SOUND, false, true));
    }
    void tags_external_indices(const Invader::Map &map) {
        print_all_indices(map, find_external_tags_indices(map, Map::DataMapType::DATA_MAP_BITMAP, true, false));
        print_all_indices(map, find_external_tags_indices(map, Map::DataMapType::DATA_MAP_LOC, true, false));
        print_all_indices(map, find_external_tags_indices(map, Map::DataMapType::DATA_MAP_SOUND, true, false));
    }
    
    void uncompressed_size(const Invader::Map &map) {
        oprintf("%zu\n", map.get_data_length());
    }
}
