// SPDX-License-Identifier: GPL-3.0-only

#ifndef INVADER__TAG__HEK__CLASS__ACTOR_VARIANT_HPP
#define INVADER__TAG__HEK__CLASS__ACTOR_VARIANT_HPP

#include "../../compiled_tag.hpp"
#include "../../../hek/data_type.hpp"
#include "../header.hpp"

namespace Invader::HEK {
    enum ActorVariantMovementType : TagEnum {
        ACTOR_VARIANT_MOVEMENT_TYPE_ALWAYS_RUN,
        ACTOR_VARIANT_MOVEMENT_TYPE_ALWAYS_CROUCH,
        ACTOR_VARIANT_MOVEMENT_TYPE_SWITCH_TYPES
    };

    enum ActorVariantSpecialFireMode : TagEnum {
        ACTOR_VARIANT_SPECIAL_FIRE_MODE_NONE,
        ACTOR_VARIANT_SPECIAL_FIRE_MODE_OVERCHARGE,
        ACTOR_VARIANT_SPECIAL_FIRE_MODE_SECONDARY_TRIGGER
    };

    enum ActorVariantSpecialFireSituation : TagEnum {
        ACTOR_VARIANT_SPECIAL_FIRE_SITUATION_NEVER,
        ACTOR_VARIANT_SPECIAL_FIRE_SITUATION_ENEMY_VISIBLE,
        ACTOR_VARIANT_SPECIAL_FIRE_SITUATION_ENEMY_OUT_OF_SIGHT,
        ACTOR_VARIANT_SPECIAL_FIRE_SITUATION_STRAFING
    };

    enum ActorVariantGrenadeType : TagEnum {
        ACTOR_VARIANT_GRENADE_TYPE_HUMAN_FRAGMENTATION,
        ACTOR_VARIANT_GRENADE_TYPE_COVENANT_PLASMA,
        ACTOR_VARIANT_GRENADE_TYPE_CUSTOM_2,
        ACTOR_VARIANT_GRENADE_TYPE_CUSTOM_3
    };

    enum ActorVariantTrajectoryType : TagEnum {
        ACTOR_VARIANT_TRAJECTORY_TYPE_TOSS,
        ACTOR_VARIANT_TRAJECTORY_TYPE_LOB,
        ACTOR_VARIANT_TRAJECTORY_TYPE_BOUNCE
    };

    enum ActorVariantGrenadeStimulus : TagEnum {
        ACTOR_VARIANT_GRENADE_STIMULUS_NEVER,
        ACTOR_VARIANT_GRENADE_STIMULUS_VISIBLE_TARGET,
        ACTOR_VARIANT_GRENADE_STIMULUS_SEEK_COVER
    };

    ENDIAN_TEMPLATE(EndianType) struct ActorVariantChangeColors {
        ColorRGB<EndianType> color_lower_bound;
        ColorRGB<EndianType> color_upper_bound;
        PAD(0x8);

        ENDIAN_TEMPLATE(NewType) operator ActorVariantChangeColors<NewType>() const noexcept {
            ActorVariantChangeColors<NewType> copy = {};
            COPY_THIS(color_lower_bound);
            COPY_THIS(color_upper_bound);
            return copy;
        }
    };
    static_assert(sizeof(ActorVariantChangeColors<BigEndian>) == 0x20);

    struct ActorVariantFlags {
        std::uint32_t can_shoot_while_flying : 1;
        std::uint32_t interpolate_color_in_hsv : 1;
        std::uint32_t has_unlimited_grenades : 1;
        std::uint32_t moveswitch_stay_w_friends : 1;
        std::uint32_t active_camouflage : 1;
        std::uint32_t super_active_camouflage : 1;
        std::uint32_t cannot_use_ranged_weapons : 1;
        std::uint32_t prefer_passenger_seat : 1;
    };

