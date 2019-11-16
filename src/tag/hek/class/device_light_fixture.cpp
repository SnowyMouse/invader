// SPDX-License-Identifier: GPL-3.0-only

#include <invader/tag/hek/compile.hpp>
#include <invader/tag/hek/class/device_light_fixture.hpp>

namespace Invader::HEK {
    void compile_device_light_fixture_tag(CompiledTag &compiled, const std::byte *data, std::size_t size) {
        BEGIN_COMPILE(DeviceLightFixture)
        COMPILE_DEVICE_DATA
        FINISH_COMPILE
    }
}
