// SPDX-License-Identifier: GPL-3.0-only

#include "../compile.hpp"

#include "hud_number.hpp"

namespace Invader::HEK {
    void compile_hud_number_tag(CompiledTag &compiled, const std::byte *data, std::size_t size) {
        BEGIN_COMPILE(HUDNumber)
        ADD_DEPENDENCY_ADJUST_SIZES(tag.digits_bitmap);
        FINISH_COMPILE
    }
}
