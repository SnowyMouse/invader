/*
 * Invader (c) 2018 Kavawuvi
 *
 * This program is free software under the GNU General Public License v3.0 or later. See LICENSE for more information.
 */

#include "../compile.hpp"

#include "shader_transparent_water.hpp"

namespace Invader::HEK {
    void compile_shader_transparent_water_tag(CompiledTag &compiled, const std::byte *data, std::size_t size) {
        BEGIN_COMPILE(ShaderTransparentWater)
        ADD_DEPENDENCY_ADJUST_SIZES(tag.base_map);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.reflection_map);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.ripple_maps);
        ADD_REFLEXIVE(tag.ripples);
        FINISH_COMPILE
    }
}