    ENDIAN_TEMPLATE(EndianType) struct ActorVariant {
        EndianType<ActorVariantFlags> flags;
        TagDependency<EndianType> actor_definition; // actor
        TagDependency<EndianType> unit; // unit
        TagDependency<EndianType> major_variant; // actor_variant
        PAD(0x18);

        EndianType<ActorVariantMovementType> movement_type;
        PAD(0x2);
        EndianType<float> initial_crouch_chance;
        Bounds<EndianType<float>> crouch_time;
        Bounds<EndianType<float>> run_time;

        TagDependency<EndianType> weapon; // weapon
        EndianType<float> maximum_firing_distance;
        EndianType<float> rate_of_fire;
        EndianType<Angle> projectile_error;
        Bounds<EndianType<float>> first_burst_delay_time;
        EndianType<float> new_target_firing_pattern_time;
        EndianType<float> surprise_delay_time;
        EndianType<float> surprise_fire_wildly_time;
        EndianType<float> death_fire_wildly_chance;
        EndianType<float> death_fire_wildly_time;
        Bounds<EndianType<float>> desired_combat_range;
        Vector3D<EndianType> custom_stand_gun_offset;
        Vector3D<EndianType> custom_crouch_gun_offset;
        EndianType<float> target_tracking;
        EndianType<float> target_leading;
        EndianType<float> weapon_damage_modifier;
        EndianType<float> damage_per_second;

        EndianType<float> burst_origin_radius;
        EndianType<Angle> burst_origin_angle;
        Bounds<EndianType<float>> burst_return_length;
        EndianType<Angle> burst_return_angle;
        Bounds<EndianType<float>> burst_duration;
        Bounds<EndianType<float>> burst_separation;
        EndianType<Angle> burst_angular_velocity;
        PAD(0x4);
        EndianType<float> special_damage_modifier;
        EndianType<Angle> special_projectile_error;

        EndianType<float> new_target_burst_duration;
        EndianType<float> new_target_burst_separation;
        EndianType<float> new_target_rate_of_fire;
        EndianType<float> new_target_projectile_error;
        PAD(0x8);
        EndianType<float> moving_burst_duration;
        EndianType<float> moving_burst_separation;
        EndianType<float> moving_rate_of_fire;
        EndianType<float> moving_projectile_error;
        PAD(0x8);
        EndianType<float> berserk_burst_duration;
        EndianType<float> berserk_burst_separation;
        EndianType<float> berserk_rate_of_fire;
        EndianType<float> berserk_projectile_error;
        PAD(0x8);

        EndianType<float> super_ballistic_range;
        EndianType<float> bombardment_range;
        EndianType<float> modified_vision_range;
        EndianType<ActorVariantSpecialFireMode> special_fire_mode;
        EndianType<ActorVariantSpecialFireSituation> special_fire_situation;
        EndianType<float> special_fire_chance;
        EndianType<float> special_fire_delay;

        EndianType<float> melee_range;
        EndianType<float> melee_abort_range;
        Bounds<EndianType<float>> berserk_firing_ranges;
        EndianType<float> berserk_melee_range;
        EndianType<float> berserk_melee_abort_range;
        PAD(0x8);

        EndianType<ActorVariantGrenadeType> grenade_type;
        EndianType<ActorVariantTrajectoryType> trajectory_type;
        EndianType<ActorVariantGrenadeStimulus> grenade_stimulus;
        EndianType<std::int16_t> minimum_enemy_count;
        EndianType<float> enemy_radius;
        PAD(0x4);
        EndianType<float> grenade_velocity;
        Bounds<EndianType<float>> grenade_ranges;
        EndianType<float> collateral_damage_radius;
        EndianType<Fraction> grenade_chance;
        EndianType<float> grenade_check_time;
        EndianType<float> encounter_grenade_timeout;
        PAD(0x14);

        TagDependency<EndianType> equipment; // equipment
        Bounds<EndianType<std::int16_t>> grenade_count;
        EndianType<float> dont_drop_grenades_chance;
        Bounds<EndianType<float>> drop_weapon_loaded;
        Bounds<EndianType<std::int16_t>> drop_weapon_ammo;
        PAD(0xC);
        PAD(0x10);

        EndianType<float> body_vitality;
        EndianType<float> shield_vitality;
        EndianType<float> shield_sapping_radius;
        EndianType<Index> forced_shader_permutation;
        PAD(0x2);
        PAD(0x10);
        PAD(0xC);

        TagReflexive<EndianType, ActorVariantChangeColors> change_colors;

        ENDIAN_TEMPLATE(NewType) operator ActorVariant<NewType>() const noexcept {
            ActorVariant<NewType> copy = {};
            COPY_THIS(flags);
            COPY_THIS(actor_definition);
            COPY_THIS(unit);
            COPY_THIS(major_variant);
            COPY_THIS(movement_type);
            COPY_THIS(initial_crouch_chance);
            COPY_THIS(crouch_time);
            COPY_THIS(run_time);
            COPY_THIS(weapon);
            COPY_THIS(maximum_firing_distance);
            COPY_THIS(rate_of_fire);
            COPY_THIS(projectile_error);
            COPY_THIS(first_burst_delay_time);
            COPY_THIS(new_target_firing_pattern_time);
            COPY_THIS(surprise_delay_time);
            COPY_THIS(surprise_fire_wildly_time);
            COPY_THIS(death_fire_wildly_chance);
            COPY_THIS(death_fire_wildly_time);
            COPY_THIS(desired_combat_range);
            COPY_THIS(custom_stand_gun_offset);
            COPY_THIS(custom_crouch_gun_offset);
            COPY_THIS(target_tracking);
            COPY_THIS(target_leading);
            COPY_THIS(weapon_damage_modifier);
            COPY_THIS(damage_per_second);
            COPY_THIS(burst_origin_radius);
            COPY_THIS(burst_origin_angle);
            COPY_THIS(burst_return_length);
            COPY_THIS(burst_return_angle);
            COPY_THIS(burst_duration);
            COPY_THIS(burst_separation);
            COPY_THIS(burst_angular_velocity);
            COPY_THIS(special_damage_modifier);
            COPY_THIS(special_projectile_error);
            COPY_THIS(new_target_burst_duration);
            COPY_THIS(new_target_burst_separation);
            COPY_THIS(new_target_rate_of_fire);
            COPY_THIS(new_target_projectile_error);
            COPY_THIS(moving_burst_duration);
            COPY_THIS(moving_burst_separation);
            COPY_THIS(moving_rate_of_fire);
            COPY_THIS(moving_projectile_error);
            COPY_THIS(berserk_burst_duration);
            COPY_THIS(berserk_burst_separation);
            COPY_THIS(berserk_rate_of_fire);
            COPY_THIS(berserk_projectile_error);
            COPY_THIS(super_ballistic_range);
            COPY_THIS(bombardment_range);
            COPY_THIS(modified_vision_range);
            COPY_THIS(special_fire_mode);
            COPY_THIS(special_fire_situation);
            COPY_THIS(special_fire_chance);
            COPY_THIS(special_fire_delay);
            COPY_THIS(melee_range);
            COPY_THIS(melee_abort_range);
            COPY_THIS(berserk_firing_ranges);
            COPY_THIS(berserk_melee_range);
            COPY_THIS(berserk_melee_abort_range);
            COPY_THIS(grenade_type);
            COPY_THIS(trajectory_type);
            COPY_THIS(grenade_stimulus);
            COPY_THIS(minimum_enemy_count);
            COPY_THIS(enemy_radius);
            COPY_THIS(grenade_velocity);
            COPY_THIS(grenade_ranges);
            COPY_THIS(collateral_damage_radius);
            COPY_THIS(grenade_chance);
            COPY_THIS(grenade_check_time);
            COPY_THIS(encounter_grenade_timeout);
            COPY_THIS(equipment);
            COPY_THIS(grenade_count);
            COPY_THIS(dont_drop_grenades_chance);
            COPY_THIS(drop_weapon_loaded);
            COPY_THIS(drop_weapon_ammo);
            COPY_THIS(body_vitality);
            COPY_THIS(shield_vitality);
            COPY_THIS(shield_sapping_radius);
            COPY_THIS(forced_shader_permutation);
            COPY_THIS(change_colors);
            return copy;
        }
    };
    static_assert(sizeof(ActorVariant<BigEndian>) == 0x238);

    void compile_actor_variant_tag(CompiledTag &compiled, const std::byte *data, std::size_t size);
}
#endif
