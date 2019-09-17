/*
 * Invader (c) 2019 Kavawuvi
 *
 * This program is free software under the GNU General Public License v3.0. See LICENSE for more information.
 */

#include "../compile.hpp"
#include "color_table.hpp"

namespace Invader::HEK {
    void compile_color_table_tag(CompiledTag &compiled, const std::byte *data, std::size_t size) {
        BEGIN_COMPILE(ColorTable);
        ADD_REFLEXIVE(tag.colors);
        FINISH_COMPILE
    }
}
