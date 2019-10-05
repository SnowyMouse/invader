// SPDX-License-Identifier: GPL-3.0-only

#include "../compile.hpp"
#include "shader_transparent_generic.hpp"

namespace Invader::HEK {
    void compile_shader_transparent_generic_tag(CompiledTag &compiled, const std::byte *data, std::size_t size) {
        BEGIN_COMPILE(ShaderTransparentGeneric);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.lens_flare);
        ADD_BASIC_DEPENDENCY_REFLEXIVE(tag.extra_layers, shader);
        ADD_REFLEXIVE_START(tag.maps) {
            DEFAULT_VALUE(reflexive.map_u_scale, 1.0F);
            DEFAULT_VALUE(reflexive.map_v_scale, 1.0F);
            DEFAULT_VALUE(reflexive.u_animation_period, 1.0F);
            DEFAULT_VALUE(reflexive.u_animation_scale, 1.0F);
            DEFAULT_VALUE(reflexive.v_animation_period, 1.0F);
            DEFAULT_VALUE(reflexive.v_animation_scale, 1.0F);
            DEFAULT_VALUE(reflexive.rotation_animation_period, 1.0F);
            DEFAULT_VALUE(reflexive.rotation_animation_scale, 360.0F);
            ADD_DEPENDENCY_ADJUST_SIZES(reflexive.map);
        } ADD_REFLEXIVE_END;
        ADD_REFLEXIVE_START(tag.stages) {
            DEFAULT_VALUE(reflexive.color0_animation_period, 1.0F);
        } ADD_REFLEXIVE_END;
        tag.shader_type = SHADER_TYPE_SHADER_TRANSPARENT_GENERIC;
        FINISH_COMPILE
    }
}
