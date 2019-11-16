// SPDX-License-Identifier: GPL-3.0-only

#ifndef INVADER__TAG__HEK__CLASS__HUD_GLOBALS_HPP
#define INVADER__TAG__HEK__CLASS__HUD_GLOBALS_HPP

#include "hud_interface_types.hpp"

namespace Invader::HEK {
    ENDIAN_TEMPLATE(EndianType) struct HUDGlobalsButtonIcon {
        EndianType<Index> sequence_index;
        EndianType<std::int16_t> width_offset;
        Point2DInt<EndianType> offset_from_reference_corner;
        EndianType<ColorARGBInt> override_icon_color;
        std::int8_t frame_rate;
        HUDInterfaceMessagingFlags flags;
        EndianType<Index> text_index;

        ENDIAN_TEMPLATE(NewType) operator HUDGlobalsButtonIcon<NewType>() const noexcept {
            HUDGlobalsButtonIcon<NewType> copy;
            COPY_THIS(sequence_index);
            COPY_THIS(width_offset);
            COPY_THIS(offset_from_reference_corner);
            COPY_THIS(override_icon_color);
            COPY_THIS(frame_rate);
            COPY_THIS(flags);
            COPY_THIS(text_index);
            return copy;
        }
    };
    static_assert(sizeof(HUDGlobalsButtonIcon<BigEndian>) == 0x10);

    struct HUDGlobalsWaypointArrowFlags {
        std::uint32_t dont_rotate_when_pointing_offscreen : 1;
    };
    ENDIAN_TEMPLATE(EndianType) struct HUDGlobalsWaypointArrow {
        TagString name;
        PAD(0x8);
        EndianType<ColorARGBInt> color;
        EndianType<float> opacity;
        EndianType<float> translucency;
        EndianType<Index> on_screen_sequence_index;
        EndianType<Index> off_screen_sequence_index;
        EndianType<Index> occluded_sequence_index;
        PAD(0x2);
        PAD(0x10);
        EndianType<HUDGlobalsWaypointArrowFlags> flags;
        PAD(0x18);

        ENDIAN_TEMPLATE(NewType) operator HUDGlobalsWaypointArrow<NewType>() const noexcept {
            HUDGlobalsWaypointArrow<NewType> copy = {};
            COPY_THIS(name);
            COPY_THIS(color);
            COPY_THIS(opacity);
            COPY_THIS(translucency);
            COPY_THIS(on_screen_sequence_index);
            COPY_THIS(off_screen_sequence_index);
            COPY_THIS(occluded_sequence_index);
            COPY_THIS(flags);
            return copy;
        }
    };
    static_assert(sizeof(HUDGlobalsWaypointArrow<BigEndian>) == 0x68);

