/*
 * Invader (c) 2018 Kavawuvi
 *
 * This program is free software under the GNU General Public License v3.0 or later. See LICENSE for more information.
 */

#include "../hek/map.hpp"
#include "../tag/hek/class/bitmap.hpp"
#include "../tag/hek/class/gbxmodel.hpp"
#include "../tag/hek/class/scenario.hpp"
#include "../tag/hek/class/sound.hpp"

#include "map.hpp"

namespace Invader {
    Map Map::map_with_copy(const std::byte *data, std::size_t data_size,
                           const std::byte *bitmaps_data, std::size_t bitmaps_data_size,
                           const std::byte *loc_data, std::size_t loc_data_size,
                           const std::byte *sounds_data, std::size_t sounds_data_size) {
        Map map;
        map.data_m.insert(map.data_m.end(), data, data + data_size);
        map.bitmap_data_m.insert(map.bitmap_data_m.end(), bitmaps_data, bitmaps_data + bitmaps_data_size);
        map.sound_data_m.insert(map.sound_data_m.end(), sounds_data, sounds_data + sounds_data_size);
        map.loc_data_m.insert(map.loc_data_m.end(), loc_data, loc_data + loc_data_size);
        map.data_length = data_size;
        map.bitmap_data_length = bitmaps_data_size;
        map.loc_data_length = loc_data_size;
        map.sound_data_length = sounds_data_size;
        map.load_map();
        return map;
    }

    Map Map::map_with_pointer(std::byte *data, std::size_t data_size,
                              std::byte *bitmaps_data, std::size_t bitmaps_data_size,
                              std::byte *loc_data, std::size_t loc_data_size,
                              std::byte *sounds_data, std::size_t sounds_data_size) {
        Map map;
        map.data = data;
        map.data_length = data_size;
        map.bitmap_data = bitmaps_data;
        map.bitmap_data_length = bitmaps_data_size;
        map.loc_data = loc_data;
        map.loc_data_length = loc_data_size;
        map.sound_data = sounds_data;
        map.sound_data_length = sounds_data_size;
        map.load_map();
        return map;
    }

    std::byte *Map::get_data_at_offset(std::size_t offset, std::size_t minimum_size, DataMapType map_type) {
        std::size_t max_length;
        std::byte *data_ptr;
        switch(map_type) {
            case DATA_MAP_CACHE:
                max_length = this->data_length;
                data_ptr = this->data;
                break;

            case DATA_MAP_BITMAP:
                max_length = this->bitmap_data_length;
                data_ptr = this->bitmap_data;
                break;

            case DATA_MAP_SOUND:
                max_length = this->sound_data_length;
                data_ptr = this->sound_data;
                break;

            case DATA_MAP_LOC:
                max_length = this->loc_data_length;
                data_ptr = this->loc_data;
                break;

            default:
                throw OutOfBoundsException();
        }

        if(offset >= max_length || offset + minimum_size > max_length) {
            throw OutOfBoundsException();
        }
        else {
            return data_ptr + offset;
        }
    }

    const std::byte *Map::get_data_at_offset(std::size_t offset, std::size_t minimum_size, DataMapType map_type) const {
        return const_cast<Map *>(this)->get_data_at_offset(offset, minimum_size, map_type);
    }

    std::byte *Map::get_tag_data_at_offset(std::size_t offset, std::size_t minimum_size) {
        if(offset >= this->tag_data_length || offset + minimum_size > this->tag_data_length) {
            throw OutOfBoundsException();
        }
        else {
            return this->tag_data + offset;
        }
    }

    const std::byte *Map::get_tag_data_at_offset(std::size_t offset, std::size_t minimum_size) const {
        return const_cast<Map *>(this)->get_tag_data_at_offset(offset, minimum_size);
    }

    std::byte *Map::resolve_tag_data_pointer(HEK::Pointer pointer, std::size_t minimum_size) {
        return this->get_tag_data_at_offset(pointer - this->base_memory_address, minimum_size);
    }

    const std::byte *Map::resolve_tag_data_pointer(HEK::Pointer pointer, std::size_t minimum_size) const {
        return const_cast<Map *>(this)->resolve_tag_data_pointer(pointer, minimum_size);
    }

