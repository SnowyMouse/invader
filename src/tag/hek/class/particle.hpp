/*
 * Invader (c) 2018 Kavawuvi
 *
 * This program is free software under the GNU General Public License v3.0 or later. See LICENSE for more information.
 */

#ifndef INVADER__TAG__HEK__CLASS__PARTICLE_HPP
#define INVADER__TAG__HEK__CLASS__PARTICLE_HPP

#include "../../compiled_tag.hpp"
#include "../../../hek/data_type.hpp"
#include "../header.hpp"

namespace Invader::HEK {
    enum ParticleOrientation : TagEnum {
        PARTICLE_ORIENTATION_SCREEN_FACING,
        PARTICLE_ORIENTATION_PARALLEL_TO_DIRECTION,
        PARTICLE_ORIENTATION_PERPENDICULAR_TO_DIRECTION
    };

    enum ParticleFramebufferBlendFunction : TagEnum {
        PARTICLE_FRAMEBUFFER_BLEND_FUNCTION_ALPHA_BLEND,
        PARTICLE_FRAMEBUFFER_BLEND_FUNCTION_MULTIPLY,
        PARTICLE_FRAMEBUFFER_BLEND_FUNCTION_DOUBLE_MULTIPLY,
        PARTICLE_FRAMEBUFFER_BLEND_FUNCTION_ADD,
        PARTICLE_FRAMEBUFFER_BLEND_FUNCTION_SUBTRACT,
        PARTICLE_FRAMEBUFFER_BLEND_FUNCTION_COMPONENT_MIN,
        PARTICLE_FRAMEBUFFER_BLEND_FUNCTION_COMPONENT_MAX,
        PARTICLE_FRAMEBUFFER_BLEND_FUNCTION_ALPHA_MULTIPLY_ADD
    };

    enum ParticleFramebufferFadeMode : TagEnum {
        PARTICLE_FRAMEBUFFER_FADE_MODE_NONE,
        PARTICLE_FRAMEBUFFER_FADE_MODE_FADE_WHEN_PERPENDICULAR,
        PARTICLE_FRAMEBUFFER_FADE_MODE_FADE_WHEN_PARALLEL
    };

    enum ParticleAnchor : TagEnum {
        PARTICLE_ANCHOR_WITH_PRIMARY,
        PARTICLE_ANCHOR_WITH_SCREEN_SPACE,
        PARTICLE_ANCHOR_ZSPRITE
    };

    enum ParticleAnimationSource : TagEnum {
        PARTICLE_ANIMATION_SOURCE_NONE,
        PARTICLE_ANIMATION_SOURCE_A_OUT,
        PARTICLE_ANIMATION_SOURCE_B_OUT,
        PARTICLE_ANIMATION_SOURCE_C_OUT,
        PARTICLE_ANIMATION_SOURCE_D_OUT
    };

    struct ParticleFlags {
        std::uint32_t can_animate_backwards : 1;
        std::uint32_t animation_stops_at_rest : 1;
        std::uint32_t animation_starts_on_random_frame : 1;
        std::uint32_t animate_once_per_frame : 1;
        std::uint32_t dies_at_rest : 1;
        std::uint32_t dies_on_contact_with_structure : 1;
        std::uint32_t tint_from_diffuse_texture : 1;
        std::uint32_t dies_on_contact_with_water : 1;
        std::uint32_t dies_on_contact_with_air : 1;
        std::uint32_t self_illuminated : 1;
        std::uint32_t random_horizontal_mirroring : 1;
        std::uint32_t random_vertical_mirroring : 1;
    };

    struct ParticleShaderFlags {
        std::uint16_t sort_bias : 1;
        std::uint16_t nonlinear_tint : 1;
        std::uint16_t don_t_overdraw_fp_weapon : 1;
    };

    struct ParticleMapFlags {
        std::uint16_t unfiltered : 1;
    };

