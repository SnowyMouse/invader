// SPDX-License-Identifier: GPL-3.0-only

#include "invader/tag/hek/compile.hpp"

#include "invader/tag/hek/class/shader_transparent_plasma.hpp"

namespace Invader::HEK {
    void compile_shader_transparent_plasma_tag(CompiledTag &compiled, const std::byte *data, std::size_t size) {
        BEGIN_COMPILE(ShaderTransparentPlasma)
        DEFAULT_VALUE(tag.intensity_exponent, 1.0f);
        DEFAULT_VALUE(tag.offset_exponent, 1.0f);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.primary_noise_map);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.secondary_noise_map);
        tag.shader_type = SHADER_TYPE_SHADER_TRANSPARENT_PLASMA;
        FINISH_COMPILE
    }
}
