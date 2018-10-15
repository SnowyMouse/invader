/*
 * Invader (c) 2018 Kavawuvi
 *
 * This program is free software under the GNU General Public License v3.0 or later. See LICENSE for more information.
 */

#pragma once

#include "object.hpp"

namespace Invader::HEK {
    enum ProjectileResponse : TagEnum {
        PROJECTILE_RESPONSE_DISAPPEAR,
        PROJECTILE_RESPONSE_DETONATE,
        PROJECTILE_RESPONSE_REFLECT,
        PROJECTILE_RESPONSE_OVERPENETRATE,
        PROJECTILE_RESPONSE_ATTACH
    };

    enum ProjectileScaleEffectsBy : TagEnum {
        PROJECTILE_SCALE_EFFECTS_BY_DAMAGE,
        PROJECTILE_SCALE_EFFECTS_BY_ANGLE
    };

    enum ProjectileDetonationTimerStarts : TagEnum {
        PROJECTILE_DETONATION_TIMER_STARTS_IMMEDIATELY,
        PROJECTILE_DETONATION_TIMER_STARTS_AFTER_FIRST_BOUNCE,
        PROJECTILE_DETONATION_TIMER_STARTS_WHEN_AT_REST
    };

    enum ProjectileImpactNoise : TagEnum {
        PROJECTILE_IMPACT_NOISE_SILENT,
        PROJECTILE_IMPACT_NOISE_MEDIUM,
        PROJECTILE_IMPACT_NOISE_LOUD,
        PROJECTILE_IMPACT_NOISE_SHOUT,
        PROJECTILE_IMPACT_NOISE_QUIET
    };

    enum ProjectileFunctionIn : TagEnum {
        PROJECTILE_FUNCTION_IN_NONE,
        PROJECTILE_FUNCTION_IN_RANGE_REMAINING,
        PROJECTILE_FUNCTION_IN_TIME_REMAINING,
        PROJECTILE_FUNCTION_IN_TRACER
    };

    struct ProjectileFlags {
        std::uint32_t oriented_along_velocity : 1;
        std::uint32_t ai_must_use_ballistic_aiming : 1;
        std::uint32_t detonation_max_time_if_attached : 1;
        std::uint32_t has_super_combining_explosion : 1;
        std::uint32_t combine_initial_velocity_with_parent_velocity : 1;
        std::uint32_t random_attached_detonation_time : 1;
        std::uint32_t minimum_unattached_detonation_time : 1;
    };

    struct ProjectileMaterialResponseFlags {
        std::uint16_t cannot_be_overpenetrated : 1;
    };

    struct ProjectileMaterialResponsePotentialFlags {
        std::uint16_t only_against_units;
    };

    ENDIAN_TEMPLATE(EndianType) struct ProjectileMaterialResponse {
        EndianType<ProjectileMaterialResponseFlags> flags;
        EndianType<ProjectileResponse> default_response;
        TagDependency<EndianType> default_effect; // effect
        PAD(0x10);
        EndianType<ProjectileResponse> potential_response;
        EndianType<ProjectileMaterialResponsePotentialFlags> potential_flags;
        EndianType<Fraction> potential_skip_fraction;
        Bounds<EndianType<float>> potential_between;
        Bounds<EndianType<float>> potential_and;
        TagDependency<EndianType> potential_effect; // effect
        PAD(0x10);
        EndianType<ProjectileScaleEffectsBy> scale_effects_by;
        PAD(0x2);
        EndianType<Angle> angular_noise;
        EndianType<float> velocity_noise;
        TagDependency<EndianType> detonation_effect; // effect
        PAD(0x18);
        EndianType<float> initial_friction;
        EndianType<float> maximum_distance;
        EndianType<float> parallel_friction;
        EndianType<float> perpendicular_friction;

        ENDIAN_TEMPLATE(NewType) operator ProjectileMaterialResponse<NewType>() const noexcept {
            ProjectileMaterialResponse<NewType> copy = {};
            COPY_THIS(flags);
            COPY_THIS(default_response);
            COPY_THIS(default_effect);
            COPY_THIS(potential_response);
            COPY_THIS(potential_flags);
            COPY_THIS(potential_skip_fraction);
            COPY_THIS(potential_between);
            COPY_THIS(potential_and);
            COPY_THIS(potential_effect);
            COPY_THIS(scale_effects_by);
            COPY_THIS(angular_noise);
            COPY_THIS(velocity_noise);
            COPY_THIS(detonation_effect);
            COPY_THIS(initial_friction);
            COPY_THIS(maximum_distance);
            COPY_THIS(parallel_friction);
            COPY_THIS(perpendicular_friction);
            return copy;
        }
    };
    static_assert(sizeof(ProjectileMaterialResponse<BigEndian>) == 0xA0);

