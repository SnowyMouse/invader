// SPDX-License-Identifier: GPL-3.0-only

#ifndef INVADER__TAG__HEK__CLASS__UNIT_HPP
#define INVADER__TAG__HEK__CLASS__UNIT_HPP

#include "object.hpp"

namespace Invader::HEK {
    enum UnitFunctionIn : TagEnum {
        UNIT_FUNCTION_IN_NONE,
        UNIT_FUNCTION_IN_DRIVER_SEAT_POWER,
        UNIT_FUNCTION_IN_GUNNER_SEAT_POWER,
        UNIT_FUNCTION_IN_AIMING_CHANGE,
        UNIT_FUNCTION_IN_MOUTH_APERTURE,
        UNIT_FUNCTION_IN_INTEGRATED_LIGHT_POWER,
        UNIT_FUNCTION_IN_CAN_BLINK,
        UNIT_FUNCTION_IN_SHIELD_SAPPING
    };

    enum UnitDefaultTeam : TagEnum {
        UNIT_DEFAULT_TEAM_NONE,
        UNIT_DEFAULT_TEAM_PLAYER,
        UNIT_DEFAULT_TEAM_HUMAN,
        UNIT_DEFAULT_TEAM_COVENANT,
        UNIT_DEFAULT_TEAM_FLOOD,
        UNIT_DEFAULT_TEAM_SENTINEL,
        UNIT_DEFAULT_TEAM_UNUSED_6,
        UNIT_DEFAULT_TEAM_UNUSED_7,
        UNIT_DEFAULT_TEAM_UNUSED_8,
        UNIT_DEFAULT_TEAM_UNUSED_9
    };

    enum UnitConstantSoundVolume : TagEnum {
        UNIT_CONSTANT_SOUND_VOLUME_SILENT,
        UNIT_CONSTANT_SOUND_VOLUME_MEDIUM,
        UNIT_CONSTANT_SOUND_VOLUME_LOUD,
        UNIT_CONSTANT_SOUND_VOLUME_SHOUT,
        UNIT_CONSTANT_SOUND_VOLUME_QUIET
    };

    enum UnitMotionSensorBlipSize : TagEnum {
        UNIT_MOTION_SENSOR_BLIP_SIZE_MEDIUM,
        UNIT_MOTION_SENSOR_BLIP_SIZE_SMALL,
        UNIT_MOTION_SENSOR_BLIP_SIZE_LARGE
    };

    enum UnitGrenadeType : TagEnum {
        UNIT_GRENADE_TYPE_HUMAN_FRAGMENTATION,
        UNIT_GRENADE_TYPE_COVENANT_PLASMA
    };

    ENDIAN_TEMPLATE(EndianType) struct UnitPoweredSeat {
        PAD(0x4);
        EndianType<float> driver_powerup_time;
        EndianType<float> driver_powerdown_time;
        PAD(0x38);

        ENDIAN_TEMPLATE(NewType) operator UnitPoweredSeat<NewType>() const noexcept {
            UnitPoweredSeat<NewType> copy = {};
            COPY_THIS(driver_powerup_time);
            COPY_THIS(driver_powerdown_time);
            return copy;
        }
    };
    static_assert(sizeof(UnitPoweredSeat<BigEndian>) == 0x44);

    struct UnitFlags {
        std::uint32_t circular_aiming : 1;
        std::uint32_t destroyed_after_dying : 1;
        std::uint32_t half_speed_interpolation : 1;
        std::uint32_t fires_from_camera : 1;
        std::uint32_t entrance_inside_bounding_sphere : 1;
        std::uint32_t unused : 1;
        std::uint32_t causes_passenger_dialogue : 1;
        std::uint32_t resists_pings : 1;
        std::uint32_t melee_attack_is_fatal : 1;
        std::uint32_t don_t_reface_during_pings : 1;
        std::uint32_t has_no_aiming : 1;
        std::uint32_t simple_creature : 1;
        std::uint32_t impact_melee_attaches_to_unit : 1;
        std::uint32_t impact_melee_dies_on_shields : 1;
        std::uint32_t cannot_open_doors_automatically : 1;
        std::uint32_t melee_attackers_cannot_attach : 1;
        std::uint32_t not_instantly_killed_by_melee : 1;
        std::uint32_t shield_sapping : 1;
        std::uint32_t runs_around_flaming : 1;
        std::uint32_t inconsequential : 1;
        std::uint32_t special_cinematic_unit : 1;
        std::uint32_t ignored_by_autoaiming : 1;
        std::uint32_t shields_fry_infection_forms : 1;
        std::uint32_t integrated_light_cntrls_weapon : 1;
        std::uint32_t integrated_light_lasts_forever : 1;
    };

