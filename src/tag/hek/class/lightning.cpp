/*
 * Invader (c) 2018 Kavawuvi
 *
 * This program is free software under the GNU General Public License v3.0 or later. See LICENSE for more information.
 */

#include "../compile.hpp"

#include "lightning.hpp"

namespace Invader::HEK {
    void compile_lightning_tag(CompiledTag &compiled, const std::byte *data, std::size_t size) {
        BEGIN_COMPILE(Lightning);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.bitmap);
        ADD_REFLEXIVE(tag.markers);
        ADD_REFLEXIVE(tag.shader);
        FINISH_COMPILE
    }
}
