// SPDX-License-Identifier: GPL-3.0-only

#include "invader/tag/hek/compile.hpp"
#include "invader/hek/constants.hpp"

#include "invader/tag/hek/class/projectile.hpp"

namespace Invader::HEK {
    void compile_projectile_tag(CompiledTag &compiled, const std::byte *data, std::size_t size) {
        BEGIN_COMPILE(Projectile)
        COMPILE_OBJECT_DATA
        ADD_DEPENDENCY_ADJUST_SIZES(tag.super_detonation);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.effect);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.detonation_started);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.flyby_sound);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.attached_detonation_damage);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.impact_damage);
        ADD_REFLEXIVE_START(tag.projectile_material_response) {
            ADD_DEPENDENCY_ADJUST_SIZES(reflexive.default_effect);
            ADD_DEPENDENCY_ADJUST_SIZES(reflexive.potential_effect);
            ADD_DEPENDENCY_ADJUST_SIZES(reflexive.detonation_effect);
        } ADD_REFLEXIVE_END

        tag.initial_velocity = tag.initial_velocity / TICK_RATE;
        tag.final_velocity = tag.final_velocity / TICK_RATE;
        tag.object_type = OBJECT_TYPE_PROJECTILE;

        FINISH_COMPILE
    }
}
