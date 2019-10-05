// SPDX-License-Identifier: GPL-3.0-only

#ifndef INVADER__TAG__HEK__CLASS__DAMAGE_EFFECT_HPP
#define INVADER__TAG__HEK__CLASS__DAMAGE_EFFECT_HPP

#include "../../compiled_tag.hpp"
#include "../../../hek/data_type.hpp"
#include "../header.hpp"

namespace Invader::HEK {
    enum DamageEffectScreenFlashType : TagEnum {
        DAMAGE_EFFECT_SCREEN_FLASH_TYPE_NONE,
        DAMAGE_EFFECT_SCREEN_FLASH_TYPE_LIGHTEN,
        DAMAGE_EFFECT_SCREEN_FLASH_TYPE_DARKEN,
        DAMAGE_EFFECT_SCREEN_FLASH_TYPE_MAX,
        DAMAGE_EFFECT_SCREEN_FLASH_TYPE_MIN,
        DAMAGE_EFFECT_SCREEN_FLASH_TYPE_INVERT,
        DAMAGE_EFFECT_SCREEN_FLASH_TYPE_TINT
    };

    enum DamageEffectScreenFlashPriority : TagEnum {
        DAMAGE_EFFECT_SCREEN_FLASH_PRIORITY_LOW,
        DAMAGE_EFFECT_SCREEN_FLASH_PRIORITY_MEDIUM,
        DAMAGE_EFFECT_SCREEN_FLASH_PRIORITY_HIGH
    };

    enum DamageEffectWobbleFunction : TagEnum {
        DAMAGE_EFFECT_WOBBLE_FUNCTION_ONE,
        DAMAGE_EFFECT_WOBBLE_FUNCTION_ZERO,
        DAMAGE_EFFECT_WOBBLE_FUNCTION_COSINE,
        DAMAGE_EFFECT_WOBBLE_FUNCTION_COSINE_VARIABLE_PERIOD,
        DAMAGE_EFFECT_WOBBLE_FUNCTION_DIAGONAL_WAVE,
        DAMAGE_EFFECT_WOBBLE_FUNCTION_DIAGONAL_WAVE_VARIABLE_PERIOD,
        DAMAGE_EFFECT_WOBBLE_FUNCTION_SLIDE,
        DAMAGE_EFFECT_WOBBLE_FUNCTION_SLIDE_VARIABLE_PERIOD,
        DAMAGE_EFFECT_WOBBLE_FUNCTION_NOISE,
        DAMAGE_EFFECT_WOBBLE_FUNCTION_JITTER,
        DAMAGE_EFFECT_WOBBLE_FUNCTION_WANDER,
        DAMAGE_EFFECT_WOBBLE_FUNCTION_SPARK
    };

    enum DamageEffectSideEffect : TagEnum {
        DAMAGE_EFFECT_SIDE_EFFECT_NONE,
        DAMAGE_EFFECT_SIDE_EFFECT_HARMLESS,
        DAMAGE_EFFECT_SIDE_EFFECT_LETHAL_TO_THE_UNSUSPECTING,
        DAMAGE_EFFECT_SIDE_EFFECT_EMP
    };

    enum DamageEffectCategory : TagEnum {
        DAMAGE_EFFECT_CATEGORY_NONE,
        DAMAGE_EFFECT_CATEGORY_FALLING,
        DAMAGE_EFFECT_CATEGORY_BULLET,
        DAMAGE_EFFECT_CATEGORY_GRENADE,
        DAMAGE_EFFECT_CATEGORY_HIGH_EXPLOSIVE,
        DAMAGE_EFFECT_CATEGORY_SNIPER,
        DAMAGE_EFFECT_CATEGORY_MELEE,
        DAMAGE_EFFECT_CATEGORY_FLAME,
        DAMAGE_EFFECT_CATEGORY_MOUNTED_WEAPON,
        DAMAGE_EFFECT_CATEGORY_VEHICLE,
        DAMAGE_EFFECT_CATEGORY_PLASMA,
        DAMAGE_EFFECT_CATEGORY_NEEDLE,
        DAMAGE_EFFECT_CATEGORY_SHOTGUN
    };

