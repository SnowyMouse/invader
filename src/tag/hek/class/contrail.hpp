// SPDX-License-Identifier: GPL-3.0-only

#ifndef INVADER__TAG__HEK__CLASS__CONTRAIL_HPP
#define INVADER__TAG__HEK__CLASS__CONTRAIL_HPP

#include "../../compiled_tag.hpp"
#include "../../../hek/data_type.hpp"
#include "../header.hpp"
#include "particle.hpp"

namespace Invader::HEK {
    enum ContrailRenderType : TagEnum {
        CONTRAIL_RENDER_TYPE_VERTICAL_ORIENTATION,
        CONTRAIL_RENDER_TYPE_HORIZONTAL_ORIENTATION,
        CONTRAIL_RENDER_TYPE_MEDIA_MAPPED,
        CONTRAIL_RENDER_TYPE_GROUND_MAPPED,
        CONTRAIL_RENDER_TYPE_VIEWER_FACING,
        CONTRAIL_RENDER_TYPE_DOUBLE_MARKER_LINKED
    };

    enum ContrailFramebufferBlendFunction : TagEnum {
        CONTRAIL_FRAMEBUFFER_BLEND_FUNCTION_ALPHA_BLEND,
        CONTRAIL_FRAMEBUFFER_BLEND_FUNCTION_MULTIPLY,
        CONTRAIL_FRAMEBUFFER_BLEND_FUNCTION_DOUBLE_MULTIPLY,
        CONTRAIL_FRAMEBUFFER_BLEND_FUNCTION_ADD,
        CONTRAIL_FRAMEBUFFER_BLEND_FUNCTION_SUBTRACT,
        CONTRAIL_FRAMEBUFFER_BLEND_FUNCTION_COMPONENT_MIN,
        CONTRAIL_FRAMEBUFFER_BLEND_FUNCTION_COMPONENT_MAX,
        CONTRAIL_FRAMEBUFFER_BLEND_FUNCTION_ALPHA_MULTIPLY_ADD
    };

    enum ContrailFramebufferFadeMode : TagEnum {
        CONTRAIL_FRAMEBUFFER_FADE_MODE_NONE,
        CONTRAIL_FRAMEBUFFER_FADE_MODE_FADE_WHEN_PERPENDICULAR,
        CONTRAIL_FRAMEBUFFER_FADE_MODE_FADE_WHEN_PARALLEL
    };

    enum ContrailAnimationSource : TagEnum {
        CONTRAIL_ANIMATION_SOURCE_NONE,
        CONTRAIL_ANIMATION_SOURCE_A_OUT,
        CONTRAIL_ANIMATION_SOURCE_B_OUT,
        CONTRAIL_ANIMATION_SOURCE_C_OUT,
        CONTRAIL_ANIMATION_SOURCE_D_OUT
    };

    struct ContrailPointStateScaleFlags {
        std::uint32_t duration : 1;
        std::uint32_t duration_delta : 1;
        std::uint32_t transition_duration : 1;
        std::uint32_t transition_duration_delta : 1;
        std::uint32_t width : 1;
        std::uint32_t color : 1;
    };

    ENDIAN_TEMPLATE(EndianType) struct ContrailPointState {
        Bounds<EndianType<float>> duration;
        Bounds<EndianType<float>> transition_duration;
        TagDependency<EndianType> physics; // point_physics
        PAD(0x20);
        EndianType<float> width;
        ColorARGB<EndianType> color_lower_bound;
        ColorARGB<EndianType> color_upper_bound;
        EndianType<ContrailPointStateScaleFlags> scale_flags;

        ENDIAN_TEMPLATE(NewType) operator ContrailPointState<NewType>() const noexcept {
            ContrailPointState<NewType> copy = {};
            COPY_THIS(duration);
            COPY_THIS(transition_duration);
            COPY_THIS(physics);
            COPY_THIS(width);
            COPY_THIS(color_lower_bound);
            COPY_THIS(color_upper_bound);
            COPY_THIS(scale_flags);
            return copy;
        }
    };
    static_assert(sizeof(ContrailPointState<BigEndian>) == 0x68);

    struct ContrailFlags {
        std::uint16_t first_point_unfaded : 1;
        std::uint16_t last_point_unfaded : 1;
        std::uint16_t points_start_pinned_to_media : 1;
        std::uint16_t points_start_pinned_to_ground : 1;
        std::uint16_t points_always_pinned_to_media : 1;
        std::uint16_t points_always_pinned_to_ground : 1;
        std::uint16_t edge_effect_fades_slowly : 1;
    };

