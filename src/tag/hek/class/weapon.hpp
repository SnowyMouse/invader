/*
 * Invader (c) 2018 Kavawuvi
 *
 * This program is free software under the GNU General Public License v3.0 or later. See LICENSE for more information.
 */

#pragma once

#include "item.hpp"

namespace Invader::HEK {
    enum WeaponSecondaryTriggerMode : TagEnum {
        WEAPON_SECONDARY_TRIGGER_MODE_NORMAL,
        WEAPON_SECONDARY_TRIGGER_MODE_SLAVED_TO_PRIMARY,
        WEAPON_SECONDARY_TRIGGER_MODE_INHIBITS_PRIMARY,
        WEAPON_SECONDARY_TRIGGER_MODE_LOADS_ALTERATE_AMMUNITION,
        WEAPON_SECONDARY_TRIGGER_MODE_LOADS_MULTIPLE_PRIMARY_AMMUNITION
    };

    enum WeaponFunctionIn : TagEnum {
        WEAPON_FUNCTION_IN_NONE,
        WEAPON_FUNCTION_IN_HEAT,
        WEAPON_FUNCTION_IN_PRIMARY_AMMUNITION,
        WEAPON_FUNCTION_IN_SECONDARY_AMMUNITION,
        WEAPON_FUNCTION_IN_PRIMARY_RATE_OF_FIRE,
        WEAPON_FUNCTION_IN_SECONDARY_RATE_OF_FIRE,
        WEAPON_FUNCTION_IN_READY,
        WEAPON_FUNCTION_IN_PRIMARY_EJECTION_PORT,
        WEAPON_FUNCTION_IN_SECONDARY_EJECTION_PORT,
        WEAPON_FUNCTION_IN_OVERHEATED,
        WEAPON_FUNCTION_IN_PRIMARY_CHARGED,
        WEAPON_FUNCTION_IN_SECONDARY_CHARGED,
        WEAPON_FUNCTION_IN_ILLUMINATION,
        WEAPON_FUNCTION_IN_AGE,
        WEAPON_FUNCTION_IN_INTEGRATED_LIGHT,
        WEAPON_FUNCTION_IN_PRIMARY_FIRING,
        WEAPON_FUNCTION_IN_SECONDARY_FIRING,
        WEAPON_FUNCTION_IN_PRIMARY_FIRING_ON,
        WEAPON_FUNCTION_IN_SECONDARY_FIRING_ON
    };

    enum WeaponMovementPenalized : TagEnum {
        WEAPON_MOVEMENT_PENALIZED_ALWAYS,
        WEAPON_MOVEMENT_PENALIZED_WHEN_ZOOMED,
        WEAPON_MOVEMENT_PENALIZED_WHEN_ZOOMED_OR_RELOADING
    };

    enum WeaponType : TagEnum {
        WEAPON_TYPE_UNDEFINED,
        WEAPON_TYPE_SHOTGUN,
        WEAPON_TYPE_NEEDLER,
        WEAPON_TYPE_PLASMA_PISTOL,
        WEAPON_TYPE_PLASMA_RIFLE
    };

    enum WeaponFiringNoise : TagEnum {
        WEAPON_FIRING_NOISE_SILENT,
        WEAPON_FIRING_NOISE_MEDIUM,
        WEAPON_FIRING_NOISE_LOUD,
        WEAPON_FIRING_NOISE_SHOUT,
        WEAPON_FIRING_NOISE_QUIET
    };

    enum WeaponOverchargedAction : TagEnum {
        WEAPON_OVERCHARGED_ACTION_NONE,
        WEAPON_OVERCHARGED_ACTION_EXPLODE,
        WEAPON_OVERCHARGED_ACTION_DISCHARGE
    };
    enum WeaponDistributionFunction : TagEnum {
        WEAPON_DISTRIBUTION_FUNCTION_POINT,
        WEAPON_DISTRIBUTION_FUNCTION_HORIZONTAL_FAN
    };

