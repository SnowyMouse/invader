/*
 * Invader (c) 2018 Kavawuvi
 *
 * This program is free software under the GNU General Public License v3.0. See LICENSE for more information.
 */

#include "../compile.hpp"
#include "particle_system.hpp"

namespace Invader::HEK {
    void compile_particle_system_tag(CompiledTag &compiled, const std::byte *data, std::size_t size) {
        BEGIN_COMPILE(ParticleSystem)
        ADD_DEPENDENCY_ADJUST_SIZES(tag.point_physics);
        ADD_REFLEXIVE(tag.physics_constants);
        ADD_REFLEXIVE_START(tag.particle_types) {
            ADD_REFLEXIVE(reflexive.physics_constants);
            ADD_REFLEXIVE_START(reflexive.states) {
                ADD_REFLEXIVE(reflexive.physics_constants);
            } ADD_REFLEXIVE_END
            ADD_REFLEXIVE_START(reflexive.particle_states) {
                ADD_DEPENDENCY_ADJUST_SIZES(reflexive.bitmaps);
                ADD_DEPENDENCY_ADJUST_SIZES(reflexive.point_physics);
                ADD_DEPENDENCY_ADJUST_SIZES(reflexive.secondary_map_bitmap);
                ADD_REFLEXIVE(reflexive.physics_constants);
                reflexive.unknown_int = 1;
            } ADD_REFLEXIVE_END
        } ADD_REFLEXIVE_END
        FINISH_COMPILE
    }
}
