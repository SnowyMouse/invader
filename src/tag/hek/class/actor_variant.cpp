/*
 * Invader (c) 2018 Kavawuvi
 *
 * This program is free software under the GNU General Public License v3.0 or later. See LICENSE for more information.
 */

#include <iostream>

#include "../../../hek/constants.hpp"
#include "../compile.hpp"

#include "actor_variant.hpp"

namespace Invader::HEK {
    void compile_actor_variant_tag(CompiledTag &compiled, const std::byte *data, std::size_t size) {
        BEGIN_COMPILE(ActorVariant)
        ADD_DEPENDENCY_ADJUST_SIZES(tag.actor_definition)
        ADD_DEPENDENCY_ADJUST_SIZES(tag.unit)
        ADD_DEPENDENCY_ADJUST_SIZES(tag.major_variant)
        ADD_DEPENDENCY_ADJUST_SIZES(tag.weapon)
        ADD_DEPENDENCY_ADJUST_SIZES(tag.equipment)
        tag.grenade_velocity = tag.grenade_velocity.read() / TICK_RATE;
        ADD_REFLEXIVE(tag.change_colors)
        FINISH_COMPILE
    }
}
