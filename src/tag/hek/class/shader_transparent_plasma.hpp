/*
 * Invader (c) 2018 Kavawuvi
 *
 * This program is free software under the GNU General Public License v3.0 or later. See LICENSE for more information.
 */

#pragma once

#include "shader.hpp"

namespace Invader::HEK {
    enum ShaderTransparentPlasmaSource : TagEnum {
        SHADER_TRANSPARENT_PLASMA_SOURCE_NONE,
        SHADER_TRANSPARENT_PLASMA_SOURCE_A_OUT,
        SHADER_TRANSPARENT_PLASMA_SOURCE_B_OUT,
        SHADER_TRANSPARENT_PLASMA_SOURCE_C_OUT,
        SHADER_TRANSPARENT_PLASMA_SOURCE_D_OUT
    };

    enum ShaderTransparentPlasmaTintColorSource : TagEnum {
        SHADER_TRANSPARENT_PLASMA_TINT_COLOR_SOURCE_NONE,
        SHADER_TRANSPARENT_PLASMA_TINT_COLOR_SOURCE_A,
        SHADER_TRANSPARENT_PLASMA_TINT_COLOR_SOURCE_B,
        SHADER_TRANSPARENT_PLASMA_TINT_COLOR_SOURCE_C,
        SHADER_TRANSPARENT_PLASMA_TINT_COLOR_SOURCE_D
    };

    ENDIAN_TEMPLATE(EndianType) struct ShaderTransparentPlasma : Shader<EndianType> {
        PAD(0x2);
        PAD(0x2);
        EndianType<ShaderTransparentPlasmaSource> intensity_source;
        PAD(0x2);
        EndianType<float> intensity_exponent;
        EndianType<ShaderTransparentPlasmaSource> offset_source;
        PAD(0x2);
        EndianType<float> offset_amount;
        EndianType<float> offset_exponent;
        PAD(0x20);
        EndianType<Fraction> perpendicular_brightness;
        ColorRGB<EndianType> perpendicular_tint_color;
        EndianType<Fraction> parallel_brightness;
        ColorRGB<EndianType> parallel_tint_color;
        EndianType<ShaderTransparentPlasmaTintColorSource> tint_color_source;
        PAD(0x2);
        PAD(0x20);
        PAD(0x2);
        PAD(0x2);
        PAD(0x10);
        PAD(0x4);
        PAD(0x4);
        EndianType<float> primary_animation_period;
        Vector3D<EndianType> primary_animation_direction;
        EndianType<float> primary_noise_map_scale;
        TagDependency<EndianType> primary_noise_map; // bitmap
        PAD(0x20);
        PAD(0x4);
        EndianType<float> secondary_animation_period;
        Vector3D<EndianType> secondary_animation_direction;
        EndianType<float> secondary_noise_map_scale;
        TagDependency<EndianType> secondary_noise_map; // bitmap
        PAD(0x20);

        ENDIAN_TEMPLATE(NewType) operator ShaderTransparentPlasma<NewType>() const noexcept {
            ShaderTransparentPlasma<NewType> copy = {};
            COPY_THIS(intensity_source);
            COPY_THIS(intensity_exponent);
            COPY_THIS(offset_source);
            COPY_THIS(offset_amount);
            COPY_THIS(offset_exponent);
            COPY_THIS(perpendicular_brightness);
            COPY_THIS(perpendicular_tint_color);
            COPY_THIS(parallel_brightness);
            COPY_THIS(parallel_tint_color);
            COPY_THIS(tint_color_source);
            COPY_THIS(primary_animation_period);
            COPY_THIS(primary_animation_direction);
            COPY_THIS(primary_noise_map_scale);
            COPY_THIS(primary_noise_map);
            COPY_THIS(secondary_animation_period);
            COPY_THIS(secondary_animation_direction);
            COPY_THIS(secondary_noise_map_scale);
            COPY_THIS(secondary_noise_map);
            return copy;
        }
    };
    static_assert(sizeof(ShaderTransparentPlasma<BigEndian>) == 0x14C);

    void compile_shader_transparent_plasma_tag(CompiledTag &compiled, const std::byte *data, std::size_t size);
}
