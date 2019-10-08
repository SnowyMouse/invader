// SPDX-License-Identifier: GPL-3.0-only

#ifndef INVADER__TAG__HEK__CLASS__WEAPON_HUD_INTERFACE_HPP
#define INVADER__TAG__HEK__CLASS__WEAPON_HUD_INTERFACE_HPP

#include "hud_interface_types.hpp"

namespace Invader::HEK {
    enum WeaponHUDInterfaceStateAttachedTo : TagEnum {
        WEAPON_HUD_INTERFACE_STATE_ATTACHED_TO_TOTAL_AMMO,
        WEAPON_HUD_INTERFACE_STATE_ATTACHED_TO_LOADED_AMMO,
        WEAPON_HUD_INTERFACE_STATE_ATTACHED_TO_HEAT,
        WEAPON_HUD_INTERFACE_STATE_ATTACHED_TO_AGE,
        WEAPON_HUD_INTERFACE_STATE_ATTACHED_TO_SECONDARY_WEAPON_TOTAL_AMMO,
        WEAPON_HUD_INTERFACE_STATE_ATTACHED_TO_SECONDARY_WEAPON_LOADED_AMMO,
        WEAPON_HUD_INTERFACE_STATE_ATTACHED_TO_DISTANCE_TO_TARGET,
        WEAPON_HUD_INTERFACE_STATE_ATTACHED_TO_ELEVATION_TO_TARGET
    };

    enum WeaponHUDInterfaceCanUseOnMapType : TagEnum {
        WEAPON_HUD_INTERFACE_CAN_USE_ON_MAP_TYPE_ANY,
        WEAPON_HUD_INTERFACE_CAN_USE_ON_MAP_TYPE_SOLO,
        WEAPON_HUD_INTERFACE_CAN_USE_ON_MAP_TYPE_MULTIPLAYER
    };

    enum WeaponHUDInterfaceCrosshairType : TagEnum {
        WEAPON_HUD_INTERFACE_CROSSHAIR_TYPE_AIM,
        WEAPON_HUD_INTERFACE_CROSSHAIR_TYPE_ZOOM,
        WEAPON_HUD_INTERFACE_CROSSHAIR_TYPE_CHARGE,
        WEAPON_HUD_INTERFACE_CROSSHAIR_TYPE_SHOULD_RELOAD,
        WEAPON_HUD_INTERFACE_CROSSHAIR_TYPE_FLASH_HEAT,
        WEAPON_HUD_INTERFACE_CROSSHAIR_TYPE_FLASH_TOTAL_AMMO,
        WEAPON_HUD_INTERFACE_CROSSHAIR_TYPE_FLASH_BATTERY,
        WEAPON_HUD_INTERFACE_CROSSHAIR_TYPE_RELOAD_OVERHEAT,
        WEAPON_HUD_INTERFACE_CROSSHAIR_TYPE_FLASH_WHEN_FIRING_AND_NO_AMMO,
        WEAPON_HUD_INTERFACE_CROSSHAIR_TYPE_FLASH_WHEN_THROWING_AND_NO_GRENADE,
        WEAPON_HUD_INTERFACE_CROSSHAIR_TYPE_LOW_AMMO_AND_NONE_LEFT_TO_RELOAD,
        WEAPON_HUD_INTERFACE_CROSSHAIR_TYPE_SHOULD_RELOAD_SECONDARY_TRIGGER,
        WEAPON_HUD_INTERFACE_CROSSHAIR_TYPE_FLASH_SECONDARY_TOTAL_AMMO,
        WEAPON_HUD_INTERFACE_CROSSHAIR_TYPE_FLASH_SECONDARY_RELOAD,
        WEAPON_HUD_INTERFACE_CROSSHAIR_TYPE_FLASH_WHEN_FIRING_SECONDARY_TRIGGER_WITH_NO_AMMO,
        WEAPON_HUD_INTERFACE_CROSSHAIR_TYPE_LOW_SECONDARY_AMMO_AND_NONE_LEFT_TO_RELOAD,
        WEAPON_HUD_INTERFACE_CROSSHAIR_TYPE_PRIMARY_TRIGGER_READY,
        WEAPON_HUD_INTERFACE_CROSSHAIR_TYPE_SECONDARY_TRIGGER_READY,
        WEAPON_HUD_INTERFACE_CROSSHAIR_TYPE_FLASH_WHEN_FIRING_WITH_DEPLETED_BATTERY
    };

