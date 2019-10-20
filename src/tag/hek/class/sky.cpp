// SPDX-License-Identifier: GPL-3.0-only

#include "invader/tag/hek/compile.hpp"

#include "invader/tag/hek/class/sky.hpp"

namespace Invader::HEK {
    void compile_sky_tag(CompiledTag &compiled, const std::byte *data, std::size_t size) {
        BEGIN_COMPILE(Sky);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.model);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.animation_graph);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.indoor_fog_screen);
        ADD_REFLEXIVE(tag.shader_functions);
        ADD_REFLEXIVE(tag.animations);
        ADD_BASIC_DEPENDENCY_REFLEXIVE(tag.lights, lens_flare);
        FINISH_COMPILE
    }
}
