// SPDX-License-Identifier: GPL-3.0-only

#ifndef INVADER__TAG__HEK__CLASS__GLOBALS_HPP
#define INVADER__TAG__HEK__CLASS__GLOBALS_HPP

#include "../../compiled_tag.hpp"
#include "../../../hek/data_type.hpp"
#include "../../../hek/map.hpp"
#include "../header.hpp"

namespace Invader::HEK {
    SINGLE_DEPENDENCY_STRUCT(GlobalsSound, sound); // sound
    SINGLE_DEPENDENCY_STRUCT(GlobalsCamera, default_unit_camera_track); // camera_track

    ENDIAN_TEMPLATE(EndianType) struct GlobalsLookFunction {
        EndianType<float> scale;

        ENDIAN_TEMPLATE(NewType) operator GlobalsLookFunction<NewType>() const noexcept {
            GlobalsLookFunction<NewType> copy = {};
            COPY_THIS(scale);
            return copy;
        }
    };
    static_assert(sizeof(GlobalsLookFunction<BigEndian>) == 0x4);

    ENDIAN_TEMPLATE(EndianType) struct GlobalsPlayerControl {
        EndianType<float> magnetism_friction;
        EndianType<float> magnetism_adhesion;
        EndianType<float> inconsequential_target_scale;
        PAD(0x34);
        EndianType<float> look_acceleration_time;
        EndianType<float> look_acceleration_scale;
        EndianType<float> look_peg_threshold;
        EndianType<float> look_default_pitch_rate;
        EndianType<float> look_default_yaw_rate;
        EndianType<float> look_autolevelling_scale;
        PAD(0x14);
        EndianType<std::uint16_t> minimum_weapon_swap_ticks;
        EndianType<std::uint16_t> minimum_autolevelling_ticks;
        EndianType<float> minimum_angle_for_vehicle_flipping;
        TagReflexive<EndianType, GlobalsLookFunction> look_function;

        ENDIAN_TEMPLATE(NewType) operator GlobalsPlayerControl<NewType>() const noexcept {
            GlobalsPlayerControl<NewType> copy = {};
            COPY_THIS(magnetism_friction);
            COPY_THIS(magnetism_adhesion);
            COPY_THIS(inconsequential_target_scale);
            COPY_THIS(look_acceleration_time);
            COPY_THIS(look_acceleration_scale);
            COPY_THIS(look_peg_threshold);
            COPY_THIS(look_default_pitch_rate);
            COPY_THIS(look_default_yaw_rate);
            COPY_THIS(look_autolevelling_scale);
            COPY_THIS(minimum_weapon_swap_ticks);
            COPY_THIS(minimum_autolevelling_ticks);
            COPY_THIS(minimum_angle_for_vehicle_flipping);
            COPY_THIS(look_function);
            return copy;
        }
    };
    static_assert(sizeof(GlobalsPlayerControl<BigEndian>) == 0x80);

