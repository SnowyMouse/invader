/*
 * Invader (c) 2019 Kavawuvi
 *
 * This program is free software under the GNU General Public License v3.0. See LICENSE for more information.
 */

#ifndef INVADER__TAG__HEK__CLASS__BIPED_HPP
#define INVADER__TAG__HEK__CLASS__BIPED_HPP

#include "unit.hpp"

namespace Invader::HEK {
    ENDIAN_TEMPLATE(EndianType) struct BipedContactPoint {
        PAD(0x20);
        TagString marker_name;

        ENDIAN_TEMPLATE(NewType) operator BipedContactPoint<NewType>() const noexcept {
            BipedContactPoint<NewType> copy = {};
            COPY_THIS(marker_name);
            return copy;
        }
    };
    static_assert(sizeof(BipedContactPoint<BigEndian>) == 0x40); // max count: 2

    struct BipedFlags {
        std::uint32_t turns_without_animating : 1;
        std::uint32_t uses_player_physics : 1;
        std::uint32_t flying : 1;
        std::uint32_t physics_pill_centered_at_origin : 1;
        std::uint32_t spherical : 1;
        std::uint32_t passes_through_other_bipeds : 1;
        std::uint32_t can_climb_any_surface : 1;
        std::uint32_t immune_to_falling_damage : 1;
        std::uint32_t rotate_while_airborne : 1;
        std::uint32_t uses_limp_body_physics : 1;
        std::uint32_t has_no_dying_airborne : 1;
        std::uint32_t random_speed_increase : 1;
        std::uint32_t unit_uses_old_ntsc_player_physics : 1;
    };

    enum BipedFunctionIn : TagEnum {
        BIPED_FUNCTION_IN_NONE,
        BIPED_FUNCTION_IN_FLYING_VELOCITY,
    };

    ENDIAN_TEMPLATE(EndianType) struct Biped : Unit<EndianType> {
        EndianType<Angle> moving_turning_speed;
        EndianType<BipedFlags> biped_flags;
        EndianType<Angle> stationary_turning_threshold;
        PAD(0x10);
        EndianType<BipedFunctionIn> biped_a_in;
        EndianType<BipedFunctionIn> biped_b_in;
        EndianType<BipedFunctionIn> biped_c_in;
        EndianType<BipedFunctionIn> biped_d_in;
        TagDependency<EndianType> dont_use; // damage_effect
        EndianType<Angle> bank_angle;
        EndianType<float> bank_apply_time;
        EndianType<float> bank_decay_time;
        EndianType<float> pitch_ratio;
        EndianType<float> max_velocity;
        EndianType<float> max_sidestep_velocity;
        EndianType<float> acceleration;
        EndianType<float> deceleration;
        EndianType<Angle> angular_velocity_maximum;
        EndianType<Angle> angular_acceleration_maximum;
        EndianType<float> crouch_velocity_modifier;
        PAD(0x8);
        EndianType<Angle> maximum_slope_angle;
        EndianType<Angle> downhill_falloff_angle;
        EndianType<Angle> downhill_cutoff_angle;
        EndianType<float> downhill_velocity_scale;
        EndianType<Angle> uphill_falloff_angle;
        EndianType<Angle> uphill_cutoff_angle;
        EndianType<float> uphill_velocity_scale;
        PAD(0x18);
        TagDependency<EndianType> footsteps; // footsteps
        PAD(0x18);
        EndianType<float> jump_velocity;
        PAD(0x1C);
        EndianType<float> maximum_soft_landing_time;
        EndianType<float> maximum_hard_landing_time;
        EndianType<float> minimum_soft_landing_velocity;
        EndianType<float> minimum_hard_landing_velocity;
        EndianType<float> maximum_hard_landing_velocity;
        EndianType<float> death_hard_landing_velocity;
        PAD(0x14);
        EndianType<float> standing_camera_height;
        EndianType<float> crouching_camera_height;
        EndianType<float> crouch_transition_time;
        PAD(0x18);
        EndianType<float> standing_collision_height;
        EndianType<float> crouching_collision_height;
        EndianType<float> collision_radius;
        PAD(0x28);
        EndianType<float> autoaim_width;
        PAD(0x6C);
        LittleEndian<float> cosine_stationary_turning_threshold;
        LittleEndian<float> crouch_camera_velocity;
        LittleEndian<float> cosine_maximum_slope_angle;
        LittleEndian<float> negative_sine_downhill_falloff_angle;
        LittleEndian<float> negative_sine_downhill_cutoff_angle;
        LittleEndian<float> sine_uphill_falloff_angle;
        LittleEndian<float> sine_uphill_cutoff_angle;
        PAD(0x2);
        LittleEndian<std::uint16_t> head_model_node_index;
        TagReflexive<EndianType, BipedContactPoint> contact_point;

        ENDIAN_TEMPLATE(NewType) operator Biped<NewType>() const noexcept {
            Biped<NewType> copy = {};
            COPY_UNIT_DATA
            COPY_THIS(moving_turning_speed);
            COPY_THIS(biped_flags);
            COPY_THIS(stationary_turning_threshold);
            COPY_THIS(a_in);
            COPY_THIS(b_in);
            COPY_THIS(c_in);
            COPY_THIS(d_in);
            COPY_THIS(dont_use);
            COPY_THIS(bank_angle);
            COPY_THIS(bank_apply_time);
            COPY_THIS(bank_decay_time);
            COPY_THIS(pitch_ratio);
            COPY_THIS(max_velocity);
            COPY_THIS(max_sidestep_velocity);
            COPY_THIS(acceleration);
            COPY_THIS(deceleration);
            COPY_THIS(angular_velocity_maximum);
            COPY_THIS(angular_acceleration_maximum);
            COPY_THIS(crouch_velocity_modifier);
            COPY_THIS(maximum_slope_angle);
            COPY_THIS(downhill_falloff_angle);
            COPY_THIS(downhill_cutoff_angle);
            COPY_THIS(downhill_velocity_scale);
            COPY_THIS(uphill_falloff_angle);
            COPY_THIS(uphill_cutoff_angle);
            COPY_THIS(uphill_velocity_scale);
            COPY_THIS(footsteps);
            COPY_THIS(jump_velocity);
            COPY_THIS(maximum_soft_landing_time);
            COPY_THIS(maximum_hard_landing_time);
            COPY_THIS(minimum_soft_landing_velocity);
            COPY_THIS(minimum_hard_landing_velocity);
            COPY_THIS(maximum_hard_landing_velocity);
            COPY_THIS(death_hard_landing_velocity);
            COPY_THIS(standing_camera_height);
            COPY_THIS(crouching_camera_height);
            COPY_THIS(crouch_transition_time);
            COPY_THIS(standing_collision_height);
            COPY_THIS(crouching_collision_height);
            COPY_THIS(collision_radius);
            COPY_THIS(autoaim_width);
            COPY_THIS(cosine_stationary_turning_threshold);
            COPY_THIS(crouch_camera_velocity);
            COPY_THIS(cosine_maximum_slope_angle);
            COPY_THIS(negative_sine_downhill_falloff_angle);
            COPY_THIS(negative_sine_downhill_cutoff_angle);
            COPY_THIS(sine_uphill_falloff_angle);
            COPY_THIS(sine_uphill_cutoff_angle);
            COPY_THIS(head_model_node_index);
            COPY_THIS(contact_point);
            return copy;
        }
    };
    static_assert(sizeof(Biped<BigEndian>) == 0x4F4);

    void compile_biped_tag(CompiledTag &compiled, const std::byte *data, std::size_t size);
}
#endif
