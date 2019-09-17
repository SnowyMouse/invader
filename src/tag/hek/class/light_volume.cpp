/*
 * Invader (c) 2018 Kavawuvi
 *
 * This program is free software under the GNU General Public License v3.0. See LICENSE for more information.
 */

#include "../compile.hpp"

#include "light_volume.hpp"

namespace Invader::HEK {
    void compile_light_volume_tag(CompiledTag &compiled, const std::byte *data, std::size_t size) {
        BEGIN_COMPILE(LightVolume);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.map);
        DEFAULT_VALUE(tag.parallel_brightness_scale, 1.0f);
        DEFAULT_VALUE(tag.perpendicular_brightness_scale, 1.0f);
        ADD_REFLEXIVE_START(tag.frames) {
            DEFAULT_VALUE(reflexive.offset_exponent, 1.0f);
            DEFAULT_VALUE(reflexive.radius_exponent, 1.0f);
            DEFAULT_VALUE(reflexive.tint_color_exponent, 1.0f);
            DEFAULT_VALUE(reflexive.brightness_exponent, 1.0f);
        } ADD_REFLEXIVE_END
        FINISH_COMPILE
    }
}