    ENDIAN_TEMPLATE(EndianType) struct GlobalsDifficulty {
        EndianType<float> easy_enemy_damage;
        EndianType<float> normal_enemy_damage;
        EndianType<float> hard_enemy_damage;
        EndianType<float> imposs_enemy_damage;
        EndianType<float> easy_enemy_vitality;
        EndianType<float> normal_enemy_vitality;
        EndianType<float> hard_enemy_vitality;
        EndianType<float> imposs_enemy_vitality;
        EndianType<float> easy_enemy_shield;
        EndianType<float> normal_enemy_shield;
        EndianType<float> hard_enemy_shield;
        EndianType<float> imposs_enemy_shield;
        EndianType<float> easy_enemy_recharge;
        EndianType<float> normal_enemy_recharge;
        EndianType<float> hard_enemy_recharge;
        EndianType<float> imposs_enemy_recharge;
        EndianType<float> easy_friend_damage;
        EndianType<float> normal_friend_damage;
        EndianType<float> hard_friend_damage;
        EndianType<float> imposs_friend_damage;
        EndianType<float> easy_friend_vitality;
        EndianType<float> normal_friend_vitality;
        EndianType<float> hard_friend_vitality;
        EndianType<float> imposs_friend_vitality;
        EndianType<float> easy_friend_shield;
        EndianType<float> normal_friend_shield;
        EndianType<float> hard_friend_shield;
        EndianType<float> imposs_friend_shield;
        EndianType<float> easy_friend_recharge;
        EndianType<float> normal_friend_recharge;
        EndianType<float> hard_friend_recharge;
        EndianType<float> imposs_friend_recharge;
        EndianType<float> easy_infection_forms;
        EndianType<float> normal_infection_forms;
        EndianType<float> hard_infection_forms;
        EndianType<float> imposs_infection_forms;
        PAD(0x10);
        EndianType<float> easy_rate_of_fire;
        EndianType<float> normal_rate_of_fire;
        EndianType<float> hard_rate_of_fire;
        EndianType<float> imposs_rate_of_fire;
        EndianType<float> easy_projectile_error;
        EndianType<float> normal_projectile_error;
        EndianType<float> hard_projectile_error;
        EndianType<float> imposs_projectile_error;
        EndianType<float> easy_burst_error;
        EndianType<float> normal_burst_error;
        EndianType<float> hard_burst_error;
        EndianType<float> imposs_burst_error;
        EndianType<float> easy_new_target_delay;
        EndianType<float> normal_new_target_delay;
        EndianType<float> hard_new_target_delay;
        EndianType<float> imposs_new_target_delay;
        EndianType<float> easy_burst_separation;
        EndianType<float> normal_burst_separation;
        EndianType<float> hard_burst_separation;
        EndianType<float> imposs_burst_separation;
        EndianType<float> easy_target_tracking;
        EndianType<float> normal_target_tracking;
        EndianType<float> hard_target_tracking;
        EndianType<float> imposs_target_tracking;
        EndianType<float> easy_target_leading;
        EndianType<float> normal_target_leading;
        EndianType<float> hard_target_leading;
        EndianType<float> imposs_target_leading;
        EndianType<float> easy_overcharge_chance;
        EndianType<float> normal_overcharge_chance;
        EndianType<float> hard_overcharge_chance;
        EndianType<float> imposs_overcharge_chance;
        EndianType<float> easy_special_fire_delay;
        EndianType<float> normal_special_fire_delay;
        EndianType<float> hard_special_fire_delay;
        EndianType<float> imposs_special_fire_delay;
        EndianType<float> easy_guidance_vs_player;
        EndianType<float> normal_guidance_vs_player;
        EndianType<float> hard_guidance_vs_player;
        EndianType<float> imposs_guidance_vs_player;
        EndianType<float> easy_melee_delay_base;
        EndianType<float> normal_melee_delay_base;
        EndianType<float> hard_melee_delay_base;
        EndianType<float> imposs_melee_delay_base;
        EndianType<float> easy_melee_delay_scale;
        EndianType<float> normal_melee_delay_scale;
        EndianType<float> hard_melee_delay_scale;
        EndianType<float> imposs_melee_delay_scale;
        PAD(0x10);
        EndianType<float> easy_grenade_chance_scale;
        EndianType<float> normal_grenade_chance_scale;
        EndianType<float> hard_grenade_chance_scale;
        EndianType<float> imposs_grenade_chance_scale;
        EndianType<float> easy_grenade_timer_scale;
        EndianType<float> normal_grenade_timer_scale;
        EndianType<float> hard_grenade_timer_scale;
        EndianType<float> imposs_grenade_timer_scale;
        PAD(0x10);
        PAD(0x10);
        PAD(0x10);
        EndianType<float> easy_major_upgrade;
        EndianType<float> normal_major_upgrade;
        EndianType<float> hard_major_upgrade;
        EndianType<float> imposs_major_upgrade;
        EndianType<float> easy_major_upgrade_1;
        EndianType<float> normal_major_upgrade_1;
        EndianType<float> hard_major_upgrade_1;
        EndianType<float> imposs_major_upgrade_1;
        EndianType<float> easy_major_upgrade_2;
        EndianType<float> normal_major_upgrade_2;
        EndianType<float> hard_major_upgrade_2;
        EndianType<float> imposs_major_upgrade_2;
        PAD(0x10);
        PAD(0x10);
        PAD(0x10);
        PAD(0x10);
        PAD(0x54);

        ENDIAN_TEMPLATE(NewType) operator GlobalsDifficulty<NewType>() const noexcept {
            GlobalsDifficulty<NewType> copy = {};
            COPY_THIS(easy_enemy_damage);
            COPY_THIS(normal_enemy_damage);
            COPY_THIS(hard_enemy_damage);
            COPY_THIS(imposs_enemy_damage);
            COPY_THIS(easy_enemy_vitality);
            COPY_THIS(normal_enemy_vitality);
            COPY_THIS(hard_enemy_vitality);
            COPY_THIS(imposs_enemy_vitality);
            COPY_THIS(easy_enemy_shield);
            COPY_THIS(normal_enemy_shield);
            COPY_THIS(hard_enemy_shield);
            COPY_THIS(imposs_enemy_shield);
            COPY_THIS(easy_enemy_recharge);
            COPY_THIS(normal_enemy_recharge);
            COPY_THIS(hard_enemy_recharge);
            COPY_THIS(imposs_enemy_recharge);
            COPY_THIS(easy_friend_damage);
            COPY_THIS(normal_friend_damage);
            COPY_THIS(hard_friend_damage);
            COPY_THIS(imposs_friend_damage);
            COPY_THIS(easy_friend_vitality);
            COPY_THIS(normal_friend_vitality);
            COPY_THIS(hard_friend_vitality);
            COPY_THIS(imposs_friend_vitality);
            COPY_THIS(easy_friend_shield);
            COPY_THIS(normal_friend_shield);
            COPY_THIS(hard_friend_shield);
            COPY_THIS(imposs_friend_shield);
            COPY_THIS(easy_friend_recharge);
            COPY_THIS(normal_friend_recharge);
            COPY_THIS(hard_friend_recharge);
            COPY_THIS(imposs_friend_recharge);
            COPY_THIS(easy_infection_forms);
            COPY_THIS(normal_infection_forms);
            COPY_THIS(hard_infection_forms);
            COPY_THIS(imposs_infection_forms);
            COPY_THIS(easy_rate_of_fire);
            COPY_THIS(normal_rate_of_fire);
            COPY_THIS(hard_rate_of_fire);
            COPY_THIS(imposs_rate_of_fire);
            COPY_THIS(easy_projectile_error);
            COPY_THIS(normal_projectile_error);
            COPY_THIS(hard_projectile_error);
            COPY_THIS(imposs_projectile_error);
            COPY_THIS(easy_burst_error);
            COPY_THIS(normal_burst_error);
            COPY_THIS(hard_burst_error);
            COPY_THIS(imposs_burst_error);
            COPY_THIS(easy_new_target_delay);
            COPY_THIS(normal_new_target_delay);
            COPY_THIS(hard_new_target_delay);
            COPY_THIS(imposs_new_target_delay);
            COPY_THIS(easy_burst_separation);
            COPY_THIS(normal_burst_separation);
            COPY_THIS(hard_burst_separation);
            COPY_THIS(imposs_burst_separation);
            COPY_THIS(easy_target_tracking);
            COPY_THIS(normal_target_tracking);
            COPY_THIS(hard_target_tracking);
            COPY_THIS(imposs_target_tracking);
            COPY_THIS(easy_target_leading);
            COPY_THIS(normal_target_leading);
            COPY_THIS(hard_target_leading);
            COPY_THIS(imposs_target_leading);
            COPY_THIS(easy_overcharge_chance);
            COPY_THIS(normal_overcharge_chance);
            COPY_THIS(hard_overcharge_chance);
            COPY_THIS(imposs_overcharge_chance);
            COPY_THIS(easy_special_fire_delay);
            COPY_THIS(normal_special_fire_delay);
            COPY_THIS(hard_special_fire_delay);
            COPY_THIS(imposs_special_fire_delay);
            COPY_THIS(easy_guidance_vs_player);
            COPY_THIS(normal_guidance_vs_player);
            COPY_THIS(hard_guidance_vs_player);
            COPY_THIS(imposs_guidance_vs_player);
            COPY_THIS(easy_melee_delay_base);
            COPY_THIS(normal_melee_delay_base);
            COPY_THIS(hard_melee_delay_base);
            COPY_THIS(imposs_melee_delay_base);
            COPY_THIS(easy_melee_delay_scale);
            COPY_THIS(normal_melee_delay_scale);
            COPY_THIS(hard_melee_delay_scale);
            COPY_THIS(imposs_melee_delay_scale);
            COPY_THIS(easy_grenade_chance_scale);
            COPY_THIS(normal_grenade_chance_scale);
            COPY_THIS(hard_grenade_chance_scale);
            COPY_THIS(imposs_grenade_chance_scale);
            COPY_THIS(easy_grenade_timer_scale);
            COPY_THIS(normal_grenade_timer_scale);
            COPY_THIS(hard_grenade_timer_scale);
            COPY_THIS(imposs_grenade_timer_scale);
            COPY_THIS(easy_major_upgrade);
            COPY_THIS(normal_major_upgrade);
            COPY_THIS(hard_major_upgrade);
            COPY_THIS(imposs_major_upgrade);
            COPY_THIS(easy_major_upgrade_1);
            COPY_THIS(normal_major_upgrade_1);
            COPY_THIS(hard_major_upgrade_1);
            COPY_THIS(imposs_major_upgrade_1);
            COPY_THIS(easy_major_upgrade_2);
            COPY_THIS(normal_major_upgrade_2);
            COPY_THIS(hard_major_upgrade_2);
            COPY_THIS(imposs_major_upgrade_2);
            return copy;
        }
    };
    static_assert(sizeof(GlobalsDifficulty<BigEndian>) == 0x284);

