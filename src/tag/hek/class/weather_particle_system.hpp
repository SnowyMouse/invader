/*
 * Invader (c) 2018 Kavawuvi
 *
 * This program is free software under the GNU General Public License v3.0 or later. See LICENSE for more information.
 */

#ifndef INVADER__TAG__HEK__CLASS__WEATHER_PARTICLE_SYSTEM_HPP
#define INVADER__TAG__HEK__CLASS__WEATHER_PARTICLE_SYSTEM_HPP

#include "../../compiled_tag.hpp"
#include "../../../hek/data_type.hpp"
#include "../header.hpp"

namespace Invader::HEK {
    enum WeatherParticleSystemRenderMode : TagEnum {
        WEATHER_PARTICLE_SYSTEM_RENDER_MODE_SCREEN_FACING,
        WEATHER_PARTICLE_SYSTEM_RENDER_MODE_PARALLEL_TO_DIRECTION,
        WEATHER_PARTICLE_SYSTEM_RENDER_MODE_PERPENDICULAR_TO_DIRECTION,
    };

    enum WeatherParticleSystemRenderDirectionSource : TagEnum {
        WEATHER_PARTICLE_SYSTEM_RENDER_DIRECTION_SOURCE_FROM_VELOCITY,
        WEATHER_PARTICLE_SYSTEM_RENDER_DIRECTION_SOURCE_FROM_ACCELERATION,
    };

    enum WeatherParticleSystemFramebufferBlendFunction : TagEnum {
        WEATHER_PARTICLE_SYSTEM_FRAMEBUFFER_BLEND_FUNCTION_ALPHA_BLEND,
        WEATHER_PARTICLE_SYSTEM_FRAMEBUFFER_BLEND_FUNCTION_MULTIPLY,
        WEATHER_PARTICLE_SYSTEM_FRAMEBUFFER_BLEND_FUNCTION_DOUBLE_MULTIPLY,
        WEATHER_PARTICLE_SYSTEM_FRAMEBUFFER_BLEND_FUNCTION_ADD,
        WEATHER_PARTICLE_SYSTEM_FRAMEBUFFER_BLEND_FUNCTION_SUBTRACT,
        WEATHER_PARTICLE_SYSTEM_FRAMEBUFFER_BLEND_FUNCTION_COMPONENT_MIN,
        WEATHER_PARTICLE_SYSTEM_FRAMEBUFFER_BLEND_FUNCTION_COMPONENT_MAX,
        WEATHER_PARTICLE_SYSTEM_FRAMEBUFFER_BLEND_FUNCTION_ALPHA_MULTIPLY_ADD
    };

    enum WeatherParticleSystemFramebufferFadeMode : TagEnum {
        WEATHER_PARTICLE_SYSTEM_FRAMEBUFFER_FADE_MODE_NONE,
        WEATHER_PARTICLE_SYSTEM_FRAMEBUFFER_FADE_MODE_FADE_WHEN_PERPENDICULAR,
        WEATHER_PARTICLE_SYSTEM_FRAMEBUFFER_FADE_MODE_FADE_WHEN_PARALLEL
    };

    enum WeatherParticleSystemAnchor : TagEnum {
        WEATHER_PARTICLE_SYSTEM_ANCHOR_WITH_PRIMARY,
        WEATHER_PARTICLE_SYSTEM_ANCHOR_WITH_SCREEN_SPACE,
        WEATHER_PARTICLE_SYSTEM_ANCHOR_ZSPRITE
    };

    enum WeatherParticleSystemAnimationSource : TagEnum {
        WEATHER_PARTICLE_SYSTEM_ANIMATION_SOURCE_NONE,
        WEATHER_PARTICLE_SYSTEM_ANIMATION_SOURCE_A_OUT,
        WEATHER_PARTICLE_SYSTEM_ANIMATION_SOURCE_B_OUT,
        WEATHER_PARTICLE_SYSTEM_ANIMATION_SOURCE_C_OUT,
        WEATHER_PARTICLE_SYSTEM_ANIMATION_SOURCE_D_OUT
    };

