// SPDX-License-Identifier: GPL-3.0-only

#ifndef INVADER__TAG__HEK__HEADER_HPP
#define INVADER__TAG__HEK__HEADER_HPP

#include <optional>

#include "../../hek/pad.hpp"
#include "../../hek/fourcc.hpp"
#include "../../hek/data_type.hpp"
#include "../../hek/endian.hpp"

namespace Invader::HEK {
    /**
     * This header takes up the first 64 bytes of a Halo Editing Kit tag file.
     */
    struct TagFileHeader {
        enum : std::uint32_t {
            BLAM = 0x626C616D
        };

        /** In some of the older point physics tags, this is some sort of tag ID; it's unused */
        BigEndian<TagID> tag_id_unused;

        /** In some of the older point physics tags, this is set to the tag name; it's unused */
        TagString tag_name_unused;

        /** Tag class of this tag */
        BigEndian<TagFourCC> tag_fourcc;

        /** CRC32? Unread? */
        BigEndian<std::uint32_t> crc32;

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
        static std::uint16_t version_for_tag(TagFourCC tag_fourcc);

        /** Generate a new tag file header for a tag class */
        TagFileHeader(TagFourCC tag_fourcc);

        /**
         * Validate the header, throwing an exception if not valid
         * @param header             pointer to the header to validate
         * @param tag_file_size      size of the tag file
         * @param expected_tag_class tag class expected
         * @throws                   std::exception if not valid
         */
        static void validate_header(const TagFileHeader *header, std::size_t tag_file_size, std::optional<TagFourCC> expected_tag_class = std::nullopt);

        /** Make a blank tag file header. No values will be initialized. */
        TagFileHeader() = default;

        TagFileHeader(const TagFileHeader &copy) = default;
        ~TagFileHeader() = default;
    };

    static_assert(sizeof(TagFileHeader) == 0x40);
}
#endif