    ENDIAN_TEMPLATE(EndianType) struct GlobalsGrenade {
        EndianType<std::uint16_t> maximum_count;
        EndianType<std::uint16_t> mp_spawn_default;
        TagDependency<EndianType> throwing_effect; // effect
        TagDependency<EndianType> hud_interface; // grenade_hud_interface
        TagDependency<EndianType> equipment; // equipment
        TagDependency<EndianType> projectile; // projectile

        ENDIAN_TEMPLATE(NewType) operator GlobalsGrenade<NewType>() const noexcept {
            GlobalsGrenade<NewType> copy = {};
            COPY_THIS(maximum_count);
            COPY_THIS(mp_spawn_default);
            COPY_THIS(throwing_effect);
            COPY_THIS(hud_interface);
            COPY_THIS(equipment);
            COPY_THIS(projectile);
            return copy;
        }
    };
    static_assert(sizeof(GlobalsGrenade<BigEndian>) == 0x44);

    struct GlobalsRasterizerDataFlags {
        std::uint16_t tint_edge_density : 1;
    };

    ENDIAN_TEMPLATE(EndianType) struct GlobalsRasterizerData {
        TagDependency<EndianType> distance_attenuation; // bitmap
        TagDependency<EndianType> vector_normalization; // bitmap
        TagDependency<EndianType> atmospheric_fog_density; // bitmap
        TagDependency<EndianType> planar_fog_density; // bitmap
        TagDependency<EndianType> linear_corner_fade; // bitmap
        TagDependency<EndianType> active_camouflage_distortion; // bitmap
        TagDependency<EndianType> glow; // bitmap
        PAD(0x3C);
        TagDependency<EndianType> default_2d; // bitmap
        TagDependency<EndianType> default_3d; // bitmap
        TagDependency<EndianType> default_cube_map; // bitmap
        TagDependency<EndianType> test_0; // bitmap
        TagDependency<EndianType> test_1; // bitmap
        TagDependency<EndianType> test_2; // bitmap
        TagDependency<EndianType> test_3; // bitmap
        TagDependency<EndianType> video_scanline_map; // bitmap
        TagDependency<EndianType> video_noise_map; // bitmap
        PAD(0x34);
        EndianType<GlobalsRasterizerDataFlags> flags;
        PAD(0x2);
        EndianType<float> refraction_amount;
        EndianType<float> distance_falloff;
        ColorRGB<EndianType> tint_color;
        EndianType<float> hyper_stealth_refraction;
        EndianType<float> hyper_stealth_distance_falloff;
        ColorRGB<EndianType> hyper_stealth_tint_color;
        TagDependency<EndianType> distance_attenuation_2d; // bitmap

        ENDIAN_TEMPLATE(NewType) operator GlobalsRasterizerData<NewType>() const noexcept {
            GlobalsRasterizerData<NewType> copy = {};
            COPY_THIS(distance_attenuation);
            COPY_THIS(vector_normalization);
            COPY_THIS(atmospheric_fog_density);
            COPY_THIS(planar_fog_density);
            COPY_THIS(linear_corner_fade);
            COPY_THIS(active_camouflage_distortion);
            COPY_THIS(glow);
            COPY_THIS(default_2d);
            COPY_THIS(default_3d);
            COPY_THIS(default_cube_map);
            COPY_THIS(test_0);
            COPY_THIS(test_1);
            COPY_THIS(test_2);
            COPY_THIS(test_3);
            COPY_THIS(video_scanline_map);
            COPY_THIS(video_noise_map);
            COPY_THIS(flags);
            COPY_THIS(refraction_amount);
            COPY_THIS(distance_falloff);
            COPY_THIS(tint_color);
            COPY_THIS(hyper_stealth_refraction);
            COPY_THIS(hyper_stealth_distance_falloff);
            COPY_THIS(hyper_stealth_tint_color);
            COPY_THIS(distance_attenuation_2d);
            return copy;
        }
    };
    static_assert(sizeof(GlobalsRasterizerData<BigEndian>) == 0x1AC);

