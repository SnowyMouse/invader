// SPDX-License-Identifier: GPL-3.0-only

#ifndef INVADER__TAG__HEK__CLASS__UNIT_HUD_INTERFACE_HPP
#define INVADER__TAG__HEK__CLASS__UNIT_HUD_INTERFACE_HPP

#include "hud_interface_types.hpp"

namespace Invader::HEK {
    enum UnitHUDInterfacePanelType : TagEnum {
        UNIT_HUD_INTERFACE_PANEL_INTEGRATED_LIGHT
    };

    struct UnitHUDInterfaceAuxiliaryOverlayFlags {
        std::uint16_t use_team_color : 1;
    };

    ENDIAN_TEMPLATE(EndianType) struct UnitHUDInterfaceAuxiliaryOverlay {
        Point2DInt<EndianType> anchor_offset;
        EndianType<float> width_scale;
        EndianType<float> height_scale;
        EndianType<HUDInterfaceScalingFlags> scaling_flags;
        PAD(0x2);
        PAD(0x14);
        TagDependency<EndianType> interface_bitmap; // bitmap
        EndianType<ColorARGBInt> default_color;
        EndianType<ColorARGBInt> flashing_color;
        EndianType<float> flash_period;
        EndianType<float> flash_delay;
        EndianType<std::uint16_t> number_of_flashes;
        EndianType<HUDInterfaceFlashFlags> flash_flags;
        EndianType<float> flash_length;
        EndianType<ColorARGBInt> disabled_color;
        PAD(0x4);
        EndianType<Index> sequence_index;
        PAD(0x2);
        TagReflexive<EndianType, HUDInterfaceMultitextureOverlay> multitex_overlay;
        PAD(0x4);
        EndianType<UnitHUDInterfacePanelType> type;
        EndianType<UnitHUDInterfaceAuxiliaryOverlayFlags> flags;
        PAD(0x18);

        ENDIAN_TEMPLATE(NewType) operator UnitHUDInterfaceAuxiliaryOverlay<NewType>() const noexcept {
            UnitHUDInterfaceAuxiliaryOverlay<NewType> copy = {};
            COPY_THIS(anchor_offset);
            COPY_THIS(width_scale);
            COPY_THIS(height_scale);
            COPY_THIS(scaling_flags);
            COPY_THIS(interface_bitmap);
            COPY_THIS(default_color);
            COPY_THIS(flashing_color);
            COPY_THIS(flash_period);
            COPY_THIS(flash_delay);
            COPY_THIS(number_of_flashes);
            COPY_THIS(flash_flags);
            COPY_THIS(flash_length);
            COPY_THIS(disabled_color);
            COPY_THIS(sequence_index);
            COPY_THIS(multitex_overlay);
            COPY_THIS(type);
            COPY_THIS(flags);
            return copy;
        }
    };
    static_assert(sizeof(UnitHUDInterfaceAuxiliaryOverlay<BigEndian>) == 0x84);

    struct UnitHUDInterfaceHUDSoundLatchedTo {
        std::uint32_t shield_recharging : 1;
        std::uint32_t shield_damaged : 1;
        std::uint32_t shield_low : 1;
        std::uint32_t shield_empty : 1;
        std::uint32_t health_low : 1;
        std::uint32_t health_empty : 1;
        std::uint32_t health_minor_damage : 1;
        std::uint32_t health_major_damage : 1;
    };

    ENDIAN_TEMPLATE(EndianType) struct UnitHUDInterfaceHUDSound {
        TagDependency<EndianType> sound; // sound, sound_looping
        EndianType<UnitHUDInterfaceHUDSoundLatchedTo> latched_to;
        EndianType<float> scale;
        PAD(0x20);

        ENDIAN_TEMPLATE(NewType) operator UnitHUDInterfaceHUDSound<NewType>() const noexcept {
            UnitHUDInterfaceHUDSound<NewType> copy = {};
            COPY_THIS(sound);
            COPY_THIS(latched_to);
            COPY_THIS(scale);
            return copy;
        }
    };
    static_assert(sizeof(UnitHUDInterfaceHUDSound<BigEndian>) == 0x38);

