// SPDX-License-Identifier: GPL-3.0-only

#ifndef INVADER__TAG__HEK__CLASS__SHADER_TRANSPARENT_METER_HPP
#define INVADER__TAG__HEK__CLASS__SHADER_TRANSPARENT_METER_HPP

#include "shader.hpp"

namespace Invader::HEK {
    struct ShaderTransparentMeterFlags {
        std::uint16_t decal : 1;
        std::uint16_t two_sided : 1;
        std::uint16_t flash_color_is_negative : 1;
        std::uint16_t tint_mode_2 : 1;
        std::uint16_t unfiltered : 1;
    };

    ENDIAN_TEMPLATE(EndianType) struct ShaderTransparentMeter : Shader<EndianType> {
        EndianType<ShaderTransparentMeterFlags> meter_flags;
        PAD(0x2);
        PAD(0x20);
        TagDependency<EndianType> map; // bitmap
        PAD(0x20);
        ColorRGB<EndianType> gradient_min_color;
        ColorRGB<EndianType> gradient_max_color;
        ColorRGB<EndianType> background_color;
        ColorRGB<EndianType> flash_color;
        ColorRGB<EndianType> meter_tint_color;
        EndianType<Fraction> meter_transparency;
        EndianType<Fraction> background_transparency;
        PAD(0x18);
        EndianType<FunctionOut> meter_brightness_source;
        EndianType<FunctionOut> flash_brightness_source;
        EndianType<FunctionOut> value_source;
        EndianType<FunctionOut> gradient_source;
        EndianType<FunctionOut> flash_extension_source;
        PAD(0x2);
        PAD(0x20);

        ENDIAN_TEMPLATE(NewType) operator ShaderTransparentMeter<NewType>() const noexcept {
            ShaderTransparentMeter<NewType> copy = {};
            COPY_SHADER_DATA
            COPY_THIS(meter_flags);
            COPY_THIS(map);
            COPY_THIS(gradient_min_color);
            COPY_THIS(gradient_max_color);
            COPY_THIS(background_color);
            COPY_THIS(flash_color);
            COPY_THIS(meter_tint_color);
            COPY_THIS(meter_transparency);
            COPY_THIS(background_transparency);
            COPY_THIS(meter_brightness_source);
            COPY_THIS(flash_brightness_source);
            COPY_THIS(value_source);
            COPY_THIS(gradient_source);
            COPY_THIS(flash_extension_source);
            return copy;
        }
    };
    static_assert(sizeof(ShaderTransparentMeter<BigEndian>) == 0x104);

    void compile_shader_transparent_meter_tag(CompiledTag &compiled, const std::byte *data, std::size_t size);
}
#endif