    ENDIAN_TEMPLATE(EndianType) struct WeaponMagazineObject {
        EndianType<std::int16_t> rounds;
        PAD(0xA);
        TagDependency<EndianType> equipment; // equipment

        ENDIAN_TEMPLATE(NewType) operator WeaponMagazineObject<NewType>() const noexcept {
            WeaponMagazineObject<NewType> copy = {};
            COPY_THIS(rounds);
            COPY_THIS(equipment);
            return copy;
        }
    };
    static_assert(sizeof(WeaponMagazineObject<BigEndian>) == 0x1C);

    struct WeaponMagazineFlags {
        std::uint32_t wastes_rounds_when_reloaded : 1;
        std::uint32_t every_round_must_be_chambered : 1;
    };

    ENDIAN_TEMPLATE(EndianType) struct WeaponMagazine {
        EndianType<WeaponMagazineFlags> flags;
        EndianType<std::int16_t> rounds_recharged;
        EndianType<std::int16_t> rounds_total_initial;
        EndianType<std::int16_t> rounds_total_maximum;
        EndianType<std::int16_t> rounds_loaded_maximum;
        PAD(0x8);
        EndianType<float> reload_time;
        EndianType<std::int16_t> rounds_reloaded;
        PAD(0x2);
        EndianType<float> chamber_time;
        PAD(0x8);
        PAD(0x10);
        TagDependency<EndianType> reloading_effect; // sound, effect
        TagDependency<EndianType> chambering_effect; // sound, effect
        PAD(0xC);
        TagReflexive<EndianType, WeaponMagazineObject> magazine_objects;

        ENDIAN_TEMPLATE(NewType) operator WeaponMagazine<NewType>() const noexcept {
            WeaponMagazine<NewType> copy = {};
            COPY_THIS(flags);
            COPY_THIS(rounds_recharged);
            COPY_THIS(rounds_total_initial);
            COPY_THIS(rounds_total_maximum);
            COPY_THIS(rounds_loaded_maximum);
            COPY_THIS(reload_time);
            COPY_THIS(rounds_reloaded);
            COPY_THIS(chamber_time);
            COPY_THIS(reloading_effect);
            COPY_THIS(chambering_effect);
            COPY_THIS(magazine_objects);
            return copy;
        }
    };
    static_assert(sizeof(WeaponMagazine<BigEndian>) == 0x70);

    ENDIAN_TEMPLATE(EndianType) struct WeaponTriggerFiringEffect {
        EndianType<std::int16_t> shot_count_lower_bound;
        EndianType<std::int16_t> shot_count_upper_bound;
        PAD(0x20);
        TagDependency<EndianType> firing_effect; // sound, effect
        TagDependency<EndianType> misfire_effect; // sound, effect
        TagDependency<EndianType> empty_effect; // sound, effect
        TagDependency<EndianType> firing_damage; // damage
        TagDependency<EndianType> misfire_damage; // damage
        TagDependency<EndianType> empty_damage; // damage

        ENDIAN_TEMPLATE(NewType) operator WeaponTriggerFiringEffect<NewType>() const noexcept {
            WeaponTriggerFiringEffect<NewType> copy = {};
            COPY_THIS(shot_count_lower_bound);
            COPY_THIS(shot_count_upper_bound);
            COPY_THIS(firing_effect);
            COPY_THIS(misfire_effect);
            COPY_THIS(empty_effect);
            COPY_THIS(firing_damage);
            COPY_THIS(misfire_damage);
            COPY_THIS(empty_damage);
            return copy;
        }
    };
    static_assert(sizeof(WeaponTriggerFiringEffect<BigEndian>) == 0x84);

    struct WeaponTriggerFlags {
        std::uint32_t tracks_fired_projectile : 1;
        std::uint32_t random_firing_effects : 1;
        std::uint32_t can_fire_with_partial_ammo : 1;
        std::uint32_t does_not_repeat_automatically : 1;
        std::uint32_t locks_in_on_off_state : 1;
        std::uint32_t projectiles_use_weapon_origin : 1;
        std::uint32_t sticks_when_dropped : 1;
        std::uint32_t ejects_during_chamber : 1;
        std::uint32_t discharging_spews : 1;
        std::uint32_t analog_rate_of_fire : 1;
        std::uint32_t use_error_when_unzoomed : 1;
        std::uint32_t projectile_vector_cannot_be_adjusted : 1;
        std::uint32_t projectiles_have_identical_error : 1;
        std::uint32_t projectile_is_client_side_only : 1;
    };

