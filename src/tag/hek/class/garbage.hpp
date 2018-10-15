/*
 * Invader (c) 2018 Kavawuvi
 *
 * This program is free software under the GNU General Public License v3.0 or later. See LICENSE for more information.
 */

#pragma once

#include "item.hpp"

namespace Invader::HEK {
    ENDIAN_TEMPLATE(EndianType) struct Garbage : Item<EndianType> {
        PAD(0xA8);

        ENDIAN_TEMPLATE(NewType) operator Garbage<NewType>() const noexcept {
            Garbage<NewType> copy = {};
            COPY_ITEM_DATA
            return copy;
        }
    };
    static_assert(sizeof(Garbage<BigEndian>) == 0x3B0);

    void compile_garbage_tag(CompiledTag &compiled, const std::byte *data, std::size_t size);
}
