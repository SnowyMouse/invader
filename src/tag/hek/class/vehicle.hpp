/*
 * Invader (c) 2018 Kavawuvi
 *
 * This program is free software under the GNU General Public License v3.0 or later. See LICENSE for more information.
 */

#pragma once

#include "unit.hpp"

namespace Invader::HEK {
    enum VehicleType : TagEnum {
        VEHICLE_TYPE_HUMAN_TANK,
        VEHICLE_TYPE_HUMAN_JEEP,
        VEHICLE_TYPE_HUMAN_BOAT,
        VEHICLE_TYPE_HUMAN_PLANE,
        VEHICLE_TYPE_ALIEN_SCOUT,
        VEHICLE_TYPE_ALIEN_FIGHTER,
        VEHICLE_TYPE_TURRET
    };
    enum VehicleFunctionIn : TagEnum {
        VEHICLE_FUNCTION_IN_NONE,
        VEHICLE_FUNCTION_IN_SPEED_ABSOLUTE,
        VEHICLE_FUNCTION_IN_SPEED_FORWARD,
        VEHICLE_FUNCTION_IN_SPEED_BACKWARD,
        VEHICLE_FUNCTION_IN_SLIDE_ABSOLUTE,
        VEHICLE_FUNCTION_IN_SLIDE_LEFT,
        VEHICLE_FUNCTION_IN_SLIDE_RIGHT,
        VEHICLE_FUNCTION_IN_SPEED_SLIDE_MAXIMUM,
        VEHICLE_FUNCTION_IN_TURN_ABSOLUTE,
        VEHICLE_FUNCTION_IN_TURN_LEFT,
        VEHICLE_FUNCTION_IN_TURN_RIGHT,
        VEHICLE_FUNCTION_IN_CROUCH,
        VEHICLE_FUNCTION_IN_JUMP,
        VEHICLE_FUNCTION_IN_WALK,
        VEHICLE_FUNCTION_IN_VELOCITY_AIR,
        VEHICLE_FUNCTION_IN_VELOCITY_WATER,
        VEHICLE_FUNCTION_IN_VELOCITY_GROUND,
        VEHICLE_FUNCTION_IN_VELOCITY_FORWARD,
        VEHICLE_FUNCTION_IN_VELOCITY_LEFT,
        VEHICLE_FUNCTION_IN_VELOCITY_UP,
        VEHICLE_FUNCTION_IN_LEFT_TREAD_POSITION,
        VEHICLE_FUNCTION_IN_RIGHT_TREAD_POSITION,
        VEHICLE_FUNCTION_IN_LEFT_TREAD_VELOCITY,
        VEHICLE_FUNCTION_IN_RIGHT_TREAD_VELOCITY,
        VEHICLE_FUNCTION_IN_FRONT_LEFT_TIRE_POSITION,
        VEHICLE_FUNCTION_IN_FRONT_RIGHT_TIRE_POSITION,
        VEHICLE_FUNCTION_IN_BACK_LEFT_TIRE_POSITION,
        VEHICLE_FUNCTION_IN_BACK_RIGHT_TIRE_POSITION,
        VEHICLE_FUNCTION_IN_FRONT_LEFT_TIRE_VELOCITY,
        VEHICLE_FUNCTION_IN_FRONT_RIGHT_TIRE_VELOCITY,
        VEHICLE_FUNCTION_IN_BACK_LEFT_TIRE_VELOCITY,
        VEHICLE_FUNCTION_IN_BACK_RIGHT_TIRE_VELOCITY,
        VEHICLE_FUNCTION_IN_WINGTIP_CONTRAIL,
        VEHICLE_FUNCTION_IN_HOVER,
        VEHICLE_FUNCTION_IN_THRUST,
        VEHICLE_FUNCTION_IN_ENGINE_HACK,
        VEHICLE_FUNCTION_IN_WINGTIP_CONTRAIL_NEW
    };

    struct VehicleFlags {
        std::uint32_t speed_wakes_physics : 1;
        std::uint32_t turn_wakes_physics : 1;
        std::uint32_t driver_power_wakes_physics : 1;
        std::uint32_t gunner_power_wakes_physics : 1;
        std::uint32_t control_opposite_speed_sets_brake : 1;
        std::uint32_t slide_wakes_physics : 1;
        std::uint32_t kills_riders_at_terminal_velocity : 1;
        std::uint32_t causes_collision_damage : 1;
        std::uint32_t ai_weapon_cannot_rotate : 1;
        std::uint32_t ai_does_not_require_driver : 1;
        std::uint32_t ai_unused : 1;
        std::uint32_t ai_driver_enable : 1;
        std::uint32_t ai_driver_flying : 1;
        std::uint32_t ai_driver_can_sidestep : 1;
        std::uint32_t ai_driver_hovering : 1;
    };
    static_assert(sizeof(VehicleFlags) == sizeof(std::uint32_t));

