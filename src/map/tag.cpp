// SPDX-License-Identifier: GPL-3.0-only

#include <invader/map/tag.hpp>
#include <invader/map/map.hpp>
#include <invader/tag/hek/definition.hpp>

namespace Invader {
    bool Tag::data_is_available() const noexcept {
        using namespace HEK;

        // If it's indexed, check if the corresponding data is available
        if(this->is_indexed()) {
            switch(this->tag_class_int) {
                case TagClassInt::TAG_CLASS_BITMAP:
                    return this->map.bitmap_data != nullptr;
                case TagClassInt::TAG_CLASS_SOUND:
                    return this->map.sound_data != nullptr;
                default:
                    return this->map.loc_data != nullptr;
            }
        }

        // If the base struct pointer is 0xFFFFFFFF, well... lol
        else if(this->base_struct_pointer == CacheFileTagDataBaseMemoryAddress::CACHE_FILE_STUB_MEMORY_ADDRESS) {
            return false;
        }

        // Return true otherwise
        else {
            return true;
        }
    }

    std::byte *Tag::data(HEK::Pointer pointer, std::size_t minimum) {
        using namespace HEK;

        // Indexed sound tags can use data in both the sound tag in the cache file and the sound tag in sounds.map
        if(this->tag_class_int == TagClassInt::TAG_CLASS_SOUND && this->indexed) {
            if(pointer > this->get_map().base_memory_address) {
                return this->map.resolve_tag_data_pointer(pointer, minimum);
            }
            else {
                return this->map.get_data_at_offset(pointer + this->base_struct_offset, minimum, Map::DataMapType::DATA_MAP_SOUND);
            }
        }

        if(this->indexed || (this->tag_class_int == TagClassInt::TAG_CLASS_SCENARIO_STRUCTURE_BSP && pointer >= this->base_struct_pointer)) {
            auto edge = this->base_struct_offset + this->tag_data_size;
            auto offset = this->base_struct_offset + pointer - this->base_struct_pointer;

            if((offset >= edge) || (offset + minimum > edge)) {
                throw OutOfBoundsException();
            }

            Map::DataMapType type;
            if(this->indexed) {
                switch(this->tag_class_int) {
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

            return this->map.get_data_at_offset(offset, minimum, type);
        }
        else {
            return this->map.resolve_tag_data_pointer(pointer, minimum);
        }
    }

    HEK::CacheFileTagDataTag &Tag::get_tag_data_index() noexcept {
        return *reinterpret_cast<HEK::CacheFileTagDataTag *>(this->map.get_tag_data_at_offset(this->tag_data_index_offset, sizeof(HEK::CacheFileTagDataTag)));
    }

    const HEK::CacheFileTagDataTag &Tag::get_tag_data_index() const noexcept {
        return const_cast<Tag *>(this)->get_tag_data_index();
    }

    Tag::Tag(Map &map) : map(map) {}
}
