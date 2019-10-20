// SPDX-License-Identifier: GPL-3.0-only

#ifndef INVADER__TAG__HEK__CLASS__FOG_HPP
#define INVADER__TAG__HEK__CLASS__FOG_HPP

#include "../../compiled_tag.hpp"
#include "../../../hek/data_type.hpp"
#include "../header.hpp"

namespace Invader::HEK {
    struct FogFlags {
        std::uint32_t is_water : 1;
        std::uint32_t atmosphere_dominant : 1;
        std::uint32_t fog_screen_only : 1;
    };

    struct FogScreenFlags {
        std::uint16_t no_environment_multipass : 1;
        std::uint16_t no_model_multipass : 1;
        std::uint16_t no_texture_based_falloff : 1;
    };

    ENDIAN_TEMPLATE(EndianType) struct Fog {
        EndianType<FogFlags> flags;
        PAD(0x4);
        PAD(0x4C);
        PAD(0x4);
        EndianType<Fraction> maximum_density;
        PAD(0x4);
        EndianType<float> opaque_distance;
        PAD(0x4);
        EndianType<float> opaque_depth;
        PAD(0x8);
        EndianType<float> distance_to_water_plane;
        ColorRGB<EndianType> color;
        EndianType<FogScreenFlags> flags_1;
        EndianType<std::int16_t> layer_count;
        Bounds<EndianType<float>> distance_gradient;
        Bounds<EndianType<Fraction>> density_gradient;
        EndianType<float> start_distance_from_fog_plane;
        PAD(0x4);
        EndianType<ColorARGBInt> screen_layers_color;
        EndianType<Fraction> rotation_multiplier;
        EndianType<Fraction> strafing_multiplier;
        EndianType<Fraction> zoom_multiplier;
        PAD(0x8);
        EndianType<float> map_scale;
        TagDependency<EndianType> map; // bitmap
        EndianType<float> animation_period;
        PAD(0x4);
        Bounds<EndianType<float>> wind_velocity;
        Bounds<EndianType<float>> wind_period;
        EndianType<Fraction> wind_acceleration_weight;
        EndianType<Fraction> wind_perpendicular_weight;
        PAD(0x8);
        TagDependency<EndianType> background_sound; // sound_looping
        TagDependency<EndianType> sound_environment; // sound_environment
        PAD(0x78);

        ENDIAN_TEMPLATE(NewType) operator Fog<NewType>() const noexcept {
            Fog<NewType> copy = {};
            COPY_THIS(flags);
            COPY_THIS(maximum_density);
            COPY_THIS(opaque_distance);
            COPY_THIS(opaque_depth);
            COPY_THIS(distance_to_water_plane);
            COPY_THIS(color);
            COPY_THIS(flags_1);
            COPY_THIS(layer_count);
            COPY_THIS(distance_gradient);
            COPY_THIS(density_gradient);
            COPY_THIS(start_distance_from_fog_plane);
            COPY_THIS(screen_layers_color);
            COPY_THIS(rotation_multiplier);
            COPY_THIS(strafing_multiplier);
            COPY_THIS(zoom_multiplier);
            COPY_THIS(map_scale);
            COPY_THIS(map);
            COPY_THIS(animation_period);
            COPY_THIS(wind_velocity);
            COPY_THIS(wind_period);
            COPY_THIS(wind_acceleration_weight);
            COPY_THIS(wind_perpendicular_weight);
            COPY_THIS(background_sound);
            COPY_THIS(sound_environment);
            return copy;
        }
    };
    static_assert(sizeof(Fog<NativeEndian>) == 0x18C);

    void compile_fog_tag(CompiledTag &compiled, const std::byte *data, std::size_t size);
}
#endif
