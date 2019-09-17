/*
 * Invader (c) 2018 Kavawuvi
 *
 * This program is free software under the GNU General Public License v3.0. See LICENSE for more information.
 */

#include "../compile.hpp"
#include "device_light_fixture.hpp"

namespace Invader::HEK {
    void compile_device_light_fixture_tag(CompiledTag &compiled, const std::byte *data, std::size_t size) {
        BEGIN_COMPILE(DeviceLightFixture)
        COMPILE_DEVICE_DATA
        FINISH_COMPILE
    }
}