    ENDIAN_TEMPLATE(EndianType) struct Particle {
        EndianType<ParticleFlags> flags;
        TagDependency<EndianType> bitmap; // bitmap
        TagDependency<EndianType> physics; // point_physics
        TagDependency<EndianType> sir_marty_exchanged_his_children_for_thine; // material_effect
        PAD(0x4);
        Bounds<EndianType<float>> lifespan;
        EndianType<float> fade_in_time;
        EndianType<float> fade_out_time;
        TagDependency<EndianType> collision_effect; // sound, effect
        TagDependency<EndianType> death_effect; // sound, effect
        EndianType<float> minimum_size;
        BigEndian<float> unknown_floats[2]; // 2 and 1, respectively, sometimes - copied as-is. no known usage
        Bounds<EndianType<float>> radius_animation;
        PAD(0x4);
        Bounds<EndianType<float>> animation_rate;
        EndianType<float> contact_deterioration;
        EndianType<float> fade_start_size;
        EndianType<float> fade_end_size;
        PAD(0x4);
        EndianType<std::int16_t> first_sequence_index;
        EndianType<std::int16_t> initial_sequence_count;
        EndianType<std::int16_t> looping_sequence_count;
        EndianType<std::int16_t> final_sequence_count;
        PAD(0x8);
        EndianType<float> unknown;
        EndianType<ParticleOrientation> orientation;
        PAD(0x2);
        PAD(0x24);
        LittleEndian<std::uint32_t> one;
        EndianType<ParticleShaderFlags> shader_flags;
        EndianType<ParticleFramebufferBlendFunction> framebuffer_blend_function;
        EndianType<ParticleFramebufferFadeMode> framebuffer_fade_mode;
        EndianType<ParticleMapFlags> map_flags;
        PAD(0x1C);
        TagDependency<EndianType> bitmap1; // bitmap
        EndianType<ParticleAnchor> anchor;
        EndianType<ParticleMapFlags> map_flags1;
        EndianType<ParticleAnimationSource> u_animation_source;
        EndianType<FunctionType2> u_animation_function;
        EndianType<float> u_animation_period;
        EndianType<float> u_animation_phase;
        EndianType<float> u_animation_scale;
        EndianType<ParticleAnimationSource> v_animation_source;
        EndianType<FunctionType2> v_animation_function;
        EndianType<float> v_animation_period;
        EndianType<float> v_animation_phase;
        EndianType<float> v_animation_scale;
        EndianType<ParticleAnimationSource> rotation_animation_source;
        EndianType<FunctionType2> rotation_animation_function;
        EndianType<float> rotation_animation_period;
        EndianType<float> rotation_animation_phase;
        EndianType<float> rotation_animation_scale;
        Point2D<EndianType> rotation_animation_center;
        PAD(0x4);
        EndianType<float> zsprite_radius_scale;
        PAD(0x14);

        ENDIAN_TEMPLATE(NewType) operator Particle<NewType>() const noexcept {
            Particle<NewType> copy = {};
            COPY_THIS(flags);
            COPY_THIS(bitmap);
            COPY_THIS(physics);
            COPY_THIS(sir_marty_exchanged_his_children_for_thine);
            COPY_THIS(lifespan);
            COPY_THIS(fade_in_time);
            COPY_THIS(fade_out_time);
            COPY_THIS(collision_effect);
            COPY_THIS(death_effect);
            COPY_THIS(minimum_size);
            COPY_THIS_ARRAY(unknown_floats);
            COPY_THIS(radius_animation);
            COPY_THIS(animation_rate);
            COPY_THIS(contact_deterioration);
            COPY_THIS(fade_start_size);
            COPY_THIS(fade_end_size);
            COPY_THIS(first_sequence_index);
            COPY_THIS(initial_sequence_count);
            COPY_THIS(looping_sequence_count);
            COPY_THIS(final_sequence_count);
            COPY_THIS(one);
            COPY_THIS(orientation);
            COPY_THIS(shader_flags);
            COPY_THIS(framebuffer_blend_function);
            COPY_THIS(framebuffer_fade_mode);
            COPY_THIS(map_flags);
            COPY_THIS(bitmap1);
            COPY_THIS(anchor);
            COPY_THIS(map_flags1);
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
            return copy;
        }
    };
    static_assert(sizeof(Particle<BigEndian>) == 0x164);

    void compile_particle_tag(CompiledTag &compiled, const std::byte *data, std::size_t size);
}
#endif
