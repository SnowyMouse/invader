/*
 * Invader (c) 2019 Kavawuvi
 *
 * This program is free software under the GNU General Public License v3.0. See LICENSE for more information.
 */

#include "../compile.hpp"
#include "vehicle.hpp"

namespace Invader::HEK {
    void compile_vehicle_tag(CompiledTag &compiled, const std::byte *data, std::size_t size) {
        BEGIN_COMPILE(Vehicle)
        COMPILE_UNIT_DATA
        ADD_DEPENDENCY_ADJUST_SIZES(tag.suspension_sound);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.crash_sound);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.material_effects);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.effect);
        tag.object_type = OBJECT_TYPE_VEHICLE;
        DEFAULT_VALUE(tag.minimum_flipping_angular_velocity, 0.2f);
        DEFAULT_VALUE(tag.maximum_flipping_angular_velocity, 0.75f);
        FINISH_COMPILE
    }
}
