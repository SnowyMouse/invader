// SPDX-License-Identifier: GPL-3.0-only

#include <invader/tag/hek/compile.hpp>
#include <invader/tag/hek/class/shader_transparent_chicago_extended.hpp>

namespace Invader::HEK {
    void compile_shader_transparent_chicago_extended_tag(CompiledTag &compiled, const std::byte *data, std::size_t size) {
        BEGIN_COMPILE(ShaderTransparentChicagoExtended);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.lens_flare);
        ADD_BASIC_DEPENDENCY_REFLEXIVE(tag.extra_layers, shader);
        ADD_CHICAGO_MAP(tag.maps_4_stage);
        ADD_CHICAGO_MAP(tag.maps_2_stage);
        tag.shader_type = SHADER_TYPE_SHADER_TRANSPARENT_CHICAGO_EXTENDED;
        FINISH_COMPILE
    }
}
