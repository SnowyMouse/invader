// SPDX-License-Identifier: GPL-3.0-only

#include <invader/tag/hek/compile.hpp>
#include <invader/tag/hek/definition.hpp>

namespace Invader::HEK {
    void compile_shader_transparent_glass_tag(CompiledTag &compiled, const std::byte *data, std::size_t size) {
        BEGIN_COMPILE(ShaderTransparentGlass)
        ADD_DEPENDENCY_ADJUST_SIZES(tag.background_tint_map)
        ADD_DEPENDENCY_ADJUST_SIZES(tag.reflection_map)
        ADD_DEPENDENCY_ADJUST_SIZES(tag.bump_map)
        ADD_DEPENDENCY_ADJUST_SIZES(tag.diffuse_map)
        ADD_DEPENDENCY_ADJUST_SIZES(tag.diffuse_detail_map)
        ADD_DEPENDENCY_ADJUST_SIZES(tag.specular_map)
        ADD_DEPENDENCY_ADJUST_SIZES(tag.specular_detail_map)
        DEFAULT_VALUE(tag.background_tint_map_scale, 1.0f);
        DEFAULT_VALUE(tag.bump_map_scale, 1.0f);
        DEFAULT_VALUE(tag.diffuse_map_scale, 1.0f);
        DEFAULT_VALUE(tag.diffuse_detail_map_scale, 1.0f);
        DEFAULT_VALUE(tag.specular_map_scale, 1.0f);
        DEFAULT_VALUE(tag.specular_detail_map_scale, 1.0f);
        tag.shader_type = SHADER_TYPE_SHADER_TRANSPARENT_GLASS;
        FINISH_COMPILE
    }
}