    struct UnitHUDInterfaceFlags {
        std::uint8_t use_min_max_for_state_changes : 1;
        std::uint8_t interpolate_between_min_max_flash_colors_as_state_changes : 1;
        std::uint8_t interpolate_color_along_hsv_space : 1;
        std::uint8_t _more_colors_for_hsv_interpolation : 1;
        std::uint8_t invert_interpolation : 1;
    };

    struct UnitHUDInterfaceAuxiliaryPanelMeterMoreFlags {
        std::uint32_t show_only_when_active : 1;
        std::uint32_t flash_once_if_activated_while_disabled : 1;
    };

    ENDIAN_TEMPLATE(EndianType) struct UnitHUDInterfaceAuxiliaryPanel {
        EndianType<UnitHUDInterfacePanelType> type;
        PAD(0x2);
        PAD(0x10);
        Point2DInt<EndianType> background_anchor_offset;
        EndianType<float> background_width_scale;
        EndianType<float> background_height_scale;
        EndianType<HUDInterfaceScalingFlags> background_scaling_flags;
        PAD(0x2);
        PAD(0x14);
        TagDependency<EndianType> background_interface_bitmap; // bitmap
        EndianType<ColorARGBInt> background_default_color;
        EndianType<ColorARGBInt> background_flashing_color;
        EndianType<float> background_flash_period;
        EndianType<float> background_flash_delay;
        EndianType<std::uint16_t> background_number_of_flashes;
        EndianType<HUDInterfaceFlashFlags> background_flash_flags;
        EndianType<float> background_flash_length;
        EndianType<ColorARGBInt> background_disabled_color;
        PAD(0x4);
        EndianType<Index> background_sequence_index;
        PAD(0x2);
        TagReflexive<EndianType, HUDInterfaceMultitextureOverlay> background_multitex_overlay;
        PAD(0x4);
        Point2DInt<EndianType> meter_anchor_offset;
        EndianType<float> meter_width_scale;
        EndianType<float> meter_height_scale;
        EndianType<HUDInterfaceScalingFlags> meter_scaling_flags;
        PAD(0x2);
        PAD(0x14);
        TagDependency<EndianType> meter_meter_bitmap; // bitmap
        EndianType<ColorARGBInt> meter_color_at_meter_minimum;
        EndianType<ColorARGBInt> meter_color_at_meter_maximum;
        EndianType<ColorARGBInt> meter_flash_color;
        EndianType<ColorARGBInt> meter_empty_color;
        EndianType<UnitHUDInterfaceFlags> meter_flags;
        std::int8_t meter_minimum_meter_value;
        EndianType<Index> meter_sequence_index;
        std::int8_t meter_alpha_multiplier;
        std::int8_t meter_alpha_bias;
        EndianType<std::int16_t> meter_value_scale;
        EndianType<float> meter_opacity;
        EndianType<float> meter_translucency;
        EndianType<ColorARGBInt> meter_disabled_color;
        PAD(0x10);
        EndianType<float> meter_minimum_fraction_cutoff;
        EndianType<UnitHUDInterfaceAuxiliaryPanelMeterMoreFlags> meter_more_flags;
        PAD(0x18);
        PAD(0x40);

        ENDIAN_TEMPLATE(NewType) operator UnitHUDInterfaceAuxiliaryPanel<NewType>() const noexcept {
            UnitHUDInterfaceAuxiliaryPanel<NewType> copy = {};
            COPY_THIS(type);
            COPY_THIS(background_anchor_offset);
            COPY_THIS(background_width_scale);
            COPY_THIS(background_height_scale);
            COPY_THIS(background_scaling_flags);
            COPY_THIS(background_interface_bitmap);
            COPY_THIS(background_default_color);
            COPY_THIS(background_flashing_color);
            COPY_THIS(background_flash_period);
            COPY_THIS(background_flash_delay);
            COPY_THIS(background_number_of_flashes);
            COPY_THIS(background_flash_flags);
            COPY_THIS(background_flash_length);
            COPY_THIS(background_disabled_color);
            COPY_THIS(background_sequence_index);
            COPY_THIS(background_multitex_overlay);
            COPY_THIS(meter_anchor_offset);
            COPY_THIS(meter_width_scale);
            COPY_THIS(meter_height_scale);
            COPY_THIS(meter_scaling_flags);
            COPY_THIS(meter_meter_bitmap);
            COPY_THIS(meter_color_at_meter_minimum);
            COPY_THIS(meter_color_at_meter_maximum);
            COPY_THIS(meter_flash_color);
            COPY_THIS(meter_empty_color);
            COPY_THIS(meter_flags);
            COPY_THIS(meter_minimum_meter_value);
            COPY_THIS(meter_sequence_index);
            COPY_THIS(meter_alpha_multiplier);
            COPY_THIS(meter_alpha_bias);
            COPY_THIS(meter_value_scale);
            COPY_THIS(meter_opacity);
            COPY_THIS(meter_translucency);
            COPY_THIS(meter_disabled_color);
            COPY_THIS(meter_minimum_fraction_cutoff);
            COPY_THIS(meter_more_flags);
            return copy;
        }
    };
    static_assert(sizeof(UnitHUDInterfaceAuxiliaryPanel<BigEndian>) == 0x144);

