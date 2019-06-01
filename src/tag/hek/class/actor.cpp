/*
 * Invader (c) 2018 Kavawuvi
 *
 * This program is free software under the GNU General Public License v3.0 or later. See LICENSE for more information.
 */

#include <iostream>
#include <cmath>

#include "../../../hek/constants.hpp"
#include "../compile.hpp"

#include "actor.hpp"

namespace Invader::HEK {
    void compile_actor_tag(CompiledTag &compiled, const std::byte *data, std::size_t size) {
        BEGIN_COMPILE(Actor)
        ADD_DEPENDENCY_ADJUST_SIZES(tag.do_not_use)
        ADD_DEPENDENCY_ADJUST_SIZES(tag.do_not_use_1)
        ADD_DEPENDENCY_ADJUST_SIZES(tag.do_not_use_2)

        #define SET_INVERTED_VALUE(normal,inverted) { float normal_val = normal.read(); if(normal_val > 0.0F) { inverted = 1.0F / (TICK_RATE * normal_val); } else { inverted = 0.0F; } }

        SET_INVERTED_VALUE(tag.combat_perception_time, tag.inverse_combat_perception_time);
        SET_INVERTED_VALUE(tag.guard_perception_time, tag.inverse_guard_perception_time);
        SET_INVERTED_VALUE(tag.non_combat_perception_time, tag.inverse_non_combat_perception_time);

        tag.cosine_maximum_aiming_deviation.i = std::cos(tag.maximum_aiming_deviation.i);
        tag.cosine_maximum_aiming_deviation.j = std::cos(tag.maximum_aiming_deviation.j);
        tag.cosine_maximum_looking_deviation.i = std::cos(tag.maximum_looking_deviation.i);
        tag.cosine_maximum_looking_deviation.j = std::cos(tag.maximum_looking_deviation.j);

        FINISH_COMPILE
    }
}
