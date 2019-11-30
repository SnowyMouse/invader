// SPDX-License-Identifier: GPL-3.0-only

#include <invader/tag/parser/parser.hpp>

namespace Invader::Parser {
    void WeaponHUDInterface::pre_compile(BuildWorkload2 &, std::size_t, std::size_t, std::size_t) {
        std::uint32_t crosshair_types = 0;
        for(auto &c : this->crosshairs) {
            crosshair_types |= (1 << c.crosshair_type);
        }
        this->crosshair_types = crosshair_types;
    }
}
