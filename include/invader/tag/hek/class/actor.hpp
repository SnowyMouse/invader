// SPDX-License-Identifier: GPL-3.0-only

#ifndef INVADER__TAG__HEK__CLASS__ACTOR_HPP
#define INVADER__TAG__HEK__CLASS__ACTOR_HPP

#include "../../compiled_tag.hpp"
#include "../../../hek/data_type.hpp"
#include "../header.hpp"

namespace Invader::HEK {
    enum ActorType : TagEnum {
        ACTOR_TYPE_ELITE,
        ACTOR_TYPE_JACKAL,
        ACTOR_TYPE_GRUNT,
        ACTOR_TYPE_HUNTER,
        ACTOR_TYPE_ENGINEER,
        ACTOR_TYPE_ASSASSIN,
        ACTOR_TYPE_PLAYER,
        ACTOR_TYPE_MARINE,
        ACTOR_TYPE_CREW,
        ACTOR_TYPE_COMBAT_FORM,
        ACTOR_TYPE_INFECTION_FORM,
        ACTOR_TYPE_CARRIER_FORM,
        ACTOR_TYPE_MONITOR,
        ACTOR_TYPE_SENTINEL,
        ACTOR_TYPE_NONE,
        ACTOR_TYPE_MOUNTED_WEAPON
    };

    enum ActorUnreachableDangerTrigger : TagEnum {
        ACTOR_UNREACHABLE_DANGER_TRIGGER_NEVER,
        ACTOR_UNREACHABLE_DANGER_TRIGGER_VISIBLE,
        ACTOR_UNREACHABLE_DANGER_TRIGGER_SHOOTING,
        ACTOR_UNREACHABLE_DANGER_TRIGGER_SHOOTING_NEAR_US,
        ACTOR_UNREACHABLE_DANGER_TRIGGER_DAMAGING_US,
        ACTOR_UNREACHABLE_DANGER_TRIGGER_UNUSED,
        ACTOR_UNREACHABLE_DANGER_TRIGGER_UNUSED_1,
        ACTOR_UNREACHABLE_DANGER_TRIGGER_UNUSED_2,
        ACTOR_UNREACHABLE_DANGER_TRIGGER_UNUSED_3,
        ACTOR_UNREACHABLE_DANGER_TRIGGER_UNUSED_4
    };

    enum ActorDefensiveCrouchType : TagEnum {
        ACTOR_DEFENSIVE_CROUCH_TYPE_NEVER,
        ACTOR_DEFENSIVE_CROUCH_TYPE_DANGER,
        ACTOR_DEFENSIVE_CROUCH_TYPE_LOW_SHIELDS,
        ACTOR_DEFENSIVE_CROUCH_TYPE_HIDE_BEHIND_SHIELD,
        ACTOR_DEFENSIVE_CROUCH_TYPE_ANY_TARGET,
        ACTOR_DEFENSIVE_CROUCH_TYPE_FLOOD_SHAMBLE
    };

    struct ActorFlags {
        std::uint32_t can_see_in_darkness : 1;
        std::uint32_t sneak_uncovering_target : 1;
        std::uint32_t sneak_uncovering_pursuit_position : 1;
        std::uint32_t unused : 1;
        std::uint32_t shoot_at_target_s_last_location : 1;
        std::uint32_t try_to_stay_still_when_crouched : 1;
        std::uint32_t crouch_when_not_in_combat : 1;
        std::uint32_t crouch_when_guarding : 1;
        std::uint32_t unused_1 : 1;
        std::uint32_t must_crouch_to_shoot : 1;
        std::uint32_t panic_when_surprised : 1;
        std::uint32_t always_charge_at_enemies : 1;
        std::uint32_t gets_in_vehicles_with_player : 1;
        std::uint32_t start_firing_before_aligned : 1;
        std::uint32_t standing_must_move_forward : 1;
        std::uint32_t crouching_must_move_forward : 1;
        std::uint32_t defensive_crouch_while_charging : 1;
        std::uint32_t use_stalking_behavior : 1;
        std::uint32_t stalking_freeze_if_exposed : 1;
        std::uint32_t always_berserk_in_attacking_mode : 1;
        std::uint32_t berserking_uses_panicked_movement : 1;
        std::uint32_t flying : 1;
        std::uint32_t panicked_by_unopposable_enemy : 1;
        std::uint32_t crouch_when_hiding_from_unopposable : 1;
        std::uint32_t always_charge_in_attacking_mode : 1;
        std::uint32_t dive_off_ledges : 1;
        std::uint32_t swarm : 1;
        std::uint32_t suicidal_melee_attack : 1;
        std::uint32_t cannot_move_while_crouching : 1;
        std::uint32_t fixed_crouch_facing : 1;
        std::uint32_t crouch_when_in_line_of_fire : 1;
        std::uint32_t avoid_friends_line_of_fire : 1;
    };