    ENDIAN_TEMPLATE(EndianType) struct WeaponTrigger {
        EndianType<WeaponTriggerFlags> flags;
        Bounds<EndianType<float>> rounds_per_second;
        EndianType<float> acceleration_time;
        EndianType<float> deceleration_time;
        EndianType<Fraction> blurred_rate_of_fire;
        PAD(0x8);
        EndianType<std::int16_t> magazine;
        EndianType<std::int16_t> rounds_per_shot;
        EndianType<std::int16_t> minimum_rounds_loaded;
        EndianType<std::int16_t> rounds_between_tracers;
        PAD(0x6);
        EndianType<WeaponFiringNoise> firing_noise;
        Bounds<EndianType<float>> error;
        EndianType<float> error_acceleration_time;
        EndianType<float> error_deceleration_time;
        PAD(0x8);
        EndianType<float> charging_time;
        EndianType<float> charged_time;
        EndianType<WeaponDistributionFunction> overcharged_action;
        PAD(0x2);
        EndianType<float> charged_illumination;
        EndianType<float> spew_time;
        TagDependency<EndianType> charging_effect; // sound, effect
        EndianType<WeaponDistributionFunction> distribution_function;
        EndianType<std::int16_t> projectiles_per_shot;
        EndianType<float> distribution_angle;
        PAD(0x4);
        EndianType<Angle> minimum_error;
        Bounds<EndianType<Angle>> error_angle;
        Point3D<EndianType> first_person_offset;
        PAD(0x4);
        TagDependency<EndianType> projectile; // object
        EndianType<float> ejection_port_recovery_time;
        EndianType<float> illumination_recovery_time;
        PAD(0xC);
        EndianType<Fraction> heat_generated_per_round;
        EndianType<Fraction> age_generated_per_round;
        PAD(0x4);
        EndianType<float> overload_time;
        PAD(0x8);
        PAD(0x20);
        EndianType<float> ejection_port_recovery_rate;
        EndianType<float> illumination_recovery_rate;
        EndianType<float> firing_acceleration_rate;
        EndianType<float> firing_deceleration_rate;
        EndianType<float> error_acceleration_rate;
        EndianType<float> error_deceleration_rate;
        TagReflexive<EndianType, WeaponTriggerFiringEffect> firing_effects;

        ENDIAN_TEMPLATE(NewType) operator WeaponTrigger<NewType>() const noexcept {
            WeaponTrigger<NewType> copy = {};
            COPY_THIS(flags);
            COPY_THIS(rounds_per_second);
            COPY_THIS(acceleration_time);
            COPY_THIS(deceleration_time);
            COPY_THIS(blurred_rate_of_fire);
            COPY_THIS(magazine);
            COPY_THIS(rounds_per_shot);
            COPY_THIS(minimum_rounds_loaded);
            COPY_THIS(rounds_between_tracers);
            COPY_THIS(firing_noise);
            COPY_THIS(error);
            COPY_THIS(error_acceleration_time);
            COPY_THIS(error_deceleration_time);
            COPY_THIS(charging_time);
            COPY_THIS(charged_time);
            COPY_THIS(overcharged_action);
            COPY_THIS(charged_illumination);
            COPY_THIS(spew_time);
            COPY_THIS(charging_effect);
            COPY_THIS(distribution_function);
            COPY_THIS(projectiles_per_shot);
            COPY_THIS(distribution_angle);
            COPY_THIS(minimum_error);
            COPY_THIS(error_angle);
            COPY_THIS(first_person_offset);
            COPY_THIS(projectile);
            COPY_THIS(ejection_port_recovery_time);
            COPY_THIS(illumination_recovery_time);
            COPY_THIS(heat_generated_per_round);
            COPY_THIS(age_generated_per_round);
            COPY_THIS(overload_time);
            COPY_THIS(firing_effects);
            return copy;
        }
    };
    static_assert(sizeof(WeaponTrigger<BigEndian>) == 0x114);

