// SPDX-License-Identifier: GPL-3.0-only

#include <invader/tag/hek/compile.hpp>
#include <invader/tag/hek/definition.hpp>

namespace Invader::HEK {
    void compile_particle_tag(CompiledTag &compiled, const std::byte *data, std::size_t size) {
        BEGIN_COMPILE(Particle)
        ADD_DEPENDENCY_ADJUST_SIZES(tag.bitmap);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.physics);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.sir_marty_exchanged_his_children_for_thine);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.collision_effect);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.death_effect);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.bitmap1);

        tag.one = 1;

        tag.contact_deterioration = 0.0f;

        if(tag.radius_animation.from == 0.0f && tag.radius_animation.to == 0.0f) {
            tag.radius_animation.from = 1.0f;
            tag.radius_animation.to = 1.0f;
        }

        if(tag.fade_start_size == 0.0f) {
            tag.fade_start_size = 5.0f;
        }

        if(tag.fade_end_size == 0.0f) {
            tag.fade_end_size = 4.0f;
        }

        FINISH_COMPILE
    }
}