    ENDIAN_TEMPLATE(EndianType) struct WeaponHUDInterfaceStaticElement {
        EndianType<WeaponHUDInterfaceStateAttachedTo> state_attached_to;
        PAD(0x2);
        EndianType<WeaponHUDInterfaceCanUseOnMapType> can_use_on_map_type;
        PAD(0x2);
        PAD(0x1C);
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
        EndianType<std::int16_t> number_of_flashes;
        EndianType<HUDInterfaceFlashFlags> flash_flags;
        EndianType<float> flash_length;
        EndianType<ColorARGBInt> disabled_color;
        PAD(0x4);
        EndianType<std::int16_t> sequence_index;
        PAD(0x2);
        TagReflexive<EndianType, HUDInterfaceMultitextureOverlay> multitex_overlay;
        PAD(0x4);
        PAD(0x28);

        ENDIAN_TEMPLATE(NewType) operator WeaponHUDInterfaceStaticElement<NewType>() const noexcept {
            WeaponHUDInterfaceStaticElement<NewType> copy = {};
            COPY_THIS(state_attached_to);
            COPY_THIS(can_use_on_map_type);
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
            return copy;
        }
    };
    static_assert(sizeof(WeaponHUDInterfaceStaticElement<BigEndian>) == 0xB4);

    ENDIAN_TEMPLATE(EndianType) struct WeaponHUDInterfaceMeter {
        EndianType<WeaponHUDInterfaceStateAttachedTo> state_attached_to;
        PAD(0x2);
        EndianType<WeaponHUDInterfaceCanUseOnMapType> can_use_on_map_type;
        PAD(0x2);
        PAD(0x1C);
        Point2DInt<EndianType> anchor_offset;
        EndianType<float> width_scale;
        EndianType<float> height_scale;
        EndianType<HUDInterfaceScalingFlags> scaling_flags;
        PAD(0x2);
        PAD(0x14);
        TagDependency<EndianType> meter_bitmap; // bitmap
        EndianType<ColorARGBInt> color_at_meter_minimum;
        EndianType<ColorARGBInt> color_at_meter_maximum;
        EndianType<ColorARGBInt> flash_color;
        EndianType<ColorARGBInt> empty_color;
        EndianType<HUDInterfaceMeterFlags> flags;
        std::int8_t minumum_meter_value;
        EndianType<std::int16_t> sequence_index;
        std::int8_t alpha_multiplier;
        std::int8_t alpha_bias;
        EndianType<std::int16_t> value_scale;
        EndianType<float> opacity;
        EndianType<float> translucency;
        EndianType<ColorARGBInt> disabled_color;
        PAD(0x10);
        PAD(0x28);

        ENDIAN_TEMPLATE(NewType) operator WeaponHUDInterfaceMeter<NewType>() const noexcept {
            WeaponHUDInterfaceMeter<NewType> copy = {};
            COPY_THIS(state_attached_to);
            COPY_THIS(can_use_on_map_type);
            COPY_THIS(anchor_offset);
            COPY_THIS(width_scale);
            COPY_THIS(height_scale);
            COPY_THIS(scaling_flags);
            COPY_THIS(meter_bitmap);
            COPY_THIS(color_at_meter_minimum);
            COPY_THIS(color_at_meter_maximum);
            COPY_THIS(flash_color);
            COPY_THIS(empty_color);
            COPY_THIS(flags);
            COPY_THIS(minumum_meter_value);
            COPY_THIS(sequence_index);
            COPY_THIS(alpha_multiplier);
            COPY_THIS(alpha_bias);
            COPY_THIS(value_scale);
            COPY_THIS(opacity);
            COPY_THIS(translucency);
            COPY_THIS(disabled_color);
            return copy;
        }
    };
    static_assert(sizeof(WeaponHUDInterfaceMeter<BigEndian>) == 0xB4);