    ENDIAN_TEMPLATE(EndianType) struct Vehicle : Unit<EndianType> {
        EndianType<VehicleFlags> vehicle_flags;
        EndianType<VehicleType> vehicle_type;
        PAD(0x2);
        EndianType<float> maximum_forward_speed;
        EndianType<float> maximum_reverse_speed;
        EndianType<float> speed_acceleration;
        EndianType<float> speed_deceleration;
        EndianType<float> maximum_left_turn;
        EndianType<float> maximum_right_turn;
        EndianType<float> wheel_circumference;
        EndianType<float> turn_rate;
        EndianType<float> blur_speed;
        EndianType<VehicleFunctionIn> vehicle_a_in;
        EndianType<VehicleFunctionIn> vehicle_b_in;
        EndianType<VehicleFunctionIn> vehicle_c_in;
        EndianType<VehicleFunctionIn> vehicle_d_in;
        PAD(0xC);
        EndianType<float> maximum_left_slide;
        EndianType<float> maximum_right_slide;
        EndianType<float> slide_acceleration;
        EndianType<float> slide_deceleration;
        EndianType<float> minimum_flipping_angular_velocity;
        EndianType<float> maximum_flipping_angular_velocity;
        PAD(0x18);
        EndianType<float> fixed_gun_yaw;
        EndianType<float> fixed_gun_pitch;
        PAD(0x18);
        EndianType<float> ai_sideslip_distance;
        EndianType<float> ai_destination_radius;
        EndianType<float> ai_avoidance_distance;
        EndianType<float> ai_pathfinding_radius;
        EndianType<float> ai_charge_repeat_timeout;
        EndianType<float> ai_strafing_abort_range;
        Bounds<EndianType<float>> ai_oversteering_bounds;
        EndianType<Angle> ai_steering_maximum;
        EndianType<float> ai_throttle_maximum;
        EndianType<float> ai_move_position_time;
        PAD(0x4);
        TagDependency<EndianType> suspension_sound; // sound
        TagDependency<EndianType> crash_sound; // sound
        TagDependency<EndianType> material_effects; // material_effects
        TagDependency<EndianType> effect; // effect

        ENDIAN_TEMPLATE(NewType) operator Vehicle<NewType>() const noexcept {
            Vehicle<NewType> copy = {};
            COPY_UNIT_DATA
            COPY_THIS(vehicle_flags);
            COPY_THIS(vehicle_type);
            COPY_THIS(maximum_forward_speed);
            COPY_THIS(maximum_reverse_speed);
            COPY_THIS(speed_acceleration);
            COPY_THIS(speed_deceleration);
            COPY_THIS(maximum_left_turn);
            COPY_THIS(maximum_right_turn);
            COPY_THIS(wheel_circumference);
            COPY_THIS(turn_rate);
            COPY_THIS(blur_speed);
            COPY_THIS(vehicle_a_in);
            COPY_THIS(vehicle_b_in);
            COPY_THIS(vehicle_c_in);
            COPY_THIS(vehicle_d_in);
            COPY_THIS(maximum_left_slide);
            COPY_THIS(maximum_right_slide);
            COPY_THIS(slide_acceleration);
            COPY_THIS(slide_deceleration);
            COPY_THIS(minimum_flipping_angular_velocity);
            COPY_THIS(maximum_flipping_angular_velocity);
            COPY_THIS(fixed_gun_yaw);
            COPY_THIS(fixed_gun_pitch);
            COPY_THIS(ai_sideslip_distance);
            COPY_THIS(ai_destination_radius);
            COPY_THIS(ai_avoidance_distance);
            COPY_THIS(ai_pathfinding_radius);
            COPY_THIS(ai_charge_repeat_timeout);
            COPY_THIS(ai_strafing_abort_range);
            COPY_THIS(ai_oversteering_bounds);
            COPY_THIS(ai_steering_maximum);
            COPY_THIS(ai_throttle_maximum);
            COPY_THIS(ai_move_position_time);
            COPY_THIS(suspension_sound);
            COPY_THIS(crash_sound);
            COPY_THIS(material_effects);
            COPY_THIS(effect);
            return copy;
        }
    };
    static_assert(sizeof(Vehicle<BigEndian>) == 0x3F0);

    void compile_vehicle_tag(CompiledTag &compiled, const std::byte *data, std::size_t size);
}
