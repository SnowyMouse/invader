// SPDX-License-Identifier: GPL-3.0-only

#include <invader/tag/hek/compile.hpp>
#include <invader/tag/hek/definition.hpp>

namespace Invader::HEK {
    void compile_shader_transparent_meter_tag(CompiledTag &compiled, const std::byte *data, std::size_t size) {
        BEGIN_COMPILE(ShaderTransparentMeter)
        ADD_DEPENDENCY_ADJUST_SIZES(tag.map);
        tag.shader_type = SHADER_TYPE_SHADER_TRANSPARENT_METER;
        FINISH_COMPILE
    }
}
