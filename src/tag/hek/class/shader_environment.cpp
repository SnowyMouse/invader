/*
 * Invader (c) 2018 Kavawuvi
 *
 * This program is free software under the GNU General Public License v3.0 or later. See LICENSE for more information.
 */

#include "../compile.hpp"
#include "shader_environment.hpp"

namespace Invader::HEK {
    void compile_shader_environment_tag(CompiledTag &compiled, const std::byte *data, std::size_t size) {
        BEGIN_COMPILE(ShaderEnvironment);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.lens_flare);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.base_map);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.primary_detail_map);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.secondary_detail_map);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.micro_detail_map);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.bump_map);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.map);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.reflection_cube_map);
        MAKE_WHITE_IF_BLACK(tag.material_color);
        DEFAULT_VALUE(tag.bump_map_scale, 1.0f);
        DEFAULT_VALUE(tag.u_animation_period, 1.0f);
        DEFAULT_VALUE(tag.u_animation_scale, 1.0f);
        DEFAULT_VALUE(tag.v_animation_period, 1.0f);
        DEFAULT_VALUE(tag.v_animation_scale, 1.0f);
        DEFAULT_VALUE(tag.primary_animation_period, 1.0f);
        DEFAULT_VALUE(tag.secondary_animation_period, 1.0f);
        DEFAULT_VALUE(tag.plasma_animation_period, 1.0f);
        DEFAULT_VALUE(tag.map_scale, 1.0f);
        tag.shader_type = SHADER_TYPE_SHADER_ENVIRONMENT;
        FINISH_COMPILE
    }
}