    std::size_t Map::tag_count() const noexcept {
        return this->tags.size();
    }

    Tag &Map::get_tag(std::size_t index) {
        if(index >= this->tag_count()) {
            throw OutOfBoundsException();
        }
        else {
            return this->tags[index];
        }
    }

    const Tag &Map::get_tag(std::size_t index) const {
        return const_cast<Map *>(this)->get_tag(index);
    }

    std::size_t Map::get_scenario_tag_id() const noexcept {
        return this->scenario_tag_id;
    }

    HEK::CacheFileTagDataHeader &Map::get_tag_data_header() noexcept {
        return *reinterpret_cast<HEK::CacheFileTagDataHeader *>(this->tag_data);
    }

    const HEK::CacheFileTagDataHeader &Map::get_tag_data_header() const noexcept {
        return const_cast<Map *>(this)->get_tag_data_header();
    }

    HEK::CacheFileHeader &Map::get_cache_file_header() noexcept {
        return *reinterpret_cast<HEK::CacheFileHeader *>(this->data);
    }

    const HEK::CacheFileHeader &Map::get_cache_file_header() const noexcept {
        return const_cast<Map *>(this)->get_cache_file_header();
    }

    void Map::load_map() {
        using namespace Invader::HEK;

        // Get header
        const auto &header = *reinterpret_cast<const CacheFileHeader *>(this->get_data_at_offset(0, sizeof(CacheFileHeader)));

        // Check if invalid
        if(header.file_size > this->data_length || header.head_literal != CACHE_FILE_HEAD || header.foot_literal != CACHE_FILE_FOOT || header.build.string[31] != 0) {
            throw InvalidMapException();
        }

        // Get tag data
        this->tag_data_length = header.tag_data_size;
        this->tag_data = this->get_data_at_offset(header.tag_data_offset, this->tag_data_length);

        this->populate_tag_array();
    }

    void Map::populate_tag_array() {
        using namespace Invader::HEK;
        const auto &header = *reinterpret_cast<const CacheFileTagDataHeaderPC *>(this->get_tag_data_at_offset(0, sizeof(CacheFileTagDataHeaderPC)));

        std::size_t tag_count = header.tag_count;
        this->tags.reserve(tag_count);

        this->scenario_tag_id = header.scenario_tag.read().index;
        if(this->scenario_tag_id >= tag_count) {
            throw OutOfBoundsException();
        }

        // First get the tags
        const auto *tags = reinterpret_cast<const CacheFileTagDataTag *>(this->resolve_tag_data_pointer(header.tag_array_address, sizeof(CacheFileTagDataTag) * tag_count));
        for(std::size_t i = 0; i < tag_count; i++) {
            this->tags.push_back(Tag(*this));
            auto &tag = this->tags[i];
            tag.p_tag_class_int = tags[i].primary_class;
            tag.p_path = reinterpret_cast<const char *>(this->resolve_tag_data_pointer(tags[i].tag_path));

            if(tag.p_tag_class_int == TagClassInt::TAG_CLASS_SCENARIO_STRUCTURE_BSP) {
                continue;
            }
            else if(tags[i].indexed) {
                // TODO: Handle indexed tags
                tag.indexed = true;
            }
            else {
                tag.base_struct_pointer = tags[i].tag_data;
            }
        }

        this->get_bsps();
    }

    void Map::get_bsps() {
        using namespace Invader::HEK;

        auto &scenario_tag = this->tags[this->scenario_tag_id];
        auto &tag = scenario_tag.get_base_struct<Scenario>();
        std::size_t bsp_count = tag.structure_bsps.count;
        auto *bsps = scenario_tag.resolve_reflexive(tag.structure_bsps);

        for(std::size_t i = 0; i < bsp_count; i++) {
            auto &bsp = bsps[i];
            std::size_t bsp_id = bsp.structure_bsp.tag_id.read().index;
            if(bsp_id >= this->tags.size()) {
                throw OutOfBoundsException();
            }

            auto &bsp_tag = this->tags[bsp_id];
            bsp_tag.tag_data_size = bsp.bsp_size;
            bsp_tag.base_struct_offset = bsp.bsp_start;
            bsp_tag.base_struct_pointer = bsp.bsp_address;
        }
    }
}
