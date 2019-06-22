/*
 * Invader (c) 2018 Kavawuvi
 *
 * This program is free software under the GNU General Public License v3.0 or later. See LICENSE for more information.
 */

#ifndef INVADER__TAG__HEK__CLASS__LIGHT_VOLUME_HPP
#define INVADER__TAG__HEK__CLASS__LIGHT_VOLUME_HPP

#include "../../compiled_tag.hpp"
#include "../../../hek/data_type.hpp"
#include "../header.hpp"

namespace Invader::HEK {
    ENDIAN_TEMPLATE(EndianType) struct LightVolumeFrame {
        PAD(0x10);
        EndianType<float> offset_from_marker;
        EndianType<float> offset_exponent;
        EndianType<float> length;
        PAD(0x20);
        EndianType<float> radius_hither;
        EndianType<float> radius_yon;
        EndianType<float> radius_exponent;
        PAD(0x20);
        ColorARGB<EndianType> tint_color_hither;
        ColorARGB<EndianType> tint_color_yon;
        EndianType<float> tint_color_exponent;
        EndianType<float> brightness_exponent;
        PAD(0x20);

        ENDIAN_TEMPLATE(NewType) operator LightVolumeFrame<NewType>() const noexcept {
            LightVolumeFrame<NewType> copy = {};
            COPY_THIS(offset_from_marker);
            COPY_THIS(offset_exponent);
            COPY_THIS(length);
            COPY_THIS(radius_hither);
            COPY_THIS(radius_yon);
            COPY_THIS(radius_exponent);
            COPY_THIS(tint_color_hither);
            COPY_THIS(tint_color_yon);
            COPY_THIS(tint_color_exponent);
            COPY_THIS(brightness_exponent);
            return copy;
        }
    };
    static_assert(sizeof(LightVolumeFrame<BigEndian>) == 0xB0);

    enum LightVolumeScaleSource : TagEnum {
        LIGHT_VOLUME_SCALE_SOURCE_NONE,
        LIGHT_VOLUME_SCALE_SOURCE_A_OUT,
        LIGHT_VOLUME_SCALE_SOURCE_B_OUT,
        LIGHT_VOLUME_SCALE_SOURCE_C_OUT,
        LIGHT_VOLUME_SCALE_SOURCE_D_OUT
    };

    struct LightVolumeFlags {
        std::uint16_t interpolate_color_in_hsv : 1;
        std::uint16_t _more_colors : 1;
    };

    ENDIAN_TEMPLATE(EndianType) struct LightVolume {
        TagString attachment_marker;
        PAD(0x2);
        EndianType<LightVolumeFlags> flags;
        PAD(0x10);
        EndianType<float> near_fade_distance;
        EndianType<float> far_fade_distance;
        EndianType<Fraction> perpendicular_brightness_scale;
        EndianType<Fraction> parallel_brightness_scale;
        EndianType<LightVolumeScaleSource> brightness_scale_source;
        PAD(0x2);
        PAD(0x14);
        TagDependency<EndianType> map; // bitmap
        EndianType<std::int16_t> sequence_index;
        EndianType<std::int16_t> count;
        PAD(0x48);
        EndianType<LightVolumeScaleSource> frame_animation_source;
        PAD(0x2);
        PAD(0x24);
        PAD(0x40);
        TagReflexive<EndianType, LightVolumeFrame> frames;
        PAD(0x20);

        ENDIAN_TEMPLATE(NewType) operator LightVolume<NewType>() const noexcept {
            LightVolume<NewType> copy = {};
            COPY_THIS(attachment_marker);
            COPY_THIS(flags);
            COPY_THIS(near_fade_distance);
            COPY_THIS(far_fade_distance);
            COPY_THIS(perpendicular_brightness_scale);
            COPY_THIS(parallel_brightness_scale);
            COPY_THIS(brightness_scale_source);
            COPY_THIS(map);
            COPY_THIS(sequence_index);
            COPY_THIS(count);
            COPY_THIS(frame_animation_source);
            COPY_THIS(frames);
            return copy;
        }
    };
    static_assert(sizeof(LightVolume<BigEndian>) == 0x14C);

    void compile_light_volume_tag(CompiledTag &compiled, const std::byte *data, std::size_t size);
}
#endif
