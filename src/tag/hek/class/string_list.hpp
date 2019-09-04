/*
 * Invader (c) 2018 Kavawuvi
 *
 * This program is free software under the GNU General Public License v3.0 or later. See LICENSE for more information.
 */

#ifndef INVADER__TAG__HEK__CLASS__STRING_LIST_HPP
#define INVADER__TAG__HEK__CLASS__STRING_LIST_HPP

#include "../../compiled_tag.hpp"
#include "../../../hek/data_type.hpp"
#include "../header.hpp"

namespace Invader::HEK {
    ENDIAN_TEMPLATE(EndianType) struct StringListString {
        TagDataOffset<EndianType> string;
        ENDIAN_TEMPLATE(NewType) operator StringListString<NewType>() const noexcept {
            StringListString<NewType> copy;
            COPY_THIS(string);
            return copy;
        }
    };
    static_assert(sizeof(StringListString<BigEndian>) == 0x14);

    ENDIAN_TEMPLATE(EndianType) struct StringList {
        TagReflexive<EndianType, StringListString> strings;
        ENDIAN_TEMPLATE(NewType) operator StringList<NewType>() const noexcept {
            StringList<NewType> copy;
            COPY_THIS(strings);
            return copy;
        }
    };
    static_assert(sizeof(StringList<BigEndian>) == 0xC);

    void compile_string_list_tag(CompiledTag &compiled, const std::byte *data, std::size_t size);
}
#endif
