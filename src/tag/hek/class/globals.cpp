// SPDX-License-Identifier: GPL-3.0-only

#include <cmath>

#include <invader/hek/constants.hpp>
#include <invader/tag/hek/compile.hpp>
#include <invader/tag/hek/definition.hpp>

namespace Invader::HEK {
    void compile_globals_tag(CompiledTag &compiled, const std::byte *data, std::size_t size, CacheFileType type) {
        BEGIN_COMPILE(Globals)

        ADD_BASIC_DEPENDENCY_REFLEXIVE(tag.sounds, sound);
        ADD_BASIC_DEPENDENCY_REFLEXIVE(tag.camera, default_unit_camera_track);

        ADD_REFLEXIVE_START(tag.player_control)
        ADD_REFLEXIVE(reflexive.look_function);
        ADD_REFLEXIVE_END

        ADD_REFLEXIVE(tag.difficulty);

        ADD_REFLEXIVE_START(tag.grenades) {
            ADD_DEPENDENCY_ADJUST_SIZES(reflexive.throwing_effect);
            ADD_DEPENDENCY_ADJUST_SIZES(reflexive.hud_interface);
            ADD_DEPENDENCY_ADJUST_SIZES(reflexive.equipment);
            ADD_DEPENDENCY_ADJUST_SIZES(reflexive.projectile);
        } ADD_REFLEXIVE_END

        ADD_REFLEXIVE_START(tag.rasterizer_data) {
            ADD_DEPENDENCY_ADJUST_SIZES(reflexive.distance_attenuation);
            ADD_DEPENDENCY_ADJUST_SIZES(reflexive.vector_normalization);
            ADD_DEPENDENCY_ADJUST_SIZES(reflexive.atmospheric_fog_density);
            ADD_DEPENDENCY_ADJUST_SIZES(reflexive.planar_fog_density);
            ADD_DEPENDENCY_ADJUST_SIZES(reflexive.linear_corner_fade);
            ADD_DEPENDENCY_ADJUST_SIZES(reflexive.active_camouflage_distortion);
            ADD_DEPENDENCY_ADJUST_SIZES(reflexive.glow);
            ADD_DEPENDENCY_ADJUST_SIZES(reflexive.default_2d);
            ADD_DEPENDENCY_ADJUST_SIZES(reflexive.default_3d);
            ADD_DEPENDENCY_ADJUST_SIZES(reflexive.default_cube_map);
            ADD_DEPENDENCY_ADJUST_SIZES(reflexive.test_0);
            ADD_DEPENDENCY_ADJUST_SIZES(reflexive.test_1);
            ADD_DEPENDENCY_ADJUST_SIZES(reflexive.test_2);
            ADD_DEPENDENCY_ADJUST_SIZES(reflexive.test_3);
            ADD_DEPENDENCY_ADJUST_SIZES(reflexive.video_scanline_map);
            ADD_DEPENDENCY_ADJUST_SIZES(reflexive.video_noise_map);
            ADD_DEPENDENCY_ADJUST_SIZES(reflexive.distance_attenuation_2d);
        } ADD_REFLEXIVE_END

        ADD_REFLEXIVE_START(tag.interface_bitmaps) {
            ADD_DEPENDENCY_ADJUST_SIZES(reflexive.font_system);
            ADD_DEPENDENCY_ADJUST_SIZES(reflexive.font_terminal);
            ADD_DEPENDENCY_ADJUST_SIZES(reflexive.screen_color_table);
            ADD_DEPENDENCY_ADJUST_SIZES(reflexive.hud_color_table);
            ADD_DEPENDENCY_ADJUST_SIZES(reflexive.editor_color_table);
            ADD_DEPENDENCY_ADJUST_SIZES(reflexive.dialog_color_table);
            ADD_DEPENDENCY_ADJUST_SIZES(reflexive.hud_globals);
            ADD_DEPENDENCY_ADJUST_SIZES(reflexive.motion_sensor_sweep_bitmap);
            ADD_DEPENDENCY_ADJUST_SIZES(reflexive.motion_sensor_sweep_bitmap_mask);
            ADD_DEPENDENCY_ADJUST_SIZES(reflexive.multiplayer_hud_bitmap);
            ADD_DEPENDENCY_ADJUST_SIZES(reflexive.localization);
            ADD_DEPENDENCY_ADJUST_SIZES(reflexive.hud_digits_definition);
            ADD_DEPENDENCY_ADJUST_SIZES(reflexive.motion_sensor_blip_bitmap);
            ADD_DEPENDENCY_ADJUST_SIZES(reflexive.interface_goo_map1);
            ADD_DEPENDENCY_ADJUST_SIZES(reflexive.interface_goo_map2);
            ADD_DEPENDENCY_ADJUST_SIZES(reflexive.interface_goo_map3);
        } ADD_REFLEXIVE_END

        skip_data = type != CacheFileType::CACHE_FILE_MULTIPLAYER;

        ADD_BASIC_DEPENDENCY_REFLEXIVE(tag.weapon_list, weapon);
        ADD_BASIC_DEPENDENCY_REFLEXIVE(tag.cheat_powerups, powerup);

        ADD_REFLEXIVE_START(tag.multiplayer_information) {
            ADD_DEPENDENCY_ADJUST_SIZES(reflexive.flag);
            ADD_DEPENDENCY_ADJUST_SIZES(reflexive.unit);
            ADD_BASIC_DEPENDENCY_REFLEXIVE(reflexive.vehicles, vehicle);
            ADD_DEPENDENCY_ADJUST_SIZES(reflexive.hill_shader);
            ADD_DEPENDENCY_ADJUST_SIZES(reflexive.flag_shader);
            ADD_DEPENDENCY_ADJUST_SIZES(reflexive.ball);
            ADD_BASIC_DEPENDENCY_REFLEXIVE(reflexive.sounds, sound);
        } ADD_REFLEXIVE_END

        skip_data = false;

        ADD_REFLEXIVE_START(tag.player_information) {
            skip_data = type == CacheFileType::CACHE_FILE_USER_INTERFACE;
            ADD_DEPENDENCY_ADJUST_SIZES(reflexive.unit);
            skip_data = false;
            ADD_DEPENDENCY_ADJUST_SIZES(reflexive.coop_respawn_effect);
        } ADD_REFLEXIVE_END

        ADD_REFLEXIVE_START(tag.first_person_interface) {
            ADD_DEPENDENCY_ADJUST_SIZES(reflexive.first_person_hands);
            ADD_DEPENDENCY_ADJUST_SIZES(reflexive.base_bitmap);
            ADD_DEPENDENCY_ADJUST_SIZES(reflexive.shield_meter);
            ADD_DEPENDENCY_ADJUST_SIZES(reflexive.body_meter);
            ADD_DEPENDENCY_ADJUST_SIZES(reflexive.night_vision_off_on_effect);
            ADD_DEPENDENCY_ADJUST_SIZES(reflexive.night_vision_on_off_effect);
        } ADD_REFLEXIVE_END

        skip_data = type == CacheFileType::CACHE_FILE_USER_INTERFACE;

        ADD_REFLEXIVE_START(tag.falling_damage) {
            ADD_DEPENDENCY_ADJUST_SIZES(reflexive.falling_damage);
            ADD_DEPENDENCY_ADJUST_SIZES(reflexive.distance_damage);
            ADD_DEPENDENCY_ADJUST_SIZES(reflexive.vehicle_environment_collision_damage_effect);
            ADD_DEPENDENCY_ADJUST_SIZES(reflexive.vehicle_killed_unit_damage_effect);
            ADD_DEPENDENCY_ADJUST_SIZES(reflexive.vehicle_collision_damage);
            ADD_DEPENDENCY_ADJUST_SIZES(reflexive.flaming_death_damage);
            reflexive.maximum_falling_velocity = static_cast<float>(std::sqrt(reflexive.maximum_falling_distance * GRAVITY * 2.0f));
            reflexive.harmful_falling_velocity.from = static_cast<float>(std::sqrt(reflexive.harmful_falling_distance.from * GRAVITY * 2.0f));
            reflexive.harmful_falling_velocity.to = static_cast<float>(std::sqrt(reflexive.harmful_falling_distance.to * GRAVITY * 2.0f));
        } ADD_REFLEXIVE_END

        ADD_REFLEXIVE_START(tag.materials) {
            ADD_DEPENDENCY_ADJUST_SIZES(reflexive.effect);
            ADD_DEPENDENCY_ADJUST_SIZES(reflexive.sound);
            ADD_BASIC_DEPENDENCY_REFLEXIVE(reflexive.particle_effects, particle_type);
            ADD_DEPENDENCY_ADJUST_SIZES(reflexive.melee_hit_sound);
        } ADD_REFLEXIVE_END

        skip_data = false;

        ADD_REFLEXIVE(tag.playlist_members);

        FINISH_COMPILE
    }
}
