/*
 * Invader (c) 2019 Kavawuvi
 *
 * This program is free software under the GNU General Public License v3.0. See LICENSE for more information.
 */

#include "../compile.hpp"

#include "material_effects.hpp"

namespace Invader::HEK {
    void compile_material_effects_tag(CompiledTag &compiled, const std::byte *data, std::size_t size) {
        BEGIN_COMPILE(MaterialEffects)

        ADD_REFLEXIVE_START(tag.effects) {
            ADD_REFLEXIVE_START(reflexive.materials) {
                ADD_DEPENDENCY_ADJUST_SIZES(reflexive.effect)
                ADD_DEPENDENCY_ADJUST_SIZES(reflexive.sound)
            } ADD_REFLEXIVE_END
        } ADD_REFLEXIVE_END

        FINISH_COMPILE
    }
}
