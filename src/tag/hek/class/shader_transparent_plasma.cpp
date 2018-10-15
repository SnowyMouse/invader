/*
 * Invader (c) 2018 Kavawuvi
 *
 * This program is free software under the GNU General Public License v3.0 or later. See LICENSE for more information.
 */

#include "../compile.hpp"

#include "shader_transparent_plasma.hpp"

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
