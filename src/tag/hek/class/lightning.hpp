/*
 * Invader (c) 2018 Kavawuvi
 *
 * This program is free software under the GNU General Public License v3.0 or later. See LICENSE for more information.
 */

#pragma once

#include "../../compiled_tag.hpp"
#include "../../../hek/data_type.hpp"
#include "../header.hpp"

namespace Invader::HEK {
    enum LightningScaleSource : TagEnum {
        LIGHTNING_SCALE_SOURCE_NONE,
        LIGHTNING_SCALE_SOURCE_A_OUT,
        LIGHTNING_SCALE_SOURCE_B_OUT,
        LIGHTNING_SCALE_SOURCE_C_OUT,
        LIGHTNING_SCALE_SOURCE_D_OUT
    };
    enum LightningTintModulationSource : TagEnum {
        LIGHTNING_TINT_MODULATION_SOURCE_NONE,
        LIGHTNING_TINT_MODULATION_SOURCE_A,
        LIGHTNING_TINT_MODULATION_SOURCE_B,
        LIGHTNING_TINT_MODULATION_SOURCE_C,
        LIGHTNING_TINT_MODULATION_SOURCE_D,
    };

    enum LightningFramebufferBlendFunction : TagEnum {
        LIGHTNING_FRAMEBUFFER_BLEND_FUNCTION_ALPHA_BLEND,
        LIGHTNING_FRAMEBUFFER_BLEND_FUNCTION_MULTIPLY,
        LIGHTNING_FRAMEBUFFER_BLEND_FUNCTION_DOUBLE_MULTIPLY,
        LIGHTNING_FRAMEBUFFER_BLEND_FUNCTION_ADD,
        LIGHTNING_FRAMEBUFFER_BLEND_FUNCTION_SUBTRACT,
        LIGHTNING_FRAMEBUFFER_BLEND_FUNCTION_COMPONENT_MIN,
        LIGHTNING_FRAMEBUFFER_BLEND_FUNCTION_COMPONENT_MAX,
        LIGHTNING_FRAMEBUFFER_BLEND_FUNCTION_ALPHA_MULTIPLY_ADD
    };
    enum LightningFramebufferFadeMode : TagEnum {
        LIGHTNING_FRAMEBUFFER_FADE_MODE_NONE,
        LIGHTNING_FRAMEBUFFER_FADE_MODE_FADE_WHEN_PERPENDICULAR,
        LIGHTNING_FRAMEBUFFER_FADE_MODE_FADE_WHEN_PARALLEL
    };

    struct LightningMarkerFlag {
        std::uint16_t not_connected_to_next_marker : 1;
    };

    ENDIAN_TEMPLATE(EndianType) struct LightningMarker {
        TagString attachment_marker;
        EndianType<LightningMarkerFlag> flags;
        PAD(0x2);
        EndianType<std::int16_t> octaves_to_next_marker;
        PAD(0x2);
        PAD(0x4C);
        Vector3D<EndianType> random_position_bounds;
        EndianType<float> random_jitter;
        EndianType<float> thickness;
        ColorARGB<EndianType> tint;
        PAD(0x4C);

        ENDIAN_TEMPLATE(NewType) operator LightningMarker<NewType>() const noexcept {
            LightningMarker<NewType> copy = {};
            COPY_THIS(attachment_marker);
            COPY_THIS(flags);
            COPY_THIS(octaves_to_next_marker);
            COPY_THIS(random_position_bounds);
            COPY_THIS(random_jitter);
            COPY_THIS(thickness);
            COPY_THIS(tint);
            return copy;
        }
    };
    static_assert(sizeof(LightningMarker<NativeEndian>) == 0xE4);

    struct LightningShaderFlags {
        std::uint16_t sort_bias : 1;
        std::uint16_t nonlinear_tint : 1;
        std::uint16_t don_t_overdraw_fp_weapon : 1;
    };

    struct LightningShaderMapFlags {
        std::uint16_t unfiltered : 1;
    };

    ENDIAN_TEMPLATE(EndianType) struct LightningShader {
        PAD(0x28);
        EndianType<LightningShaderFlags> shader_flags;
        EndianType<LightningFramebufferBlendFunction> framebuffer_blend_function;
        EndianType<LightningFramebufferFadeMode> framebuffer_fade_mode;
        EndianType<LightningShaderMapFlags> map_flags;
        PAD(0x1C);
        PAD(0x10);
        PAD(0x2);
        PAD(0x2);
        PAD(0x38);
        PAD(0x1C);

        ENDIAN_TEMPLATE(NewType) operator LightningShader<NewType>() const noexcept {
            LightningShader<NewType> copy = {};
            COPY_THIS(shader_flags);
            COPY_THIS(framebuffer_blend_function);
            COPY_THIS(framebuffer_fade_mode);
            COPY_THIS(map_flags);
            return copy;
        }
    };
    static_assert(sizeof(LightningShader<NativeEndian>) == 0xB4);

    ENDIAN_TEMPLATE(EndianType) struct Lightning {
        PAD(0x2);
        EndianType<std::int16_t> count;
        PAD(0x10);
        EndianType<float> near_fade_distance;
        EndianType<float> far_fade_distance;
        PAD(0x10);
        EndianType<LightningScaleSource> jitter_scale_source;
        EndianType<LightningScaleSource> thickness_scale_source;
        EndianType<LightningTintModulationSource> tint_modulation_source;
        EndianType<LightningScaleSource> brightness_scale_source;
        TagDependency<EndianType> bitmap; // bitmap
        PAD(0x54);
        TagReflexive<EndianType, LightningMarker> markers;
        TagReflexive<EndianType, LightningShader> shader;
        PAD(0x58);

        ENDIAN_TEMPLATE(NewType) operator Lightning<NewType>() const noexcept {
            Lightning<NewType> copy = {};
            COPY_THIS(count);
            COPY_THIS(near_fade_distance);
            COPY_THIS(far_fade_distance);
            COPY_THIS(jitter_scale_source);
            COPY_THIS(thickness_scale_source);
            COPY_THIS(tint_modulation_source);
            COPY_THIS(brightness_scale_source);
            COPY_THIS(bitmap);
            COPY_THIS(markers);
            COPY_THIS(shader);
            return copy;
        }
    };
    static_assert(sizeof(Lightning<BigEndian>) == 0x108);

    void compile_lightning_tag(CompiledTag &compiled, const std::byte *data, std::size_t size);
}
