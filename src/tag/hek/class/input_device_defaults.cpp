// SPDX-License-Identifier: GPL-3.0-only

#include "invader/tag/hek/compile.hpp"

#include "invader/tag/hek/class/input_device_defaults.hpp"

namespace Invader::HEK {
    void compile_input_device_defaults_tag(CompiledTag &compiled, const std::byte *data, std::size_t size) {
        BEGIN_COMPILE(InputDeviceDefaults);
        ADD_POINTER_FROM_INT32(tag.device_id.pointer, compiled.data.size());
        compiled.data.insert(compiled.data.end(), data, data + tag.device_id.size);
        INCREMENT_DATA_PTR(tag.device_id.size);
        ADD_POINTER_FROM_INT32(tag.profile.pointer, compiled.data.size());
        compiled.data.insert(compiled.data.end(), data, data + tag.profile.size);
        INCREMENT_DATA_PTR(tag.profile.size);
        FINISH_COMPILE
    }
}
