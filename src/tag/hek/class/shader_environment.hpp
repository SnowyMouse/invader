/*
 * Invader (c) 2018 Kavawuvi
 *
 * This program is free software under the GNU General Public License v3.0 or later. See LICENSE for more information.
 */

#ifndef INVADER__TAG__HEK__CLASS__SHADER_ENVIRONMENT_HPP
#define INVADER__TAG__HEK__CLASS__SHADER_ENVIRONMENT_HPP

#include "shader.hpp"

namespace Invader::HEK {
    struct ShaderEnvironmentFlags {
        std::uint16_t alpha_tested : 1;
        std::uint16_t bump_map_is_specular_mask : 1;
        std::uint16_t true_atmospheric_fog : 1;
    };

    struct ShaderEnvironmentDiffuseFlags {
        std::uint16_t rescale_detail_maps : 1;
        std::uint16_t rescale_bump_map : 1;
    };

    struct ShaderEnvironmentSelfIlluminationFlags {
        std::uint16_t unfiltered : 1;
    };

    struct ShaderEnvironmentSpecularFlags {
        std::uint16_t overbright : 1;
        std::uint16_t extra_shiny : 1;
        std::uint16_t lightmap_is_specular : 1;
    };

    struct ShaderEnvironmentReflectionFlags {
        std::uint16_t dynamic_mirror : 1;
    };

    enum ShaderEnvironmentType : TagEnum {
        SHADER_ENVIRONMENT_TYPE_NORMAL,
        SHADER_ENVIRONMENT_TYPE_BLENDED,
        SHADER_ENVIRONMENT_TYPE_BLENDED_BASE_SPECULAR
    };

    enum ShaderEnvironmentReflectionType : TagEnum {
        SHADER_ENVIRONMENT_REFLECTION_TYPE__BUMPED_CUBE_MAP,
        SHADER_ENVIRONMENT_REFLECTION_TYPE__FLAT_CUBE_MAP,
        SHADER_ENVIRONMENT_REFLECTION_TYPE__BUMPED_RADIOSITY
    };

    enum ShaderEnvironmentDetailMapFunction : TagEnum {
        SHADER_ENVIRONMENT_DETAIL_MAP_FUNCTION_DOUBLE_BIASED_MULTIPLY,
        SHADER_ENVIRONMENT_DETAIL_MAP_FUNCTION_MULTIPLY,
        SHADER_ENVIRONMENT_DETAIL_MAP_FUNCTION_DOUBLE_BIASED_ADD
    };

