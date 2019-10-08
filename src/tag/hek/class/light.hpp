// SPDX-License-Identifier: GPL-3.0-only

#ifndef INVADER__TAG__HEK__CLASS__LIGHT_HPP
#define INVADER__TAG__HEK__CLASS__LIGHT_HPP

#include "../../compiled_tag.hpp"
#include "../../../hek/data_type.hpp"
#include "../header.hpp"
#include "enum.hpp"

namespace Invader::HEK {
    struct LightFlags {
        std::uint32_t dynamic : 1;
        std::uint32_t no_specular : 1;
        std::uint32_t don_t_light_own_object : 1;
        std::uint32_t supersize_in_first_person : 1;
        std::uint32_t first_person_flashlight : 1;
        std::uint32_t don_t_fade_active_camouflage : 1;
    };

    struct LightInterpolationFlags {
        std::uint32_t blend_in_hsv : 1;
        std::uint32_t _more_colors : 1;
    };

    ENDIAN_TEMPLATE(EndianType) struct Light {
        EndianType<LightFlags> flags;
        EndianType<float> radius;
        Bounds<EndianType<float>> radius_modifer;
        EndianType<Angle> falloff_angle;
        EndianType<Angle> cutoff_angle;
        EndianType<float> lens_flare_only_radius;
        EndianType<float> cos_falloff_angle;
        EndianType<float> cos_cutoff_angle;
        EndianType<float> unknown_two;
        EndianType<float> sin_cutoff_angle;
        PAD(0x8);
        EndianType<LightInterpolationFlags> interpolation_flags;
        ColorARGB<EndianType> color_lower_bound;
        ColorARGB<EndianType> color_upper_bound;
        PAD(0xC);
        TagDependency<EndianType> primary_cube_map; // bitmap
        PAD(0x2);
        EndianType<WaveFunction> texture_animation_function;
        EndianType<float> texture_animation_period;
        TagDependency<EndianType> secondary_cube_map; // bitmap
        PAD(0x2);
        EndianType<WaveFunction> yaw_function;
        EndianType<float> yaw_period;
        PAD(0x2);
        EndianType<WaveFunction> roll_function;
        EndianType<float> roll_period;
        PAD(0x2);
        EndianType<WaveFunction> pitch_function;
        EndianType<float> pitch_period;
        PAD(0x8);
        TagDependency<EndianType> lens_flare; // lens_flare
        PAD(0x18);
        EndianType<float> intensity;
        ColorRGB<EndianType> color;
        PAD(0x10);
        EndianType<float> duration;
        PAD(0x2);
        EndianType<FunctionType> falloff_function;
        PAD(0x8);
        PAD(0x5C);

        ENDIAN_TEMPLATE(NewType) operator Light<NewType>() const noexcept {
            Light<NewType> copy = {};

            COPY_THIS(flags);
            COPY_THIS(radius);
            COPY_THIS(radius_modifer);
            COPY_THIS(falloff_angle);
            COPY_THIS(cutoff_angle);
            COPY_THIS(cos_falloff_angle);
            COPY_THIS(cos_cutoff_angle);
            COPY_THIS(sin_cutoff_angle);
            COPY_THIS(lens_flare_only_radius);
            COPY_THIS(interpolation_flags);
            COPY_THIS(color_lower_bound);
            COPY_THIS(color_upper_bound);
            COPY_THIS(primary_cube_map);
            COPY_THIS(texture_animation_function);
            COPY_THIS(texture_animation_period);
            COPY_THIS(secondary_cube_map);
            COPY_THIS(yaw_function);
            COPY_THIS(yaw_period);
            COPY_THIS(roll_function);
            COPY_THIS(roll_period);
            COPY_THIS(pitch_function);
            COPY_THIS(pitch_period);
            COPY_THIS(lens_flare);
            COPY_THIS(intensity);
            COPY_THIS(color);
            COPY_THIS(duration);
            COPY_THIS(falloff_function);
            COPY_THIS(unknown_two);

            return copy;
        }
    };
    static_assert(sizeof(Light<BigEndian>) == 0x160);

    void compile_light_tag(CompiledTag &compiled, const std::byte *data, std::size_t size);
}
#endif
