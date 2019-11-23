// SPDX-License-Identifier: GPL-3.0-only

#include <invader/tag/hek/compile.hpp>
#include <invader/tag/hek/definition.hpp>

namespace Invader::HEK {
    void compile_meter_tag(CompiledTag &compiled, const std::byte *data, std::size_t size) {
        BEGIN_COMPILE(Meter)
        // These don't work in Halo anyway
        //ADD_DEPENDENCY_ADJUST_SIZES(tag.stencil_bitmaps);
        //ADD_DEPENDENCY_ADJUST_SIZES(tag.source_bitmap);
        tag.stencil_bitmaps.tag_id = TagID::null_tag_id();
        tag.source_bitmap.tag_id = TagID::null_tag_id();
        ADD_POINTER_FROM_INT32(tag.encoded_stencil.pointer, compiled.data.size());
        std::size_t data_size = tag.encoded_stencil.size;
        compiled.data.insert(compiled.data.end(), data, data + data_size);
        PAD_32_BIT
        INCREMENT_DATA_PTR(data_size);
        FINISH_COMPILE
    }
}
