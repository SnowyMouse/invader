// SPDX-License-Identifier: GPL-3.0-only

#ifndef INVADER__TAG__HEK__CLASS__GARBAGE_HPP
#define INVADER__TAG__HEK__CLASS__GARBAGE_HPP

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
#endif
