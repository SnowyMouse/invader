// SPDX-License-Identifier: GPL-3.0-only

#include <invader/tag/hek/compile.hpp>
#include <invader/tag/hek/definition.hpp>
#include "compile.hpp"

namespace Invader::HEK {
    void compile_device_light_fixture_tag(CompiledTag &compiled, const std::byte *data, std::size_t size) {
        BEGIN_COMPILE(DeviceLightFixture)
        COMPILE_DEVICE_DATA
        tag.object_type = ObjectType::OBJECT_TYPE_DEVICE_LIGHT_FIXTURE;
        FINISH_COMPILE
    }
}
