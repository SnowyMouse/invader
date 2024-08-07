// SPDX-License-Identifier: GPL-3.0-only

#include <invader/tag/parser/parser.hpp>

namespace Invader::Parser {
    void PointPhysics::pre_compile(BuildWorkload &, std::size_t, std::size_t, std::size_t) {
        this->air_friction = this->air_friction * 10000.0F;
        this->water_friction = this->water_friction * 10000.0F;

        this->mass_scale = this->density * 118613.34F;

        this->water_gravity_scale = (1.0 + this->density * (-1.0F / WATER_DENSITY)) / (1.0 + this->density * (+1.0F / WATER_DENSITY));
        this->air_gravity_scale = (1.0 + this->density * (-1.0F / AIR_DENSITY)) / (1.0 + this->density * (+1.0F / AIR_DENSITY));
    }
}
