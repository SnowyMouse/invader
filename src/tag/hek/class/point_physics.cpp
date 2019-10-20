// SPDX-License-Identifier: GPL-3.0-only

#include "invader/tag/hek/compile.hpp"
#include "invader/hek/constants.hpp"
#include "invader/tag/hek/class/point_physics.hpp"

namespace Invader::HEK {
    void compile_point_physics_tag(CompiledTag &compiled, const std::byte *data, std::size_t size) {
        BEGIN_COMPILE(PointPhysics);

        tag.air_friction = tag.air_friction * 10000.0f;
        tag.water_friction = tag.water_friction * 10000.0f;

        auto density = tag.density.read();
        tag.unknown_constant = density * 118613.3333333f;
        tag.water_gravity_scale = -1.0f * (density / WATER_DENSITY - 1.0f) / (density / WATER_DENSITY + 1.0f);
        tag.air_gravity_scale = -1.0f * (density / AIR_DENSITY - 1.0f) / (density / AIR_DENSITY + 1.0f);

        FINISH_COMPILE
    }
}
