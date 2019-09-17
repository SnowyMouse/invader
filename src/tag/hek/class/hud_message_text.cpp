/*
 * Invader (c) 2019 Kavawuvi
 *
 * This program is free software under the GNU General Public License v3.0. See LICENSE for more information.
 */

#include "../compile.hpp"
#include "hud_message_text.hpp"

namespace Invader::HEK {
    void compile_hud_message_text_tag(CompiledTag &compiled, const std::byte *data, std::size_t size) {
        BEGIN_COMPILE(HUDMessageText)

        std::size_t data_size = tag.text_data.size;
        ASSERT_SIZE(data_size);
        ADD_POINTER_FROM_INT32(tag.text_data.pointer, compiled.data.size());
        tag.text_data.file_offset = static_cast<std::int32_t>(compiled.data.size());
        compiled.data.insert(compiled.data.end(), data, data + data_size);
        PAD_32_BIT
        INCREMENT_DATA_PTR(data_size);

        ADD_REFLEXIVE(tag.message_elements);
        ADD_REFLEXIVE(tag.messages);

        FINISH_COMPILE
    }
}
