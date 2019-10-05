// SPDX-License-Identifier: GPL-3.0-only

#include "../compile.hpp"
#include "color_table.hpp"

namespace Invader::HEK {
    void compile_color_table_tag(CompiledTag &compiled, const std::byte *data, std::size_t size) {
        BEGIN_COMPILE(ColorTable);
        ADD_REFLEXIVE(tag.colors);
        FINISH_COMPILE
    }
}
