// SPDX-License-Identifier: GPL-3.0-only

#include <invader/tag/hek/compile.hpp>
#include <invader/tag/hek/definition.hpp>

namespace Invader::HEK {
    void compile_shader_transparent_water_tag(CompiledTag &compiled, const std::byte *data, std::size_t size) {
        BEGIN_COMPILE(ShaderTransparentWater)
        ADD_DEPENDENCY_ADJUST_SIZES(tag.base_map);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.reflection_map);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.ripple_maps);
        ADD_REFLEXIVE_START(tag.ripples) {
            DEFAULT_VALUE(reflexive.map_repeats, 1);
        } ADD_REFLEXIVE_END;
        DEFAULT_VALUE(tag.ripple_scale, 1.0F);
        DEFAULT_VALUE(tag.ripple_mipmap_levels, 1);
        DEFAULT_VALUE(tag.view_parallel_brightness, 1.0F);
        tag.shader_type = SHADER_TYPE_SHADER_TRANSPARENT_WATER;
        FINISH_COMPILE
    }
}
