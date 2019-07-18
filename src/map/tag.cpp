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

    std::byte *Tag::data(std::size_t minimum) {
        if(this->p_data_size >= minimum) {
            return this->p_data;
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

    Tag::Tag(Map &map) : p_map(map) {}
}
