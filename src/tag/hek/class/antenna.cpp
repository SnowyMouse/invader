/*
 * Invader (c) 2018 Kavawuvi
 *
 * This program is free software under the GNU General Public License v3.0 or later. See LICENSE for more information.
 */

#include "../compile.hpp"
#include "antenna.hpp"

namespace Invader::HEK {
    void compile_antenna_tag(CompiledTag &compiled, const std::byte *data, std::size_t size) {
        BEGIN_COMPILE(Antenna)
        ADD_DEPENDENCY_ADJUST_SIZES(tag.bitmaps)
        ADD_DEPENDENCY_ADJUST_SIZES(tag.physics)
        ADD_REFLEXIVE(tag.vertices)
        FINISH_COMPILE
    }
}
