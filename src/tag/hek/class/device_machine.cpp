// SPDX-License-Identifier: GPL-3.0-only

#include <invader/tag/hek/compile.hpp>
#include <invader/tag/hek/class/device_machine.hpp>

namespace Invader::HEK {
    void compile_device_machine_tag(CompiledTag &compiled, const std::byte *data, std::size_t size) {
        BEGIN_COMPILE(DeviceMachine)
        COMPILE_DEVICE_DATA
        tag.object_type = ObjectType::OBJECT_TYPE_DEVICE_MACHINE;
        tag.door_open_time_ticks = static_cast<std::uint32_t>(tag.door_open_time * TICK_RATE);
        FINISH_COMPILE
    }
}
