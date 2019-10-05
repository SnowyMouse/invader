// SPDX-License-Identifier: GPL-3.0-only

#include "../compile.hpp"
#include "meter.hpp"

namespace Invader::HEK {
    void compile_meter_tag(CompiledTag &compiled, const std::byte *data, std::size_t size) {
        BEGIN_COMPILE(Meter)
        ADD_DEPENDENCY_ADJUST_SIZES(tag.stencil_bitmaps);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.source_bitmap);
        ADD_POINTER_FROM_INT32(tag.encoded_stencil.pointer, compiled.data.size());
        std::size_t data_size = tag.encoded_stencil.size;
        compiled.data.insert(compiled.data.end(), data, data + data_size);
        PAD_32_BIT
        INCREMENT_DATA_PTR(data_size);
        FINISH_COMPILE
    }
}
