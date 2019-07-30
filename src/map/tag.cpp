/*
 * Invader (c) 2018 Kavawuvi
 *
 * This program is free software under the GNU General Public License v3.0 or later. See LICENSE for more information.
 */

#include "tag.hpp"
#include "map.hpp"

namespace Invader {
    const std::string &Tag::path() const noexcept {
        return this->p_path;
    }

    bool Tag::is_indexed() const noexcept {
        return this->indexed;
    }

    HEK::TagClassInt Tag::tag_class_int() const noexcept {
        return this->p_tag_class_int;
    }

    std::byte *Tag::data(HEK::Pointer pointer, std::size_t minimum) {
        using namespace HEK;

        if(this->indexed || this->p_tag_class_int == TagClassInt::TAG_CLASS_SCENARIO_STRUCTURE_BSP) {
            auto edge = this->base_struct_offset + this->base_struct_offset;
            auto offset = pointer - this->base_struct_pointer;

            if((offset >= edge) || (offset + minimum > edge)) {
                throw OutOfBoundsException();
            }

            Map::DataMapType type;
            if(this->indexed) {
                switch(this->p_tag_class_int) {
                    case TagClassInt::TAG_CLASS_BITMAP:
                        type = Map::DataMapType::DATA_MAP_BITMAP;
                        break;
                    case TagClassInt::TAG_CLASS_SOUND:
                        type = Map::DataMapType::DATA_MAP_SOUND;
                        break;
                    default:
                        type = Map::DataMapType::DATA_MAP_LOC;
                        break;
                }
            }
            else {
                type = Map::DataMapType::DATA_MAP_CACHE;
            }

            return this->p_map.get_data_at_offset(pointer - this->base_struct_pointer, minimum, type);
        }
        else {
            return this->p_map.resolve_tag_data_pointer(pointer, minimum);
        }
    }

    Tag::Tag(Map &map) : p_map(map) {}
}
