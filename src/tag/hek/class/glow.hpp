/*
 * Invader (c) 2019 Kavawuvi
 *
 * This program is free software under the GNU General Public License v3.0. See LICENSE for more information.
 */

#ifndef INVADER__TAG__HEK__CLASS__GLOW_HPP
#define INVADER__TAG__HEK__CLASS__GLOW_HPP

#include "../../compiled_tag.hpp"
#include "../../../hek/data_type.hpp"
#include "../../../hek/map.hpp"
#include "../header.hpp"

namespace Invader::HEK {
    enum GlowBoundaryEffect : std::uint16_t {
        GLOW_BOUNDARY_EFFECT_BOUNCE,
        GLOW_BOUNDARY_EFFECT_WRAP,
    };

    enum GlowNormalParticleDistribution : std::uint16_t {
        GLOW_NORMAL_PARTICLE_DISTRIBUTION_DISTRIBUTED_RANDOMLY,
        GLOW_NORMAL_PARTICLE_DISTRIBUTION_DISTRIBUTED_UNIFORMLY,
    };

    enum GlowTrailingParticleDistribution : std::uint16_t {
        GLOW_TRAILING_PARTICLE_DISTRIBUTION_EMIT_VERTICALLY,
        GLOW_TRAILING_PARTICLE_DISTRIBUTION_EMIT_NORMAL_UP,
        GLOW_TRAILING_PARTICLE_DISTRIBUTION_EMIT_RANDOMLY,
    };

    // Increase by 1 if BigEndian.
    enum GlowAttachment : std::uint16_t {
        GLOW_ATTACHMENT_NONE = 0xFFFF,
        GLOW_ATTACHMENT_A_OUT = 0,
        GLOW_ATTACHMENT_B_OUT = 1,
        GLOW_ATTACHMENT_C_OUT = 2,
        GLOW_ATTACHMENT_D_OUT = 3,
    };

    struct GlowFlags {
        std::uint32_t modify_particle_color_in_range : 1;
        std::uint32_t particles_move_backwards : 1;
        std::uint32_t partices_move_in_both_directions : 1;
        std::uint32_t trailing_particles_fade_over_time : 1;
        std::uint32_t trailing_particles_shrink_over_time : 1;
        std::uint32_t trailing_particles_slow_over_time : 1;
    };

    ENDIAN_TEMPLATE(EndianType) struct Glow {
        TagString attachment_marker;
        EndianType<std::uint16_t> number_of_particles;
        EndianType<GlowBoundaryEffect> boundary_effect;
        EndianType<GlowNormalParticleDistribution> normal_particle_distribution;
        EndianType<GlowTrailingParticleDistribution> trailing_particle_distribution;
        EndianType<GlowFlags> glow_flags;
        PAD(0x1C);
        PAD(0x2);
        PAD(0x2);
        PAD(0x4);
        EndianType<GlowAttachment> attachment_0;
        PAD(0x2);
        EndianType<float> particle_rotational_velocity;
        EndianType<float> particle_rot_vel_mul_low;
        EndianType<float> particle_rot_vel_mul_high;
        EndianType<GlowAttachment> attachment_1;
        PAD(0x2);
        EndianType<float> effect_rotational_velocity;
        EndianType<float> effect_rot_vel_mul_low;
        EndianType<float> effect_rot_vel_mul_high;
        EndianType<GlowAttachment> attachment_2;
        PAD(0x2);
        EndianType<float> effect_translational_velocity;
        EndianType<float> effect_trans_vel_mul_low;
        EndianType<float> effect_trans_vel_mul_high;
        EndianType<GlowAttachment> attachment_3;
        PAD(0x2);
        EndianType<float> min_distance_particle_to_object;
        EndianType<float> max_distance_particle_to_object;
        EndianType<float> distance_to_object_mul_low;
        EndianType<float> distance_to_object_mul_high;
        PAD(0x8);
        EndianType<GlowAttachment> attachment_4;
        PAD(0x2);
        Bounds<EndianType<float>> particle_size_bounds;
        Bounds<EndianType<float>> size_attachment_multiplier;
        EndianType<GlowAttachment> attachment_5;
        PAD(0x2);
        ColorARGB<EndianType> color_bound_0;
        ColorARGB<EndianType> color_bound_1;
        ColorARGB<EndianType> scale_color_0;
        ColorARGB<EndianType> scale_color_1;
        EndianType<float> color_rate_of_change;
        EndianType<float> fading_percentage_of_glow;
        EndianType<float> particle_generation_freq;
        EndianType<float> lifetime_of_trailing_particles;
        EndianType<float> velocity_of_trailing_particles;
        EndianType<float> trailing_particle_minimum_t;
        EndianType<float> trailing_particle_maximum_t;
        PAD(0x34);
        TagDependency<EndianType> texture; // bitmap

        ENDIAN_TEMPLATE(NewType) operator Glow<NewType>() const noexcept {
            Glow<NewType> copy = {};
            COPY_THIS(attachment_marker);
            COPY_THIS(number_of_particles);
            COPY_THIS(boundary_effect);
            COPY_THIS(normal_particle_distribution);
            COPY_THIS(trailing_particle_distribution);
            COPY_THIS(glow_flags);
            COPY_THIS(attachment_0);
            COPY_THIS(particle_rotational_velocity);
            COPY_THIS(particle_rot_vel_mul_low);
            COPY_THIS(particle_rot_vel_mul_high);
            COPY_THIS(attachment_1);
            COPY_THIS(effect_rotational_velocity);
            COPY_THIS(effect_rot_vel_mul_low);
            COPY_THIS(effect_rot_vel_mul_high);
            COPY_THIS(attachment_2);
            COPY_THIS(effect_translational_velocity);
            COPY_THIS(effect_trans_vel_mul_low);
            COPY_THIS(effect_trans_vel_mul_high);
            COPY_THIS(attachment_3);
            COPY_THIS(min_distance_particle_to_object);
            COPY_THIS(max_distance_particle_to_object);
            COPY_THIS(distance_to_object_mul_low);
            COPY_THIS(distance_to_object_mul_high);
            COPY_THIS(attachment_4);
            COPY_THIS(particle_size_bounds);
            COPY_THIS(size_attachment_multiplier);
            COPY_THIS(attachment_5);
            COPY_THIS(color_bound_0);
            COPY_THIS(color_bound_1);
            COPY_THIS(scale_color_0);
            COPY_THIS(scale_color_1);
            COPY_THIS(color_rate_of_change);
            COPY_THIS(fading_percentage_of_glow);
            COPY_THIS(particle_generation_freq);
            COPY_THIS(lifetime_of_trailing_particles);
            COPY_THIS(velocity_of_trailing_particles);
            COPY_THIS(trailing_particle_minimum_t);
            COPY_THIS(trailing_particle_maximum_t);
            COPY_THIS(texture);
            return copy;
        }
    };
    static_assert(sizeof(Glow<BigEndian>) == 0x154);

    void compile_glow_tag(CompiledTag &compiled, const std::byte *data, std::size_t size);
}
#endif
