/*
 * Invader (c) 2018 Kavawuvi
 *
 * This program is free software under the GNU General Public License v3.0 or later. See LICENSE for more information.
 */

#include "../compile.hpp"
#include "glow.hpp"

namespace Invader::HEK {
    void compile_glow_tag(CompiledTag &compiled, const std::byte *data, std::size_t size) {
        BEGIN_COMPILE(Glow)
        ADD_DEPENDENCY_ADJUST_SIZES(tag.texture)
        FINISH_COMPILE
    }
}