    enum WeatherParticleSystemAnimationFunction : TagEnum {
        WEATHER_PARTICLE_SYSTEM_ANIMATION_FUNCTION_ONE,
        WEATHER_PARTICLE_SYSTEM_ANIMATION_FUNCTION_ZERO,
        WEATHER_PARTICLE_SYSTEM_ANIMATION_FUNCTION_COSINE,
        WEATHER_PARTICLE_SYSTEM_ANIMATION_FUNCTION_COSINE_VARIABLE_PERIOD,
        WEATHER_PARTICLE_SYSTEM_ANIMATION_FUNCTION_DIAGONAL_WAVE,
        WEATHER_PARTICLE_SYSTEM_ANIMATION_FUNCTION_DIAGONAL_WAVE_VARIABLE_PERIOD,
        WEATHER_PARTICLE_SYSTEM_ANIMATION_FUNCTION_SLIDE,
        WEATHER_PARTICLE_SYSTEM_ANIMATION_FUNCTION_SLIDE_VARIABLE_PERIOD,
        WEATHER_PARTICLE_SYSTEM_ANIMATION_FUNCTION_NOISE,
        WEATHER_PARTICLE_SYSTEM_ANIMATION_FUNCTION_JITTER,
        WEATHER_PARTICLE_SYSTEM_ANIMATION_FUNCTION_WANDER,
        WEATHER_PARTICLE_SYSTEM_ANIMATION_FUNCTION_SPARK
    };

    struct WeatherParticleSystemParticleTypeFlags {
        std::uint32_t interpolate_colors_in_hsv : 1;
        std::uint32_t _along_long_hue_path : 1;
        std::uint32_t random_rotation : 1;
    };

    struct WeatherParticleSystemParticleTypeShaderFlags {
        std::uint16_t sort_bias : 1;
        std::uint16_t nonlinear_tint : 1;
        std::uint16_t don_t_overdraw_fp_weapon : 1;
    };

    struct WeatherParticleSystemParticleTypeMapFlags {
        std::uint16_t unfiltered : 1;
    };