    struct WeaponHUDInterfaceNumberWeaponSpecificFlags {
        std::uint16_t divide_number_by_clip_size : 1;
    };

    ENDIAN_TEMPLATE(EndianType) struct WeaponHUDInterfaceNumber {
        EndianType<WeaponHUDInterfaceStateAttachedTo> state_attached_to;
        PAD(0x2);
        EndianType<WeaponHUDInterfaceCanUseOnMapType> can_use_on_map_type;
        PAD(0x2);
        PAD(0x1C);
        Point2DInt<EndianType> anchor_offset;
        EndianType<float> width_scale;
        EndianType<float> height_scale;
        EndianType<HUDInterfaceScalingFlags> scaling_flags;
        PAD(0x2);
        PAD(0x14);
        EndianType<ColorARGBInt> default_color;
        EndianType<ColorARGBInt> flashing_color;
        EndianType<float> flash_period;
        EndianType<float> flash_delay;
        EndianType<std::int16_t> number_of_flashes;
        EndianType<HUDInterfaceFlashFlags> flash_flags;
        EndianType<float> flash_length;
        EndianType<ColorARGBInt> disabled_color;
        PAD(0x4);
        std::int8_t maximum_number_of_digits;
        EndianType<HUDInterfaceNumberFlags> flags;
        std::int8_t number_of_fractional_digits;
        PAD(0x1);
        PAD(0xC);
        EndianType<WeaponHUDInterfaceNumberWeaponSpecificFlags> weapon_specific_flags;
        PAD(0x2);
        PAD(0x24);

        ENDIAN_TEMPLATE(NewType) operator WeaponHUDInterfaceNumber<NewType>() const noexcept {
            WeaponHUDInterfaceNumber<NewType> copy = {};
            COPY_THIS(state_attached_to);
            COPY_THIS(can_use_on_map_type);
            COPY_THIS(anchor_offset);
            COPY_THIS(width_scale);
            COPY_THIS(height_scale);
            COPY_THIS(scaling_flags);
            COPY_THIS(default_color);
            COPY_THIS(flashing_color);
            COPY_THIS(flash_period);
            COPY_THIS(flash_delay);
            COPY_THIS(number_of_flashes);
            COPY_THIS(flash_flags);
            COPY_THIS(flash_length);
            COPY_THIS(disabled_color);
            COPY_THIS(maximum_number_of_digits);
            COPY_THIS(flags);
            COPY_THIS(number_of_fractional_digits);
            COPY_THIS(weapon_specific_flags);
            return copy;
        }
    };
    static_assert(sizeof(WeaponHUDInterfaceNumber<BigEndian>) == 0xA0);

    struct WeaponHUDInterfaceCrosshairOverlayFlags {
        std::uint32_t flashes_when_active : 1;
        std::uint32_t not_a_sprite : 1;
        std::uint32_t show_only_when_zoomed : 1;
        std::uint32_t show_sniper_data : 1;
        std::uint32_t hide_area_outside_reticle : 1;
        std::uint32_t one_zoom_level : 1;
        std::uint32_t don_t_show_when_zoomed : 1;
    };

