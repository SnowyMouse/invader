/*
 * Invader (c) 2018 Kavawuvi
 *
 * This program is free software under the GNU General Public License v3.0 or later. See LICENSE for more information.
 */

#ifndef INVADER__TAG__HEK__CLASS__SHADER_TRANSPARENT_GENERIC_HPP
#define INVADER__TAG__HEK__CLASS__SHADER_TRANSPARENT_GENERIC_HPP

#include "shader.hpp"

namespace Invader::HEK {
    struct ShaderTransparentGenericMapFlag {
        std::uint16_t unfiltered : 1;
        std::uint16_t u_clamped : 1;
        std::uint16_t v_clamped : 1;
    };

    ENDIAN_TEMPLATE(EndianType) struct ShaderTransparentGenericMap {
        EndianType<ShaderTransparentGenericMapFlag> flags;
        PAD(0x2);
        EndianType<float> map_u_scale;
        EndianType<float> map_v_scale;
        EndianType<float> map_u_offset;
        EndianType<float> map_v_offset;
        EndianType<float> map_rotation;
        EndianType<Fraction> mapmap_bias;
        TagDependency<EndianType> map; // .bitmap
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

        ENDIAN_TEMPLATE(NewType) operator ShaderTransparentGenericMap<NewType>() const noexcept {
            ShaderTransparentGenericMap<NewType> copy = {};
            COPY_THIS(flags);
            COPY_THIS(map_u_scale);
            COPY_THIS(map_v_scale);
            COPY_THIS(map_u_offset);
            COPY_THIS(map_v_offset);
            COPY_THIS(map_rotation);
            COPY_THIS(mapmap_bias);
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
    static_assert(sizeof(ShaderTransparentGenericMap<BigEndian>) == 0x64);

    enum ShaderTransparentGenericStageColorSource : TagEnum {
        SHADER_TRANSPARENT_GENERIC_COLOR_SOURCE_NONE,
        SHADER_TRANSPARENT_GENERIC_COLOR_SOURCE_A,
        SHADER_TRANSPARENT_GENERIC_COLOR_SOURCE_B,
        SHADER_TRANSPARENT_GENERIC_COLOR_SOURCE_C,
        SHADER_TRANSPARENT_GENERIC_COLOR_SOURCE_D
    };

    enum ShaderTransparentGenericStageInputColor : TagEnum {
        SHADER_TRANSPARENT_GENERIC_STAGE_INPUT_COLOR_ZERO,
        SHADER_TRANSPARENT_GENERIC_STAGE_INPUT_COLOR_ONE,
        SHADER_TRANSPARENT_GENERIC_STAGE_INPUT_COLOR_ONE_HALF,
        SHADER_TRANSPARENT_GENERIC_STAGE_INPUT_COLOR_NEGATIVE_ONE,
        SHADER_TRANSPARENT_GENERIC_STAGE_INPUT_COLOR_NEGATIVE_ONE_HALF,
        SHADER_TRANSPARENT_GENERIC_STAGE_INPUT_COLOR_MAP_COLOR_0,
        SHADER_TRANSPARENT_GENERIC_STAGE_INPUT_COLOR_MAP_COLOR_1,
        SHADER_TRANSPARENT_GENERIC_STAGE_INPUT_COLOR_MAP_COLOR_2,
        SHADER_TRANSPARENT_GENERIC_STAGE_INPUT_COLOR_MAP_COLOR_3,
        SHADER_TRANSPARENT_GENERIC_STAGE_INPUT_COLOR_VERTEX_COLOR_0__DIFFUSE_LIGHT,
        SHADER_TRANSPARENT_GENERIC_STAGE_INPUT_COLOR_VERTEX_COLOR_1__FADE_PERPENDICULAR,
        SHADER_TRANSPARENT_GENERIC_STAGE_INPUT_COLOR_SCRATCH_COLOR_0,
        SHADER_TRANSPARENT_GENERIC_STAGE_INPUT_COLOR_SCRATCH_COLOR_1,
        SHADER_TRANSPARENT_GENERIC_STAGE_INPUT_COLOR_CONSTANT_COLOR_0,
        SHADER_TRANSPARENT_GENERIC_STAGE_INPUT_COLOR_CONSTANT_COLOR_1,
        SHADER_TRANSPARENT_GENERIC_STAGE_INPUT_COLOR_MAP_ALPHA_0,
        SHADER_TRANSPARENT_GENERIC_STAGE_INPUT_COLOR_MAP_ALPHA_1,
        SHADER_TRANSPARENT_GENERIC_STAGE_INPUT_COLOR_MAP_ALPHA_2,
        SHADER_TRANSPARENT_GENERIC_STAGE_INPUT_COLOR_MAP_ALPHA_3,
        SHADER_TRANSPARENT_GENERIC_STAGE_INPUT_COLOR_VERTEX_ALPHA_0__FADE_NONE,
        SHADER_TRANSPARENT_GENERIC_STAGE_INPUT_COLOR_VERTEX_ALPHA_1__FADE_PERPENDICULAR,
        SHADER_TRANSPARENT_GENERIC_STAGE_INPUT_COLOR_SCRATCH_ALPHA_0,
        SHADER_TRANSPARENT_GENERIC_STAGE_INPUT_COLOR_SCRATCH_ALPHA_1,
        SHADER_TRANSPARENT_GENERIC_STAGE_INPUT_COLOR_CONSTANT_ALPHA_0,
        SHADER_TRANSPARENT_GENERIC_STAGE_INPUT_COLOR_CONSTANT_ALPHA_1
    };

    enum ShaderTransparentGenericStageInputAlpha : TagEnum {
        SHADER_TRANSPARENT_GENERIC_STAGE_INPUT_ALPHA_ZERO,
        SHADER_TRANSPARENT_GENERIC_STAGE_INPUT_ALPHA_ONE,
        SHADER_TRANSPARENT_GENERIC_STAGE_INPUT_ALPHA_ONE_HALF,
        SHADER_TRANSPARENT_GENERIC_STAGE_INPUT_ALPHA_NEGATIVE_ONE,
        SHADER_TRANSPARENT_GENERIC_STAGE_INPUT_ALPHA_NEGATIVE_ONE_HALF,
        SHADER_TRANSPARENT_GENERIC_STAGE_INPUT_ALPHA_MAP_ALPHA_0,
        SHADER_TRANSPARENT_GENERIC_STAGE_INPUT_ALPHA_MAP_ALPHA_1,
        SHADER_TRANSPARENT_GENERIC_STAGE_INPUT_ALPHA_MAP_ALPHA_2,
        SHADER_TRANSPARENT_GENERIC_STAGE_INPUT_ALPHA_MAP_ALPHA_3,
        SHADER_TRANSPARENT_GENERIC_STAGE_INPUT_ALPHA_VERTEX_ALPHA_0__FADE_NONE,
        SHADER_TRANSPARENT_GENERIC_STAGE_INPUT_ALPHA_VERTEX_ALPHA_1__FADE_PERPENDICULAR,
        SHADER_TRANSPARENT_GENERIC_STAGE_INPUT_ALPHA_SCRATCH_ALPHA_0,
        SHADER_TRANSPARENT_GENERIC_STAGE_INPUT_ALPHA_SCRATCH_ALPHA_1,
        SHADER_TRANSPARENT_GENERIC_STAGE_INPUT_ALPHA_CONSTANT_ALPHA_0,
        SHADER_TRANSPARENT_GENERIC_STAGE_INPUT_ALPHA_CONSTANT_ALPHA_1,
        SHADER_TRANSPARENT_GENERIC_STAGE_INPUT_ALPHA_MAP_BLUE_0,
        SHADER_TRANSPARENT_GENERIC_STAGE_INPUT_ALPHA_MAP_BLUE_1,
        SHADER_TRANSPARENT_GENERIC_STAGE_INPUT_ALPHA_MAP_BLUE_2,
        SHADER_TRANSPARENT_GENERIC_STAGE_INPUT_ALPHA_MAP_BLUE_3,
        SHADER_TRANSPARENT_GENERIC_STAGE_INPUT_ALPHA_VERTEX_BLUE_0__BLUE_LIGHT,
        SHADER_TRANSPARENT_GENERIC_STAGE_INPUT_ALPHA_VERTEX_BLUE_1__FADE_PARALLEL,
        SHADER_TRANSPARENT_GENERIC_STAGE_INPUT_ALPHA_SCRATCH_BLUE_0,
        SHADER_TRANSPARENT_GENERIC_STAGE_INPUT_ALPHA_SCRATCH_BLUE_1,
        SHADER_TRANSPARENT_GENERIC_STAGE_INPUT_ALPHA_CONSTANT_BLUE_0,
        SHADER_TRANSPARENT_GENERIC_STAGE_INPUT_ALPHA_CONSTANT_BLUE_1
    };

    enum ShaderTransparentGenericStageInputMappingColor : TagEnum {
        SHADER_TRANSPARENT_GENERIC_STAGE_INPUT_MAPPING_COLOR_CLAMP_X,
        SHADER_TRANSPARENT_GENERIC_STAGE_INPUT_MAPPING_COLOR__1__CLAMP_X,
        SHADER_TRANSPARENT_GENERIC_STAGE_INPUT_MAPPING_COLOR__2,
        SHADER_TRANSPARENT_GENERIC_STAGE_INPUT_MAPPING_COLOR__1__2,
        SHADER_TRANSPARENT_GENERIC_STAGE_INPUT_MAPPING_COLOR_CLAMP_X__1_2,
        SHADER_TRANSPARENT_GENERIC_STAGE_INPUT_MAPPING_COLOR__1_2__CLAMP_X,
        SHADER_TRANSPARENT_GENERIC_STAGE_INPUT_MAPPING_COLOR_X,
        SHADER_TRANSPARENT_GENERIC_STAGE_INPUT_MAPPING_COLOR_X_1
    };

    enum ShaderTransparentGenericStageOutputFunction : TagEnum {
        SHADER_TRANSPARENT_GENERIC_STAGE_OUTPUT_FUNCTION_MULTIPLY,
        SHADER_TRANSPARENT_GENERIC_STAGE_OUTPUT_FUNCTION_DOT_PRODUCT
    };

    enum ShaderTransparentGenericStageOutputMapping : TagEnum {
        SHADER_TRANSPARENT_GENERIC_STAGE_OUTPUT_MAPPING_COLOR_IDENTITY,
        SHADER_TRANSPARENT_GENERIC_STAGE_OUTPUT_MAPPING_COLOR_SCALE_BY_1_2,
        SHADER_TRANSPARENT_GENERIC_STAGE_OUTPUT_MAPPING_COLOR_SCALE_BY_2,
        SHADER_TRANSPARENT_GENERIC_STAGE_OUTPUT_MAPPING_COLOR_SCALE_BY_4,
        SHADER_TRANSPARENT_GENERIC_STAGE_OUTPUT_MAPPING_COLOR_BIAS_BY_1_2,
        SHADER_TRANSPARENT_GENERIC_STAGE_OUTPUT_MAPPING_COLOR_EXPAND_NORMAL
    };

    enum ShaderTransparentGenericStageOutput : TagEnum {
        SHADER_TRANSPARENT_GENERIC_STAGE_OUTPUT_ALPHA_DISCARD,
        SHADER_TRANSPARENT_GENERIC_STAGE_OUTPUT_ALPHA_SCRATCH_ALPHA_0__FINAL_ALPHA,
        SHADER_TRANSPARENT_GENERIC_STAGE_OUTPUT_ALPHA_SCRATCH_ALPHA_1,
        SHADER_TRANSPARENT_GENERIC_STAGE_OUTPUT_ALPHA_VERTEX_ALPHA_0__FOG,
        SHADER_TRANSPARENT_GENERIC_STAGE_OUTPUT_ALPHA_VERTEX_ALPHA_1,
        SHADER_TRANSPARENT_GENERIC_STAGE_OUTPUT_ALPHA_MAP_ALPHA_0,
        SHADER_TRANSPARENT_GENERIC_STAGE_OUTPUT_ALPHA_MAP_ALPHA_1,
        SHADER_TRANSPARENT_GENERIC_STAGE_OUTPUT_ALPHA_MAP_ALPHA_2,
        SHADER_TRANSPARENT_GENERIC_STAGE_OUTPUT_ALPHA_MAP_ALPHA_3
    };

    struct ShaderTransparentGenericStageFlags {
        std::uint16_t color_mux : 1;
        std::uint16_t alpha_mux : 1;
        std::uint16_t a_out_controls_color0_animation : 1;
    };

    ENDIAN_TEMPLATE(EndianType) struct ShaderTransparentGenericStage {
        EndianType<ShaderTransparentGenericStageFlags> flags;
        PAD(0x2);
        EndianType<ShaderTransparentGenericStageColorSource> color0_source;
        EndianType<ShaderAnimationSource> color0_animation_function;
        EndianType<float> color0_animation_period;
        ColorARGB<EndianType> color0_animation_lower_bound;
        ColorARGB<EndianType> color0_animation_upper_bound;
        ColorARGB<EndianType> color1;

        EndianType<ShaderTransparentGenericStageInputColor> input_a;
        EndianType<ShaderTransparentGenericStageInputMappingColor> input_a_mapping;
        EndianType<ShaderTransparentGenericStageInputColor> input_b;
        EndianType<ShaderTransparentGenericStageInputMappingColor> input_b_mapping;
        EndianType<ShaderTransparentGenericStageInputColor> input_c;
        EndianType<ShaderTransparentGenericStageInputMappingColor> input_c_mapping;
        EndianType<ShaderTransparentGenericStageInputColor> input_d;
        EndianType<ShaderTransparentGenericStageInputMappingColor> input_d_mapping;

        EndianType<ShaderTransparentGenericStageOutput> output_ab;
        EndianType<ShaderTransparentGenericStageOutputFunction> output_ab_function;
        EndianType<ShaderTransparentGenericStageOutput> output_bc;
        EndianType<ShaderTransparentGenericStageOutputFunction> output_cd_function;
        EndianType<ShaderTransparentGenericStageOutput> output_ab_cd_mux_sum;
        EndianType<ShaderTransparentGenericStageOutputMapping> output_mapping_color;

        EndianType<ShaderTransparentGenericStageInputAlpha> input_a_alpha;
        EndianType<ShaderTransparentGenericStageInputMappingColor> input_a_mapping_alpha;
        EndianType<ShaderTransparentGenericStageInputAlpha> input_b_alpha;
        EndianType<ShaderTransparentGenericStageInputMappingColor> input_b_mapping_alpha;
        EndianType<ShaderTransparentGenericStageInputAlpha> input_c_alpha;
        EndianType<ShaderTransparentGenericStageInputMappingColor> input_c_mapping_alpha;
        EndianType<ShaderTransparentGenericStageInputAlpha> input_d_alpha;
        EndianType<ShaderTransparentGenericStageInputMappingColor> input_d_mapping_alpha;

        EndianType<ShaderTransparentGenericStageOutput> output_ab_alpha;
        EndianType<ShaderTransparentGenericStageOutput> output_cd_alpha;
        EndianType<ShaderTransparentGenericStageOutput> output_ab_cd_mux_sum_alpha;
        EndianType<ShaderTransparentGenericStageOutputMapping> output_mapping_alpha;

        ENDIAN_TEMPLATE(NewType) operator ShaderTransparentGenericStage<NewType>() const noexcept {
            ShaderTransparentGenericStage<NewType> copy = {};
            COPY_THIS(flags);
            COPY_THIS(color0_source);
            COPY_THIS(color0_animation_function);
            COPY_THIS(color0_animation_period);
            COPY_THIS(color0_animation_lower_bound);
            COPY_THIS(color0_animation_upper_bound);
            COPY_THIS(color1);
            COPY_THIS(input_a);
            COPY_THIS(input_a_mapping);
            COPY_THIS(input_b);
            COPY_THIS(input_b_mapping);
            COPY_THIS(input_c);
            COPY_THIS(input_c_mapping);
            COPY_THIS(input_d);
            COPY_THIS(input_d_mapping);
            COPY_THIS(output_ab);
            COPY_THIS(output_ab_function);
            COPY_THIS(output_bc);
            COPY_THIS(output_cd_function);
            COPY_THIS(output_ab_cd_mux_sum);
            COPY_THIS(output_mapping_color);
            COPY_THIS(input_a_alpha);
            COPY_THIS(input_a_mapping_alpha);
            COPY_THIS(input_b_alpha);
            COPY_THIS(input_b_mapping_alpha);
            COPY_THIS(input_c_alpha);
            COPY_THIS(input_c_mapping_alpha);
            COPY_THIS(input_d_alpha);
            COPY_THIS(input_d_mapping_alpha);
            COPY_THIS(output_ab_alpha);
            COPY_THIS(output_cd_alpha);
            COPY_THIS(output_ab_cd_mux_sum_alpha);
            COPY_THIS(output_mapping_alpha);
            return copy;
        }
    };
    static_assert(sizeof(ShaderTransparentGenericStage<BigEndian>) == 0x70);

    struct ShaderTransparentGenericFlags {
		std::uint8_t alpha_tested : 1;
		std::uint8_t decal : 1;
		std::uint8_t two_sided : 1;
		std::uint8_t first_map_is_in_screenspace : 1;
		std::uint8_t draw_before_water : 1;
		std::uint8_t ignore_effect : 1;
		std::uint8_t scale_first_map_with_distance : 1;
		std::uint8_t numeric : 1;
	};

    ENDIAN_TEMPLATE(EndianType) struct ShaderTransparentGeneric : Shader<EndianType> {
        std::uint8_t numeric_counter_limit;
        ShaderTransparentGenericFlags shader_transparent_generic_flags;
        EndianType<ShaderFirstMapType> first_map_type;
        EndianType<ShaderFramebufferBlendFunction> framebuffer_blend_function;
        EndianType<ShaderFramebufferFadeMode> framebuffer_fade_mode;
        EndianType<ShaderAnimationSource> framebuffer_fade_source;
        PAD(0x2);
        EndianType<float> lens_flare_spacing;
        TagDependency<EndianType> lens_flare; // lens_flare
        TagReflexive<EndianType, ShaderTransparentExtraLayer> extra_layers;
        TagReflexive<EndianType, ShaderTransparentGenericMap> maps;
        TagReflexive<EndianType, ShaderTransparentGenericStage> stages;

        ENDIAN_TEMPLATE(NewType) operator ShaderTransparentGeneric<NewType>() const noexcept {
            ShaderTransparentGeneric<NewType> copy = {};
            COPY_SHADER_DATA
            COPY_THIS(numeric_counter_limit);
            COPY_THIS(shader_transparent_generic_flags);
            COPY_THIS(first_map_type);
            COPY_THIS(framebuffer_blend_function);
            COPY_THIS(framebuffer_fade_mode);
            COPY_THIS(framebuffer_fade_source);
            COPY_THIS(lens_flare_spacing);
            COPY_THIS(lens_flare);
            COPY_THIS(extra_layers);
            COPY_THIS(maps);
            COPY_THIS(stages);
            return copy;
        }
    };
    static_assert(sizeof(ShaderTransparentGeneric<BigEndian>) == 0x6C);

    void compile_shader_transparent_generic_tag(CompiledTag &compiled, const std::byte *data, std::size_t size);
}
#endif
