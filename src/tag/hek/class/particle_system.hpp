/*
 * Invader (c) 2019 Kavawuvi
 *
 * This program is free software under the GNU General Public License v3.0. See LICENSE for more information.
 */

#ifndef INVADER__TAG__HEK__CLASS__PARTICLE_SYSTEM_HPP
#define INVADER__TAG__HEK__CLASS__PARTICLE_SYSTEM_HPP

#include "../../compiled_tag.hpp"
#include "../../../hek/data_type.hpp"
#include "../header.hpp"

namespace Invader::HEK {
    enum ParticleSystemParticleCreationPhysics : TagEnum {
        PARTICLE_SYSTEM_PARTICLE_CREATION_PHYSICS_DEFAULT,
        PARTICLE_SYSTEM_PARTICLE_CREATION_PHYSICS_EXPLOSION,
        PARTICLE_SYSTEM_PARTICLE_CREATION_PHYSICS_JET
    };

    enum ParticleSystemParticleUpdatePhysics : TagEnum {
        PARTICLE_SYSTEM_PARTICLE_UPDATE_PHYSICS_DEFAULT
    };

    enum ParticleSystemFramebufferBlendFunction : TagEnum {
        PARTICLE_SYSTEM_FRAMEBUFFER_BLEND_FUNCTION_ALPHA_BLEND,
        PARTICLE_SYSTEM_FRAMEBUFFER_BLEND_FUNCTION_MULTIPLY,
        PARTICLE_SYSTEM_FRAMEBUFFER_BLEND_FUNCTION_DOUBLE_MULTIPLY,
        PARTICLE_SYSTEM_FRAMEBUFFER_BLEND_FUNCTION_ADD,
        PARTICLE_SYSTEM_FRAMEBUFFER_BLEND_FUNCTION_SUBTRACT,
        PARTICLE_SYSTEM_FRAMEBUFFER_BLEND_FUNCTION_COMPONENT_MIN,
        PARTICLE_SYSTEM_FRAMEBUFFER_BLEND_FUNCTION_COMPONENT_MAX,
        PARTICLE_SYSTEM_FRAMEBUFFER_BLEND_FUNCTION_ALPHA_MULTIPLY_ADD
    };

    enum ParticleSystemFramebufferFadeMode : TagEnum {
        PARTICLE_SYSTEM_FRAMEBUFFER_FADE_MODE_NONE,
        PARTICLE_SYSTEM_FRAMEBUFFER_FADE_MODE_FADE_WHEN_PERPENDICULAR,
        PARTICLE_SYSTEM_FRAMEBUFFER_FADE_MODE_FADE_WHEN_PARALLEL
    };

    enum ParticleSystemAnchor : TagEnum {
        PARTICLE_SYSTEM_ANCHOR_WITH_PRIMARY,
        PARTICLE_SYSTEM_ANCHOR_WITH_SCREEN_SPACE,
        PARTICLE_SYSTEM_ANCHOR_ZSPRITE
    };

    enum ParticleSystemAnimationSource : TagEnum {
        PARTICLE_SYSTEM_ANIMATION_SOURCE_NONE,
        PARTICLE_SYSTEM_ANIMATION_SOURCE_A_OUT,
        PARTICLE_SYSTEM_ANIMATION_SOURCE_B_OUT,
        PARTICLE_SYSTEM_ANIMATION_SOURCE_C_OUT,
        PARTICLE_SYSTEM_ANIMATION_SOURCE_D_OUT
    };

    enum ParticleSystemComplexSpriteRenderModes : TagEnum {
        PARTICLE_SYSTEM_COMPLEX_SPRITE_RENDER_MODES_SIMPLE,
        PARTICLE_SYSTEM_COMPLEX_SPRITE_RENDER_MODES_ROTATIONAL
    };

    enum ParticleSystemSystemUpdatePhysics : TagEnum {
        PARTICLE_SYSTEM_SYSTEM_UPDATE_PHYSICS_DEFAULT,
        PARTICLE_SYSTEM_SYSTEM_UPDATE_PHYSICS_EXPLOSION
    };

    ENDIAN_TEMPLATE(EndianType) struct ParticleSystemPhysicsConstant {
        EndianType<float> k;

        ENDIAN_TEMPLATE(NewType) operator ParticleSystemPhysicsConstant<NewType>() const noexcept {
            ParticleSystemPhysicsConstant<NewType> copy;
            COPY_THIS(k);
            return copy;
        }
    };
    static_assert(sizeof(ParticleSystemPhysicsConstant<BigEndian>) == 0x4);