    ENDIAN_TEMPLATE(EndianType) struct WeaponHUDInterfaceCrosshairOverlay {
        Point2DInt<EndianType> anchor_offset;
        EndianType<float> width_scale;
        EndianType<float> height_scale;
        EndianType<HUDInterfaceScalingFlags> scaling_flags;
        PAD(0x2);
        PAD(0x14);
        EndianType<ColorARGBInt> default_color;
        EndianType<ColorARGBInt> flashing_color;
        EndianType<float> flash_period;
        EndianType<float> flash_delay;
        EndianType<std::int16_t> number_of_flashes;
        EndianType<HUDInterfaceFlashFlags> flash_flags;
        EndianType<float> flash_length;
        EndianType<ColorARGBInt> disabled_color;
        PAD(0x4);
        EndianType<std::int16_t> frame_rate;
        EndianType<std::int16_t> sequence_index;
        EndianType<WeaponHUDInterfaceCrosshairOverlayFlags> flags;
        PAD(0x20);

        ENDIAN_TEMPLATE(NewType) operator WeaponHUDInterfaceCrosshairOverlay<NewType>() const noexcept {
            WeaponHUDInterfaceCrosshairOverlay<NewType> copy = {};
            COPY_THIS(anchor_offset);
            COPY_THIS(width_scale);
            COPY_THIS(height_scale);
            COPY_THIS(scaling_flags);
            COPY_THIS(default_color);
            COPY_THIS(flashing_color);
            COPY_THIS(flash_period);
            COPY_THIS(flash_delay);
            COPY_THIS(number_of_flashes);
            COPY_THIS(flash_flags);
            COPY_THIS(flash_length);
            COPY_THIS(disabled_color);
            COPY_THIS(frame_rate);
            COPY_THIS(sequence_index);
            COPY_THIS(flags);
            return copy;
        }
    };
    static_assert(sizeof(WeaponHUDInterfaceCrosshairOverlay<BigEndian>) == 0x6C);

    ENDIAN_TEMPLATE(EndianType) struct WeaponHUDInterfaceCrosshair {
        EndianType<WeaponHUDInterfaceCrosshairType> crosshair_type;
        PAD(0x2);
        EndianType<WeaponHUDInterfaceCanUseOnMapType> can_use_on_map_type;
        PAD(0x2);
        PAD(0x1C);
        TagDependency<EndianType> crosshair_bitmap; // bitmap
        TagReflexive<EndianType, WeaponHUDInterfaceCrosshairOverlay> crosshair_overlays;
        PAD(0x28);

        ENDIAN_TEMPLATE(NewType) operator WeaponHUDInterfaceCrosshair<NewType>() const noexcept {
            WeaponHUDInterfaceCrosshair<NewType> copy = {};
            COPY_THIS(crosshair_type);
            COPY_THIS(can_use_on_map_type);
            COPY_THIS(crosshair_bitmap);
            COPY_THIS(crosshair_overlays);
            return copy;
        }
    };
    static_assert(sizeof(WeaponHUDInterfaceCrosshair<BigEndian>) == 0x68);

    struct WeaponHUDInterfaceOverlayType {
        std::uint16_t show_on_flashing : 1;
        std::uint16_t show_on_empty : 1;
        std::uint16_t show_on_reload_overheating : 1;
        std::uint16_t show_on_default : 1;
        std::uint16_t show_always : 1;
    };

    ENDIAN_TEMPLATE(EndianType) struct WeaponHUDInterfaceOverlay {
        Point2DInt<EndianType> anchor_offset;
        EndianType<float> width_scale;
        EndianType<float> height_scale;
        EndianType<HUDInterfaceScalingFlags> scaling_flags;
        PAD(0x2);
        PAD(0x14);
        EndianType<ColorARGBInt> default_color;
        EndianType<ColorARGBInt> flashing_color;
        EndianType<float> flash_period;
        EndianType<float> flash_delay;
        EndianType<std::int16_t> number_of_flashes;
        EndianType<HUDInterfaceFlashFlags> flash_flags;
        EndianType<float> flash_length;
        EndianType<ColorARGBInt> disabled_color;
        PAD(0x4);
        EndianType<std::int16_t> frame_rate;
        PAD(0x2);
        EndianType<std::int16_t> sequence_index;
        EndianType<WeaponHUDInterfaceOverlayType> type;
        EndianType<HUDInterfaceOverlayFlashFlags> flags;
        PAD(0x10);
        PAD(0x28);

        ENDIAN_TEMPLATE(NewType) operator WeaponHUDInterfaceOverlay<NewType>() const noexcept {
            WeaponHUDInterfaceOverlay<NewType> copy = {};
            COPY_THIS(anchor_offset);
            COPY_THIS(width_scale);
            COPY_THIS(height_scale);
            COPY_THIS(scaling_flags);
            COPY_THIS(default_color);
            COPY_THIS(flashing_color);
            COPY_THIS(flash_period);
            COPY_THIS(flash_delay);
            COPY_THIS(number_of_flashes);
            COPY_THIS(flash_flags);
            COPY_THIS(flash_length);
            COPY_THIS(disabled_color);
            COPY_THIS(frame_rate);
            COPY_THIS(sequence_index);
            COPY_THIS(type);
            COPY_THIS(flags);
            return copy;
        }
    };
    static_assert(sizeof(WeaponHUDInterfaceOverlay<BigEndian>) == 0x88);