    ENDIAN_TEMPLATE(EndianType) struct ShaderEnvironment : Shader<EndianType> {
        EndianType<ShaderEnvironmentFlags> shader_environment_flags;
        EndianType<ShaderEnvironmentType> shader_environment_type;
        EndianType<float> lens_flare_spacing;
        TagDependency<EndianType> lens_flare; // lens_flare
        PAD(0x2C);
        EndianType<ShaderEnvironmentDiffuseFlags> diffuse_flags;
        PAD(0x2);
        PAD(0x18);
        TagDependency<EndianType> base_map; // bitmap
        PAD(0x18);
        EndianType<ShaderEnvironmentDetailMapFunction> detail_map_function;
        PAD(0x2);
        EndianType<float> primary_detail_map_scale;
        TagDependency<EndianType> primary_detail_map; // bitmap
        EndianType<float> secondary_detail_map_scale;
        TagDependency<EndianType> secondary_detail_map; // bitmap
        PAD(0x18);
        EndianType<ShaderEnvironmentDetailMapFunction> micro_detail_map_function;
        PAD(0x2);
        EndianType<float> micro_detail_map_scale;
        TagDependency<EndianType> micro_detail_map; // bitmap
        ColorRGB<EndianType> material_color;
        PAD(0xC);
        EndianType<float> bump_map_scale;
        TagDependency<EndianType> bump_map; // bitmap
        Point2D<LittleEndian> bump_map_scale_xy;
        PAD(0x10);
        EndianType<FunctionType2> u_animation_function;
        PAD(0x2);
        EndianType<float> u_animation_period;
        EndianType<float> u_animation_scale;
        EndianType<FunctionType2> v_animation_function;
        PAD(0x2);
        EndianType<float> v_animation_period;
        EndianType<float> v_animation_scale;
        PAD(0x18);
        EndianType<ShaderEnvironmentSelfIlluminationFlags> self_illumination_flags;
        PAD(0x2);
        PAD(0x18);
        ColorRGB<EndianType> primary_on_color;
        ColorRGB<EndianType> primary_off_color;
        EndianType<FunctionType2> primary_animation_function;
        PAD(0x2);
        EndianType<float> primary_animation_period;
        EndianType<float> primary_animation_phase;
        PAD(0x18);
        ColorRGB<EndianType> secondary_on_color;
        ColorRGB<EndianType> secondary_off_color;
        EndianType<FunctionType2> secondary_animation_function;
        PAD(0x2);
        EndianType<float> secondary_animation_period;
        EndianType<float> secondary_animation_phase;
        PAD(0x18);
        ColorRGB<EndianType> plasma_on_color;
        ColorRGB<EndianType> plasma_off_color;
        EndianType<FunctionType2> plasma_animation_function;
        PAD(0x2);
        EndianType<float> plasma_animation_period;
        EndianType<float> plasma_animation_phase;
        PAD(0x18);
        EndianType<float> map_scale;
        TagDependency<EndianType> map; // bitmap
        PAD(0x18);
        EndianType<ShaderEnvironmentSpecularFlags> specular_flags;
        PAD(0x2);
        PAD(0x10);
        EndianType<Fraction> brightness;
        PAD(0x14);
        ColorRGB<EndianType> perpendicular_color;
        ColorRGB<EndianType> parallel_color;
        PAD(0x10);
        EndianType<ShaderEnvironmentReflectionFlags> reflection_flags;
        EndianType<ShaderEnvironmentReflectionType> reflection_type;
        EndianType<Fraction> lightmap_brightness_scale;
        PAD(0x1C);
        EndianType<Fraction> perpendicular_brightness;
        EndianType<Fraction> parallel_brightness;
        PAD(0x10);
        PAD(0x8);
        PAD(0x10);
        TagDependency<EndianType> reflection_cube_map; // bitmap
        PAD(0x10);

        ENDIAN_TEMPLATE(NewType) operator ShaderEnvironment<NewType>() const noexcept {
            ShaderEnvironment<NewType> copy = {};
            COPY_SHADER_DATA
            COPY_THIS(shader_environment_flags);
            COPY_THIS(shader_environment_type);
            COPY_THIS(lens_flare_spacing);
            COPY_THIS(lens_flare);
            COPY_THIS(diffuse_flags);
            COPY_THIS(base_map);
            COPY_THIS(detail_map_function);
            COPY_THIS(primary_detail_map_scale);
            COPY_THIS(primary_detail_map);
            COPY_THIS(secondary_detail_map_scale);
            COPY_THIS(secondary_detail_map);
            COPY_THIS(micro_detail_map_function);
            COPY_THIS(micro_detail_map_scale);
            COPY_THIS(micro_detail_map);
            COPY_THIS(material_color);
            COPY_THIS(bump_map_scale);
            COPY_THIS(bump_map);
            COPY_THIS(bump_map_scale_xy);
            COPY_THIS(u_animation_function);
            COPY_THIS(u_animation_period);
            COPY_THIS(u_animation_scale);
            COPY_THIS(v_animation_function);
            COPY_THIS(v_animation_period);
            COPY_THIS(v_animation_scale);
            COPY_THIS(self_illumination_flags);
            COPY_THIS(primary_on_color);
            COPY_THIS(primary_off_color);
            COPY_THIS(primary_animation_function);
            COPY_THIS(primary_animation_period);
            COPY_THIS(primary_animation_phase);
            COPY_THIS(secondary_on_color);
            COPY_THIS(secondary_off_color);
            COPY_THIS(secondary_animation_function);
            COPY_THIS(secondary_animation_period);
            COPY_THIS(secondary_animation_phase);
            COPY_THIS(plasma_on_color);
            COPY_THIS(plasma_off_color);
            COPY_THIS(plasma_animation_function);
            COPY_THIS(plasma_animation_period);
            COPY_THIS(plasma_animation_phase);
            COPY_THIS(map_scale);
            COPY_THIS(map);
            COPY_THIS(specular_flags);
            COPY_THIS(brightness);
            COPY_THIS(perpendicular_color);
            COPY_THIS(parallel_color);
            COPY_THIS(reflection_flags);
            COPY_THIS(reflection_type);
            COPY_THIS(lightmap_brightness_scale);
            COPY_THIS(perpendicular_brightness);
            COPY_THIS(parallel_brightness);
            COPY_THIS(reflection_cube_map);
            return copy;
        }
    };
    static_assert(sizeof(ShaderEnvironment<BigEndian>) == 0x344);

    void compile_shader_environment_tag(CompiledTag &compiled, const std::byte *data, std::size_t size);
}
#endif
