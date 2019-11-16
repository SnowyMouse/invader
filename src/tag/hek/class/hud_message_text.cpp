// SPDX-License-Identifier: GPL-3.0-only

#include <invader/tag/hek/compile.hpp>
#include <invader/tag/hek/definition.hpp>

namespace Invader::HEK {
    void compile_hud_message_text_tag(CompiledTag &compiled, const std::byte *data, std::size_t size) {
        BEGIN_COMPILE(HUDMessageText)

        std::size_t data_size = tag.text_data.size;
        ASSERT_SIZE(data_size);
        ADD_POINTER_FROM_INT32(tag.text_data.pointer, compiled.data.size());
        tag.text_data.file_offset = static_cast<std::uint32_t>(compiled.data.size());
        compiled.data.insert(compiled.data.end(), data, data + data_size);
        PAD_32_BIT
        INCREMENT_DATA_PTR(data_size);

        ADD_REFLEXIVE(tag.message_elements);
        ADD_REFLEXIVE(tag.messages);

        FINISH_COMPILE
    }
}