    ENDIAN_TEMPLATE(EndianType) struct WeatherParticleSystemParticleType {
        TagString name;
        EndianType<WeatherParticleSystemParticleTypeFlags> flags;
        EndianType<float> fade_in_start_distance;
        EndianType<float> fade_in_end_distance;
        EndianType<float> fade_out_start_distance;
        EndianType<float> fade_out_end_distance;
        EndianType<float> fade_in_start_height;
        EndianType<float> fade_in_end_height;
        EndianType<float> fade_out_start_height;
        EndianType<float> fade_out_end_height;
        PAD(0x60);
        Bounds<EndianType<float>> particle_count;
        TagDependency<EndianType> physics; // .point_physics
        PAD(0x10);
        Bounds<EndianType<float>> acceleration_magnitude;
        EndianType<Fraction> acceleration_turning_rate;
        EndianType<float> acceleration_change_rate;
        PAD(0x20);
        Bounds<EndianType<float>> particle_radius;
        Bounds<EndianType<float>> animation_rate;
        Bounds<EndianType<Angle>> rotation_rate;
        PAD(0x20);
        ColorARGB<EndianType> color_lower_bound;
        ColorARGB<EndianType> color_upper_bound;
        EndianType<float> unknown;
        PAD(0x3C);

        TagDependency<EndianType> sprite_bitmap; // .bitmap
        EndianType<WeatherParticleSystemRenderMode> render_mode;
        EndianType<WeatherParticleSystemRenderDirectionSource> render_direction_source;
        PAD(0x24);
        LittleEndian<std::uint32_t> not_broken;
        EndianType<WeatherParticleSystemParticleTypeShaderFlags> shader_flags;
        EndianType<WeatherParticleSystemFramebufferBlendFunction> framebuffer_blend_function;
        EndianType<WeatherParticleSystemFramebufferFadeMode> framebuffer_fade_mode;
        EndianType<WeatherParticleSystemParticleTypeMapFlags> map_flags;
        PAD(0x1C);

        TagDependency<EndianType> bitmap; // .bitmap
        EndianType<WeatherParticleSystemAnchor> anchor;
        EndianType<WeatherParticleSystemParticleTypeMapFlags> flags_1;
        EndianType<WeatherParticleSystemAnimationSource> u_animation_source;
        EndianType<WeatherParticleSystemAnimationFunction> u_animation_function;
        EndianType<float> u_animation_period;
        EndianType<float> u_animation_phase;
        EndianType<float> u_animation_scale;
        EndianType<WeatherParticleSystemAnimationSource> v_animation_source;
        EndianType<WeatherParticleSystemAnimationFunction> v_animation_function;
        EndianType<float> v_animation_period;
        EndianType<float> v_animation_phase;
        EndianType<float> v_animation_scale;
        EndianType<WeatherParticleSystemAnimationSource> rotation_animation_source;
        EndianType<WeatherParticleSystemAnimationFunction> rotation_animation_function;
        EndianType<float> rotation_animation_period;
        EndianType<float> rotation_animation_phase;
        EndianType<float> rotation_animation_scale;
        Point2D<EndianType> rotation_animation_center;
        PAD(0x4);
        EndianType<float> zsprite_radius_scale;
        PAD(0x14);

        ENDIAN_TEMPLATE(OtherType) operator WeatherParticleSystemParticleType<OtherType>() const noexcept {
            WeatherParticleSystemParticleType<OtherType> copy = {};
            COPY_THIS(name);
            COPY_THIS(flags);
            COPY_THIS(fade_in_start_distance);
            COPY_THIS(fade_in_end_distance);
            COPY_THIS(fade_out_start_distance);
            COPY_THIS(fade_out_end_distance);
            COPY_THIS(fade_in_start_height);
            COPY_THIS(fade_in_end_height);
            COPY_THIS(fade_out_start_height);
            COPY_THIS(fade_out_end_height);
            COPY_THIS(particle_count);
            COPY_THIS(physics);
            COPY_THIS(acceleration_magnitude);
            COPY_THIS(acceleration_turning_rate);
            COPY_THIS(acceleration_change_rate);
            COPY_THIS(particle_radius);
            COPY_THIS(animation_rate);
            COPY_THIS(rotation_rate);
            COPY_THIS(color_lower_bound);
            COPY_THIS(color_upper_bound);
            COPY_THIS(sprite_bitmap);
            COPY_THIS(render_mode);
            COPY_THIS(render_direction_source);
            COPY_THIS(not_broken);
            COPY_THIS(shader_flags);
            COPY_THIS(framebuffer_blend_function);
            COPY_THIS(framebuffer_fade_mode);
            COPY_THIS(map_flags);
            COPY_THIS(bitmap);
            COPY_THIS(anchor);
            COPY_THIS(flags_1);
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
    static_assert(sizeof(WeatherParticleSystemParticleType<BigEndian>) == 0x25C);

    struct WeatherParticleSystemFlags {
        std::uint32_t unused : 1;
    };
    ENDIAN_TEMPLATE(EndianType) struct WeatherParticleSystem {
        EndianType<WeatherParticleSystemFlags> flags;
        PAD(0x20);
        TagReflexive<EndianType, WeatherParticleSystemParticleType> particle_types;

        ENDIAN_TEMPLATE(OtherType) operator WeatherParticleSystem<OtherType>() const noexcept {
            WeatherParticleSystem<OtherType> copy = {};
            COPY_THIS(flags);
            COPY_THIS(particle_types);
            return copy;
        }
    };
    static_assert(sizeof(WeatherParticleSystem<BigEndian>) == 0x30);

    void compile_weather_particle_system_tag(CompiledTag &compiled, const std::byte *data, std::size_t size);
}
#endif
