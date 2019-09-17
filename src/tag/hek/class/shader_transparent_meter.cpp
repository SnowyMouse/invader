/*
 * Invader (c) 2019 Kavawuvi
 *
 * This program is free software under the GNU General Public License v3.0. See LICENSE for more information.
 */

#include "../compile.hpp"

#include "shader_transparent_meter.hpp"

namespace Invader::HEK {
    void compile_shader_transparent_meter_tag(CompiledTag &compiled, const std::byte *data, std::size_t size) {
        BEGIN_COMPILE(ShaderTransparentMeter)
        ADD_DEPENDENCY_ADJUST_SIZES(tag.map);
        tag.shader_type = SHADER_TYPE_SHADER_TRANSPARENT_METER;
        FINISH_COMPILE
    }
}
