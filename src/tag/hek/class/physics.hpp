/*
 * Invader (c) 2018 Kavawuvi
 *
 * This program is free software under the GNU General Public License v3.0 or later. See LICENSE for more information.
 */

#ifndef INVADER__TAG__HEK__CLASS__PHYSICS_HPP
#define INVADER__TAG__HEK__CLASS__PHYSICS_HPP

#include "../../compiled_tag.hpp"
#include "../../../hek/data_type.hpp"
#include "../header.hpp"

namespace Invader::HEK {
    enum PhysicsFrictionType : TagEnum {
        PHYSICS_FRICTION_TYPE_POINT,
        PHYSICS_FRICTION_TYPE_FORWARD,
        PHYSICS_FRICTION_TYPE_LEFT,
        PHYSICS_FRICTION_TYPE_UP
    };

    ENDIAN_TEMPLATE(EndianType) struct PhysicsInertialMatrix {
        Matrix<EndianType> matrix;
        //Vector3D<EndianType> yy_zz___xy___zx;
        //Vector3D<EndianType> _xy__zz_xx___yz;
        //Vector3D<EndianType> _zx___yz__xx_yy;

        ENDIAN_TEMPLATE(NewType) operator PhysicsInertialMatrix<NewType>() const noexcept {
            PhysicsInertialMatrix<NewType> copy;
            COPY_THIS(matrix);
            //COPY_THIS(yy_zz___xy___zx);
            //COPY_THIS(_xy__zz_xx___yz);
            //COPY_THIS(_zx___yz__xx_yy);
            return copy;
        }
    };
    static_assert(sizeof(PhysicsInertialMatrix<BigEndian>) == 0x24);

    struct PhysicsPoweredMassPointFlags {
        std::uint32_t ground_friction : 1;
        std::uint32_t water_friction : 1;
        std::uint32_t air_friction : 1;
        std::uint32_t water_lift : 1;
        std::uint32_t air_lift : 1;
        std::uint32_t thrust : 1;
        std::uint32_t antigrav : 1;
    };

    ENDIAN_TEMPLATE(EndianType) struct PhysicsPoweredMassPoint {
        TagString name;
        EndianType<PhysicsPoweredMassPointFlags> flags;
        EndianType<float> antigrav_strength;
        EndianType<float> antigrav_offset;
        EndianType<float> antigrav_height;
        EndianType<float> antigrav_damp_fraction;
        EndianType<float> antigrav_normal_k1;
        EndianType<float> antigrav_normal_k0;
        PAD(0x44);

        ENDIAN_TEMPLATE(NewType) operator PhysicsPoweredMassPoint<NewType>() const noexcept {
            PhysicsPoweredMassPoint<NewType> copy = {};
            COPY_THIS(name);
            COPY_THIS(flags);
            COPY_THIS(antigrav_strength);
            COPY_THIS(antigrav_offset);
            COPY_THIS(antigrav_height);
            COPY_THIS(antigrav_damp_fraction);
            COPY_THIS(antigrav_normal_k1);
            COPY_THIS(antigrav_normal_k0);
            return copy;
        }
    };
    static_assert(sizeof(PhysicsPoweredMassPoint<BigEndian>) == 0x80);

    struct PhysicsMassPointFlags {
        std::uint32_t metallic : 1;
    };

    ENDIAN_TEMPLATE(EndianType) struct PhysicsMassPoint {
        TagString name;
        EndianType<std::int16_t> powered_mass_point;
        EndianType<std::int16_t> model_node;
        EndianType<PhysicsMassPointFlags> flags;
        EndianType<float> relative_mass;
        EndianType<float> mass;
        EndianType<float> relative_density;
        EndianType<float> density;
        Point3D<EndianType> position;
        Vector3D<EndianType> forward;
        Vector3D<EndianType> up;
        EndianType<PhysicsFrictionType> friction_type;
        PAD(0x2);
        EndianType<float> friction_parallel_scale;
        EndianType<float> friction_perpendicular_scale;
        EndianType<float> radius;
        PAD(0x14);

        ENDIAN_TEMPLATE(NewType) operator PhysicsMassPoint<NewType>() const noexcept {
            PhysicsMassPoint<NewType> copy = {};
            COPY_THIS(name);
            COPY_THIS(powered_mass_point);
            COPY_THIS(model_node);
            COPY_THIS(flags);
            COPY_THIS(relative_mass);
            COPY_THIS(mass);
            COPY_THIS(relative_density);
            COPY_THIS(density);
            COPY_THIS(position);
            COPY_THIS(forward);
            COPY_THIS(up);
            COPY_THIS(friction_type);
            COPY_THIS(friction_parallel_scale);
            COPY_THIS(friction_perpendicular_scale);
            COPY_THIS(radius);
            return copy;
        }
    };
    static_assert(sizeof(PhysicsMassPoint<BigEndian>) == 0x80);

    ENDIAN_TEMPLATE(EndianType) struct Physics {
        EndianType<float> radius;
        EndianType<Fraction> moment_scale;
        EndianType<float> mass;
        Point3D<EndianType> center_of_mass;
        EndianType<float> density;
        EndianType<float> gravity_scale;
        EndianType<float> ground_friction;
        EndianType<float> ground_depth;
        EndianType<Fraction> ground_damp_fraction;
        EndianType<float> ground_normal_k1;
        EndianType<float> ground_normal_k0;
        PAD(0x4);
        EndianType<float> water_friction;
        EndianType<float> water_depth;
        EndianType<float> water_density;
        PAD(0x4);
        EndianType<Fraction> air_friction;
        PAD(0x4);
        EndianType<float> xx_moment;
        EndianType<float> yy_moment;
        EndianType<float> zz_moment;
        TagReflexive<EndianType, PhysicsInertialMatrix> inertial_matrix_and_inverse;
        TagReflexive<EndianType, PhysicsPoweredMassPoint> powered_mass_points;
        TagReflexive<EndianType, PhysicsMassPoint> mass_points;

        ENDIAN_TEMPLATE(NewType) operator Physics<NewType>() const noexcept {
            Physics<NewType> copy = {};
            COPY_THIS(radius);
            COPY_THIS(moment_scale);
            COPY_THIS(mass);
            COPY_THIS(center_of_mass);
            COPY_THIS(density);
            COPY_THIS(gravity_scale);
            COPY_THIS(ground_friction);
            COPY_THIS(ground_depth);
            COPY_THIS(ground_damp_fraction);
            COPY_THIS(ground_normal_k1);
            COPY_THIS(ground_normal_k0);
            COPY_THIS(water_friction);
            COPY_THIS(water_depth);
            COPY_THIS(water_density);
            COPY_THIS(air_friction);
            COPY_THIS(xx_moment);
            COPY_THIS(yy_moment);
            COPY_THIS(zz_moment);
            COPY_THIS(inertial_matrix_and_inverse);
            COPY_THIS(powered_mass_points);
            COPY_THIS(mass_points);
            return copy;
        }
    };
    static_assert(sizeof(Physics<BigEndian>) == 0x80);

    void compile_physics_tag(CompiledTag &compiled, const std::byte *data, std::size_t size);
}
#endif
