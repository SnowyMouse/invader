/*
 * Invader (c) 2018 Kavawuvi
 *
 * This program is free software under the GNU General Public License v3.0 or later. See LICENSE for more information.
 */

#ifndef INVADER__TAG__HEK__CLASS__DEVICE_HPP
#define INVADER__TAG__HEK__CLASS__DEVICE_HPP

#include "../../../hek/constants.hpp"
#include "object.hpp"

namespace Invader::HEK {
    struct DeviceFlags {
        std::uint32_t position_loops : 1;
        std::uint32_t position_not_interpolated : 1;
    };

    enum DeviceIn : TagEnum {
        DEVICE_IN_NONE,
        DEVICE_IN_POWER,
        DEVICE_IN_CHANGE_IN_POWER,
        DEVICE_IN_POSITION,
        DEVICE_IN_CHANGE_IN_POSITION,
        DEVICE_IN_LOCKED,
        DEVICE_IN_DELAY,
    };

    ENDIAN_TEMPLATE(EndianType) struct Device : Object<EndianType> {
        EndianType<DeviceFlags> device_flags;
        EndianType<float> power_transition_time;
        EndianType<float> power_acceleration_time;
        EndianType<float> position_transition_time;
        EndianType<float> position_acceleration_time;
        EndianType<float> depowered_position_transition_time;
        EndianType<float> depowered_position_acceleration_time;
        EndianType<DeviceIn> device_a_in;
        EndianType<DeviceIn> device_b_in;
        EndianType<DeviceIn> device_c_in;
        EndianType<DeviceIn> device_d_in;
        TagDependency<EndianType> open; // ..sound / .effect
        TagDependency<EndianType> close; // ..sound / .effect
        TagDependency<EndianType> opened; // ..sound / .effect
        TagDependency<EndianType> closed; // ..sound / .effect
        TagDependency<EndianType> depowered; // ..sound / .effect
        TagDependency<EndianType> repowered; // ..sound / .effect
        EndianType<float> delay_time;
        PAD(0x8);
        TagDependency<EndianType> delay_effect; // ..sound / .effect
        EndianType<float> automatic_activation_radius;
        PAD(0x54);
        LittleEndian<float> inverse_power_acceleration_time;
        LittleEndian<float> inverse_power_transition_time;
        PAD(0x8);
        LittleEndian<float> inverse_position_acceleration_time;
        LittleEndian<float> inverse_position_transition_time;
        PAD(0x4);

        #define COPY_DEVICE_DATA COPY_OBJECT_DATA \
                                 COPY_THIS(device_flags); \
                                 COPY_THIS(power_transition_time); \
                                 COPY_THIS(power_acceleration_time); \
                                 COPY_THIS(position_transition_time); \
                                 COPY_THIS(position_acceleration_time); \
                                 COPY_THIS(depowered_position_transition_time); \
                                 COPY_THIS(depowered_position_acceleration_time); \
                                 COPY_THIS(device_a_in); \
                                 COPY_THIS(device_b_in); \
                                 COPY_THIS(device_c_in); \
                                 COPY_THIS(device_d_in); \
                                 COPY_THIS(open); \
                                 COPY_THIS(close); \
                                 COPY_THIS(opened); \
                                 COPY_THIS(closed); \
                                 COPY_THIS(depowered); \
                                 COPY_THIS(repowered); \
                                 COPY_THIS(delay_time); \
                                 COPY_THIS(delay_effect); \
                                 COPY_THIS(automatic_activation_radius); \
                                 COPY_THIS(inverse_position_acceleration_time);\
                                 COPY_THIS(inverse_position_transition_time); \
                                 COPY_THIS(inverse_power_acceleration_time); \
                                 COPY_THIS(inverse_power_transition_time);

        ENDIAN_TEMPLATE(NewType) operator Device<NewType>() const noexcept {
            Device<NewType> copy = {};
            COPY_DEVICE_DATA
            return copy;
        }
    };
    static_assert(sizeof(Device<NativeEndian>) == 0x290);

    #define COMPILE_DEVICE_DATA COMPILE_OBJECT_DATA \
                                ADD_DEPENDENCY_ADJUST_SIZES(tag.open); \
                                ADD_DEPENDENCY_ADJUST_SIZES(tag.close); \
                                ADD_DEPENDENCY_ADJUST_SIZES(tag.opened); \
                                ADD_DEPENDENCY_ADJUST_SIZES(tag.closed); \
                                ADD_DEPENDENCY_ADJUST_SIZES(tag.depowered); \
                                ADD_DEPENDENCY_ADJUST_SIZES(tag.repowered); \
                                ADD_DEPENDENCY_ADJUST_SIZES(tag.delay_effect); \
                                tag.inverse_power_transition_time = 1.0F / (TICK_RATE * tag.power_transition_time); \
                                tag.inverse_power_acceleration_time = 1.0F / (TICK_RATE * tag.power_acceleration_time); \
                                tag.inverse_position_transition_time = 1.0f / (TICK_RATE * tag.position_transition_time); \
                                tag.inverse_position_acceleration_time = 1.0f / (TICK_RATE * tag.position_acceleration_time);
}
#endif