    ENDIAN_TEMPLATE(EndianType) struct GlobalsInterfaceBitmaps {
        TagDependency<EndianType> font_system; // font
        TagDependency<EndianType> font_terminal; // font
        TagDependency<EndianType> screen_color_table; // color_table
        TagDependency<EndianType> hud_color_table; // color_table
        TagDependency<EndianType> editor_color_table; // color_table
        TagDependency<EndianType> dialog_color_table; // color_table
        TagDependency<EndianType> hud_globals; // hud_globals
        TagDependency<EndianType> motion_sensor_sweep_bitmap; // bitmap
        TagDependency<EndianType> motion_sensor_sweep_bitmap_mask; // bitmap
        TagDependency<EndianType> multiplayer_hud_bitmap; // bitmap
        TagDependency<EndianType> localization; // string_list
        TagDependency<EndianType> hud_digits_definition; // hud_numbers
        TagDependency<EndianType> motion_sensor_blip_bitmap; // bitmap
        TagDependency<EndianType> interface_goo_map1; // bitmap
        TagDependency<EndianType> interface_goo_map2; // bitmap
        TagDependency<EndianType> interface_goo_map3; // bitmap
        PAD(0x30);

        ENDIAN_TEMPLATE(NewType) operator GlobalsInterfaceBitmaps<NewType>() const noexcept {
            GlobalsInterfaceBitmaps<NewType> copy = {};
            COPY_THIS(font_system);
            COPY_THIS(font_terminal);
            COPY_THIS(screen_color_table);
            COPY_THIS(hud_color_table);
            COPY_THIS(editor_color_table);
            COPY_THIS(dialog_color_table);
            COPY_THIS(hud_globals);
            COPY_THIS(motion_sensor_sweep_bitmap);
            COPY_THIS(motion_sensor_sweep_bitmap_mask);
            COPY_THIS(multiplayer_hud_bitmap);
            COPY_THIS(localization);
            COPY_THIS(hud_digits_definition);
            COPY_THIS(motion_sensor_blip_bitmap);
            COPY_THIS(interface_goo_map1);
            COPY_THIS(interface_goo_map2);
            COPY_THIS(interface_goo_map3);
            return copy;
        }
    };
    static_assert(sizeof(GlobalsInterfaceBitmaps<BigEndian>) == 0x130);

