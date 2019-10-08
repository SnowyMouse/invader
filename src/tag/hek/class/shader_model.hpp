// SPDX-License-Identifier: GPL-3.0-only

#ifndef INVADER__TAG__HEK__CLASS__SHADER_MODEL_HPP
#define INVADER__TAG__HEK__CLASS__SHADER_MODEL_HPP

#include "shader.hpp"

namespace Invader::HEK {
    struct ShaderModelFlags {
        std::uint16_t detail_after_reflection : 1;
        std::uint16_t two_sided : 1;
        std::uint16_t not_alpha_tested : 1;
        std::uint16_t alpha_blended_decal : 1;
        std::uint16_t true_atmospheric_fog : 1;
        std::uint16_t disable_two_sided_culling : 1;
    };

    struct ShaderModelMoreFlags {
        std::uint16_t no_random_phase : 1;
    };

    enum ShaderModelDetailMask : TagEnum {
        SHADER_MODEL_DETAIL_MASK_NONE,
        SHADER_MODEL_DETAIL_MASK_REFLECTION_MASK_INVERSE,
        SHADER_MODEL_DETAIL_MASK_REFLECTION_MASK,
        SHADER_MODEL_DETAIL_MASK_SELF_ILLUMINATION_MASK_INVERSE,
        SHADER_MODEL_DETAIL_MASK_SELF_ILLUMINATION_MASK,
        SHADER_MODEL_DETAIL_MASK_CHANGE_COLOR_MASK_INVERSE,
        SHADER_MODEL_DETAIL_MASK_CHANGE_COLOR_MASK,
        SHADER_MODEL_DETAIL_MASK_MULTIPURPOSE_MAP_ALPHA_INVERSE,
        SHADER_MODEL_DETAIL_MASK_MULTIPURPOSE_MAP_ALPHA
    };

    ENDIAN_TEMPLATE(EndianType) struct ShaderModel : Shader<EndianType> {
        EndianType<ShaderModelFlags> shader_model_flags;
        PAD(0x2);
        PAD(0xC);
        EndianType<Fraction> translucency;
        PAD(0x10);
        EndianType<FunctionNameNullable> change_color_source;
        PAD(0x2);
        PAD(0x1C);
        EndianType<ShaderModelMoreFlags> shader_model_more_flags;
        PAD(0x2);
        EndianType<FunctionNameNullable> color_source;
        EndianType<WaveFunction> animation_function;
        EndianType<float> animation_period;
        ColorRGB<EndianType> animation_color_lower_bound;
        ColorRGB<EndianType> animation_color_upper_bound;
        PAD(0xC);
        EndianType<float> map_u_scale;
        EndianType<float> map_v_scale;
        TagDependency<EndianType> base_map; // bitmap
        PAD(0x8);
        TagDependency<EndianType> multipurpose_map; // bitmap
        PAD(0x8);
        EndianType<ShaderDetailFunction> detail_function;
        EndianType<ShaderModelDetailMask> detail_mask;
        EndianType<float> detail_map_scale;
        TagDependency<EndianType> detail_map; // bitmap
        EndianType<float> detail_map_v_scale;
        PAD(0xC);
        EndianType<FunctionOut> u_animation_source;
        EndianType<WaveFunction> u_animation_function;
        EndianType<float> u_animation_period;
        EndianType<float> u_animation_phase;
        EndianType<float> u_animation_scale;
        EndianType<FunctionOut> v_animation_source;
        EndianType<WaveFunction> v_animation_function;
        EndianType<float> v_animation_period;
        EndianType<float> v_animation_phase;
        EndianType<float> v_animation_scale;
        EndianType<FunctionOut> rotation_animation_source;
        EndianType<WaveFunction> rotation_animation_function;
        EndianType<float> rotation_animation_period;
        EndianType<float> rotation_animation_phase;
        EndianType<float> rotation_animation_scale;
        Point2D<EndianType> rotation_animation_center;
        PAD(0x8);
        EndianType<float> reflection_falloff_distance;
        EndianType<float> reflection_cutoff_distance;
        EndianType<Fraction> perpendicular_brightness;
        ColorRGB<EndianType> perpendicular_tint_color;
        EndianType<Fraction> parallel_brightness;
        ColorRGB<EndianType> parallel_tint_color;
        TagDependency<EndianType> reflection_cube_map; // bitmap
        PAD(0x10);
        EndianType<float> unknown;
        PAD(0x10);
        PAD(0x20);

        ENDIAN_TEMPLATE(NewEndian) operator ShaderModel<NewEndian>() const noexcept {
            ShaderModel<NewEndian> copy = {};
            COPY_SHADER_DATA
            COPY_THIS(shader_model_flags);
            COPY_THIS(translucency);
            COPY_THIS(change_color_source);
            COPY_THIS(shader_model_more_flags);
            COPY_THIS(color_source);
            COPY_THIS(animation_function);
            COPY_THIS(animation_period);
            COPY_THIS(animation_color_lower_bound);
            COPY_THIS(animation_color_upper_bound);
            COPY_THIS(map_u_scale);
            COPY_THIS(map_v_scale);
            COPY_THIS(base_map);
            COPY_THIS(multipurpose_map);
            COPY_THIS(detail_function);
            COPY_THIS(detail_mask);
            COPY_THIS(detail_map_scale);
            COPY_THIS(detail_map);
            COPY_THIS(detail_map_v_scale);
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
            COPY_THIS(reflection_falloff_distance);
            COPY_THIS(reflection_cutoff_distance);
            COPY_THIS(perpendicular_brightness);
            COPY_THIS(perpendicular_tint_color);
            COPY_THIS(parallel_brightness);
            COPY_THIS(parallel_tint_color);
            COPY_THIS(reflection_cube_map);
            return copy;
        }
    };
    static_assert(sizeof(ShaderModel<BigEndian>) == 0x1B8);

    void compile_shader_model_tag(CompiledTag &compiled, const std::byte *data, std::size_t size);
}
#endif
