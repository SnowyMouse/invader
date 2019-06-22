/*
 * Invader (c) 2018 Kavawuvi
 *
 * This program is free software under the GNU General Public License v3.0 or later. See LICENSE for more information.
 */

#ifndef INVADER__TAG__HEK__HEADER_HPP
#define INVADER__TAG__HEK__HEADER_HPP

#include "../../hek/pad.hpp"
#include "../../hek/class_int.hpp"
#include "../../hek/endian.hpp"

namespace Invader::HEK {
    /**
     * This header takes up the first 64 bytes of a Halo Editing Kit tag file.
     */
    struct TagFileHeader {
        PAD(0x24);

        /** Tag class of this tag */
        BigEndian<TagClassInt> tag_class_int;

        BigEndian<std::uint32_t> unknown;

        /** Equals 0x40. Unread? */
        BigEndian<std::uint32_t> header_size;

        PAD(0x8);

        BigEndian<std::uint32_t> unknown2;

        BigEndian<std::uint32_t> blam;
    };

    static_assert(sizeof(TagFileHeader) == 0x40);
}
#endif
