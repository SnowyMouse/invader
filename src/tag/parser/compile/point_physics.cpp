// SPDX-License-Identifier: GPL-3.0-only

#include <invader/tag/parser/parser.hpp>

namespace Invader::Parser {
    void PointPhysics::pre_compile(BuildWorkload &, std::size_t, std::size_t, std::size_t) {
        this->air_friction = this->air_friction * 10000.0f;
        this->water_friction = this->water_friction * 10000.0f;

        this->unknown_constant = density * 118613.3333333f;
        auto density = this->density;
        this->water_gravity_scale = -1.0f * (density / WATER_DENSITY - 1.0f) / (density / WATER_DENSITY + 1.0f);
        this->air_gravity_scale = -1.0f * (density / AIR_DENSITY - 1.0f) / (density / AIR_DENSITY + 1.0f);
    }
}
