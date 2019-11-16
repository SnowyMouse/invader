// SPDX-License-Identifier: GPL-3.0-only

#include <invader/tag/hek/compile.hpp>
#include <invader/tag/hek/definition.hpp>
#include "compile.hpp"

namespace Invader::HEK {
    void compile_shader_model_tag(CompiledTag &compiled, const std::byte *data, std::size_t size) {
        BEGIN_COMPILE(ShaderModel);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.base_map);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.multipurpose_map);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.detail_map);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.reflection_cube_map);
        INITIALIZE_SHADER_ANIMATION(tag);
        DEFAULT_VALUE(tag.animation_period, 1.0f);
        DEFAULT_VALUE(tag.map_u_scale, 1.0f);
        DEFAULT_VALUE(tag.map_v_scale, tag.map_u_scale);
        DEFAULT_VALUE(tag.detail_map_scale, 1.0f);
        DEFAULT_VALUE(tag.detail_map_v_scale, 1.0f);
        DEFAULT_VALUE(tag.unknown, 1.0f);
        tag.shader_type = SHADER_TYPE_SHADER_MODEL;
        FINISH_COMPILE
    }
}
