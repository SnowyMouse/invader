// SPDX-License-Identifier: GPL-3.0-only

#include <invader/tag/parser/parser.hpp>

namespace Invader::Parser {
    void Antenna::pre_compile(BuildWorkload &, std::size_t, std::size_t, std::size_t) {
        float length = 0.0f;

        for(auto &part : this->vertices) {
            float vertex_length = part.length;
            length += vertex_length;

            float sp = std::sin(part.angles.pitch);
            float sy = std::sin(part.angles.yaw);
            float cp = std::cos(part.angles.pitch);
            float cy = std::cos(part.angles.yaw);

            part.offset.x = vertex_length * sp * cy;
            part.offset.y = vertex_length * sy * sp;
            part.offset.z = vertex_length * cp;
        }

        this->length = length;
    }
}
