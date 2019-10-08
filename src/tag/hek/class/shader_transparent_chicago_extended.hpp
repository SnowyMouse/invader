// SPDX-License-Identifier: GPL-3.0-only

#ifndef INVADER__TAG__HEK__CLASS__SHADER_TRANSPARENT_CHICAGO_EXTENDED_HPP
#define INVADER__TAG__HEK__CLASS__SHADER_TRANSPARENT_CHICAGO_EXTENDED_HPP

#include "shader_transparent_chicago.hpp"

namespace Invader::HEK {
    ENDIAN_TEMPLATE(EndianType) struct ShaderTransparentChicagoExtended : Shader<EndianType> {
        std::uint8_t numeric_counter_limit;
        ShaderTransparentChicagoFlags shader_transparent_chicago_extended_flags;
        EndianType<ShaderFirstMapType> first_map_type;
        EndianType<FramebufferBlendFunction> framebuffer_blend_function;
        EndianType<FramebufferFadeMode> framebuffer_fade_mode;
        EndianType<FunctionOut> framebuffer_fade_source;
        PAD(0x2);
        EndianType<float> lens_flare_spacing;
        TagDependency<EndianType> lens_flare; // lens_flare
        TagReflexive<EndianType, ShaderTransparentExtraLayer> extra_layers;
        TagReflexive<EndianType, ShaderTransparentChicagoMap> maps_4_stage;
        TagReflexive<EndianType, ShaderTransparentChicagoMap> maps_2_stage;
        EndianType<ShaderTransparentChicagoExtraFlags> extra_flags;
        PAD(0x8);

        ENDIAN_TEMPLATE(NewType) operator ShaderTransparentChicagoExtended<NewType>() const noexcept {
            ShaderTransparentChicagoExtended<NewType> copy = {};
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
            COPY_THIS(maps_4_stage);
            COPY_THIS(maps_2_stage);
            COPY_THIS(extra_flags);
            return copy;
        }
    };
    static_assert(sizeof(ShaderTransparentChicagoExtended<BigEndian>) == 0x78);

    void compile_shader_transparent_chicago_extended_tag(CompiledTag &compiled, const std::byte *data, std::size_t size);
}
#endif
