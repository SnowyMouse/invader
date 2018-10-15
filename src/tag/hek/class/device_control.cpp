/*
 * Invader (c) 2018 Kavawuvi
 *
 * This program is free software under the GNU General Public License v3.0 or later. See LICENSE for more information.
 */

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
