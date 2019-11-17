// SPDX-License-Identifier: GPL-3.0-only

#include <invader/tag/hek/compile.hpp>
#include <invader/tag/hek/definition.hpp>

namespace Invader::HEK {
    void compile_light_tag(CompiledTag &compiled, const std::byte *data, std::size_t size) {
        BEGIN_COMPILE(Light);

        if(tag.radius_modifer.from == 0.0f && tag.radius_modifer.to == 0.0f) {
            tag.radius_modifer.from = 1.0f;
            tag.radius_modifer.to = 1.0f;
        }
        DEFAULT_VALUE(tag.yaw_period, 1.0f);
        DEFAULT_VALUE(tag.roll_period, 1.0f);
        DEFAULT_VALUE(tag.pitch_period, 1.0f);
        DEFAULT_VALUE(tag.cutoff_angle, static_cast<float>(HALO_PI));
        DEFAULT_VALUE(tag.falloff_angle, static_cast<float>(HALO_PI));
        tag.cos_cutoff_angle = std::cos(tag.cutoff_angle);
        tag.cos_falloff_angle = std::cos(tag.falloff_angle);
        DEFAULT_VALUE(tag.unknown_two, 2.0f);
        tag.sin_cutoff_angle = std::sin(tag.cutoff_angle);

        ADD_DEPENDENCY_ADJUST_SIZES(tag.primary_cube_map);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.secondary_cube_map);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.lens_flare);

        FINISH_COMPILE
    }
}