    SINGLE_DEPENDENCY_STRUCT(GlobalsWeapon, weapon); // item
    SINGLE_DEPENDENCY_STRUCT(GlobalsCheatPowerup, powerup); // equipment
    SINGLE_DEPENDENCY_STRUCT(GlobalsVehicle, vehicle); // unit
    SINGLE_DEPENDENCY_STRUCT(GlobalsMultiplayerSound, sound); // sound

    ENDIAN_TEMPLATE(EndianType) struct GlobalsMultiplayerInformation {
        TagDependency<EndianType> flag; // item
        TagDependency<EndianType> unit; // unit
        TagReflexive<EndianType, GlobalsVehicle> vehicles;
        TagDependency<EndianType> hill_shader; // shader
        TagDependency<EndianType> flag_shader; // shader
        TagDependency<EndianType> ball; // item
        TagReflexive<EndianType, GlobalsMultiplayerSound> sounds;
        PAD(0x38);

        ENDIAN_TEMPLATE(NewType) operator GlobalsMultiplayerInformation<NewType>() const noexcept {
            GlobalsMultiplayerInformation<NewType> copy = {};
            COPY_THIS(flag);
            COPY_THIS(unit);
            COPY_THIS(vehicles);
            COPY_THIS(hill_shader);
            COPY_THIS(flag_shader);
            COPY_THIS(ball);
            COPY_THIS(sounds);
            return copy;
        }
    };
    static_assert(sizeof(GlobalsMultiplayerInformation<BigEndian>) == 0xA0);

