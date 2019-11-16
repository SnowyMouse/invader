// SPDX-License-Identifier: GPL-3.0-only

#ifndef INVADER__TAG__HEK__CLASS__GRENADE_HUD_INTERFACE_HPP
#define INVADER__TAG__HEK__CLASS__GRENADE_HUD_INTERFACE_HPP

#include "hud_interface_types.hpp"

namespace Invader::HEK {
    struct GrenadeHUDInterfaceOverlayType {
        std::uint16_t show_on_flashing : 1;
        std::uint16_t show_on_empty : 1;
        std::uint16_t show_on_default : 1;
        std::uint16_t show_always : 1;
    };

    ENDIAN_TEMPLATE(EndianType) struct GrenadeHUDInterfaceOverlay {
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
        EndianType<std::uint16_t> number_of_flashes;
        EndianType<HUDInterfaceFlashFlags> flash_flags;
        EndianType<float> flash_length;
        EndianType<ColorARGBInt> disabled_color;
        PAD(0x4);
        EndianType<float> frame_rate;
        EndianType<Index> sequence_index;
        EndianType<GrenadeHUDInterfaceOverlayType> type;
        EndianType<HUDInterfaceOverlayFlashFlags> flags;
        PAD(0x10);
        PAD(0x28);

        ENDIAN_TEMPLATE(NewType) operator GrenadeHUDInterfaceOverlay<NewType>() const noexcept {
            GrenadeHUDInterfaceOverlay<NewType> copy = {};
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
    static_assert(sizeof(GrenadeHUDInterfaceOverlay<BigEndian>) == 0x88);

    struct GrenadeHUDInterfaceSoundLatchedTo {
        std::uint32_t low_grenade_count : 1;
        std::uint32_t no_grenades_left : 1;
        std::uint32_t throw_on_no_grenades : 1;
    };

    ENDIAN_TEMPLATE(EndianType) struct GrenadeHUDInterfaceSound {
        TagDependency<EndianType> sound; // sound, sound_looping
        EndianType<GrenadeHUDInterfaceSoundLatchedTo> latched_to;
        EndianType<float> scale;
        PAD(0x20);

        ENDIAN_TEMPLATE(NewType) operator GrenadeHUDInterfaceSound<NewType>() const noexcept {
            GrenadeHUDInterfaceSound<NewType> copy = {};
            COPY_THIS(sound);
            COPY_THIS(latched_to);
            COPY_THIS(scale);
            return copy;
        }
    };
    static_assert(sizeof(GrenadeHUDInterfaceSound<BigEndian>) == 0x38);

    ENDIAN_TEMPLATE(EndianType) struct GrenadeHUDInterface {
        EndianType<HUDInterfaceAnchor> anchor;
        PAD(0x2);
        PAD(0x20);
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
        Point2DInt<EndianType> total_grenades_background_anchor_offset;
        EndianType<float> total_grenades_background_width_scale;
        EndianType<float> total_grenades_background_height_scale;
        EndianType<HUDInterfaceScalingFlags> total_grenades_background_scaling_flags;
        PAD(0x2);
        PAD(0x14);
        TagDependency<EndianType> total_grenades_background_interface_bitmap; // bitmap
        EndianType<ColorARGBInt> total_grenades_background_default_color;
        EndianType<ColorARGBInt> total_grenades_background_flashing_color;
        EndianType<float> total_grenades_background_flash_period;
        EndianType<float> total_grenades_background_flash_delay;
        EndianType<std::uint16_t> total_grenades_background_number_of_flashes;
        EndianType<HUDInterfaceFlashFlags> total_grenades_background_flash_flags;
        EndianType<float> total_grenades_background_flash_length;
        EndianType<ColorARGBInt> total_grenades_background_disabled_color;
        PAD(0x4);
        EndianType<Index> total_grenades_background_sequence_index;
        PAD(0x2);
        TagReflexive<EndianType, HUDInterfaceMultitextureOverlay> total_grenades_background_multitex_overlay;
        PAD(0x4);
        Point2DInt<EndianType> total_grenades_numbers_anchor_offset;
        EndianType<float> total_grenades_numbers_width_scale;
        EndianType<float> total_grenades_numbers_height_scale;
        EndianType<HUDInterfaceScalingFlags> total_grenades_numbers_scaling_flags;
        PAD(0x2);
        PAD(0x14);
        EndianType<ColorARGBInt> total_grenades_numbers_default_color;
        EndianType<ColorARGBInt> total_grenades_numbers_flashing_color;
        EndianType<float> total_grenades_numbers_flash_period;
        EndianType<float> total_grenades_numbers_flash_delay;
        EndianType<std::uint16_t> total_grenades_numbers_number_of_flashes;
        EndianType<HUDInterfaceFlashFlags> total_grenades_numbers_flash_flags;
        EndianType<float> total_grenades_numbers_flash_length;
        EndianType<ColorARGBInt> total_grenades_numbers_disabled_color;
        PAD(0x4);
        std::int8_t total_grenades_numbers_maximum_number_of_digits;
        HUDInterfaceNumberFlags total_grenades_numbers_flags;
        std::int8_t total_grenades_numbers_number_of_fractional_digits;
        PAD(0x1);
        PAD(0xC);
        EndianType<std::uint16_t> flash_cutoff;
        PAD(0x2);
        TagDependency<EndianType> total_grenades_overlay_bitmap; // bitmap
        TagReflexive<EndianType, GrenadeHUDInterfaceOverlay> total_grenades_overlays;
        TagReflexive<EndianType, GrenadeHUDInterfaceSound> total_grenades_warning_sounds;
        PAD(0x44);
        EndianType<Index> messaging_information_sequence_index;
        EndianType<std::int16_t> messaging_information_width_offset;
        Point2DInt<EndianType> messaging_information_offset_from_reference_corner;
        EndianType<ColorARGBInt> messaging_information_override_icon_color;
        std::int8_t messaging_information_frame_rate;
        HUDInterfaceMessagingFlags messaging_information_flags;
        EndianType<Index> messaging_information_text_index;
        PAD(0x30);

        ENDIAN_TEMPLATE(NewType) operator GrenadeHUDInterface<NewType>() const noexcept {
            GrenadeHUDInterface<NewType> copy = {};
            COPY_THIS(anchor);
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
            COPY_THIS(total_grenades_background_anchor_offset);
            COPY_THIS(total_grenades_background_width_scale);
            COPY_THIS(total_grenades_background_height_scale);
            COPY_THIS(total_grenades_background_scaling_flags);
            COPY_THIS(total_grenades_background_interface_bitmap);
            COPY_THIS(total_grenades_background_default_color);
            COPY_THIS(total_grenades_background_flashing_color);
            COPY_THIS(total_grenades_background_flash_period);
            COPY_THIS(total_grenades_background_flash_delay);
            COPY_THIS(total_grenades_background_number_of_flashes);
            COPY_THIS(total_grenades_background_flash_flags);
            COPY_THIS(total_grenades_background_flash_length);
            COPY_THIS(total_grenades_background_disabled_color);
            COPY_THIS(total_grenades_background_sequence_index);
            COPY_THIS(total_grenades_background_multitex_overlay);
            COPY_THIS(total_grenades_numbers_anchor_offset);
            COPY_THIS(total_grenades_numbers_width_scale);
            COPY_THIS(total_grenades_numbers_height_scale);
            COPY_THIS(total_grenades_numbers_scaling_flags);
            COPY_THIS(total_grenades_numbers_default_color);
            COPY_THIS(total_grenades_numbers_flashing_color);
            COPY_THIS(total_grenades_numbers_flash_period);
            COPY_THIS(total_grenades_numbers_flash_delay);
            COPY_THIS(total_grenades_numbers_number_of_flashes);
            COPY_THIS(total_grenades_numbers_flash_flags);
            COPY_THIS(total_grenades_numbers_flash_length);
            COPY_THIS(total_grenades_numbers_disabled_color);
            COPY_THIS(total_grenades_numbers_maximum_number_of_digits);
            COPY_THIS(total_grenades_numbers_flags);
            COPY_THIS(total_grenades_numbers_number_of_fractional_digits);
            COPY_THIS(flash_cutoff);
            COPY_THIS(total_grenades_overlay_bitmap);
            COPY_THIS(total_grenades_overlays);
            COPY_THIS(total_grenades_warning_sounds);
            COPY_THIS(messaging_information_sequence_index);
            COPY_THIS(messaging_information_width_offset);
            COPY_THIS(messaging_information_offset_from_reference_corner);
            COPY_THIS(messaging_information_override_icon_color);
            COPY_THIS(messaging_information_frame_rate);
            COPY_THIS(messaging_information_flags);
            COPY_THIS(messaging_information_text_index);
            return copy;
        }
    };
    static_assert(sizeof(GrenadeHUDInterface<BigEndian>) == 0x1F8);

    void compile_grenade_hud_interface_tag(CompiledTag &compiled, const std::byte *data, std::size_t size);
}
#endif
