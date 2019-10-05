// SPDX-License-Identifier: GPL-3.0-only

#ifndef INVADER__TAG__HEK__CLASS__LENS_FLARE_HPP
#define INVADER__TAG__HEK__CLASS__LENS_FLARE_HPP

#include "../../compiled_tag.hpp"
#include "../../../hek/data_type.hpp"
#include "../header.hpp"

namespace Invader::HEK {
    enum LensFlareRadiusScaledBy : TagEnum {
        LENS_FLARE_RADIUS_SCALED_BY_NONE,
        LENS_FLARE_RADIUS_SCALED_BY_ROTATION,
        LENS_FLARE_RADIUS_SCALED_BY_ROTATION_AND_STRAFING,
        LENS_FLARE_RADIUS_SCALED_BY_DISTANCE_FROM_CENTER
    };

    enum LensFlareOcclusionOffsetDirection : TagEnum {
        LENS_FLARE_OCCLUSION_OFFSET_DIRECTION_TOWARD_VIEWER,
        LENS_FLARE_OCCLUSION_OFFSET_DIRECTION_MARKER_FORWARD,
        LENS_FLARE_OCCLUSION_OFFSET_DIRECTION_NONE
    };

    enum LensFlareRotationFunction : TagEnum {
        LENS_FLARE_ROTATION_FUNCTION_NONE,
        LENS_FLARE_ROTATION_FUNCTION_ROTATION_A,
        LENS_FLARE_ROTATION_FUNCTION_ROTATION_B,
        LENS_FLARE_ROTATION_FUNCTION_ROTATION_TRANSLATION,
        LENS_FLARE_ROTATION_FUNCTION_TRANSLATION
    };

    struct LensFlareReflectionFlags {
        std::uint16_t align_rotation_with_screen_center : 1;
        std::uint16_t radius_not_scaled_by_distance : 1;
        std::uint16_t radius_scaled_by_occlusion_factor : 1;
        std::uint16_t occluded_by_solid_objects : 1;
    };

    struct LensFlareReflectionMoreFlags {
        std::uint16_t interpolate_colors_in_hsv : 1;
        std::uint16_t more_colors : 1;
    };

    ENDIAN_TEMPLATE(EndianType) struct LensFlareReflection {
        EndianType<LensFlareReflectionFlags> flags;
        PAD(0x2);
        EndianType<std::int16_t> bitmap_index;
        PAD(0x2);
        PAD(0x14);
        EndianType<float> position;
        EndianType<float> rotation_offset;
        PAD(0x4);
        Bounds<EndianType<float>> radius;
        EndianType<LensFlareRadiusScaledBy> radius_scaled_by;
        PAD(0x2);
        Bounds<EndianType<Fraction>> brightness;
        EndianType<LensFlareRadiusScaledBy> brightness_scaled_by;
        PAD(0x2);
        ColorARGB<EndianType> tint_color;
        ColorARGB<EndianType> color_lower_bound;
        ColorARGB<EndianType> color_upper_bound;
        EndianType<LensFlareReflectionMoreFlags> more_flags;
        EndianType<FunctionType2> animation_function;
        EndianType<float> animation_period;
        EndianType<float> animation_phase;
        PAD(0x4);

        ENDIAN_TEMPLATE(NewType) operator LensFlareReflection<NewType>() const noexcept {
            LensFlareReflection<NewType> copy = {};
            COPY_THIS(flags);
            COPY_THIS(bitmap_index);
            COPY_THIS(position);
            COPY_THIS(rotation_offset);
            COPY_THIS(radius);
            COPY_THIS(radius_scaled_by);
            COPY_THIS(brightness);
            COPY_THIS(brightness_scaled_by);
            COPY_THIS(tint_color);
            COPY_THIS(color_lower_bound);
            COPY_THIS(color_upper_bound);
            COPY_THIS(more_flags);
            COPY_THIS(animation_function);
            COPY_THIS(animation_period);
            COPY_THIS(animation_phase);
            return copy;
        }
    };
    static_assert(sizeof(LensFlareReflection<BigEndian>) == 0x80);

    struct LensFlareFlags {
        std::uint16_t sun : 1;
    };

    ENDIAN_TEMPLATE(EndianType) struct LensFlare {
        EndianType<Angle> falloff_angle;
        EndianType<Angle> cutoff_angle;
        LittleEndian<float> cos_falloff_angle;
        LittleEndian<float> cos_cutoff_angle;
        EndianType<float> occlusion_radius;
        EndianType<LensFlareOcclusionOffsetDirection> occlusion_offset_direction;
        PAD(0x2);
        EndianType<float> near_fade_distance;
        EndianType<float> far_fade_distance;
        TagDependency<EndianType> bitmap; // bitmap
        EndianType<LensFlareFlags> flags;
        PAD(0x2);
        PAD(0x4C);
        EndianType<LensFlareRotationFunction> rotation_function;
        PAD(0x2);
        EndianType<Angle> rotation_function_scale;
        PAD(0x18);
        EndianType<float> horizontal_scale;
        EndianType<float> vertical_scale;
        PAD(0x1C);
        TagReflexive<EndianType, LensFlareReflection> reflections;
        PAD(0x20);

        ENDIAN_TEMPLATE(NewType) operator LensFlare<NewType>() const noexcept {
            LensFlare<NewType> copy = {};
            COPY_THIS(falloff_angle);
            COPY_THIS(cutoff_angle);
            COPY_THIS(cos_falloff_angle);
            COPY_THIS(cos_cutoff_angle);
            COPY_THIS(occlusion_radius);
            COPY_THIS(occlusion_offset_direction);
            COPY_THIS(near_fade_distance);
            COPY_THIS(far_fade_distance);
            COPY_THIS(bitmap);
            COPY_THIS(flags);
            COPY_THIS(rotation_function);
            COPY_THIS(rotation_function_scale);
            COPY_THIS(horizontal_scale);
            COPY_THIS(vertical_scale);
            COPY_THIS(reflections);
            return copy;
        }
    };
    static_assert(sizeof(LensFlare<BigEndian>) == 0xF0);

    void compile_lens_flare_tag(CompiledTag &compiled, const std::byte *data, std::size_t size);
}
#endif
