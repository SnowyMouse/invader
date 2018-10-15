/*
 * Invader (c) 2018 Kavawuvi
 *
 * This program is free software under the GNU General Public License v3.0 or later. See LICENSE for more information.
 */

#pragma once

#include "shader.hpp"

namespace Invader::HEK {
    SINGLE_DEPENDENCY_STRUCT(ShaderTransparentChicagoTransparentLayer, shader); //shader

    struct ShaderTransparentChicagoMapFlags {
        std::uint16_t unfiltered : 1;
        std::uint16_t alpha_replicate : 1;
        std::uint16_t u_clamped : 1;
        std::uint16_t v_clamped : 1;
    };

    ENDIAN_TEMPLATE(EndianType) struct ShaderTransparentChicagoMap {
        EndianType<ShaderTransparentChicagoMapFlags> flags;
        PAD(0x2);
        PAD(0x28);
        EndianType<ShaderColorFunctionType> color_function;
        EndianType<ShaderColorFunctionType> alpha_function;
        PAD(0x24);
        EndianType<float> map_u_scale;
        EndianType<float> map_v_scale;
        EndianType<float> map_u_offset;
        EndianType<float> map_v_offset;
        EndianType<float> map_rotation;
        EndianType<Fraction> mipmap_bias;
        TagDependency<EndianType> map; // bitmap
        PAD(0x28);
        EndianType<ShaderAnimationSource> u_animation_source;
        EndianType<FunctionType2> u_animation_function;
        EndianType<float> u_animation_period;
        EndianType<float> u_animation_phase;
        EndianType<float> u_animation_scale;
        EndianType<ShaderAnimationSource> v_animation_source;
        EndianType<FunctionType2> v_animation_function;
        EndianType<float> v_animation_period;
        EndianType<float> v_animation_phase;
        EndianType<float> v_animation_scale;
        EndianType<ShaderAnimationSource> rotation_animation_source;
        EndianType<FunctionType2> rotation_animation_function;
        EndianType<float> rotation_animation_period;
        EndianType<float> rotation_animation_phase;
        EndianType<float> rotation_animation_scale;
        Point2D<EndianType> rotation_animation_center;

        ENDIAN_TEMPLATE(NewType) operator ShaderTransparentChicagoMap<NewType>() const noexcept {
            ShaderTransparentChicagoMap<NewType> copy = {};
            COPY_THIS(flags);
            COPY_THIS(color_function);
            COPY_THIS(alpha_function);
            COPY_THIS(map_u_scale);
            COPY_THIS(map_v_scale);
            COPY_THIS(map_u_offset);
            COPY_THIS(map_v_offset);
            COPY_THIS(map_rotation);
            COPY_THIS(mipmap_bias);
            COPY_THIS(map);
            COPY_THIS(u_animation_source);
            COPY_THIS(u_animation_function);
            COPY_THIS(u_animation_period);
            COPY_THIS(u_animation_phase);
            COPY_THIS(u_animation_scale);
            COPY_THIS(v_animation_source);
            COPY_THIS(v_animation_function);
            COPY_THIS(v_animation_period);
            COPY_THIS(v_animation_phase);
            COPY_THIS(v_animation_scale);
            COPY_THIS(rotation_animation_source);
            COPY_THIS(rotation_animation_function);
            COPY_THIS(rotation_animation_period);
            COPY_THIS(rotation_animation_phase);
            COPY_THIS(rotation_animation_scale);
            COPY_THIS(rotation_animation_center);
            return copy;
        }
    };
    static_assert(sizeof(ShaderTransparentChicagoMap<BigEndian>) == 0xDC);

    #define ADD_CHICAGO_MAP(reflexive_struct) ADD_REFLEXIVE_START(reflexive_struct) { \
                                                  ADD_DEPENDENCY_ADJUST_SIZES(reflexive.map); \
                                                  DEFAULT_VALUE(reflexive.map_u_scale, 1.0f); \
                                                  DEFAULT_VALUE(reflexive.map_v_scale, 1.0f); \
                                                  INITIALIZE_SHADER_ANIMATION(reflexive); \
                                              } ADD_REFLEXIVE_END

    struct ShaderTransparentChicagoFlags {
        std::uint8_t alpha_tested : 1;
        std::uint8_t decal : 1;
        std::uint8_t two_sided : 1;
        std::uint8_t first_map_is_in_screenspace : 1;
        std::uint8_t draw_before_water : 1;
        std::uint8_t ignore_effect : 1;
        std::uint8_t scale_first_map_with_distance : 1;
        std::uint8_t numeric : 1;
    };

    struct ShaderTransparentChicagoExtraFlags {
        std::uint32_t don_t_fade_active_camouflage : 1;
        std::uint32_t numeric_countdown_timer : 1;
    };

    ENDIAN_TEMPLATE(EndianType) struct ShaderTransparentChicago : Shader<EndianType> {
        std::int8_t numeric_counter_limit;
        ShaderTransparentChicagoFlags shader_transparent_chicago_extended_flags;
        EndianType<ShaderFirstMapType> first_map_type;
        EndianType<ShaderFramebufferBlendFunction> framebuffer_blend_function;
        EndianType<ShaderFramebufferFadeMode> framebuffer_fade_mode;
        EndianType<ShaderAnimationSource> framebuffer_fade_source;
        PAD(0x2);
        EndianType<float> lens_flare_spacing;
        TagDependency<EndianType> lens_flare; // lens_flare
        TagReflexive<EndianType, ShaderTransparentChicagoTransparentLayer> extra_layers;
        TagReflexive<EndianType, ShaderTransparentChicagoMap> maps;
        EndianType<ShaderTransparentChicagoExtraFlags> extra_flags;
        PAD(0x8);

        ENDIAN_TEMPLATE(NewType) operator ShaderTransparentChicago<NewType>() const noexcept {
            ShaderTransparentChicago<NewType> copy = {};
            COPY_SHADER_DATA
            COPY_THIS(numeric_counter_limit);
            COPY_THIS(shader_transparent_chicago_extended_flags);
            COPY_THIS(first_map_type);
            COPY_THIS(framebuffer_blend_function);
            COPY_THIS(framebuffer_fade_mode);
            COPY_THIS(framebuffer_fade_source);
            COPY_THIS(lens_flare_spacing);
            COPY_THIS(lens_flare);
            COPY_THIS(extra_layers);
            COPY_THIS(maps);
            COPY_THIS(extra_flags);
            return copy;
        }
    };
    static_assert(sizeof(ShaderTransparentChicago<BigEndian>) == 0x6C);

    void compile_shader_transparent_chicago_tag(CompiledTag &compiled, const std::byte *data, std::size_t size);
}
