// SPDX-License-Identifier: GPL-3.0-only

#ifndef INVADER__RESOURCE__HEK__IPAK_HPP
#define INVADER__RESOURCE__HEK__IPAK_HPP

#include "../../hek/endian.hpp"

namespace Invader::HEK {
    struct IPAKElement {
        char name[0x100];
        std::byte unknown[0x38];
        LittleEndian<std::uint64_t> offset;
        std::byte unknown2[0x8];
    };
    static_assert(sizeof(IPAKElement) == 0x148);

    struct IPAKBitmapHeader {
        std::byte unknown0[0x10]; // 0xF0, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 'T', 'C', 'I', 'P', 0x02, 0x01, 0xFF, 0xFF, 0xFF, 0xFF

        LittleEndian<std::uint32_t> width;
        LittleEndian<std::uint32_t> height;
        LittleEndian<std::uint32_t> depth;
        LittleEndian<std::uint32_t> unknown;

        std::byte unknown2[0x6];
        LittleEndian<std::uint16_t> format;
        std::byte unknown3[0x8];

        LittleEndian<std::uint32_t> mipmap_count;

        std::byte unknown4[0x6];
    };
    static_assert(sizeof(IPAKBitmapHeader) == 0x3A);
}

#endif
