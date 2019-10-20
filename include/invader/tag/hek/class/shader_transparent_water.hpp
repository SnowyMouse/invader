// SPDX-License-Identifier: GPL-3.0-only

#ifndef INVADER__TAG__HEK__CLASS__SHADER_TRANSPARENT_WATER_HPP
#define INVADER__TAG__HEK__CLASS__SHADER_TRANSPARENT_WATER_HPP

#include "shader.hpp"

namespace Invader::HEK {
    ENDIAN_TEMPLATE(EndianType) struct ShaderTransparentWaterRipple {
        PAD(0x2);
        PAD(0x2);
        EndianType<Fraction> contribution_factor;
        PAD(0x20);
        EndianType<Angle> animation_angle;
        EndianType<float> animation_velocity;
        Vector2D<EndianType> map_offset;
        EndianType<std::int16_t> map_repeats;
        EndianType<std::int16_t> map_index;
        PAD(0x10);

        ENDIAN_TEMPLATE(NewType) operator ShaderTransparentWaterRipple<NewType>() const noexcept {
            ShaderTransparentWaterRipple<NewType> copy = {};
            COPY_THIS(contribution_factor);
            COPY_THIS(animation_angle);
            COPY_THIS(animation_velocity);
            COPY_THIS(map_offset);
            COPY_THIS(map_repeats);
            COPY_THIS(map_index);
            return copy;
        }
    };
    static_assert(sizeof(ShaderTransparentWaterRipple<NativeEndian>) == 0x4C);

    struct ShaderTransparentWaterFlags {
        std::uint16_t base_map_alpha_modulates_reflection : 1;
        std::uint16_t base_map_color_modulates_background : 1;
        std::uint16_t atmospheric_fog : 1;
        std::uint16_t draw_before_fog : 1;
    };

    ENDIAN_TEMPLATE(EndianType) struct ShaderTransparentWater : Shader<EndianType> {
        EndianType<ShaderTransparentWaterFlags> water_flags;
        PAD(0x2);
        PAD(0x20);
        TagDependency<EndianType> base_map; // bitmap
        PAD(0x10);
        EndianType<Fraction> view_perpendicular_brightness;
        ColorRGB<EndianType> view_perpendicular_tint_color;
        EndianType<Fraction> view_parallel_brightness;
        ColorRGB<EndianType> view_parallel_tint_color;
        PAD(0x10);
        TagDependency<EndianType> reflection_map; // bitmap
        PAD(0x10);
        EndianType<Angle> ripple_animation_angle;
        EndianType<float> ripple_animation_velocity;
        EndianType<float> ripple_scale;
        TagDependency<EndianType> ripple_maps; // bitmap
        EndianType<std::uint16_t> ripple_mipmap_levels;
        PAD(0x2);
        EndianType<Fraction> ripple_mipmap_fade_factor;
        EndianType<float> ripple_mipmap_detail_bias;
        PAD(0x40);
        TagReflexive<EndianType, ShaderTransparentWaterRipple> ripples;
        PAD(0x10);

        ENDIAN_TEMPLATE(NewType) operator ShaderTransparentWater<NewType>() const noexcept {
            ShaderTransparentWater<NewType> copy = {};
            COPY_SHADER_DATA
            COPY_THIS(water_flags);
            COPY_THIS(base_map);
            COPY_THIS(view_perpendicular_brightness);
            COPY_THIS(view_perpendicular_tint_color);
            COPY_THIS(view_parallel_brightness);
            COPY_THIS(view_parallel_tint_color);
            COPY_THIS(reflection_map);
            COPY_THIS(ripple_animation_angle);
            COPY_THIS(ripple_animation_velocity);
            COPY_THIS(ripple_scale);
            COPY_THIS(ripple_maps);
            COPY_THIS(ripple_mipmap_levels);
            COPY_THIS(ripple_mipmap_fade_factor);
            COPY_THIS(ripple_mipmap_detail_bias);
            COPY_THIS(ripples);
            return copy;
        }
    };
    static_assert(sizeof(ShaderTransparentWater<NativeEndian>) == 0x140);

    void compile_shader_transparent_water_tag(CompiledTag &compiled, const std::byte *data, std::size_t size);
}
#endif
