/*
 * Invader (c) 2018 Kavawuvi
 *
 * This program is free software under the GNU General Public License v3.0 or later. See LICENSE for more information.
 */

#include "../compile.hpp"
#include "decal.hpp"

namespace Invader::HEK {
    void compile_decal_tag(CompiledTag &compiled, const std::byte *data, std::size_t size) {
        BEGIN_COMPILE(Decal);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.next_decal_in_chain);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.map);
        DEFAULT_VALUE(tag.animation_speed, 1);
        FINISH_COMPILE
    }
}
