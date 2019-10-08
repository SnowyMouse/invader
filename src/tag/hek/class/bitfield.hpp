// SPDX-License-Identifier: GPL-3.0-only

#ifndef INVADER__TAG__HEK__CLASS__BITFIELD_HPP
#define INVADER__TAG__HEK__CLASS__BITFIELD_HPP

#include <cstdint>

namespace Invader::HEK {
    struct IsUnusedFlag {
        std::uint32_t unused : 1;
    };

    struct IsUnfilteredFlag {
        std::uint16_t unfiltered : 1;
    };
}

#endif
