/*
 * Invader (c) 2018 Kavawuvi
 *
 * This program is free software under the GNU General Public License v3.0 or later. See LICENSE for more information.
 */

#ifndef INVADER__TAG__HEK__CLASS__DEVICE_MACHINE_HPP
#define INVADER__TAG__HEK__CLASS__DEVICE_MACHINE_HPP

#include "device.hpp"

namespace Invader::HEK {
    enum MachineType : TagEnum {
        MACHINE_TYPE_DOOR,
        MACHINE_TYPE_PLATFORM,
        MACHINE_TYPE_GEAR,
    };

    enum MachineCollisionResponse : TagEnum {
        MACHINE_COLLISION_RESPONSE_PAUSE_UNTIL_CRUSHED,
        MACHINE_COLLISION_RESPONSE_REVERSE_DIRECTIONS
    };

    struct MachineFlags {
        std::uint16_t pathfinding_obstacle : 1;
        std::uint16_t _but_not_when_open : 1;
        std::uint16_t elevator : 1;
    };

    ENDIAN_TEMPLATE(EndianType) struct DeviceMachine : Device<EndianType> {
        EndianType<MachineType> machine_type;
        EndianType<MachineFlags> machine_flags;
        EndianType<float> door_open_time;
        PAD(0x50);
        EndianType<MachineCollisionResponse> collision_response;
        EndianType<std::int16_t> elevator_node;
        PAD(0x34);
        LittleEndian<std::uint32_t> door_open_time_ticks;

        ENDIAN_TEMPLATE(NewType) operator DeviceMachine<NewType>() const noexcept {
            DeviceMachine<NewType> copy = {};
            COPY_DEVICE_DATA
            COPY_THIS(machine_type);
            COPY_THIS(machine_flags);
            COPY_THIS(door_open_time);
            COPY_THIS(collision_response);
            COPY_THIS(elevator_node);
            COPY_THIS(door_open_time_ticks);
            return copy;
        }
    };
    static_assert(sizeof(DeviceMachine<BigEndian>) == 0x324);

    void compile_device_machine_tag(CompiledTag &compiled, const std::byte *data, std::size_t size);
}
#endif
