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

    HEK::TagClassInt Tag::tag_class_int() const noexcept {
        return this->p_tag_class_int;
    }

    std::byte *Tag::data(std::size_t minimum) {
        if(this->p_tag_data.size() >= minimum) {
            return this->p_tag_data.data();
        }
        else if(minimum == 0) {
            return nullptr;
        }
        else {
            throw OutOfBoundsException();
        }
    }

    const std::byte *Tag::data(std::size_t minimum) const {
        return const_cast<Tag *>(this)->data(minimum);
    }

    std::size_t Tag::data_size() const noexcept {
        return this->p_tag_data.size();
    }

    std::byte *Tag::asset_data(std::size_t minimum) {
        if(this->p_asset_data.size() >= minimum) {
            return this->p_asset_data.data();
        }
        else if(minimum == 0) {
            return nullptr;
        }
        else {
            throw OutOfBoundsException();
        }
    }

    const std::byte *Tag::asset_data(std::size_t minimum) const {
        return const_cast<Tag *>(this)->asset_data(minimum);
    }

    std::size_t Tag::asset_data_size() const noexcept {
        return this->p_asset_data.size();
    }

    Tag::Tag(Map &map) : map(map) {}
}
