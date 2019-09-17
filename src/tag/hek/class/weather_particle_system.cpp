/*
 * Invader (c) 2018 Kavawuvi
 *
 * This program is free software under the GNU General Public License v3.0. See LICENSE for more information.
 */

#include "../compile.hpp"

#include "../../../hek/constants.hpp"
#include "weather_particle_system.hpp"

namespace Invader::HEK {
    void compile_weather_particle_system_tag(CompiledTag &compiled, const std::byte *data, std::size_t size) {
        BEGIN_COMPILE(WeatherParticleSystem);
        ADD_REFLEXIVE_START(tag.particle_types) {
            ADD_DEPENDENCY_ADJUST_SIZES(reflexive.physics);
            ADD_DEPENDENCY_ADJUST_SIZES(reflexive.sprite_bitmap);
            ADD_DEPENDENCY_ADJUST_SIZES(reflexive.bitmap);
            reflexive.not_broken = 1;
        } ADD_REFLEXIVE_END
        FINISH_COMPILE
    }
}
