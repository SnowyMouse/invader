/*
 * Invader (c) 2018 Kavawuvi
 *
 * This program is free software under the GNU General Public License v3.0 or later. See LICENSE for more information.
 */

#pragma once

#include "../../compiled_tag.hpp"
#include "../../../hek/data_type.hpp"
#include "../header.hpp"

namespace Invader::HEK {
    ENDIAN_TEMPLATE(EndianType) struct ColorTableColor {
        TagString name;
        ColorARGB<EndianType> color;

        ENDIAN_TEMPLATE(NewType) operator ColorTableColor<NewType>() const noexcept {
            ColorTableColor<NewType> copy;
            COPY_THIS(name);
            COPY_THIS(color);
            return copy;
        }
    };
    static_assert(sizeof(ColorTableColor<BigEndian>) == 0x30);

    ENDIAN_TEMPLATE(EndianType) struct ColorTable {
        TagReflexive<EndianType, ColorTableColor> colors;

        ENDIAN_TEMPLATE(NewType) operator ColorTable<NewType>() const noexcept {
            ColorTable<NewType> copy;
            COPY_THIS(colors);
            return copy;
        }
    };
    static_assert(sizeof(ColorTable<BigEndian>) == 0xC);

    void compile_color_table_tag(CompiledTag &compiled, const std::byte *data, std::size_t size);
}