    ENDIAN_TEMPLATE(EndianType) struct HUDGlobals {
        EndianType<HUDInterfaceAnchor> anchor;
        PAD(0x2);
        PAD(0x20);
        Point2DInt<EndianType> anchor_offset;
        EndianType<float> width_scale;
        EndianType<float> height_scale;
        EndianType<HUDInterfaceScalingFlags> scaling_flags;
        PAD(0x2);
        PAD(0x14);
        TagDependency<EndianType> single_player_font; // font
        TagDependency<EndianType> multi_player_font; // font
        EndianType<float> up_time;
        EndianType<float> fade_time;
        ColorARGB<EndianType> icon_color;
        ColorARGB<EndianType> text_color;
        EndianType<float> text_spacing;
        TagDependency<EndianType> item_message_text; // unicode_string_list
        TagDependency<EndianType> icon_bitmap; // bitmap
        TagDependency<EndianType> alternate_icon_text; // unicode_string_list
        TagReflexive<EndianType, HUDGlobalsButtonIcon> button_icons;
        EndianType<ColorARGBInt> hud_help_default_color;
        EndianType<ColorARGBInt> hud_help_flashing_color;
        EndianType<float> hud_help_flash_period;
        EndianType<float> hud_help_flash_delay;
        EndianType<std::uint16_t> hud_help_number_of_flashes;
        EndianType<HUDInterfaceFlashFlags> hud_help_flash_flags;
        EndianType<float> hud_help_flash_length;
        EndianType<ColorARGBInt> hud_help_disabled_color;
        PAD(0x4);
        TagDependency<EndianType> hud_messages; // hud_message
        EndianType<ColorARGBInt> objective_default_color;
        EndianType<ColorARGBInt> objective_flashing_color;
        EndianType<float> objective_flash_period;
        EndianType<float> objective_flash_delay;
        EndianType<std::uint16_t> objective_number_of_flashes;
        EndianType<HUDInterfaceFlashFlags> objective_flash_flags;
        EndianType<float> objective_flash_length;
        EndianType<ColorARGBInt> objective_disabled_color;
        EndianType<std::uint16_t> objective_uptime_ticks;
        EndianType<std::uint16_t> objective_fade_ticks;
        EndianType<float> top_offset;
        EndianType<float> bottom_offset;
        EndianType<float> left_offset;
        EndianType<float> right_offset;
        PAD(0x20);
        TagDependency<EndianType> arrow_bitmap; // bitmap
        TagReflexive<EndianType, HUDGlobalsWaypointArrow> waypoint_arrows;
        PAD(0x50);
        EndianType<float> hud_scale_in_multiplayer;
        PAD(0x100);
        TagDependency<EndianType> default_weapon_hud; // weapon_hud_interface
        EndianType<float> motion_sensor_range;
        EndianType<float> motion_sensor_velocity_sensitivity;
        EndianType<float> motion_sensor_scale;
        Rectangle2D<EndianType> default_chapter_title_bounds;
        PAD(0x2C);
        EndianType<std::int16_t> hud_damage_top_offset;
        EndianType<std::int16_t> hud_damage_bottom_offset;
        EndianType<std::int16_t> hud_damage_left_offset;
        EndianType<std::int16_t> hud_damage_right_offset;
        PAD(0x20);
        TagDependency<EndianType> hud_damage_indicator_bitmap; // bitmap
        EndianType<Index> hud_damage_sequence_index;
        EndianType<Index> hud_damage_multiplayer_sequence_index;
        EndianType<ColorARGBInt> hud_damage_color;
        PAD(0x10);
        EndianType<ColorARGBInt> not_much_time_left_default_color;
        EndianType<ColorARGBInt> not_much_time_left_flashing_color;
        EndianType<float> not_much_time_left_flash_period;
        EndianType<float> not_much_time_left_flash_delay;
        EndianType<std::uint16_t> not_much_time_left_number_of_flashes;
        EndianType<HUDInterfaceFlashFlags> not_much_time_left_flash_flags;
        EndianType<float> not_much_time_left_flash_length;
        EndianType<ColorARGBInt> not_much_time_left_disabled_color;
        PAD(0x4);
        EndianType<ColorARGBInt> time_out_flash_default_color;
        EndianType<ColorARGBInt> time_out_flash_flashing_color;
        EndianType<float> time_out_flash_flash_period;
        EndianType<float> time_out_flash_flash_delay;
        EndianType<std::uint16_t> time_out_flash_number_of_flashes;
        EndianType<HUDInterfaceFlashFlags> time_out_flash_flash_flags;
        EndianType<float> time_out_flash_flash_length;
        EndianType<ColorARGBInt> time_out_flash_disabled_color;
        PAD(0x4);
        PAD(0x28);
        TagDependency<EndianType> carnage_report_bitmap; // bitmap
        EndianType<Index> loading_begin_text;
        EndianType<Index> loading_end_text;
        EndianType<Index> checkpoint_begin_text;
        EndianType<Index> checkpoint_end_text;
        TagDependency<EndianType> checkpoint_sound; // sound
        PAD(0x60);

        ENDIAN_TEMPLATE(NewType) operator HUDGlobals<NewType>() const noexcept {
            HUDGlobals<NewType> copy = {};
            COPY_THIS(anchor);
            COPY_THIS(anchor_offset);
            COPY_THIS(width_scale);
            COPY_THIS(height_scale);
            COPY_THIS(scaling_flags);
            COPY_THIS(single_player_font);
            COPY_THIS(multi_player_font);
            COPY_THIS(up_time);
            COPY_THIS(fade_time);
            COPY_THIS(icon_color);
            COPY_THIS(text_color);
            COPY_THIS(text_spacing);
            COPY_THIS(item_message_text);
            COPY_THIS(icon_bitmap);
            COPY_THIS(alternate_icon_text);
            COPY_THIS(button_icons);
            COPY_THIS(hud_help_default_color);
            COPY_THIS(hud_help_flashing_color);
            COPY_THIS(hud_help_flash_period);
            COPY_THIS(hud_help_flash_delay);
            COPY_THIS(hud_help_number_of_flashes);
            COPY_THIS(hud_help_flash_flags);
            COPY_THIS(hud_help_flash_length);
            COPY_THIS(hud_help_disabled_color);
            COPY_THIS(hud_messages);
            COPY_THIS(objective_default_color);
            COPY_THIS(objective_flashing_color);
            COPY_THIS(objective_flash_period);
            COPY_THIS(objective_flash_delay);
            COPY_THIS(objective_number_of_flashes);
            COPY_THIS(objective_flash_flags);
            COPY_THIS(objective_flash_length);
            COPY_THIS(objective_disabled_color);
            COPY_THIS(objective_uptime_ticks);
            COPY_THIS(objective_fade_ticks);
            COPY_THIS(top_offset);
            COPY_THIS(bottom_offset);
            COPY_THIS(left_offset);
            COPY_THIS(right_offset);
            COPY_THIS(arrow_bitmap);
            COPY_THIS(waypoint_arrows);
            COPY_THIS(hud_scale_in_multiplayer);
            COPY_THIS(default_weapon_hud);
            COPY_THIS(motion_sensor_range);
            COPY_THIS(motion_sensor_velocity_sensitivity);
            COPY_THIS(motion_sensor_scale);
            COPY_THIS(default_chapter_title_bounds);
            COPY_THIS(hud_damage_top_offset);
            COPY_THIS(hud_damage_bottom_offset);
            COPY_THIS(hud_damage_left_offset);
            COPY_THIS(hud_damage_right_offset);
            COPY_THIS(hud_damage_indicator_bitmap);
            COPY_THIS(hud_damage_sequence_index);
            COPY_THIS(hud_damage_multiplayer_sequence_index);
            COPY_THIS(hud_damage_color);
            COPY_THIS(not_much_time_left_default_color);
            COPY_THIS(not_much_time_left_flashing_color);
            COPY_THIS(not_much_time_left_flash_period);
            COPY_THIS(not_much_time_left_flash_delay);
            COPY_THIS(not_much_time_left_number_of_flashes);
            COPY_THIS(not_much_time_left_flash_flags);
            COPY_THIS(not_much_time_left_flash_length);
            COPY_THIS(not_much_time_left_disabled_color);
            COPY_THIS(time_out_flash_default_color);
            COPY_THIS(time_out_flash_flashing_color);
            COPY_THIS(time_out_flash_flash_period);
            COPY_THIS(time_out_flash_flash_delay);
            COPY_THIS(time_out_flash_number_of_flashes);
            COPY_THIS(time_out_flash_flash_flags);
            COPY_THIS(time_out_flash_flash_length);
            COPY_THIS(time_out_flash_disabled_color);
            COPY_THIS(carnage_report_bitmap);
            COPY_THIS(loading_begin_text);
            COPY_THIS(loading_end_text);
            COPY_THIS(checkpoint_begin_text);
            COPY_THIS(checkpoint_end_text);
            COPY_THIS(checkpoint_sound);
            return copy;
        }
    };
    static_assert(sizeof(HUDGlobals<BigEndian>) == 0x450);

    void compile_hud_globals_tag(CompiledTag &compiled, const std::byte *data, std::size_t size);
}
#endif