    SINGLE_DEPENDENCY_PADDED_STRUCT(UnitCameraTrack, track, 0xC); // camera_track
    SINGLE_DEPENDENCY_PADDED_STRUCT(UnitUnitHudInterface, hud, 0x20); // unit_hud_interface

    struct UnitSeatFlags {
        std::uint32_t invisible : 1;
        std::uint32_t locked : 1;
        std::uint32_t driver : 1;
        std::uint32_t gunner : 1;
        std::uint32_t third_person_camera : 1;
        std::uint32_t allows_weapons : 1;
        std::uint32_t third_person_on_enter : 1;
        std::uint32_t first_person_camera_slaved_to_gun : 1;
        std::uint32_t allow_vehicle_communication_animations : 1;
        std::uint32_t not_valid_without_driver : 1;
        std::uint32_t allow_ai_noncombatants : 1;
        std::uint32_t reserved : 1;
        std::uint32_t reserved_1 : 1;
        std::uint32_t reserved_2 : 1;
        std::uint32_t reserved_3 : 1;
        std::uint32_t reserved_4 : 1;
        std::uint32_t reserved_5 : 1;
        std::uint32_t reserved_6 : 1;
        std::uint32_t reserved_7 : 1;
        std::uint32_t reserved_8 : 1;
        std::uint32_t allows_melee : 1;
    };

    ENDIAN_TEMPLATE(EndianType) struct UnitSeat {
        EndianType<UnitSeatFlags> flags;
        TagString label;
        TagString marker_name;
        PAD(0x20);
        Vector3D<EndianType> acceleration_scale;
        PAD(0xC);
        EndianType<float> yaw_rate;
        EndianType<float> pitch_rate;
        TagString camera_marker_name;
        TagString camera_submerged_marker_name;
        EndianType<Angle> pitch_auto_level;
        Bounds<EndianType<Angle>> pitch_range;
        TagReflexive<EndianType, UnitCameraTrack> camera_tracks;
        TagReflexive<EndianType, UnitUnitHudInterface> unit_hud_interface;
        PAD(0x4);
        EndianType<std::int16_t> hud_text_message_index;
        PAD(0x2);
        EndianType<Angle> yaw_minimum;
        EndianType<Angle> yaw_maximum;
        TagDependency<EndianType> built_in_gunner; // actor_variant
        PAD(0x14);

        ENDIAN_TEMPLATE(NewType) operator UnitSeat<NewType>() const noexcept {
            UnitSeat<NewType> copy = {};
            COPY_THIS(flags);
            COPY_THIS(label);
            COPY_THIS(marker_name);
            COPY_THIS(acceleration_scale);
            COPY_THIS(yaw_rate);
            COPY_THIS(pitch_rate);
            COPY_THIS(camera_marker_name);
            COPY_THIS(camera_submerged_marker_name);
            COPY_THIS(pitch_auto_level);
            COPY_THIS(pitch_range);
            COPY_THIS(camera_tracks);
            COPY_THIS(unit_hud_interface);
            COPY_THIS(hud_text_message_index);
            COPY_THIS(yaw_minimum);
            COPY_THIS(yaw_maximum);
            COPY_THIS(built_in_gunner);
            return copy;
        }
    };
    static_assert(sizeof(UnitSeat<BigEndian>) == 0x11C);
    SINGLE_DEPENDENCY_PADDED_STRUCT(UnitWeapon, weapon, 0x14); // weapon

