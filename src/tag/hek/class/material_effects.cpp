// SPDX-License-Identifier: GPL-3.0-only

#include <invader/tag/hek/compile.hpp>

#include <invader/tag/hek/class/material_effects.hpp>

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
