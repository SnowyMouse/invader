/*
 * Invader (c) 2018 Kavawuvi
 *
 * This program is free software under the GNU General Public License v3.0. See LICENSE for more information.
 */

#include <cmath>

#include "../../../hek/constants.hpp"

#include "../compile.hpp"
#include "biped.hpp"

namespace Invader::HEK {
    void compile_biped_tag(CompiledTag &compiled, const std::byte *data, std::size_t size) {
        BEGIN_COMPILE(Biped)
        COMPILE_UNIT_DATA
        ADD_DEPENDENCY_ADJUST_SIZES(tag.dont_use);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.footsteps);
        ADD_REFLEXIVE(tag.contact_point);
        tag.cosine_stationary_turning_threshold = std::cos(tag.stationary_turning_threshold);
        tag.crouch_camera_velocity = 1.0f / tag.crouch_transition_time / TICK_RATE;
        tag.cosine_maximum_slope_angle = static_cast<float>(std::cos(tag.maximum_slope_angle));
        tag.negative_sine_downhill_falloff_angle = static_cast<float>(-std::sin(tag.downhill_falloff_angle));
        tag.negative_sine_downhill_cutoff_angle = static_cast<float>(-std::sin(tag.downhill_cutoff_angle));
        tag.sine_uphill_falloff_angle = static_cast<float>(std::sin(tag.uphill_falloff_angle));
        tag.sine_uphill_cutoff_angle = static_cast<float>(std::sin(tag.uphill_cutoff_angle));
        tag.object_type = OBJECT_TYPE_BIPED;
        FINISH_COMPILE
    }
}