    ENDIAN_TEMPLATE(EndianType) struct GlobalsPlayerInformation {
        TagDependency<EndianType> unit; // unit
        PAD(0x1C);
        EndianType<float> walking_speed;
        EndianType<float> double_speed_multiplier;
        EndianType<float> run_forward;
        EndianType<float> run_backward;
        EndianType<float> run_sideways;
        EndianType<float> run_acceleration;
        EndianType<float> sneak_forward;
        EndianType<float> sneak_backward;
        EndianType<float> sneak_sideways;
        EndianType<float> sneak_acceleration;
        EndianType<float> airborne_acceleration;
        EndianType<float> speed_multiplier;
        PAD(0xC);
        Point3D<EndianType> grenade_origin;
        PAD(0xC);
        EndianType<float> stun_movement_penalty;
        EndianType<float> stun_turning_penalty;
        EndianType<float> stun_jumping_penalty;
        EndianType<float> minimum_stun_time;
        EndianType<float> maximum_stun_time;
        PAD(0x8);
        Bounds<EndianType<float>> first_person_idle_time;
        EndianType<float> first_person_skip_fraction;
        PAD(0x10);
        TagDependency<EndianType> coop_respawn_effect; // effect
        PAD(0x2C);

        ENDIAN_TEMPLATE(NewType) operator GlobalsPlayerInformation<NewType>() const noexcept {
            GlobalsPlayerInformation<NewType> copy = {};
            COPY_THIS(unit);
            COPY_THIS(walking_speed);
            COPY_THIS(double_speed_multiplier);
            COPY_THIS(run_forward);
            COPY_THIS(run_backward);
            COPY_THIS(run_sideways);
            COPY_THIS(run_acceleration);
            COPY_THIS(sneak_forward);
            COPY_THIS(sneak_backward);
            COPY_THIS(sneak_sideways);
            COPY_THIS(sneak_acceleration);
            COPY_THIS(airborne_acceleration);
            COPY_THIS(speed_multiplier);
            COPY_THIS(grenade_origin);
            COPY_THIS(stun_movement_penalty);
            COPY_THIS(stun_turning_penalty);
            COPY_THIS(stun_jumping_penalty);
            COPY_THIS(minimum_stun_time);
            COPY_THIS(maximum_stun_time);
            COPY_THIS(first_person_idle_time);
            COPY_THIS(first_person_skip_fraction);
            COPY_THIS(coop_respawn_effect);
            return copy;
        }
    };
    static_assert(sizeof(GlobalsPlayerInformation<BigEndian>) == 0xF4);

    ENDIAN_TEMPLATE(EndianType) struct GlobalsFirstPersonInterface {
        TagDependency<EndianType> first_person_hands; //mod2
        TagDependency<EndianType> base_bitmap; // bitmap
        TagDependency<EndianType> shield_meter; //metr
        Point2DInt<EndianType> shield_meter_origin;
        TagDependency<EndianType> body_meter; //metr
        Point2DInt<EndianType> body_meter_origin;
        TagDependency<EndianType> night_vision_off_on_effect; // effect
        TagDependency<EndianType> night_vision_on_off_effect; // effect
        PAD(0x58);

        ENDIAN_TEMPLATE(NewType) operator GlobalsFirstPersonInterface<NewType>() const noexcept {
            GlobalsFirstPersonInterface<NewType> copy = {};
            COPY_THIS(first_person_hands);
            COPY_THIS(base_bitmap);
            COPY_THIS(shield_meter);
            COPY_THIS(shield_meter_origin);
            COPY_THIS(body_meter);
            COPY_THIS(body_meter_origin);
            COPY_THIS(night_vision_off_on_effect);
            COPY_THIS(night_vision_on_off_effect);
            return copy;
        }
    };
    static_assert(sizeof(GlobalsFirstPersonInterface<BigEndian>) == 0xC0);

