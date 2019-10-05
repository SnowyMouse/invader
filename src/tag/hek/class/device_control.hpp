// SPDX-License-Identifier: GPL-3.0-only

#ifndef INVADER__TAG__HEK__CLASS__DEVICE_CONTROL_HPP
#define INVADER__TAG__HEK__CLASS__DEVICE_CONTROL_HPP

#include "device.hpp"

namespace Invader::HEK {
    enum DeviceType : TagEnum {
        DEVICE_TYPE_TOGGLE_SWITCH,
        DEVICE_TYPE_ON_BUTTON,
        DEVICE_TYPE_OFF_BUTTON,
        DEVICE_TYPE_CALL_BUTTON
    };
    enum DeviceTriggersWhen : TagEnum {
        DEVICE_TRIGGERS_WHEN_TRIGGERS_WHEN_TOUCHED_BY_PLAYER,
        DEVICE_TRIGGERS_WHEN_TRIGGERS_WHEN_DESTROYED
    };

    ENDIAN_TEMPLATE(EndianType) struct DeviceControl : Device<EndianType> {
        EndianType<DeviceType> type;
        EndianType<DeviceTriggersWhen> triggers_when;
        EndianType<float> call_value;
        PAD(0x50);
        TagDependency<EndianType> on; // .sound / .effect
        TagDependency<EndianType> off; // .sound / .effect
        TagDependency<EndianType> deny; // .sound / .effect

        ENDIAN_TEMPLATE(NewType) operator DeviceControl<NewType>() const noexcept {
            DeviceControl<NewType> copy = {};
            COPY_DEVICE_DATA
            COPY_THIS(type);
            COPY_THIS(triggers_when);
            COPY_THIS(call_value);
            COPY_THIS(on);
            COPY_THIS(off);
            COPY_THIS(deny);
            return copy;
        }
    };
    static_assert(sizeof(DeviceControl<NativeEndian>) == 0x318);

    void compile_device_control_tag(CompiledTag &compiled, const std::byte *data, std::size_t size);
}
#endif