    ENDIAN_TEMPLATE(EndianType) struct WeaponHUDInterfaceOverlayElement {
        EndianType<WeaponHUDInterfaceStateAttachedTo> state_attached_to;
        PAD(0x2);
        EndianType<WeaponHUDInterfaceCanUseOnMapType> can_use_on_map_type;
        PAD(0x2);
        PAD(0x1C);
        TagDependency<EndianType> overlay_bitmap; // bitmap
        TagReflexive<EndianType, WeaponHUDInterfaceOverlay> overlays;
        PAD(0x28);

        ENDIAN_TEMPLATE(NewType) operator WeaponHUDInterfaceOverlayElement<NewType>() const noexcept {
            WeaponHUDInterfaceOverlayElement<NewType> copy = {};
            COPY_THIS(state_attached_to);
            COPY_THIS(can_use_on_map_type);
            COPY_THIS(overlay_bitmap);
            COPY_THIS(overlays);
            return copy;
        }
    };
    static_assert(sizeof(WeaponHUDInterfaceOverlayElement<BigEndian>) == 0x68);

    struct WeaponHUDInterfaceScreenEffectDefinitionMaskFlags {
        std::uint16_t only_when_zoomed : 1;
    };

    struct WeaponHUDInterfaceScreenEffectDefinitionConvolutionFlags {
        std::uint16_t only_when_zoomed : 1;
    };

    struct WeaponHUDInterfaceScreenEffectDefinitionNightVisionFlags {
        std::uint16_t only_when_zoomed : 1;
        std::uint16_t connect_to_flashlight : 1;
        std::uint16_t masked : 1;
    };

    struct WeaponHUDInterfaceScreenEffectDefinitionDesaturationFlags {
        std::uint16_t only_when_zoomed : 1;
        std::uint16_t connect_to_flashlight : 1;
        std::uint16_t additive : 1;
        std::uint16_t masked : 1;
    };

    ENDIAN_TEMPLATE(EndianType) struct WeaponHUDInterfaceScreenEffect {
        PAD(0x4);
        EndianType<WeaponHUDInterfaceScreenEffectDefinitionMaskFlags> mask_flags;
        PAD(0x2);
        PAD(0x10);
        TagDependency<EndianType> mask_fullscreen; // bitmap
        TagDependency<EndianType> mask_splitscreen; // bitmap
        PAD(0x8);
        EndianType<WeaponHUDInterfaceScreenEffectDefinitionConvolutionFlags> convolution_flags;
        PAD(0x2);
        Bounds<EndianType<float>> convolution_fov_in_bounds;
        Bounds<EndianType<float>> convolution_radius_out_bounds;
        PAD(0x18);
        EndianType<WeaponHUDInterfaceScreenEffectDefinitionNightVisionFlags> even_more_flags;
        EndianType<std::int16_t> night_vision_script_source;
        EndianType<Fraction> night_vision_intensity;
        PAD(0x18);
        EndianType<WeaponHUDInterfaceScreenEffectDefinitionDesaturationFlags> desaturation_flags;
        EndianType<std::int16_t> desaturation_script_source;
        EndianType<Fraction> desaturation_intensity;
        ColorRGB<EndianType> effect_tint;
        PAD(0x18);

        ENDIAN_TEMPLATE(NewType) operator WeaponHUDInterfaceScreenEffect<NewType>() const noexcept {
            WeaponHUDInterfaceScreenEffect<NewType> copy = {};
            COPY_THIS(mask_flags);
            COPY_THIS(mask_fullscreen);
            COPY_THIS(mask_splitscreen);
            COPY_THIS(convolution_flags);
            COPY_THIS(convolution_fov_in_bounds);
            COPY_THIS(convolution_radius_out_bounds);
            COPY_THIS(even_more_flags);
            COPY_THIS(night_vision_script_source);
            COPY_THIS(night_vision_intensity);
            COPY_THIS(desaturation_flags);
            COPY_THIS(desaturation_script_source);
            COPY_THIS(desaturation_intensity);
            COPY_THIS(effect_tint);
            return copy;
        }
    };
    static_assert(sizeof(WeaponHUDInterfaceScreenEffect<BigEndian>) == 0xB8);