    ENDIAN_TEMPLATE(EndianType) struct GlobalsFallingDamage {
        PAD(0x8);
        Bounds<EndianType<float>> harmful_falling_distance;
        TagDependency<EndianType> falling_damage; // damage_effect
        PAD(0x8);
        EndianType<float> maximum_falling_distance;
        TagDependency<EndianType> distance_damage; // damage_effect
        TagDependency<EndianType> vehicle_environment_collision_damage_effect; // damage_effect
        TagDependency<EndianType> vehicle_killed_unit_damage_effect; // damage_effect
        TagDependency<EndianType> vehicle_collision_damage; // damage_effect
        TagDependency<EndianType> flaming_death_damage; // damage_effect
        PAD(0x10);
        LittleEndian<float> maximum_falling_velocity;
        Bounds<LittleEndian<float>> harmful_falling_velocity;

        ENDIAN_TEMPLATE(NewType) operator GlobalsFallingDamage<NewType>() const noexcept {
            GlobalsFallingDamage<NewType> copy = {};
            COPY_THIS(harmful_falling_distance);
            COPY_THIS(falling_damage);
            COPY_THIS(maximum_falling_distance);
            COPY_THIS(distance_damage);
            COPY_THIS(vehicle_environment_collision_damage_effect);
            COPY_THIS(vehicle_killed_unit_damage_effect);
            COPY_THIS(vehicle_collision_damage);
            COPY_THIS(flaming_death_damage);
            COPY_THIS(harmful_falling_velocity);
            return copy;
        }
    };
    static_assert(sizeof(GlobalsFallingDamage<BigEndian>) == 0x98);

    struct GlobalsBreakableSurfaceParticleEffectFlags {
        std::uint32_t interpolate_color_in_hsv : 1;
        std::uint32_t _more_colors : 1;
    };

    ENDIAN_TEMPLATE(EndianType) struct GlobalsBreakableSurfaceParticleEffect {
        TagDependency<EndianType> particle_type; // particle
        EndianType<GlobalsBreakableSurfaceParticleEffectFlags> flags;
        EndianType<float> density;
        Bounds<EndianType<float>> velocity_scale;
        PAD(0x4);
        Bounds<EndianType<float>> angular_velocity;
        PAD(0x8);
        Bounds<EndianType<float>> radius;
        PAD(0x8);
        ColorARGB<EndianType> tint_lower_bound;
        ColorARGB<EndianType> tint_upper_bound;
        PAD(0x1C);

        ENDIAN_TEMPLATE(NewType) operator GlobalsBreakableSurfaceParticleEffect<NewType>() const noexcept {
            GlobalsBreakableSurfaceParticleEffect<NewType> copy = {};
            COPY_THIS(particle_type);
            COPY_THIS(flags);
            COPY_THIS(density);
            COPY_THIS(velocity_scale);
            COPY_THIS(angular_velocity);
            COPY_THIS(radius);
            COPY_THIS(tint_lower_bound);
            COPY_THIS(tint_upper_bound);
            return copy;
        }
    };
    static_assert(sizeof(GlobalsBreakableSurfaceParticleEffect<BigEndian>) == 0x80);

