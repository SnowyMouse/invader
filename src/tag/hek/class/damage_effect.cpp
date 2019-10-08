// SPDX-License-Identifier: GPL-3.0-only

#include "../compile.hpp"

#include "damage_effect.hpp"

namespace Invader::HEK {
    void compile_damage_effect_tag(CompiledTag &compiled, const std::byte *data, std::size_t size, DamageEffectJasonJones jason_jones) {
        BEGIN_COMPILE(DamageEffect);

        ADD_DEPENDENCY_ADJUST_SIZES(tag.sound)

        // Jason Jones the weapon damage scaling
        switch(jason_jones) {
            case DAMAGE_EFFECT_JASON_JONES_PISTOL_SINGLEPLAYER:
                tag.elite_energy_shield = 0.8F;
                break;
            default:
                break;
        }

        FINISH_COMPILE;
    }
}