    struct ContrailScaleFlags {
        std::uint16_t point_generation_rate : 1;
        std::uint16_t point_velocity : 1;
        std::uint16_t point_velocity_delta : 1;
        std::uint16_t point_velocity_cone_angle : 1;
        std::uint16_t inherited_velocity_fraction : 1;
        std::uint16_t sequence_animation_rate : 1;
        std::uint16_t texture_scale_u : 1;
        std::uint16_t texture_scale_v : 1;
        std::uint16_t texture_animation_u : 1;
        std::uint16_t texture_animation_v : 1;
    };

    struct ContrailShaderFlags {
        std::uint16_t sort_bias : 1;
        std::uint16_t nonlinear_tint : 1;
        std::uint16_t don_t_overdraw_fp_weapon : 1;
    };

    struct ContrailMapFlags {
        std::uint16_t unfiltered : 1;
    };

    ENDIAN_TEMPLATE(EndianType) struct Contrail {
        EndianType<ContrailFlags> flags;
        EndianType<ContrailScaleFlags> scale_flags;
        EndianType<float> point_generation_rate;
        Bounds<EndianType<float>> point_velocity;
        EndianType<Angle> point_velocity_cone_angle;
        EndianType<Fraction> inherited_velocity_fraction;
        EndianType<ContrailRenderType> render_type;
        PAD(0x2);
        EndianType<float> texture_repeats_u;
        EndianType<float> texture_repeats_v;
        EndianType<float> texture_animation_u;
        EndianType<float> texture_animation_v;
        EndianType<float> animation_rate;
        TagDependency<EndianType> bitmap; // bitmap
        EndianType<std::int16_t> first_sequence_index;
        EndianType<std::int16_t> sequence_count;
        PAD(0x40);
        PAD(0x24);
        EndianType<std::uint32_t> unknown_int;
        EndianType<ContrailShaderFlags> shader_flags;
        EndianType<ContrailFramebufferBlendFunction> framebuffer_blend_function;
        EndianType<ContrailFramebufferFadeMode> framebuffer_fade_mode;
        EndianType<ContrailMapFlags> map_flags;
        PAD(0xC);
        PAD(0x10);
        TagDependency<EndianType> secondary_bitmap; // bitmap
        EndianType<ParticleAnchor> anchor;
        EndianType<ContrailMapFlags> secondary_map_flags;
        EndianType<ContrailAnimationSource> u_animation_source;
        EndianType<FunctionType2> u_animation_function;
        EndianType<float> u_animation_period;
        EndianType<float> u_animation_phase;
        EndianType<float> u_animation_scale;
        EndianType<ContrailAnimationSource> v_animation_source;
        EndianType<FunctionType2> v_animation_function;
        EndianType<float> v_animation_period;
        EndianType<float> v_animation_phase;
        EndianType<float> v_animation_scale;
        EndianType<ContrailAnimationSource> rotation_animation_source;
        EndianType<FunctionType2> rotation_animation_function;
        EndianType<float> rotation_animation_period;
        EndianType<float> rotation_animation_phase;
        EndianType<float> rotation_animation_scale;
        Point2D<EndianType> rotation_animation_center;
        PAD(0x4);
        EndianType<float> zsprite_radius_scale;
        PAD(0x14);
        TagReflexive<EndianType, ContrailPointState> point_states;

        ENDIAN_TEMPLATE(NewType) operator Contrail<NewType>() const noexcept {
            Contrail<NewType> copy = {};
            COPY_THIS(flags);
            COPY_THIS(scale_flags);
            COPY_THIS(point_generation_rate);
            COPY_THIS(point_velocity);
            COPY_THIS(point_velocity_cone_angle);
            COPY_THIS(inherited_velocity_fraction);
            COPY_THIS(render_type);
            COPY_THIS(texture_repeats_u);
            COPY_THIS(texture_repeats_v);
            COPY_THIS(texture_animation_u);
            COPY_THIS(texture_animation_v);
            COPY_THIS(animation_rate);
            COPY_THIS(bitmap);
            COPY_THIS(first_sequence_index);
            COPY_THIS(sequence_count);
            COPY_THIS(shader_flags);
            COPY_THIS(framebuffer_blend_function);
            COPY_THIS(framebuffer_fade_mode);
            COPY_THIS(map_flags);
            COPY_THIS(secondary_bitmap);
            COPY_THIS(anchor);
            COPY_THIS(secondary_map_flags);
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
            COPY_THIS(zsprite_radius_scale);
            COPY_THIS(point_states);
            COPY_THIS(unknown_int);
            return copy;
        }
    };
    static_assert(sizeof(Contrail<BigEndian>) == 0x144);

    void compile_contrail_tag(CompiledTag &compiled, const std::byte *data, std::size_t size);
}
#endif