    ENDIAN_TEMPLATE(EndianType) struct UnitDialogueVariant {
        EndianType<std::int16_t> variant_number;
        PAD(0x2);
        PAD(0x4);
        TagDependency<EndianType> dialogue; // unit_dialogue

        ENDIAN_TEMPLATE(NewType) operator UnitDialogueVariant<NewType>() const noexcept {
            UnitDialogueVariant<NewType> copy = {};
            COPY_THIS(variant_number);
            COPY_THIS(dialogue);
            return copy;
        }
    };
    static_assert(sizeof(UnitDialogueVariant<BigEndian>) == 0x18);

    ENDIAN_TEMPLATE(EndianType) struct Unit : Object<EndianType> {
        EndianType<UnitFlags> unit_flags;
        EndianType<UnitDefaultTeam> default_team;
        EndianType<UnitConstantSoundVolume> constant_sound_volume;
        EndianType<float> rider_damage_fraction;
        TagDependency<EndianType> integrated_light_toggle; // effect
        EndianType<UnitFunctionIn> unit_a_in;
        EndianType<UnitFunctionIn> unit_b_in;
        EndianType<UnitFunctionIn> unit_c_in;
        EndianType<UnitFunctionIn> unit_d_in;
        EndianType<Angle> camera_field_of_view;
        EndianType<float> camera_stiffness;
        TagString camera_marker_name;
        TagString camera_submerged_marker_name;
        EndianType<Angle> pitch_auto_level;
        Bounds<EndianType<Angle>> pitch_range;
        TagReflexive<EndianType, UnitCameraTrack> camera_tracks;
        Vector3D<EndianType> seat_acceleration_scale;
        PAD(0xC);
        EndianType<float> soft_ping_threshold;
        EndianType<float> soft_ping_interrupt_time;
        EndianType<float> hard_ping_threshold;
        EndianType<float> hard_ping_interrupt_time;
        EndianType<float> hard_death_threshold;
        EndianType<float> feign_death_threshold;
        EndianType<float> feign_death_time;
        EndianType<float> distance_of_evade_anim;
        EndianType<float> distance_of_dive_anim;
        PAD(0x4);
        EndianType<float> stunned_movement_threshold;
        EndianType<float> feign_death_chance;
        EndianType<float> feign_repeat_chance;
        TagDependency<EndianType> spawned_actor; // actor_variant
        Bounds<EndianType<std::int16_t>> spawned_actor_count;
        EndianType<float> spawned_velocity;
        EndianType<Angle> aiming_velocity_maximum;
        EndianType<Angle> aiming_acceleration_maximum;
        EndianType<Fraction> casual_aiming_modifier;
        EndianType<Angle> looking_velocity_maximum;
        EndianType<Angle> looking_acceleration_maximum;
        PAD(0x8);
        EndianType<float> ai_vehicle_radius;
        EndianType<float> ai_danger_radius;
        TagDependency<EndianType> melee_damage; // damage_effect
        EndianType<UnitMotionSensorBlipSize> motion_sensor_blip_size;
        PAD(0x2);
        PAD(0xC);
        TagReflexive<EndianType, UnitUnitHudInterface> new_hud_interfaces;
        TagReflexive<EndianType, UnitDialogueVariant> dialogue_variants;
        EndianType<float> grenade_velocity;
        EndianType<UnitGrenadeType> grenade_type;
        EndianType<std::int16_t> grenade_count;
        EndianType<std::int16_t> unknown_shorts[2];
        TagReflexive<EndianType, UnitPoweredSeat> powered_seats;
        TagReflexive<EndianType, UnitWeapon> weapons;
        TagReflexive<EndianType, UnitSeat> seats;
    };
    static_assert(sizeof(Unit<BigEndian>) == 0x2F0);

