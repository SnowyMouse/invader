/*
 * Invader (c) 2018 Kavawuvi
 *
 * This program is free software under the GNU General Public License v3.0 or later. See LICENSE for more information.
 */

#ifndef INVADER__TAG__HEK__CLASS__UNICODE_STRING_LIST_HPP
#define INVADER__TAG__HEK__CLASS__UNICODE_STRING_LIST_HPP

#include "../../compiled_tag.hpp"
#include "../../../hek/data_type.hpp"
#include "../header.hpp"

namespace Invader::HEK {
    ENDIAN_TEMPLATE(EndianType) struct UnicodeStringListString {
        TagDataOffset<EndianType> string;
        ENDIAN_TEMPLATE(NewType) operator UnicodeStringListString<NewType>() const noexcept {
            UnicodeStringListString<NewType> copy;
            COPY_THIS(string);
            return copy;
        }
    };
    static_assert(sizeof(UnicodeStringListString<BigEndian>) == 0x14);

    ENDIAN_TEMPLATE(EndianType) struct UnicodeStringList {
        TagReflexive<EndianType, UnicodeStringListString> strings;
        ENDIAN_TEMPLATE(NewType) operator UnicodeStringList<NewType>() const noexcept {
            UnicodeStringList<NewType> copy;
            COPY_THIS(strings);
            return copy;
        }
    };
    static_assert(sizeof(UnicodeStringList<BigEndian>) == 0xC);

    void compile_unicode_string_list_tag(CompiledTag &compiled, const std::byte *data, std::size_t size);
}
#endif