    ENDIAN_TEMPLATE(EndianType) struct UnitHUDInterface {
        EndianType<HUDInterfaceAnchor> anchor;
        PAD(0x2);
        PAD(0x20);
        Point2DInt<EndianType> hud_background_anchor_offset;
        EndianType<float> hud_background_width_scale;
        EndianType<float> hud_background_height_scale;
        EndianType<HUDInterfaceScalingFlags> hud_background_scaling_flags;
        PAD(0x2);
        PAD(0x14);
        TagDependency<EndianType> hud_background_interface_bitmap; // bitmap
        EndianType<ColorARGBInt> hud_background_default_color;
        EndianType<ColorARGBInt> hud_background_flashing_color;
        EndianType<float> hud_background_flash_period;
        EndianType<float> hud_background_flash_delay;
        EndianType<std::uint16_t> hud_background_number_of_flashes;
        EndianType<HUDInterfaceFlashFlags> hud_background_flash_flags;
        EndianType<float> hud_background_flash_length;
        EndianType<ColorARGBInt> hud_background_disabled_color;
        PAD(0x4);
        EndianType<Index> hud_background_sequence_index;
        PAD(0x2);
        TagReflexive<EndianType, HUDInterfaceMultitextureOverlay> hud_background_multitex_overlay;
        PAD(0x4);
        Point2DInt<EndianType> shield_panel_background_anchor_offset;
        EndianType<float> shield_panel_background_width_scale;
        EndianType<float> shield_panel_background_height_scale;
        EndianType<HUDInterfaceScalingFlags> shield_panel_background_scaling_flags;
        PAD(0x2);
        PAD(0x14);
        TagDependency<EndianType> shield_panel_background_interface_bitmap; // bitmap
        EndianType<ColorARGBInt> shield_panel_background_default_color;
        EndianType<ColorARGBInt> shield_panel_background_flashing_color;
        EndianType<float> shield_panel_background_flash_period;
        EndianType<float> shield_panel_background_flash_delay;
        EndianType<std::uint16_t> shield_panel_background_number_of_flashes;
        EndianType<HUDInterfaceFlashFlags> shield_panel_background_flash_flags;
        EndianType<float> shield_panel_background_flash_length;
        EndianType<ColorARGBInt> shield_panel_background_disabled_color;
        PAD(0x4);
        EndianType<Index> shield_panel_background_sequence_index;
        PAD(0x2);
        TagReflexive<EndianType, HUDInterfaceMultitextureOverlay> shield_panel_background_multitex_overlay;
        PAD(0x4);
        Point2DInt<EndianType> shield_panel_meter_anchor_offset;
        EndianType<float> shield_panel_meter_width_scale;
        EndianType<float> shield_panel_meter_height_scale;
        EndianType<HUDInterfaceScalingFlags> shield_panel_meter_scaling_flags;
        PAD(0x2);
        PAD(0x14);
        TagDependency<EndianType> shield_panel_meter_meter_bitmap; // bitmap
        EndianType<ColorARGBInt> shield_panel_meter_color_at_meter_minimum;
        EndianType<ColorARGBInt> shield_panel_meter_color_at_meter_maximum;
        EndianType<ColorARGBInt> shield_panel_meter_flash_color;
        EndianType<ColorARGBInt> shield_panel_meter_empty_color;
        EndianType<UnitHUDInterfaceFlags> shield_panel_meter_flags;
        std::int8_t shield_panel_meter_minimum_meter_value;
        EndianType<Index> shield_panel_meter_sequence_index;
        std::int8_t shield_panel_meter_alpha_multiplier;
        std::int8_t shield_panel_meter_alpha_bias;
        EndianType<std::int16_t> shield_panel_meter_value_scale;
        EndianType<float> shield_panel_meter_opacity;
        EndianType<float> shield_panel_meter_translucency;
        EndianType<ColorARGBInt> shield_panel_meter_disabled_color;
        PAD(0x10);
        EndianType<ColorARGBInt> shield_panel_meter_overcharge_minimum_color;
        EndianType<ColorARGBInt> shield_panel_meter_overcharge_maximum_color;
        EndianType<ColorARGBInt> shield_panel_meter_overcharge_flash_color;
        EndianType<ColorARGBInt> shield_panel_meter_overcharge_empty_color;
        PAD(0x10);
        Point2DInt<EndianType> health_panel_background_anchor_offset;
        EndianType<float> health_panel_background_width_scale;
        EndianType<float> health_panel_background_height_scale;
        EndianType<HUDInterfaceScalingFlags> health_panel_background_scaling_flags;
        PAD(0x2);
        PAD(0x14);
        TagDependency<EndianType> health_panel_background_interface_bitmap; // bitmap
        EndianType<ColorARGBInt> health_panel_background_default_color;
        EndianType<ColorARGBInt> health_panel_background_flashing_color;
        EndianType<float> health_panel_background_flash_period;
        EndianType<float> health_panel_background_flash_delay;
        EndianType<std::uint16_t> health_panel_background_number_of_flashes;
        EndianType<HUDInterfaceFlashFlags> health_panel_background_flash_flags;
        EndianType<float> health_panel_background_flash_length;
        EndianType<ColorARGBInt> health_panel_background_disabled_color;
        PAD(0x4);
        EndianType<Index> health_panel_background_sequence_index;
        PAD(0x2);
        TagReflexive<EndianType, HUDInterfaceMultitextureOverlay> health_panel_background_multitex_overlay;
        PAD(0x4);
        Point2DInt<EndianType> health_panel_meter_anchor_offset;
        EndianType<float> health_panel_meter_width_scale;
        EndianType<float> health_panel_meter_height_scale;
        EndianType<HUDInterfaceScalingFlags> health_panel_meter_scaling_flags;
        PAD(0x2);
        PAD(0x14);
        TagDependency<EndianType> health_panel_meter_meter_bitmap; // bitmap
        EndianType<ColorARGBInt> health_panel_meter_color_at_meter_minimum;
        EndianType<ColorARGBInt> health_panel_meter_color_at_meter_maximum;
        EndianType<ColorARGBInt> health_panel_meter_flash_color;
        EndianType<ColorARGBInt> health_panel_meter_empty_color;
        EndianType<UnitHUDInterfaceFlags> health_panel_meter_flags;
        std::int8_t health_panel_meter_minimum_meter_value;
        EndianType<Index> health_panel_meter_sequence_index;
        std::int8_t health_panel_meter_alpha_multiplier;
        std::int8_t health_panel_meter_alpha_bias;
        EndianType<std::int16_t> health_panel_meter_value_scale;
        EndianType<float> health_panel_meter_opacity;
        EndianType<float> health_panel_meter_translucency;
        EndianType<ColorARGBInt> health_panel_meter_disabled_color;
        PAD(0x10);
        EndianType<ColorARGBInt> health_panel_meter_medium_health_left_color;
        EndianType<float> health_panel_meter_max_color_health_fraction_cutoff;
        EndianType<float> health_panel_meter_min_color_health_fraction_cutoff;
        PAD(0x14);
        Point2DInt<EndianType> motion_sensor_background_anchor_offset;
        EndianType<float> motion_sensor_background_width_scale;
        EndianType<float> motion_sensor_background_height_scale;
        EndianType<HUDInterfaceScalingFlags> motion_sensor_background_scaling_flags;
        PAD(0x2);
        PAD(0x14);
        TagDependency<EndianType> motion_sensor_background_interface_bitmap; // bitmap
        EndianType<ColorARGBInt> motion_sensor_background_default_color;
        EndianType<ColorARGBInt> motion_sensor_background_flashing_color;
        EndianType<float> motion_sensor_background_flash_period;
        EndianType<float> motion_sensor_background_flash_delay;
        EndianType<std::uint16_t> motion_sensor_background_number_of_flashes;
        EndianType<HUDInterfaceFlashFlags> motion_sensor_background_flash_flags;
        EndianType<float> motion_sensor_background_flash_length;
        EndianType<ColorARGBInt> motion_sensor_background_disabled_color;
        PAD(0x4);
        EndianType<Index> motion_sensor_background_sequence_index;
        PAD(0x2);
        TagReflexive<EndianType, HUDInterfaceMultitextureOverlay> motion_sensor_background_multitex_overlays;
        PAD(0x4);
        Point2DInt<EndianType> motion_sensor_foreground_anchor_offset;
        EndianType<float> motion_sensor_foreground_width_scale;
        EndianType<float> motion_sensor_foreground_height_scale;
        EndianType<HUDInterfaceScalingFlags> motion_sensor_foreground_scaling_flags;
        PAD(0x2);
        PAD(0x14);
        TagDependency<EndianType> motion_sensor_foreground_interface_bitmap; // bitmap
        EndianType<ColorARGBInt> motion_sensor_foreground_default_color;
        EndianType<ColorARGBInt> motion_sensor_foreground_flashing_color;
        EndianType<float> motion_sensor_foreground_flash_period;
        EndianType<float> motion_sensor_foreground_flash_delay;
        EndianType<std::uint16_t> motion_sensor_foreground_number_of_flashes;
        EndianType<HUDInterfaceFlashFlags> motion_sensor_foreground_flash_flags;
        EndianType<float> motion_sensor_foreground_flash_length;
        EndianType<ColorARGBInt> motion_sensor_foreground_disabled_color;
        PAD(0x4);
        EndianType<Index> motion_sensor_foreground_sequence_index;
        PAD(0x2);
        TagReflexive<EndianType, HUDInterfaceMultitextureOverlay> motion_sensor_foreground_multitex_overlays;
        PAD(0x4);
        PAD(0x20);
        Point2DInt<EndianType> motion_sensor_center_anchor_offset;
        EndianType<float> motion_sensor_center_width_scale;
        EndianType<float> motion_sensor_center_height_scale;
        EndianType<HUDInterfaceScalingFlags> motion_sensor_center_scaling_flags;
        PAD(0x2);
        PAD(0x14);
        EndianType<HUDInterfaceAnchor> auxiliary_overlay_anchor;
        PAD(0x2);
        PAD(0x20);
        TagReflexive<EndianType, UnitHUDInterfaceAuxiliaryOverlay> overlays;
        PAD(0x10);
        TagReflexive<EndianType, UnitHUDInterfaceHUDSound> sounds;
        TagReflexive<EndianType, UnitHUDInterfaceAuxiliaryPanel> meters;
        PAD(0x164);
        PAD(0x30);

        ENDIAN_TEMPLATE(NewType) operator UnitHUDInterface<NewType>() const noexcept {
            UnitHUDInterface<NewType> copy = {};
            COPY_THIS(anchor);
            COPY_THIS(hud_background_anchor_offset);
            COPY_THIS(hud_background_width_scale);
            COPY_THIS(hud_background_height_scale);
            COPY_THIS(hud_background_scaling_flags);
            COPY_THIS(hud_background_interface_bitmap);
            COPY_THIS(hud_background_default_color);
            COPY_THIS(hud_background_flashing_color);
            COPY_THIS(hud_background_flash_period);
            COPY_THIS(hud_background_flash_delay);
            COPY_THIS(hud_background_number_of_flashes);
            COPY_THIS(hud_background_flash_flags);
            COPY_THIS(hud_background_flash_length);
            COPY_THIS(hud_background_disabled_color);
            COPY_THIS(hud_background_sequence_index);
            COPY_THIS(hud_background_multitex_overlay);
            COPY_THIS(shield_panel_background_anchor_offset);
            COPY_THIS(shield_panel_background_width_scale);
            COPY_THIS(shield_panel_background_height_scale);
            COPY_THIS(shield_panel_background_scaling_flags);
            COPY_THIS(shield_panel_background_interface_bitmap);
            COPY_THIS(shield_panel_background_default_color);
            COPY_THIS(shield_panel_background_flashing_color);
            COPY_THIS(shield_panel_background_flash_period);
            COPY_THIS(shield_panel_background_flash_delay);
            COPY_THIS(shield_panel_background_number_of_flashes);
            COPY_THIS(shield_panel_background_flash_flags);
            COPY_THIS(shield_panel_background_flash_length);
            COPY_THIS(shield_panel_background_disabled_color);
            COPY_THIS(shield_panel_background_sequence_index);
            COPY_THIS(shield_panel_background_multitex_overlay);
            COPY_THIS(shield_panel_meter_anchor_offset);
            COPY_THIS(shield_panel_meter_width_scale);
            COPY_THIS(shield_panel_meter_height_scale);
            COPY_THIS(shield_panel_meter_scaling_flags);
            COPY_THIS(shield_panel_meter_meter_bitmap);
            COPY_THIS(shield_panel_meter_color_at_meter_minimum);
            COPY_THIS(shield_panel_meter_color_at_meter_maximum);
            COPY_THIS(shield_panel_meter_flash_color);
            COPY_THIS(shield_panel_meter_empty_color);
            COPY_THIS(shield_panel_meter_flags);
            COPY_THIS(shield_panel_meter_minimum_meter_value);
            COPY_THIS(shield_panel_meter_sequence_index);
            COPY_THIS(shield_panel_meter_alpha_multiplier);
            COPY_THIS(shield_panel_meter_alpha_bias);
            COPY_THIS(shield_panel_meter_value_scale);
            COPY_THIS(shield_panel_meter_opacity);
            COPY_THIS(shield_panel_meter_translucency);
            COPY_THIS(shield_panel_meter_disabled_color);
            COPY_THIS(shield_panel_meter_overcharge_minimum_color);
            COPY_THIS(shield_panel_meter_overcharge_maximum_color);
            COPY_THIS(shield_panel_meter_overcharge_flash_color);
            COPY_THIS(shield_panel_meter_overcharge_empty_color);
            COPY_THIS(health_panel_background_anchor_offset);
            COPY_THIS(health_panel_background_width_scale);
            COPY_THIS(health_panel_background_height_scale);
            COPY_THIS(health_panel_background_scaling_flags);
            COPY_THIS(health_panel_background_interface_bitmap);
            COPY_THIS(health_panel_background_default_color);
            COPY_THIS(health_panel_background_flashing_color);
            COPY_THIS(health_panel_background_flash_period);
            COPY_THIS(health_panel_background_flash_delay);
            COPY_THIS(health_panel_background_number_of_flashes);
            COPY_THIS(health_panel_background_flash_flags);
            COPY_THIS(health_panel_background_flash_length);
            COPY_THIS(health_panel_background_disabled_color);
            COPY_THIS(health_panel_background_sequence_index);
            COPY_THIS(health_panel_background_multitex_overlay);
            COPY_THIS(health_panel_meter_anchor_offset);
            COPY_THIS(health_panel_meter_width_scale);
            COPY_THIS(health_panel_meter_height_scale);
            COPY_THIS(health_panel_meter_scaling_flags);
            COPY_THIS(health_panel_meter_meter_bitmap);
            COPY_THIS(health_panel_meter_color_at_meter_minimum);
            COPY_THIS(health_panel_meter_color_at_meter_maximum);
            COPY_THIS(health_panel_meter_flash_color);
            COPY_THIS(health_panel_meter_empty_color);
            COPY_THIS(health_panel_meter_flags);
            COPY_THIS(health_panel_meter_minimum_meter_value);
            COPY_THIS(health_panel_meter_sequence_index);
            COPY_THIS(health_panel_meter_alpha_multiplier);
            COPY_THIS(health_panel_meter_alpha_bias);
            COPY_THIS(health_panel_meter_value_scale);
            COPY_THIS(health_panel_meter_opacity);
            COPY_THIS(health_panel_meter_translucency);
            COPY_THIS(health_panel_meter_disabled_color);
            COPY_THIS(health_panel_meter_medium_health_left_color);
            COPY_THIS(health_panel_meter_max_color_health_fraction_cutoff);
            COPY_THIS(health_panel_meter_min_color_health_fraction_cutoff);
            COPY_THIS(motion_sensor_background_anchor_offset);
            COPY_THIS(motion_sensor_background_width_scale);
            COPY_THIS(motion_sensor_background_height_scale);
            COPY_THIS(motion_sensor_background_scaling_flags);
            COPY_THIS(motion_sensor_background_interface_bitmap);
            COPY_THIS(motion_sensor_background_default_color);
            COPY_THIS(motion_sensor_background_flashing_color);
            COPY_THIS(motion_sensor_background_flash_period);
            COPY_THIS(motion_sensor_background_flash_delay);
            COPY_THIS(motion_sensor_background_number_of_flashes);
            COPY_THIS(motion_sensor_background_flash_flags);
            COPY_THIS(motion_sensor_background_flash_length);
            COPY_THIS(motion_sensor_background_disabled_color);
            COPY_THIS(motion_sensor_background_sequence_index);
            COPY_THIS(motion_sensor_background_multitex_overlays);
            COPY_THIS(motion_sensor_foreground_anchor_offset);
            COPY_THIS(motion_sensor_foreground_width_scale);
            COPY_THIS(motion_sensor_foreground_height_scale);
            COPY_THIS(motion_sensor_foreground_scaling_flags);
            COPY_THIS(motion_sensor_foreground_interface_bitmap);
            COPY_THIS(motion_sensor_foreground_default_color);
            COPY_THIS(motion_sensor_foreground_flashing_color);
            COPY_THIS(motion_sensor_foreground_flash_period);
            COPY_THIS(motion_sensor_foreground_flash_delay);
            COPY_THIS(motion_sensor_foreground_number_of_flashes);
            COPY_THIS(motion_sensor_foreground_flash_flags);
            COPY_THIS(motion_sensor_foreground_flash_length);
            COPY_THIS(motion_sensor_foreground_disabled_color);
            COPY_THIS(motion_sensor_foreground_sequence_index);
            COPY_THIS(motion_sensor_foreground_multitex_overlays);
            COPY_THIS(motion_sensor_center_anchor_offset);
            COPY_THIS(motion_sensor_center_width_scale);
            COPY_THIS(motion_sensor_center_height_scale);
            COPY_THIS(motion_sensor_center_scaling_flags);
            COPY_THIS(auxiliary_overlay_anchor);
            COPY_THIS(overlays);
            COPY_THIS(sounds);
            COPY_THIS(meters);
            return copy;
        }
    };
    static_assert(sizeof(UnitHUDInterface<BigEndian>) == 0x56C);

    void compile_unit_hud_interface_tag(CompiledTag &compiled, const std::byte *data, std::size_t size);
}
#endif
