/*
 * Invader (c) 2018 Kavawuvi
 *
 * This program is free software under the GNU General Public License v3.0 or later. See LICENSE for more information.
 */

#ifndef INVADER__TAG__HEK__CLASS__SKY_HPP
#define INVADER__TAG__HEK__CLASS__SKY_HPP

#include "../../compiled_tag.hpp"
#include "../../../hek/data_type.hpp"
#include "../header.hpp"

namespace Invader::HEK {
    ENDIAN_TEMPLATE(EndianType) struct SkyFunction {
        PAD(0x4);
        TagString function_name;

        ENDIAN_TEMPLATE(NewType) operator SkyFunction<NewType>() const noexcept {
            SkyFunction<NewType> copy = {};
            COPY_THIS(function_name);
            return copy;
        }
    };
    static_assert(sizeof(SkyFunction<BigEndian>) == 0x24);

    ENDIAN_TEMPLATE(EndianType) struct SkyAnimation {
        EndianType<std::int16_t> animation_index;
        PAD(0x2);
        EndianType<float> period;
        PAD(0x1C);

        ENDIAN_TEMPLATE(NewType) operator SkyAnimation<NewType>() const noexcept {
            SkyAnimation<NewType> copy = {};
            COPY_THIS(animation_index);
            COPY_THIS(period);
            return copy;
        }
    };
    static_assert(sizeof(SkyAnimation<BigEndian>) == 0x24); // max count: 8

    struct SkyLightFlags {
        std::uint32_t affects_exteriors : 1;
        std::uint32_t affects_interiors : 1;
    };
    static_assert(sizeof(SkyLightFlags) == sizeof(std::uint32_t));

    ENDIAN_TEMPLATE(EndianType) struct SkyLight {
        TagDependency<EndianType> lens_flare; // lens_flare
        TagString lens_flare_marker_name;
        PAD(0x1C);
        EndianType<SkyLightFlags> flags;
        ColorRGB<EndianType> color;
        EndianType<float> power;
        EndianType<float> test_distance;
        PAD(0x4);
        Euler2D<EndianType> direction;
        EndianType<float> diameter;

        ENDIAN_TEMPLATE(NewType) operator SkyLight<NewType>() const noexcept {
            SkyLight<NewType> copy = {};
            COPY_THIS(lens_flare);
            COPY_THIS(lens_flare_marker_name);
            COPY_THIS(flags);
            COPY_THIS(color);
            COPY_THIS(power);
            COPY_THIS(test_distance);
            COPY_THIS(direction);
            COPY_THIS(diameter);
            return copy;
        }
    };
    static_assert(sizeof(SkyLight<BigEndian>) == 0x74);

    ENDIAN_TEMPLATE(EndianType) struct Sky {
        TagDependency<EndianType> model; // model
        TagDependency<EndianType> animation_graph; // model_animations
        PAD(0x18);
        ColorRGB<EndianType> indoor_ambient_radiosity_color;
        EndianType<float> indoor_ambient_radiosity_power;
        ColorRGB<EndianType> outdoor_ambient_radiosity_color;
        EndianType<float> outdoor_ambient_radiosity_power;
        ColorRGB<EndianType> outdoor_fog_color;
        PAD(0x8);
        EndianType<Fraction> outdoor_fog_maximum_density;
        EndianType<float> outdoor_fog_start_distance;
        EndianType<float> outdoor_fog_opaque_distance;
        ColorRGB<EndianType> indoor_fog_color;
        PAD(0x8);
        EndianType<Fraction> indoor_fog_maximum_density;
        EndianType<float> indoor_fog_start_distance;
        EndianType<float> indoor_fog_opaque_distance;
        TagDependency<EndianType> indoor_fog_screen; // fog
        PAD(0x4);
        TagReflexive<EndianType, SkyFunction> shader_functions;
        TagReflexive<EndianType, SkyAnimation> animations;
        TagReflexive<EndianType, SkyLight> lights;

        ENDIAN_TEMPLATE(NewType) operator Sky<NewType>() const noexcept {
            Sky<NewType> copy = {};
            COPY_THIS(model);
            COPY_THIS(animation_graph);
            COPY_THIS(indoor_ambient_radiosity_color);
            COPY_THIS(indoor_ambient_radiosity_power);
            COPY_THIS(outdoor_ambient_radiosity_color);
            COPY_THIS(outdoor_ambient_radiosity_power);
            COPY_THIS(outdoor_fog_color);
            COPY_THIS(outdoor_fog_maximum_density);
            COPY_THIS(outdoor_fog_start_distance);
            COPY_THIS(outdoor_fog_opaque_distance);
            COPY_THIS(indoor_fog_color);
            COPY_THIS(indoor_fog_maximum_density);
            COPY_THIS(indoor_fog_start_distance);
            COPY_THIS(indoor_fog_opaque_distance);
            COPY_THIS(indoor_fog_screen);
            COPY_THIS(shader_functions);
            COPY_THIS(animations);
            COPY_THIS(lights);
            return copy;
        }
    };
    static_assert(sizeof(Sky<BigEndian>) == 0xD0);

    void compile_sky_tag(CompiledTag &compiled, const std::byte *data, std::size_t size);
}
#endif