    ENDIAN_TEMPLATE(EndianType) struct Projectile : Object<EndianType> {
        EndianType<ProjectileFlags> flags;
        EndianType<ProjectileDetonationTimerStarts> detonation_timer_starts;
        EndianType<ProjectileImpactNoise> impact_noise;
        EndianType<ProjectileFunctionIn> projectile_a_in;
        EndianType<ProjectileFunctionIn> projectile_b_in;
        EndianType<ProjectileFunctionIn> projectile_c_in;
        EndianType<ProjectileFunctionIn> projectile_d_in;
        TagDependency<EndianType> super_detonation; // effect
        EndianType<float> ai_perception_radius;
        EndianType<float> collision_radius;
        EndianType<float> arming_time;
        EndianType<float> danger_radius;
        TagDependency<EndianType> effect; // effect
        Bounds<EndianType<float>> timer;
        EndianType<float> minimum_velocity;
        EndianType<float> maximum_range;
        EndianType<float> air_gravity_scale;
        Bounds<EndianType<float>> air_damage_range;
        EndianType<float> water_gravity_scale;
        Bounds<EndianType<float>> water_damage_range;
        EndianType<float> initial_velocity;
        EndianType<float> final_velocity;
        EndianType<Angle> guided_angular_velocity;
        EndianType<ProjectileImpactNoise> detonation_noise;
        PAD(0x2);
        TagDependency<EndianType> detonation_started; // effect
        TagDependency<EndianType> flyby_sound; // sound
        TagDependency<EndianType> attached_detonation_damage; // damage_effect
        TagDependency<EndianType> impact_damage; // damage_effect
        PAD(0xC);
        TagReflexive<EndianType, ProjectileMaterialResponse> projectile_material_response;

        ENDIAN_TEMPLATE(NewType) operator Projectile<NewType>() const noexcept {
            Projectile<NewType> copy = {};
            COPY_OBJECT_DATA
            COPY_THIS(flags);
            COPY_THIS(detonation_timer_starts);
            COPY_THIS(impact_noise);
            COPY_THIS(projectile_a_in);
            COPY_THIS(projectile_b_in);
            COPY_THIS(projectile_c_in);
            COPY_THIS(projectile_d_in);
            COPY_THIS(super_detonation);
            COPY_THIS(ai_perception_radius);
            COPY_THIS(collision_radius);
            COPY_THIS(arming_time);
            COPY_THIS(danger_radius);
            COPY_THIS(effect);
            COPY_THIS(timer);
            COPY_THIS(minimum_velocity);
            COPY_THIS(maximum_range);
            COPY_THIS(air_gravity_scale);
            COPY_THIS(air_damage_range);
            COPY_THIS(water_gravity_scale);
            COPY_THIS(water_damage_range);
            COPY_THIS(initial_velocity);
            COPY_THIS(final_velocity);
            COPY_THIS(guided_angular_velocity);
            COPY_THIS(detonation_noise);
            COPY_THIS(detonation_started);
            COPY_THIS(flyby_sound);
            COPY_THIS(attached_detonation_damage);
            COPY_THIS(impact_damage);
            COPY_THIS(projectile_material_response);
            return copy;
        }
    };
    static_assert(sizeof(Projectile<BigEndian>) == 0x24C);

    void compile_projectile_tag(CompiledTag &compiled, const std::byte *data, std::size_t size);
}