    ENDIAN_TEMPLATE(EndianType) struct GlobalsMaterial {
        PAD(0x64);
        PAD(0x30);
        EndianType<float> ground_friction_scale;
        EndianType<float> ground_friction_normal_k1_scale;
        EndianType<float> ground_friction_normal_k0_scale;
        EndianType<float> ground_depth_scale;
        EndianType<float> ground_damp_fraction_scale;
        PAD(0x4C);
        PAD(0x1E0);
        EndianType<float> maximum_vitality;
        PAD(0x8);
        PAD(0x4);
        TagDependency<EndianType> effect; // effect
        TagDependency<EndianType> sound; // sound
        PAD(0x18);
        TagReflexive<EndianType, GlobalsBreakableSurfaceParticleEffect> particle_effects;
        PAD(0x3C);
        TagDependency<EndianType> melee_hit_sound; // sound

        ENDIAN_TEMPLATE(NewType) operator GlobalsMaterial<NewType>() const noexcept {
            GlobalsMaterial<NewType> copy = {};
            COPY_THIS(ground_friction_scale);
            COPY_THIS(ground_friction_normal_k1_scale);
            COPY_THIS(ground_friction_normal_k0_scale);
            COPY_THIS(ground_depth_scale);
            COPY_THIS(ground_damp_fraction_scale);
            COPY_THIS(maximum_vitality);
            COPY_THIS(effect);
            COPY_THIS(sound);
            COPY_THIS(particle_effects);
            COPY_THIS(melee_hit_sound);
            return copy;
        }
    };
    static_assert(sizeof(GlobalsMaterial<BigEndian>) == 0x374);

    ENDIAN_TEMPLATE(EndianType) struct GlobalsPlaylistMember {
        TagString map_name;
        TagString game_variant;
        EndianType<std::int32_t> minimum_experience;
        EndianType<std::int32_t> maximum_experience;
        EndianType<std::int32_t> minimum_player_count;
        EndianType<std::int32_t> maximum_player_count;
        EndianType<std::int32_t> rating;
        PAD(0x40);

        ENDIAN_TEMPLATE(NewType) operator GlobalsPlaylistMember<NewType>() const noexcept {
            GlobalsPlaylistMember<NewType> copy = {};
            COPY_THIS(map_name);
            COPY_THIS(game_variant);
            COPY_THIS(minimum_experience);
            COPY_THIS(maximum_experience);
            COPY_THIS(minimum_player_count);
            COPY_THIS(maximum_player_count);
            COPY_THIS(rating);
            return copy;
        }
    };
    static_assert(sizeof(GlobalsPlaylistMember<BigEndian>) == 0x94);

    ENDIAN_TEMPLATE(EndianType) struct Globals {
        PAD(0xF8);
        TagReflexive<EndianType, GlobalsSound> sounds;
        TagReflexive<EndianType, GlobalsCamera> camera;
        TagReflexive<EndianType, GlobalsPlayerControl> player_control;
        TagReflexive<EndianType, GlobalsDifficulty> difficulty;
        TagReflexive<EndianType, GlobalsGrenade> grenades;
        TagReflexive<EndianType, GlobalsRasterizerData> rasterizer_data;
        TagReflexive<EndianType, GlobalsInterfaceBitmaps> interface_bitmaps;
        TagReflexive<EndianType, GlobalsWeapon> weapon_list;
        TagReflexive<EndianType, GlobalsCheatPowerup> cheat_powerups;
        TagReflexive<EndianType, GlobalsMultiplayerInformation> multiplayer_information;
        TagReflexive<EndianType, GlobalsPlayerInformation> player_information;
        TagReflexive<EndianType, GlobalsFirstPersonInterface> first_person_interface;
        TagReflexive<EndianType, GlobalsFallingDamage> falling_damage;
        TagReflexive<EndianType, GlobalsMaterial> materials;
        TagReflexive<EndianType, GlobalsPlaylistMember> playlist_members;

        ENDIAN_TEMPLATE(NewType) operator Globals<NewType>() const noexcept {
            Globals<NewType> copy = {};
            COPY_THIS(sounds);
            COPY_THIS(camera);
            COPY_THIS(player_control);
            COPY_THIS(difficulty);
            COPY_THIS(grenades);
            COPY_THIS(rasterizer_data);
            COPY_THIS(interface_bitmaps);
            COPY_THIS(weapon_list);
            COPY_THIS(cheat_powerups);
            COPY_THIS(multiplayer_information);
            COPY_THIS(player_information);
            COPY_THIS(first_person_interface);
            COPY_THIS(falling_damage);
            COPY_THIS(materials);
            COPY_THIS(playlist_members);
            return copy;
        }
    };
    static_assert(sizeof(Globals<BigEndian>) == 0x1AC);

    void compile_globals_tag(CompiledTag &compiled, const std::byte *data, std::size_t size, CacheFileType type);
}
#endif