    #define COPY_UNIT_DATA COPY_OBJECT_DATA \
                           COPY_THIS(unit_flags); \
                           COPY_THIS(default_team); \
                           COPY_THIS(constant_sound_volume); \
                           COPY_THIS(rider_damage_fraction); \
                           COPY_THIS(integrated_light_toggle); \
                           COPY_THIS(unit_a_in); \
                           COPY_THIS(unit_b_in); \
                           COPY_THIS(unit_c_in); \
                           COPY_THIS(unit_d_in); \
                           COPY_THIS(camera_field_of_view); \
                           COPY_THIS(camera_stiffness); \
                           COPY_THIS(camera_marker_name); \
                           COPY_THIS(camera_submerged_marker_name); \
                           COPY_THIS(pitch_auto_level); \
                           COPY_THIS(pitch_range); \
                           COPY_THIS(camera_tracks); \
                           COPY_THIS(seat_acceleration_scale); \
                           COPY_THIS(soft_ping_threshold); \
                           COPY_THIS(soft_ping_interrupt_time); \
                           COPY_THIS(hard_ping_threshold); \
                           COPY_THIS(hard_ping_interrupt_time); \
                           COPY_THIS(hard_death_threshold); \
                           COPY_THIS(feign_death_threshold); \
                           COPY_THIS(feign_death_time); \
                           COPY_THIS(distance_of_evade_anim); \
                           COPY_THIS(distance_of_dive_anim); \
                           COPY_THIS(stunned_movement_threshold); \
                           COPY_THIS(feign_death_chance); \
                           COPY_THIS(feign_repeat_chance); \
                           COPY_THIS(spawned_actor); \
                           COPY_THIS(spawned_actor_count); \
                           COPY_THIS(spawned_velocity); \
                           COPY_THIS(aiming_velocity_maximum); \
                           COPY_THIS(aiming_acceleration_maximum); \
                           COPY_THIS(casual_aiming_modifier); \
                           COPY_THIS(looking_velocity_maximum); \
                           COPY_THIS(looking_acceleration_maximum); \
                           COPY_THIS(ai_vehicle_radius); \
                           COPY_THIS(ai_danger_radius); \
                           COPY_THIS(melee_damage); \
                           COPY_THIS(motion_sensor_blip_size); \
                           COPY_THIS(new_hud_interfaces); \
                           COPY_THIS(dialogue_variants); \
                           COPY_THIS(grenade_velocity); \
                           COPY_THIS(grenade_type); \
                           COPY_THIS(grenade_count); \
                           COPY_THIS_ARRAY(unknown_shorts); \
                           COPY_THIS(powered_seats); \
                           COPY_THIS(weapons); \
                           COPY_THIS(seats);

    #define COMPILE_UNIT_DATA COMPILE_OBJECT_DATA \
                              ADD_DEPENDENCY_ADJUST_SIZES(tag.integrated_light_toggle); \
                              ADD_BASIC_DEPENDENCY_REFLEXIVE(tag.camera_tracks, track); \
                              ADD_DEPENDENCY_ADJUST_SIZES(tag.spawned_actor); \
                              ADD_DEPENDENCY_ADJUST_SIZES(tag.melee_damage); \
                              ADD_BASIC_DEPENDENCY_REFLEXIVE(tag.new_hud_interfaces, hud); \
                              ADD_BASIC_DEPENDENCY_REFLEXIVE(tag.dialogue_variants, dialogue); \
                              ADD_REFLEXIVE(tag.powered_seats); \
                              ADD_BASIC_DEPENDENCY_REFLEXIVE(tag.weapons, weapon); \
                              ADD_REFLEXIVE_START(tag.seats) { \
                                  ADD_BASIC_DEPENDENCY_REFLEXIVE(reflexive.camera_tracks, track); \
                                  ADD_BASIC_DEPENDENCY_REFLEXIVE(reflexive.unit_hud_interface, hud); \
                                  ADD_DEPENDENCY_ADJUST_SIZES(reflexive.built_in_gunner); \
                              } ADD_REFLEXIVE_END
}
#endif