    ENDIAN_TEMPLATE(EndianType) struct ParticleSystemTypeStates {
        TagString name;
        Bounds<EndianType<float>> duration_bounds;
        Bounds<EndianType<float>> transition_time_bounds;
        PAD(0x4);
        EndianType<float> scale_multiplier;
        EndianType<float> animation_rate_multiplier;
        EndianType<float> rotation_rate_multiplier;
        ColorARGB<EndianType> color_multiplier;
        EndianType<float> radius_multiplier;
        EndianType<float> minimum_particle_count;
        EndianType<float> particle_creation_rate;
        PAD(0x54);
        EndianType<ParticleSystemParticleCreationPhysics> particle_creation_physics;
        EndianType<ParticleSystemParticleUpdatePhysics> particle_update_physics;
        TagReflexive<EndianType, ParticleSystemPhysicsConstant> physics_constants;

        ENDIAN_TEMPLATE(NewType) operator ParticleSystemTypeStates<NewType>() const noexcept {
            ParticleSystemTypeStates<NewType> copy = {};
            COPY_THIS(name);
            COPY_THIS(duration_bounds);
            COPY_THIS(transition_time_bounds);
            COPY_THIS(scale_multiplier);
            COPY_THIS(animation_rate_multiplier);
            COPY_THIS(rotation_rate_multiplier);
            COPY_THIS(color_multiplier);
            COPY_THIS(radius_multiplier);
            COPY_THIS(minimum_particle_count);
            COPY_THIS(particle_creation_rate);
            COPY_THIS(particle_creation_physics);
            COPY_THIS(particle_update_physics);
            COPY_THIS(physics_constants);
            return copy;
        }
    };
    static_assert(sizeof(ParticleSystemTypeStates<BigEndian>) == 0xC0);

    struct ParticleSystemTypeParticleStateShaderFlags {
        std::uint16_t sort_bias : 1;
        std::uint16_t nonlinear_tint : 1;
        std::uint16_t don_t_overdraw_fp_weapon : 1;
    };

    struct ParticleSystemTypeParticleStateMapFlags {
        std::uint16_t unfiltered : 1;
    };

    ENDIAN_TEMPLATE(EndianType) struct ParticleSystemTypeParticleState {
        TagString name;
        Bounds<EndianType<float>> duration_bounds;
        Bounds<EndianType<float>> transition_time_bounds;
        TagDependency<EndianType> bitmaps; // bitmap
        EndianType<std::int16_t> sequence_index;
        PAD(0x2);
        PAD(0x4);
        Bounds<EndianType<float>> scale;
        Bounds<EndianType<float>> animation_rate;
        Bounds<EndianType<Angle>> rotation_rate;
        ColorARGB<EndianType> color_1;
        ColorARGB<EndianType> color_2;
        EndianType<float> radius_multiplier;
        TagDependency<EndianType> point_physics; // point_physics
        PAD(0x24);
        PAD(0x24);
        LittleEndian<std::uint32_t> unknown_int;
        EndianType<ParticleSystemTypeParticleStateShaderFlags> shader_flags;
        EndianType<ParticleSystemFramebufferBlendFunction> framebuffer_blend_function;
        EndianType<ParticleSystemFramebufferFadeMode> framebuffer_fade_mode;
        EndianType<ParticleSystemTypeParticleStateMapFlags> map_flags;
        PAD(0x1C);
        TagDependency<EndianType> secondary_map_bitmap; // bitmap
        EndianType<ParticleSystemAnchor> anchor;
        EndianType<ParticleSystemTypeParticleStateMapFlags> flags;
        EndianType<ParticleSystemAnimationSource> u_animation_source;
        EndianType<FunctionType2> u_animation_function;
        EndianType<float> u_animation_period;
        EndianType<float> u_animation_phase;
        EndianType<float> u_animation_scale;
        EndianType<ParticleSystemAnimationSource> v_animation_source;
        EndianType<FunctionType2> v_animation_function;
        EndianType<float> v_animation_period;
        EndianType<float> v_animation_phase;
        EndianType<float> v_animation_scale;
        EndianType<ParticleSystemAnimationSource> rotation_animation_source;
        EndianType<FunctionType2> rotation_animation_function;
        EndianType<float> rotation_animation_period;
        EndianType<float> rotation_animation_phase;
        EndianType<float> rotation_animation_scale;
        Point2D<EndianType> rotation_animation_center;
        PAD(0x4);
        EndianType<float> zsprite_radius_scale;
        PAD(0x14);
        TagReflexive<EndianType, ParticleSystemPhysicsConstant> physics_constants;

        ENDIAN_TEMPLATE(NewType) operator ParticleSystemTypeParticleState<NewType>() const noexcept {
            ParticleSystemTypeParticleState<NewType> copy = {};
            COPY_THIS(name);
            COPY_THIS(duration_bounds);
            COPY_THIS(transition_time_bounds);
            COPY_THIS(bitmaps);
            COPY_THIS(sequence_index);
            COPY_THIS(scale);
            COPY_THIS(animation_rate);
            COPY_THIS(rotation_rate);
            COPY_THIS(color_1);
            COPY_THIS(color_2);
            COPY_THIS(radius_multiplier);
            COPY_THIS(point_physics);
            COPY_THIS(shader_flags);
            COPY_THIS(framebuffer_blend_function);
            COPY_THIS(framebuffer_fade_mode);
            COPY_THIS(map_flags);
            COPY_THIS(secondary_map_bitmap);
            COPY_THIS(anchor);
            COPY_THIS(flags);
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
            COPY_THIS(physics_constants);
            COPY_THIS(unknown_int);
            return copy;
        }
    };
    static_assert(sizeof(ParticleSystemTypeParticleState<BigEndian>) == 0x178);

