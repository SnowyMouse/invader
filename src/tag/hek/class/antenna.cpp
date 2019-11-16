// SPDX-License-Identifier: GPL-3.0-only

#include <cmath>
#include <invader/tag/hek/compile.hpp>
#include <invader/tag/hek/definition.hpp>

namespace Invader::HEK {
    void compile_antenna_tag(CompiledTag &compiled, const std::byte *data, std::size_t size) {
        BEGIN_COMPILE(Antenna)
        ADD_DEPENDENCY_ADJUST_SIZES(tag.bitmaps)
        ADD_DEPENDENCY_ADJUST_SIZES(tag.physics)
        float length = 0.0f;
        ADD_REFLEXIVE_START(tag.vertices) {
            float vertex_length = reflexive.length;
            length += vertex_length;

            float sp = std::sin(reflexive.angles.pitch);
            float sy = std::sin(reflexive.angles.yaw);
            float cp = std::cos(reflexive.angles.pitch);
            float cy = std::cos(reflexive.angles.yaw);

            reflexive.offset.x = vertex_length * sp * cy;
            reflexive.offset.y = vertex_length * sy * sp;
            reflexive.offset.z = vertex_length * cp;
        } ADD_REFLEXIVE_END
        tag.length = length;
        FINISH_COMPILE
    }
}
