// SPDX-License-Identifier: GPL-3.0-only

#include "../compile.hpp"
#include "device_control.hpp"

namespace Invader::HEK {
    void compile_device_control_tag(CompiledTag &compiled, const std::byte *data, std::size_t size) {
        BEGIN_COMPILE(DeviceControl)
        COMPILE_DEVICE_DATA
        ADD_DEPENDENCY_ADJUST_SIZES(tag.on);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.off);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.deny);
        FINISH_COMPILE
    }
}
