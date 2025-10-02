// SPDX-License-Identifier: GPL-3.0-only

#include <invader/tag/parser/parser.hpp>

namespace Invader::Parser {
    void PointPhysics::pre_compile(BuildWorkload &, std::size_t, std::size_t, std::size_t) {
        this->air_friction *= 10000.0F;
        this->water_friction *= 10000.0F;

        this->mass_scale = 0.001F * this->density * ((4.0F / 3.0F) * HALO_PI) * ((10.0F * 0.3048F) * (10.0F * 0.3048F) * (10.0F * 0.3048F) * 100 * 100 * 100);

        this->water_gravity_scale = 1.0F - (2.0F * this->density / (this->density + WATER_DENSITY));
        this->air_gravity_scale = 1.0F - (2.0F * this->density / (this->density + AIR_DENSITY));
    }
}
