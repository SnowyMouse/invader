/*
 * Invader (c) 2018 Kavawuvi
 *
 * This program is free software under the GNU General Public License v3.0 or later. See LICENSE for more information.
 */

#pragma once

#include "../../compiled_tag.hpp"
#include "../../../hek/data_type.hpp"
#include "../header.hpp"

namespace Invader::HEK {
    struct PointPhysicsFlags {
        std::uint32_t flamethrower_particle_collision : 1;
        std::uint32_t collides_with_structures : 1;
        std::uint32_t collides_with_water_surface : 1;
        std::uint32_t uses_simple_wind : 1;
        std::uint32_t uses_damped_wind : 1;
        std::uint32_t no_gravity : 1;
    };

    ENDIAN_TEMPLATE(EndianType) struct PointPhysics {
        EndianType<PointPhysicsFlags> flags;
        LittleEndian<float> unknown_constant;
        LittleEndian<float> water_gravity_scale;
        LittleEndian<float> air_gravity_scale;
        PAD(0x10);
        EndianType<float> density;
        EndianType<float> air_friction;
        EndianType<float> water_friction;
        EndianType<float> surface_friction;
        EndianType<float> elasticity;
        PAD(0xC);

        ENDIAN_TEMPLATE(NewType) operator PointPhysics<NewType>() const noexcept {
            PointPhysics<NewType> copy = {};
            COPY_THIS(unknown_constant);
            COPY_THIS(water_gravity_scale);
            COPY_THIS(air_gravity_scale);
            COPY_THIS(flags);
            COPY_THIS(density);
            COPY_THIS(air_friction);
            COPY_THIS(water_friction);
            COPY_THIS(surface_friction);
            COPY_THIS(elasticity);
            return copy;
        }
    };
    static_assert(sizeof(PointPhysics<BigEndian>) == 0x40);

    void compile_point_physics_tag(CompiledTag &compiled, const std::byte *data, std::size_t size);
}
