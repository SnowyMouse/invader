// SPDX-License-Identifier: GPL-3.0-only

#ifndef INVADER__TAG__HEK__CLASS__EFFECT_HPP
#define INVADER__TAG__HEK__CLASS__EFFECT_HPP

#include "../../compiled_tag.hpp"
#include "../../../hek/data_type.hpp"
#include "../header.hpp"

namespace Invader::HEK {
    enum EffectCreateIn1 : TagEnum {
        EFFECT_CREATE_IN_ANY_ENVIRONMENT,
        EFFECT_CREATE_IN_AIR_ONLY,
        EFFECT_CREATE_IN_WATER_ONLY,
        EFFECT_CREATE_IN_SPACE_ONLY
    };

    enum EffectCreateIn2 : TagEnum {
        EFFECT_CREATE_IN_EITHER_MODE,
        EFFECT_CREATE_IN_VIOLENT_MODE_ONLY,
        EFFECT_CREATE_IN_NONVIOLENT_MODE_ONLY
    };

    enum EffectCreate : TagEnum {
        EFFECT_CREATE_INDEPENDENT_OF_CAMERA_MODE,
        EFFECT_CREATE_ONLY_IN_FIRST_PERSON,
        EFFECT_CREATE_ONLY_IN_THIRD_PERSON,
        EFFECT_CREATE_IN_FIRST_PERSON_IF_POSSIBLE
    };

    enum EffectDistributionFunction : TagEnum {
        EFFECT_DISTRIBUTION_FUNCTION_START,
        EFFECT_DISTRIBUTION_FUNCTION_END,
        EFFECT_DISTRIBUTION_FUNCTION_CONSTANT,
        EFFECT_DISTRIBUTION_FUNCTION_BUILDUP,
        EFFECT_DISTRIBUTION_FUNCTION_FALLOFF,
        EFFECT_DISTRIBUTION_FUNCTION_BUILDUP_AND_FALLOFF
    };

    ENDIAN_TEMPLATE(EndianType) struct EffectLocation {
        TagString marker_name;

        ENDIAN_TEMPLATE(NewType) operator EffectLocation<NewType>() const noexcept {
            EffectLocation<NewType> copy;
            COPY_THIS(marker_name);
            return copy;
        }
    };
    static_assert(sizeof(EffectLocation<BigEndian>) == 0x20);

    struct EffectPartFlags {
        std::uint16_t face_down_regardless_of_location_decals : 1;
        std::uint16_t unused : 1;
        std::uint16_t make_effect_work : 1;
    };

    struct EffectPartScalesValues {
        std::uint32_t velocity : 1;
        std::uint32_t velocity_delta : 1;
        std::uint32_t velocity_cone_angle : 1;
        std::uint32_t angular_velocity : 1;
        std::uint32_t angular_velocity_delta : 1;
        std::uint32_t type_specific_scale : 1;
    };

    ENDIAN_TEMPLATE(EndianType) struct EffectPart {
        EndianType<EffectCreateIn1> create_in_1;
        EndianType<EffectCreateIn2> create_in_2;
        EndianType<std::int16_t> location;
        EndianType<EffectPartFlags> flags;
        PAD(0xC);
        LittleEndian<std::uint32_t> type_type;
        TagDependency<EndianType> type; // damage_effect, object, particle_system, sound, decal, light
        PAD(0x18);
        Bounds<EndianType<float>> velocity_bounds;
        EndianType<Angle> velocity_cone_angle;
        Bounds<EndianType<Angle>> angular_velocity_bounds;
        Bounds<EndianType<float>> radius_modifier_bounds;
        PAD(0x4);
        EndianType<EffectPartScalesValues> a_scales_values;
        EndianType<EffectPartScalesValues> b_scales_values;

        ENDIAN_TEMPLATE(NewType) operator EffectPart<NewType>() const noexcept {
            EffectPart<NewType> copy = {};
            COPY_THIS(create_in_1);
            COPY_THIS(create_in_2);
            COPY_THIS(location);
            COPY_THIS(flags);
            COPY_THIS(type_type);
            COPY_THIS(type);
            COPY_THIS(velocity_bounds);
            COPY_THIS(velocity_cone_angle);
            COPY_THIS(angular_velocity_bounds);
            COPY_THIS(radius_modifier_bounds);
            COPY_THIS(a_scales_values);
            COPY_THIS(b_scales_values);
            return copy;
        }
    };
    static_assert(sizeof(EffectPart<BigEndian>) == 0x68);

    struct EffectParticleFlags {
        std::uint32_t stay_attached_to_marker : 1;
        std::uint32_t random_initial_angle : 1;
        std::uint32_t tint_from_object_color : 1;
        std::uint32_t interpolate_tint_as_hsv : 1;
        std::uint32_t _across_the_long_hue_path : 1;
    };

