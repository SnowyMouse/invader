/*
 * Invader (c) 2018 Kavawuvi
 *
 * This program is free software under the GNU General Public License v3.0 or later. See LICENSE for more information.
 */

#ifndef INVADER__TAG__HEK__CLASS__DECAL_HPP
#define INVADER__TAG__HEK__CLASS__DECAL_HPP

#include "../../compiled_tag.hpp"
#include "../../../hek/data_type.hpp"
#include "../header.hpp"

namespace Invader::HEK {
    enum DecalType : TagEnum {
        DECAL_TYPE_SCRATCH,
        DECAL_TYPE_SPLATTER,
        DECAL_TYPE_BURN,
        DECAL_TYPE_PAINTED_SIGN
    };
    enum DecalLayer : TagEnum {
        DECAL_LAYER_PRIMARY,
        DECAL_LAYER_SECONDARY,
        DECAL_LAYER_LIGHT,
        DECAL_LAYER_ALPHA_TESTED,
        DECAL_LAYER_WATER
    };
    enum DecalBlendFunction : TagEnum {
        DECAL_FRAMEBUFFER_BLEND_FUNCTION_ALPHA_BLEND,
        DECAL_FRAMEBUFFER_BLEND_FUNCTION_MULTIPLY,
        DECAL_FRAMEBUFFER_BLEND_FUNCTION_DOUBLE_MULTIPLY,
        DECAL_FRAMEBUFFER_BLEND_FUNCTION_ADD,
        DECAL_FRAMEBUFFER_BLEND_FUNCTION_SUBTRACT,
        DECAL_FRAMEBUFFER_BLEND_FUNCTION_COMPONENT_MIN,
        DECAL_FRAMEBUFFER_BLEND_FUNCTION_COMPONENT_MAX,
        DECAL_FRAMEBUFFER_BLEND_FUNCTION_ALPHA_MULTIPLY_ADD
    };

    struct DecalFlags {
        std::uint16_t geometry_inherited_by_next_decal_in_chain : 1;
        std::uint16_t interpolate_color_in_hsv : 1;
        std::uint16_t _more_colors : 1;
        std::uint16_t no_random_rotation : 1;
        std::uint16_t water_effect : 1;
        std::uint16_t sapien_snap_to_axis : 1;
        std::uint16_t sapien_incremental_counter : 1;
        std::uint16_t animation_loop : 1;
        std::uint16_t preserve_aspect : 1;
    };

    ENDIAN_TEMPLATE(EndianType) struct Decal {
        EndianType<DecalFlags> flags;
        EndianType<DecalType> type;
        EndianType<DecalLayer> layer;
        PAD(0x2);
        TagDependency<EndianType> next_decal_in_chain; // decal
        Bounds<EndianType<float>> radius;
        PAD(0xC);
        Bounds<EndianType<Fraction>> intensity;
        ColorRGB<EndianType> color_lower_bounds;
        ColorRGB<EndianType> color_upper_bounds;
        PAD(0xC);
        EndianType<std::int16_t> animation_loop_frame;
        EndianType<std::int16_t> animation_speed;
        PAD(0x1C);
        Bounds<EndianType<float>> lifetime;
        Bounds<EndianType<float>> decay_time;
        PAD(0xC);
        PAD(0x28);
        PAD(0x2);
        PAD(0x2);
        EndianType<DecalBlendFunction> framebuffer_blend_function;
        PAD(0x2);
        PAD(0x14);
        TagDependency<EndianType> map; // bitmap
        PAD(0x14);
        EndianType<float> maximum_sprite_extent;
        PAD(0x4);
        PAD(0x8);

        ENDIAN_TEMPLATE(NewType) operator Decal<NewType>() const noexcept {
            Decal<NewType> copy = {};
            COPY_THIS(flags);
            COPY_THIS(type);
            COPY_THIS(layer);
            COPY_THIS(next_decal_in_chain);
            COPY_THIS(radius);
            COPY_THIS(intensity);
            COPY_THIS(color_lower_bounds);
            COPY_THIS(color_upper_bounds);
            COPY_THIS(animation_loop_frame);
            COPY_THIS(animation_speed);
            COPY_THIS(lifetime);
            COPY_THIS(decay_time);
            COPY_THIS(framebuffer_blend_function);
            COPY_THIS(maximum_sprite_extent);
            COPY_THIS(map);
            return copy;
        }
    };
    static_assert(sizeof(Decal<BigEndian>) == 0x10C);

    void compile_decal_tag(CompiledTag &compiled, const std::byte *data, std::size_t size);
}
#endif