    struct DamageEffectDamageFlags {
        std::uint32_t does_not_hurt_owner : 1;
        std::uint32_t can_cause_headshots : 1;
        std::uint32_t pings_resistant_units : 1;
        std::uint32_t does_not_hurt_friends : 1;
        std::uint32_t does_not_ping_units : 1;
        std::uint32_t detonates_explosives : 1;
        std::uint32_t only_hurts_shields : 1;
        std::uint32_t causes_flaming_death : 1;
        std::uint32_t damage_indicators_always_point_down : 1;
        std::uint32_t skips_shields : 1;
        std::uint32_t only_hurts_one_infection_form : 1;
        std::uint32_t can_cause_multiplayer_headshots : 1;
        std::uint32_t infection_form_pop : 1;
    };
    static_assert(sizeof(DamageEffectDamageFlags) == sizeof(std::uint32_t));

    struct DamageEffectFlags {
        std::uint32_t do_not_scale_damage_by_distance : 1;
    };
    static_assert(sizeof(DamageEffectFlags) == sizeof(std::uint32_t));

    ENDIAN_TEMPLATE(EndianType) struct DamageEffect {
        // 0x00
        EndianType<float> radius_from;
        EndianType<float> radius_to;
        EndianType<float> cutoff_scale;
        EndianType<DamageEffectFlags> flags;

        // 0x10
        PAD(0x10);

        // 0x20
        PAD(0x4);
        EndianType<DamageEffectScreenFlashType> type;
        EndianType<DamageEffectScreenFlashPriority> priority;
        PAD(0x8);

        // 0x30
        PAD(0x4);
        EndianType<float> duration; // seconds
        EndianType<FunctionType> fade_function;
        PAD(0x2);
        PAD(0x4);

        // 0x40
        PAD(0x4);
        EndianType<float> maximum_intensity;
        PAD(0x4);
        ColorARGB<EndianType> color;

        // 0x5C
        EndianType<float> low_frequency_vibrate_frequency;

        // 0x60
        EndianType<float> low_frequency_vibrate_duration;
        EndianType<FunctionType> low_frequency_vibrate_fade_function;
        PAD(0x2);
        PAD(0x8);

        // 0x70
        EndianType<float> high_frequency_vibrate_frequency;
        EndianType<float> high_frequency_vibrate_duration;
        EndianType<FunctionType> high_frequency_vibrate_fade_function;
        PAD(0x2);
        PAD(0x4);

        // 0x80
        PAD(0x10);

        // 0x90
        PAD(0x8);
        EndianType<float> temporary_camera_impulse_duration; // seconds
        EndianType<FunctionType> temporary_camera_impulse_fade_function;
        PAD(0x2);

        // 0xA0
        EndianType<float> temporary_camera_impulse_rotation; // radians
        EndianType<float> temporary_camera_impulse_pushback;
        EndianType<float> jitter_from;
        EndianType<float> jitter_to;

        // 0xB0
        PAD(0x8);
        EndianType<float> permanent_camera_impulse_angle; // radians
        PAD(0x4);

        // 0xC0
        PAD(0xC);
        EndianType<float> camera_shaking_duration; // seconds

        // 0xD0
        EndianType<FunctionType> camera_shaking_falloff_function;
        PAD(0x2);
        EndianType<float> camera_shaking_random_translation;
        EndianType<float> camera_shaking_random_rotation; // radians
        PAD(0x4);

        // 0xE0
        PAD(0x8);
        EndianType<DamageEffectWobbleFunction> camera_shaking_wobble_function;
        PAD(0x2);
        EndianType<float> camera_shaking_wobble_period; // seconds

        // 0xF0
        EndianType<float> camera_shaking_wobble_weight;
        PAD(0xC);

        // 0x100
        PAD(0x10);

        // 0x110
        PAD(0x4);
        TagDependency<EndianType> sound; // sound

        // 0x124
        PAD(0x6C);

        // 0x190
        PAD(0x4);
        EndianType<float> breaking_effect_forward_velocity; // world units per second
        EndianType<float> breaking_effect_forward_radius; // world units
        EndianType<float> breaking_effect_forward_exponent;

        // 0x1A0
        PAD(0x4);
        PAD(0x8);
        EndianType<float> breaking_effect_outward_velocity;

        // 0x1B0
        EndianType<float> breaking_effect_outward_radius;
        EndianType<float> breaking_effect_outward_exponent;
        PAD(0x8);

        // 0x1C0
        PAD(0x4);
        EndianType<DamageEffectSideEffect> damage_side_effect;
        EndianType<DamageEffectCategory> damage_category;
        EndianType<DamageEffectDamageFlags> damage_flags;
        EndianType<float> damage_aoe_core_radius; // world units

        // 0x1D0
        EndianType<float> damage_lower_bound;
        EndianType<float> damage_upper_bound_from;
        EndianType<float> damage_upper_bound_to;
        EndianType<float> damage_vehicle_passthrough_penalty;

        // 0x1E0
        EndianType<float> damage_active_camouflage_damage;
        EndianType<float> damage_stun;
        EndianType<float> damage_maximum_stun;
        EndianType<float> damage_stun_time; // seconds

        // 0x1F0
        PAD(0x4);
        EndianType<float> damage_instantaneous_acceleration;
        PAD(0x8);

        // 0x200
        EndianType<float> damage_modifiers[33];
        PAD(0xC);

        // 0x290
        PAD(0x10);

        ENDIAN_TEMPLATE(NewType) operator DamageEffect<NewType>() const noexcept {
            DamageEffect<NewType> copy = {};

            COPY_THIS(radius_from);
            COPY_THIS(radius_to);
            COPY_THIS(cutoff_scale);
            COPY_THIS(flags);
            COPY_THIS(type);
            COPY_THIS(priority);
            COPY_THIS(duration);
            COPY_THIS(fade_function);
            COPY_THIS(maximum_intensity);
            COPY_THIS(color);
            COPY_THIS(low_frequency_vibrate_frequency);
            COPY_THIS(low_frequency_vibrate_duration);
            COPY_THIS(low_frequency_vibrate_fade_function );
            COPY_THIS(high_frequency_vibrate_frequency);
            COPY_THIS(high_frequency_vibrate_duration);
            COPY_THIS(high_frequency_vibrate_fade_function);
            COPY_THIS(temporary_camera_impulse_duration);
            COPY_THIS(temporary_camera_impulse_fade_function);
            COPY_THIS(temporary_camera_impulse_rotation);
            COPY_THIS(temporary_camera_impulse_pushback);
            COPY_THIS(jitter_from);
            COPY_THIS(jitter_to);
            COPY_THIS(permanent_camera_impulse_angle);
            COPY_THIS(camera_shaking_duration);
            COPY_THIS(camera_shaking_falloff_function);
            COPY_THIS(camera_shaking_random_translation);
            COPY_THIS(camera_shaking_random_rotation);
            COPY_THIS(camera_shaking_wobble_function);
            COPY_THIS(camera_shaking_wobble_period);
            COPY_THIS(camera_shaking_wobble_weight);
            COPY_THIS(sound);
            COPY_THIS(breaking_effect_forward_velocity);
            COPY_THIS(breaking_effect_forward_radius);
            COPY_THIS(breaking_effect_forward_exponent);
            COPY_THIS(breaking_effect_outward_velocity);
            COPY_THIS(breaking_effect_outward_radius);
            COPY_THIS(breaking_effect_outward_exponent);
            COPY_THIS(damage_side_effect);
            COPY_THIS(damage_category);
            COPY_THIS(damage_flags);
            COPY_THIS(damage_aoe_core_radius);
            COPY_THIS(damage_lower_bound);
            COPY_THIS(damage_upper_bound_from);
            COPY_THIS(damage_upper_bound_to);
            COPY_THIS(damage_vehicle_passthrough_penalty);
            COPY_THIS(damage_active_camouflage_damage);
            COPY_THIS(damage_stun);
            COPY_THIS(damage_maximum_stun);
            COPY_THIS(damage_stun_time);
            COPY_THIS(damage_instantaneous_acceleration);
            COPY_THIS_ARRAY(damage_modifiers);

            return copy;
        }
    };
    static_assert(sizeof(DamageEffect<BigEndian>) == 0x2A0);

    enum DamageEffectJasonJones {
        DAMAGE_EFFECT_JASON_JONES_NONE = 0,
        DAMAGE_EFFECT_JASON_JONES_PISTOL_SINGLEPLAYER
    };

    void compile_damage_effect_tag(CompiledTag &compiled, const std::byte *data, std::size_t size, DamageEffectJasonJones jason_jones);
}
#endif