    struct ActorMoreFlags {
        std::uint32_t avoid_all_enemy_attack_vectors : 1;
        std::uint32_t must_stand_to_fire : 1;
        std::uint32_t must_stop_to_fire : 1;
        std::uint32_t disallow_vehicle_combat : 1;
        std::uint32_t pathfinding_ignores_danger : 1;
        std::uint32_t panic_in_groups : 1;
        std::uint32_t no_corpse_shooting : 1;
    };

    ENDIAN_TEMPLATE(EndianType) struct Actor {
        EndianType<ActorFlags> flags;
        EndianType<ActorMoreFlags> more_flags;
        PAD(0xC);
        EndianType<ActorType> type;
        PAD(0x2);

        EndianType<float> max_vision_distance;
        EndianType<float> central_vision_angle;
        EndianType<float> max_vision_angle;
        PAD(0x4);
        EndianType<float> peripheral_vision_angle;
        EndianType<float> peripheral_distance;
        PAD(0x4);
        Vector3D<EndianType> standing_gun_offset;
        Vector3D<EndianType> crouching_gun_offset;
        EndianType<float> hearing_distance;
        EndianType<float> notice_projectile_chance;
        EndianType<float> notice_vehicle_chance;
        PAD(0x8);
        EndianType<float> combat_perception_time;
        EndianType<float> guard_perception_time;
        EndianType<float> non_combat_perception_time;
        LittleEndian<float> inverse_combat_perception_time;
        LittleEndian<float> inverse_guard_perception_time;
        LittleEndian<float> inverse_non_combat_perception_time;
        PAD(0x8);

        EndianType<float> dive_into_cover_chance;
        EndianType<float> emerge_from_cover_chance;
        EndianType<float> dive_from_grenade_chance;
        EndianType<float> pathfinding_radius;
        EndianType<float> glass_ignorance_chance;
        EndianType<float> stationary_movement_dist;
        EndianType<float> free_flying_sidestep;
        EndianType<float> begin_moving_angle;
        PAD(0x4);

        Vector2D<EndianType> maximum_aiming_deviation;
        Vector2D<EndianType> maximum_looking_deviation;
        EndianType<float> noncombat_look_delta_l;
        EndianType<float> noncombat_look_delta_r;
        EndianType<float> combat_look_delta_l;
        EndianType<float> combat_look_delta_r;
        Vector2D<EndianType> idle_aiming_range;
        Vector2D<EndianType> idle_looking_range;
        Bounds<EndianType<float>> event_look_time_modifier;
        Bounds<EndianType<float>> noncombat_idle_facing;
        Bounds<EndianType<float>> noncombat_idle_aiming;
        Bounds<EndianType<float>> noncombat_idle_looking;
        Bounds<EndianType<float>> guard_idle_facing;
        Bounds<EndianType<float>> guard_idle_aiming;
        Bounds<EndianType<float>> guard_idle_looking;
        Bounds<EndianType<float>> combat_idle_facing;
        Bounds<EndianType<float>> combat_idle_aiming;
        Bounds<EndianType<float>> combat_idle_looking;
        PAD(0x8);
        Vector2D<LittleEndian> cosine_maximum_aiming_deviation;
        Vector2D<LittleEndian> cosine_maximum_looking_deviation;
        TagDependency<EndianType> do_not_use; // weapon
        PAD(0x10C);
        TagDependency<EndianType> do_not_use_1; // projectile

        EndianType<ActorUnreachableDangerTrigger> unreachable_danger_trigger;
        EndianType<ActorUnreachableDangerTrigger> vehicle_danger_trigger;
        EndianType<ActorUnreachableDangerTrigger> player_danger_trigger;
        PAD(0x2);
        Bounds<EndianType<float>> danger_trigger_time;
        EndianType<std::int16_t> friends_killed_trigger;
        EndianType<std::int16_t> friends_retreating_trigger;
        PAD(0xC);
        Bounds<EndianType<float>> retreat_time;
        PAD(0x8);

        Bounds<EndianType<float>> cowering_time;
        EndianType<float> friend_killed_panic_chance;
        EndianType<ActorType> leader_type;
        PAD(0x2);
        EndianType<float> leader_killed_panic_chance;
        EndianType<float> panic_damage_threshold;
        EndianType<float> surprise_distance;
        PAD(0x1C);

        Bounds<EndianType<float>> hide_behind_cover_time;
        EndianType<float> hide_target_not_visible_time;
        EndianType<float> hide_shield_fraction;
        EndianType<float> attack_shield_fraction;
        EndianType<float> pursue_shield_fraction;
        PAD(0x10);
        EndianType<ActorDefensiveCrouchType> defensive_crouch_type;
        PAD(0x2);
        EndianType<float> attacking_crouch_threshold;
        EndianType<float> defending_crouch_threshold;
        EndianType<float> min_stand_time;
        EndianType<float> min_crouch_time;
        EndianType<float> defending_hide_time_modifier;
        EndianType<float> attacking_evasion_threshold;
        EndianType<float> defending_evasion_threshold;
        EndianType<float> evasion_seek_cover_chance;
        EndianType<float> evasion_delay_time;
        EndianType<float> max_seek_cover_distance;
        EndianType<float> cover_damage_threshold;
        EndianType<float> stalking_discovery_time;
        EndianType<float> stalking_max_distance;
        EndianType<float> stationary_facing_angle;
        EndianType<float> change_facing_stand_time;
        PAD(0x4);

        Bounds<EndianType<float>> uncover_delay_time;
        Bounds<EndianType<float>> target_search_time;
        Bounds<EndianType<float>> pursuit_position_time;
        EndianType<std::uint16_t> num_positions;
        EndianType<std::uint16_t> num_positions_1;
        PAD(0x20);

        EndianType<float> melee_attack_delay;
        EndianType<float> melee_fudge_factor;
        EndianType<float> melee_charge_time;
        Bounds<EndianType<float>> melee_leap_range;
        EndianType<float> melee_leap_velocity;
        EndianType<float> melee_leap_chance;
        EndianType<float> melee_leap_ballistic;
        EndianType<float> berserk_damage_amount;
        EndianType<float> berserk_damage_threshold;
        EndianType<float> berserk_proximity;
        EndianType<float> suicide_sensing_dist;
        EndianType<float> berserk_grenade_chance;
        PAD(0xC);

        Bounds<EndianType<float>> guard_position_time;
        Bounds<EndianType<float>> combat_position_time;
        EndianType<float> old_position_avoid_dist;
        EndianType<float> friend_avoid_dist;
        PAD(0x28);

        Bounds<EndianType<float>> noncombat_idle_speech_time;
        Bounds<EndianType<float>> combat_idle_speech_time;
        PAD(0x30);
        PAD(0x80);
        TagDependency<EndianType> do_not_use_2; // actor
        PAD(0x30);

        ENDIAN_TEMPLATE(NewType) operator Actor<NewType>() const noexcept {
            Actor<NewType> copy = {};
            COPY_THIS(flags);
            COPY_THIS(more_flags);
            COPY_THIS(type);
            COPY_THIS(max_vision_distance);
            COPY_THIS(central_vision_angle);
            COPY_THIS(max_vision_angle);
            COPY_THIS(peripheral_vision_angle);
            COPY_THIS(peripheral_distance);
            COPY_THIS(standing_gun_offset);
            COPY_THIS(crouching_gun_offset);
            COPY_THIS(hearing_distance);
            COPY_THIS(notice_projectile_chance);
            COPY_THIS(notice_vehicle_chance);
            COPY_THIS(combat_perception_time);
            COPY_THIS(guard_perception_time);
            COPY_THIS(non_combat_perception_time);
            COPY_THIS(dive_into_cover_chance);
            COPY_THIS(emerge_from_cover_chance);
            COPY_THIS(dive_from_grenade_chance);
            COPY_THIS(pathfinding_radius);
            COPY_THIS(glass_ignorance_chance);
            COPY_THIS(stationary_movement_dist);
            COPY_THIS(free_flying_sidestep);
            COPY_THIS(begin_moving_angle);
            COPY_THIS(maximum_aiming_deviation);
            COPY_THIS(maximum_looking_deviation);
            COPY_THIS(noncombat_look_delta_l);
            COPY_THIS(noncombat_look_delta_r);
            COPY_THIS(combat_look_delta_l);
            COPY_THIS(combat_look_delta_r);
            COPY_THIS(idle_aiming_range);
            COPY_THIS(idle_looking_range);
            COPY_THIS(event_look_time_modifier);
            COPY_THIS(noncombat_idle_facing);
            COPY_THIS(noncombat_idle_aiming);
            COPY_THIS(noncombat_idle_looking);
            COPY_THIS(guard_idle_facing);
            COPY_THIS(guard_idle_aiming);
            COPY_THIS(guard_idle_looking);
            COPY_THIS(combat_idle_facing);
            COPY_THIS(combat_idle_aiming);
            COPY_THIS(combat_idle_looking);
            COPY_THIS(do_not_use);
            COPY_THIS(do_not_use_1);
            COPY_THIS(unreachable_danger_trigger);
            COPY_THIS(vehicle_danger_trigger);
            COPY_THIS(player_danger_trigger);
            COPY_THIS(danger_trigger_time);
            COPY_THIS(friends_killed_trigger);
            COPY_THIS(friends_retreating_trigger);
            COPY_THIS(retreat_time);
            COPY_THIS(cowering_time);
            COPY_THIS(friend_killed_panic_chance);
            COPY_THIS(leader_type);
            COPY_THIS(leader_killed_panic_chance);
            COPY_THIS(panic_damage_threshold);
            COPY_THIS(surprise_distance);
            COPY_THIS(hide_behind_cover_time);
            COPY_THIS(hide_target_not_visible_time);
            COPY_THIS(hide_shield_fraction);
            COPY_THIS(attack_shield_fraction);
            COPY_THIS(pursue_shield_fraction);
            COPY_THIS(defensive_crouch_type);
            COPY_THIS(attacking_crouch_threshold);
            COPY_THIS(defending_crouch_threshold);
            COPY_THIS(min_stand_time);
            COPY_THIS(min_crouch_time);
            COPY_THIS(defending_hide_time_modifier);
            COPY_THIS(attacking_evasion_threshold);
            COPY_THIS(defending_evasion_threshold);
            COPY_THIS(evasion_seek_cover_chance);
            COPY_THIS(evasion_delay_time);
            COPY_THIS(max_seek_cover_distance);
            COPY_THIS(cover_damage_threshold);
            COPY_THIS(stalking_discovery_time);
            COPY_THIS(stalking_max_distance);
            COPY_THIS(stationary_facing_angle);
            COPY_THIS(change_facing_stand_time);
            COPY_THIS(uncover_delay_time);
            COPY_THIS(target_search_time);
            COPY_THIS(pursuit_position_time);
            COPY_THIS(num_positions);
            COPY_THIS(num_positions_1);
            COPY_THIS(melee_attack_delay);
            COPY_THIS(melee_fudge_factor);
            COPY_THIS(melee_charge_time);
            COPY_THIS(melee_leap_range);
            COPY_THIS(melee_leap_velocity);
            COPY_THIS(melee_leap_chance);
            COPY_THIS(melee_leap_ballistic);
            COPY_THIS(berserk_damage_amount);
            COPY_THIS(berserk_damage_threshold);
            COPY_THIS(berserk_proximity);
            COPY_THIS(suicide_sensing_dist);
            COPY_THIS(berserk_grenade_chance);
            COPY_THIS(guard_position_time);
            COPY_THIS(combat_position_time);
            COPY_THIS(old_position_avoid_dist);
            COPY_THIS(friend_avoid_dist);
            COPY_THIS(noncombat_idle_speech_time);
            COPY_THIS(combat_idle_speech_time);
            COPY_THIS(do_not_use_2);
            return copy;
        }
    };
    static_assert(sizeof(Actor<BigEndian>) == 0x4F8);

    void compile_actor_tag(CompiledTag &compiled, const std::byte *data, std::size_t size);
}
#endif