    struct EffectParticleScalesValues {
        std::uint32_t velocity : 1;
        std::uint32_t velocity_delta : 1;
        std::uint32_t velocity_cone_angle : 1;
        std::uint32_t angular_velocity : 1;
        std::uint32_t angular_velocity_delta : 1;
        std::uint32_t count : 1;
        std::uint32_t count_delta : 1;
        std::uint32_t distribution_radius : 1;
        std::uint32_t distribution_radius_delta : 1;
        std::uint32_t particle_radius : 1;
        std::uint32_t particle_radius_delta : 1;
        std::uint32_t tint : 1;
    };

    ENDIAN_TEMPLATE(EndianType) struct EffectParticle {
        EndianType<EffectCreateIn1> create_in_1;
        EndianType<EffectCreateIn2> create_in_2;
        EndianType<EffectCreate> create;
        PAD(0x2);
        EndianType<std::int16_t> location;
        PAD(0x2);
        Euler2D<EndianType> relative_direction;
        Vector3D<EndianType> relative_offset;
        Vector3D<EndianType> relative_direction_vector;
        PAD(0x28);
        TagDependency<EndianType> particle_type; // particle
        EndianType<EffectParticleFlags> flags;
        EndianType<EffectDistributionFunction> distribution_function;
        PAD(0x2);
        Bounds<EndianType<std::int16_t>> count;
        Bounds<EndianType<float>> distribution_radius;
        PAD(0xC);
        Bounds<EndianType<float>> velocity;
        EndianType<Angle> velocity_cone_angle;
        Bounds<EndianType<Angle>> angular_velocity;
        PAD(0x8);
        Bounds<EndianType<float>> radius;
        PAD(0x8);
        ColorARGB<EndianType> tint_lower_bound;
        ColorARGB<EndianType> tint_upper_bound;
        PAD(0x10);
        EndianType<EffectParticleScalesValues> a_scales_values;
        EndianType<EffectParticleScalesValues> b_scales_values;

        ENDIAN_TEMPLATE(NewType) operator EffectParticle<NewType>() const noexcept {
            EffectParticle<NewType> copy = {};
            COPY_THIS(create_in_1);
            COPY_THIS(create_in_2);
            COPY_THIS(create);
            COPY_THIS(location);
            COPY_THIS(relative_direction);
            COPY_THIS(relative_offset);
            COPY_THIS(relative_direction_vector);
            COPY_THIS(particle_type);
            COPY_THIS(flags);
            COPY_THIS(distribution_function);
            COPY_THIS(count);
            COPY_THIS(distribution_radius);
            COPY_THIS(velocity);
            COPY_THIS(velocity_cone_angle);
            COPY_THIS(angular_velocity);
            COPY_THIS(radius);
            COPY_THIS(tint_lower_bound);
            COPY_THIS(tint_upper_bound);
            COPY_THIS(a_scales_values);
            COPY_THIS(b_scales_values);
            return copy;
        }
    };
    static_assert(sizeof(EffectParticle<BigEndian>) == 0xE8);

    ENDIAN_TEMPLATE(EndianType) struct EffectEvent {
        PAD(0x4);
        EndianType<Fraction> skip_fraction;
        Bounds<EndianType<float>> delay_bounds;
        Bounds<EndianType<float>> duration_bounds;
        PAD(0x14);
        TagReflexive<EndianType, EffectPart> parts;
        TagReflexive<EndianType, EffectParticle> particles;

        ENDIAN_TEMPLATE(NewType) operator EffectEvent<NewType>() const noexcept {
            EffectEvent<NewType> copy = {};
            COPY_THIS(skip_fraction);
            COPY_THIS(delay_bounds);
            COPY_THIS(duration_bounds);
            COPY_THIS(parts);
            COPY_THIS(particles);
            return copy;
        }
    };
    static_assert(sizeof(EffectEvent<BigEndian>) == 0x44);

    struct EffectFlags {
        std::uint32_t deleted_when_attachment_deactivates : 1;
        std::uint32_t required_for_gameplay_cannot_optimize_out : 1;
        std::uint32_t use_in_dedicated_servers : 1;
    };

    ENDIAN_TEMPLATE(EndianType) struct Effect {
        EndianType<EffectFlags> flags;
        EndianType<std::int16_t> loop_start_event;
        EndianType<std::int16_t> loop_stop_event;
        PAD(0x20);
        TagReflexive<EndianType, EffectLocation> locations;
        TagReflexive<EndianType, EffectEvent> events;

        ENDIAN_TEMPLATE(NewType) operator Effect<NewType>() const noexcept {
            Effect<NewType> copy = {};
            COPY_THIS(flags);
            COPY_THIS(loop_start_event);
            COPY_THIS(loop_stop_event);
            COPY_THIS(locations);
            COPY_THIS(events);
            return copy;
        }
    };
    static_assert(sizeof(Effect<BigEndian>) == 0x40);

    void compile_effect_tag(CompiledTag &compiled, const std::byte *data, std::size_t size);
}
#endif
