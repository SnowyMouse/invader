/*
 * Invader (c) 2018 Kavawuvi
 *
 * This program is free software under the GNU General Public License v3.0. See LICENSE for more information.
 */

#ifndef INVADER__TAG__HEK__CLASS__SHADER_TRANSPARENT_GLASS_HPP
#define INVADER__TAG__HEK__CLASS__SHADER_TRANSPARENT_GLASS_HPP

#include "shader.hpp"

namespace Invader::HEK {
    enum ShaderTransparentGlassReflectionType : TagEnum {
        SHADER_TRANSPARENT_GLASS_REFLECTION_TYPE_BUMPED_CUBE_MAP,
        SHADER_TRANSPARENT_GLASS_REFLECTION_TYPE_FLAT_CUBE_MAP,
        SHADER_TRANSPARENT_GLASS_REFLECTION_TYPE_DYNAMIC_MIRROR
    };

    struct ShaderTransparentGlassFlags {
        std::uint16_t alpha_tested : 1;
        std::uint16_t decal : 1;
        std::uint16_t two_sided : 1;
        std::uint16_t bump_map_is_specular_mask : 1;
    };

    ENDIAN_TEMPLATE(EndianType) struct ShaderTransparentGlass : Shader<EndianType> {
        EndianType<ShaderTransparentGlassFlags> shader_transparent_glass_flags;
        PAD(0x2);
        PAD(0x28);
        ColorRGB<EndianType> background_tint_color;
        EndianType<float> background_tint_map_scale;
        TagDependency<EndianType> background_tint_map; // bitmap
        PAD(0x14);
        PAD(0x2);
        EndianType<ShaderTransparentGlassReflectionType> reflection_type;
        EndianType<Fraction> perpendicular_brightness;
        ColorRGB<EndianType> perpendicular_tint_color;
        EndianType<Fraction> parallel_brightness;
        ColorRGB<EndianType> parallel_tint_color;
        TagDependency<EndianType> reflection_map; // bitmap
        EndianType<float> bump_map_scale;
        TagDependency<EndianType> bump_map; // bitmap
        PAD(0x80);
        PAD(0x4);
        EndianType<float> diffuse_map_scale;
        TagDependency<EndianType> diffuse_map; // bitmap
        EndianType<float> diffuse_detail_map_scale;
        TagDependency<EndianType> diffuse_detail_map; // bitmap
        PAD(0x1C);
        PAD(0x4);
        EndianType<float> specular_map_scale;
        TagDependency<EndianType> specular_map; // bitmap
        EndianType<float> specular_detail_map_scale;
        TagDependency<EndianType> specular_detail_map; // bitmap
        PAD(0x1C);

        ENDIAN_TEMPLATE(NewType) operator ShaderTransparentGlass<NewType>() const noexcept {
            ShaderTransparentGlass<NewType> copy = {};
            COPY_SHADER_DATA
            COPY_THIS(shader_transparent_glass_flags);
            COPY_THIS(background_tint_color);
            COPY_THIS(background_tint_map_scale);
            COPY_THIS(background_tint_map);
            COPY_THIS(reflection_type);
            COPY_THIS(perpendicular_brightness);
            COPY_THIS(perpendicular_tint_color);
            COPY_THIS(parallel_brightness);
            COPY_THIS(parallel_tint_color);
            COPY_THIS(reflection_map);
            COPY_THIS(bump_map_scale);
            COPY_THIS(bump_map);
            COPY_THIS(diffuse_map_scale);
            COPY_THIS(diffuse_map);
            COPY_THIS(diffuse_detail_map_scale);
            COPY_THIS(diffuse_detail_map);
            COPY_THIS(specular_map_scale);
            COPY_THIS(specular_map);
            COPY_THIS(specular_detail_map_scale);
            COPY_THIS(specular_detail_map);
            return copy;
        }
    };
    static_assert(sizeof(ShaderTransparentGlass<BigEndian>) == 0x1E0);

    void compile_shader_transparent_glass_tag(CompiledTag &compiled, const std::byte *data, std::size_t size);
}
#endif
