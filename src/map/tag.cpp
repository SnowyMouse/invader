// SPDX-License-Identifier: GPL-3.0-only

#include <invader/map/tag.hpp>
#include <invader/map/map.hpp>

namespace Invader {
    const std::string &Tag::path() const noexcept {
        return this->p_path;
    }

    bool Tag::is_indexed() const noexcept {
        return this->indexed;
    }

    bool Tag::data_is_available() const noexcept {
        using namespace HEK;

        // If it's indexed, check if the corresponding data is available
        if(this->is_indexed()) {
            switch(this->p_tag_class_int) {
                case TagClassInt::TAG_CLASS_BITMAP:
                    return this->p_map.bitmap_data != nullptr;
                case TagClassInt::TAG_CLASS_SOUND:
                    return this->p_map.sound_data != nullptr;
                default:
                    return this->p_map.loc_data != nullptr;
            }
        }

        // If the base struct pointer is 0, well... lol
        else if(this->base_struct_pointer == 0) {
            return false;
        }

        // Return true otherwise
        else {
            return true;
        }
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
