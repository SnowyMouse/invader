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
        static const std::uint32_t BLAM = 0x626C616D;

        PAD(0x24);

        /** Tag class of this tag */
        BigEndian<TagClassInt> tag_class_int;

        /** CRC32? Unread? */
        BigEndian<std::uint32_t> unknown;

        /** Equals 0x40. Unread? */
        BigEndian<std::uint32_t> header_size;

        PAD(0x8);

        /** Some sort of version value */
        BigEndian<std::uint16_t> version;

        /** Always 255. Unread? */
        BigEndian<std::uint16_t> something_255;

        /** Must be equal to 0x626C616D */
        BigEndian<std::uint32_t> blam;

        /** Get the correct version value for the tag */
        static std::uint16_t version_for_tag(TagClassInt tag_class_int);

        /** Generate a new tag file header for a tag class */
        TagFileHeader(TagClassInt tag_class_int);

        /** Make a blank tag file header. No values will be initialized. */
        TagFileHeader() = default;

        TagFileHeader(const TagFileHeader &copy) = default;
        ~TagFileHeader() = default;
    };

    static_assert(sizeof(TagFileHeader) == 0x40);
}
#endif