    struct ParticleSystemTypeFlags {
        std::uint32_t type_states_loop : 1;
        std::uint32_t _forward_backward : 1;
        std::uint32_t particle_states_loop : 1;
        std::uint32_t _forward_backward_1 : 1;
        std::uint32_t particles_die_in_water : 1;
        std::uint32_t particles_die_in_air : 1;
        std::uint32_t particles_die_on_ground : 1;
        std::uint32_t rotational_sprites_animate_sideways : 1;
        std::uint32_t disabled : 1;
        std::uint32_t tint_by_effect_color : 1;
        std::uint32_t initial_count_scales_with_effect : 1;
        std::uint32_t minimum_count_scales_with_effect : 1;
        std::uint32_t creation_rate_scales_with_effect : 1;
        std::uint32_t scale_scales_with_effect : 1;
        std::uint32_t animation_rate_scales_with_effect : 1;
        std::uint32_t rotation_rate_scales_with_effect : 1;
        std::uint32_t don_t_draw_in_first_person : 1;
        std::uint32_t don_t_draw_in_third_person : 1;
    };

    struct ParticleSystemPhysicsFlags {
        std::uint32_t unused : 1;
    };

    ENDIAN_TEMPLATE(EndianType) struct ParticleSystemType {
        TagString name;
        EndianType<ParticleSystemTypeFlags> flags;
        EndianType<std::int16_t> initial_particle_count;
        PAD(0x2);
        EndianType<ParticleSystemComplexSpriteRenderModes> complex_sprite_render_modes;
        PAD(0x2);
        EndianType<float> radius;
        PAD(0x24);
        EndianType<ParticleSystemParticleCreationPhysics> particle_creation_physics;
        PAD(0x2);
        EndianType<ParticleSystemPhysicsFlags> physics_flags;
        TagReflexive<EndianType, ParticleSystemPhysicsConstant> physics_constants;
        TagReflexive<EndianType, ParticleSystemTypeStates> states;
        TagReflexive<EndianType, ParticleSystemTypeParticleState> particle_states;

        ENDIAN_TEMPLATE(NewType) operator ParticleSystemType<NewType>() const noexcept {
            ParticleSystemType<NewType> copy = {};
            COPY_THIS(name);
            COPY_THIS(flags);
            COPY_THIS(initial_particle_count);
            COPY_THIS(complex_sprite_render_modes);
            COPY_THIS(radius);
            COPY_THIS(particle_creation_physics);
            COPY_THIS(physics_flags);
            COPY_THIS(physics_constants);
            COPY_THIS(states);
            COPY_THIS(particle_states);
            return copy;
        }
    };
    static_assert(sizeof(ParticleSystemType<BigEndian>) == 0x80);

    struct ParticleSystemFlags {
        std::uint32_t unused : 1;
    };

    ENDIAN_TEMPLATE(EndianType) struct ParticleSystem {
        PAD(0x4);
        PAD(0x34);
        TagDependency<EndianType> point_physics; // point_physics
        EndianType<ParticleSystemSystemUpdatePhysics> system_update_physics;
        PAD(0x2);
        EndianType<ParticleSystemFlags> physics_flags;
        TagReflexive<EndianType, ParticleSystemPhysicsConstant> physics_constants;
        TagReflexive<EndianType, ParticleSystemType> particle_types;

        ENDIAN_TEMPLATE(NewType) operator ParticleSystem<NewType>() const noexcept {
            ParticleSystem<NewType> copy = {};
            COPY_THIS(point_physics);
            COPY_THIS(system_update_physics);
            COPY_THIS(physics_flags);
            COPY_THIS(physics_constants);
            COPY_THIS(particle_types);
            return copy;
        }
    };
    static_assert(sizeof(ParticleSystem<BigEndian>) == 0x68);

    void compile_particle_system_tag(CompiledTag &compiled, const std::byte *data, std::size_t size);
}
#endif