    struct WeaponHUDInterfaceFlags {
        std::uint16_t use_parent_hud_flashing_parameters : 1;
    };

    ENDIAN_TEMPLATE(EndianType) struct WeaponHUDInterface {
        TagDependency<EndianType> child_hud; // weapon_hud_interface
        EndianType<WeaponHUDInterfaceFlags> flags;
        PAD(0x2);
        EndianType<std::int16_t> total_ammo_cutoff;
        EndianType<std::int16_t> loaded_ammo_cutoff;
        EndianType<std::int16_t> heat_cutoff;
        EndianType<std::int16_t> age_cutoff;
        PAD(0x20);
        EndianType<HUDInterfaceAnchor> anchor;
        PAD(0x2);
        PAD(0x20);
        TagReflexive<EndianType, WeaponHUDInterfaceStaticElement> static_elements;
        TagReflexive<EndianType, WeaponHUDInterfaceMeter> meter_elements;
        TagReflexive<EndianType, WeaponHUDInterfaceNumber> number_elements;
        TagReflexive<EndianType, WeaponHUDInterfaceCrosshair> crosshairs;
        TagReflexive<EndianType, WeaponHUDInterfaceOverlayElement> overlay_elements;
        LittleEndian<std::uint32_t> crosshair_types;
        PAD(0xC);
        TagReflexive<EndianType, WeaponHUDInterfaceScreenEffect> screen_effect;
        PAD(0x84);
        EndianType<std::int16_t> sequence_index;
        EndianType<std::int16_t> width_offset;
        Point2DInt<EndianType> offset_from_reference_corner;
        EndianType<ColorARGBInt> override_icon_color;
        std::int8_t frame_rate;
        HUDInterfaceMessagingFlags messaging_flags;
        EndianType<std::int16_t> text_index;
        PAD(0x30);

        ENDIAN_TEMPLATE(NewType) operator WeaponHUDInterface<NewType>() const noexcept {
            WeaponHUDInterface<NewType> copy = {};
            COPY_THIS(child_hud);
            COPY_THIS(flags);
            COPY_THIS(total_ammo_cutoff);
            COPY_THIS(loaded_ammo_cutoff);
            COPY_THIS(heat_cutoff);
            COPY_THIS(age_cutoff);
            COPY_THIS(anchor);
            COPY_THIS(static_elements);
            COPY_THIS(meter_elements);
            COPY_THIS(number_elements);
            COPY_THIS(crosshairs);
            COPY_THIS(overlay_elements);
            COPY_THIS(crosshair_types);
            COPY_THIS(screen_effect);
            COPY_THIS(sequence_index);
            COPY_THIS(width_offset);
            COPY_THIS(offset_from_reference_corner);
            COPY_THIS(override_icon_color);
            COPY_THIS(frame_rate);
            COPY_THIS(messaging_flags);
            COPY_THIS(text_index);
            return copy;
        }
    };
    static_assert(sizeof(WeaponHUDInterface<BigEndian>) == 0x17C);

    void compile_weapon_hud_interface_tag(CompiledTag &compiled, const std::byte *data, std::size_t size);
}
#endif