    struct WeaponFlags {
        std::uint32_t vertical_heat_display : 1;
        std::uint32_t mutually_exclusive_triggers : 1;
        std::uint32_t attacks_automatically_on_bump : 1;
        std::uint32_t must_be_readied : 1;
        std::uint32_t doesn_t_count_toward_maximum : 1;
        std::uint32_t aim_assists_only_when_zoomed : 1;
        std::uint32_t prevents_grenade_throwing : 1;
        std::uint32_t must_be_picked_up : 1;
        std::uint32_t holds_triggers_when_dropped : 1;
        std::uint32_t prevents_melee_attack : 1;
        std::uint32_t detonates_when_dropped : 1;
        std::uint32_t cannot_fire_at_maximum_age : 1;
        std::uint32_t secondary_trigger_overrides_grenades : 1;
        std::uint32_t obsolete_does_not_depower_active_camo_in_multilplayer : 1;
        std::uint32_t enables_integrated_night_vision : 1;
        std::uint32_t ais_use_weapon_melee_damage : 1;
    };

    ENDIAN_TEMPLATE(EndianType) struct Weapon : Item<EndianType> {
        EndianType<WeaponFlags> flags;
        TagString label;
        EndianType<WeaponSecondaryTriggerMode> secondary_trigger_mode;
        EndianType<std::int16_t> maximum_alternate_shots_loaded;
        EndianType<WeaponFunctionIn> weapon_a_in;
        EndianType<WeaponFunctionIn> weapon_b_in;
        EndianType<WeaponFunctionIn> weapon_c_in;
        EndianType<WeaponFunctionIn> weapon_d_in;
        EndianType<float> ready_time;
        TagDependency<EndianType> ready_effect; // sound, effect
        EndianType<Fraction> heat_recovery_threshold;
        EndianType<Fraction> overheated_threshold;
        EndianType<Fraction> heat_detonation_threshold;
        EndianType<Fraction> heat_detonation_fraction;
        EndianType<Fraction> heat_loss_per_second;
        EndianType<Fraction> heat_illumination;
        PAD(0x10);
        TagDependency<EndianType> overheated; // sound, effect
        TagDependency<EndianType> detonation; // sound, effect
        TagDependency<EndianType> player_melee_damage; // damage_effect
        TagDependency<EndianType> player_melee_response; // damage_effect
        PAD(0x8);
        TagDependency<EndianType> actor_firing_parameters; // actor_variant
        EndianType<float> near_reticle_range;
        EndianType<float> far_reticle_range;
        EndianType<float> intersection_reticle_range;
        PAD(0x2);
        EndianType<std::int16_t> magnification_levels;
        Bounds<EndianType<float>> magnification_range;
        EndianType<Angle> autoaim_angle;
        EndianType<float> autoaim_range;
        EndianType<Angle> magnetism_angle;
        EndianType<float> magnetism_range;
        EndianType<Angle> deviation_angle;
        PAD(0x4);
        EndianType<WeaponMovementPenalized> movement_penalized;
        PAD(0x2);
        EndianType<Fraction> forward_movement_penalty;
        EndianType<Fraction> sideways_movement_penalty;
        PAD(0x4);
        EndianType<float> minimum_target_range;
        EndianType<float> looking_time_modifier;
        PAD(0x4);
        EndianType<float> light_power_on_time;
        EndianType<float> light_power_off_time;
        TagDependency<EndianType> light_power_on_effect; // sound, effect
        TagDependency<EndianType> light_power_off_effect; // sound, effect
        EndianType<float> age_heat_recovery_penalty;
        EndianType<float> age_rate_of_fire_penalty;
        EndianType<Fraction> age_misfire_start;
        EndianType<Fraction> age_misfire_chance;
        PAD(0xC);
        TagDependency<EndianType> first_person_model; // gbxmodel
        TagDependency<EndianType> first_person_animations; // model_animations
        PAD(0x4);
        TagDependency<EndianType> hud_interface; // weapon_hud_interface
        TagDependency<EndianType> pickup_sound; // sound
        TagDependency<EndianType> zoom_in_sound; // sound
        TagDependency<EndianType> zoom_out_sound; // sound
        PAD(0xC);
        EndianType<float> active_camo_ding;
        EndianType<float> active_camo_regrowth_rate;
        PAD(0xC);
        PAD(0x2);
        EndianType<WeaponType> weapon_type;
        TagReflexive<EndianType, PredictedResource> more_predicted_resources;
        TagReflexive<EndianType, WeaponMagazine> magazines;
        TagReflexive<EndianType, WeaponTrigger> triggers;

        ENDIAN_TEMPLATE(NewType) operator Weapon<NewType>() const noexcept {
            Weapon<NewType> copy = {};
            COPY_ITEM_DATA
            COPY_THIS(flags);
            COPY_THIS(label);
            COPY_THIS(secondary_trigger_mode);
            COPY_THIS(maximum_alternate_shots_loaded);
            COPY_THIS(weapon_a_in);
            COPY_THIS(weapon_b_in);
            COPY_THIS(weapon_c_in);
            COPY_THIS(weapon_d_in);
            COPY_THIS(ready_time);
            COPY_THIS(ready_effect);
            COPY_THIS(heat_recovery_threshold);
            COPY_THIS(overheated_threshold);
            COPY_THIS(heat_detonation_threshold);
            COPY_THIS(heat_detonation_fraction);
            COPY_THIS(heat_loss_per_second);
            COPY_THIS(heat_illumination);
            COPY_THIS(overheated);
            COPY_THIS(detonation);
            COPY_THIS(player_melee_damage);
            COPY_THIS(player_melee_response);
            COPY_THIS(actor_firing_parameters);
            COPY_THIS(near_reticle_range);
            COPY_THIS(far_reticle_range);
            COPY_THIS(intersection_reticle_range);
            COPY_THIS(magnification_levels);
            COPY_THIS(magnification_range);
            COPY_THIS(autoaim_angle);
            COPY_THIS(autoaim_range);
            COPY_THIS(magnetism_angle);
            COPY_THIS(magnetism_range);
            COPY_THIS(deviation_angle);
            COPY_THIS(movement_penalized);
            COPY_THIS(forward_movement_penalty);
            COPY_THIS(sideways_movement_penalty);
            COPY_THIS(minimum_target_range);
            COPY_THIS(looking_time_modifier);
            COPY_THIS(light_power_on_time);
            COPY_THIS(light_power_off_time);
            COPY_THIS(light_power_on_effect);
            COPY_THIS(light_power_off_effect);
            COPY_THIS(age_heat_recovery_penalty);
            COPY_THIS(age_rate_of_fire_penalty);
            COPY_THIS(age_misfire_start);
            COPY_THIS(age_misfire_chance);
            COPY_THIS(first_person_model);
            COPY_THIS(first_person_animations);
            COPY_THIS(hud_interface);
            COPY_THIS(pickup_sound);
            COPY_THIS(zoom_in_sound);
            COPY_THIS(zoom_out_sound);
            COPY_THIS(active_camo_ding);
            COPY_THIS(active_camo_regrowth_rate);
            COPY_THIS(weapon_type);
            COPY_THIS(more_predicted_resources);
            COPY_THIS(magazines);
            COPY_THIS(triggers);
            return copy;
        }
    };
    static_assert(sizeof(Weapon<BigEndian>) == 0x508);

    enum WeaponJasonJones {
        WEAPON_JASON_JONES_NONE = 0,
        WEAPON_JASON_JONES_PISTOL_SINGLEPLAYER,
        WEAPON_JASON_JONES_PLASMA_RIFLE_SINGLEPLAYER
    };

    void compile_weapon_tag(CompiledTag &compiled, const std::byte *data, std::size_t size, WeaponJasonJones jason_jones);
}
