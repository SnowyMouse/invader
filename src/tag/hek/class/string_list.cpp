/*
 * Invader (c) 2018 Kavawuvi
 *
 * This program is free software under the GNU General Public License v3.0 or later. See LICENSE for more information.
 */

#include "../compile.hpp"
#include "string_list.hpp"

namespace Invader::HEK {
    void compile_string_list_tag(CompiledTag &compiled, const std::byte *data, std::size_t size) {
        BEGIN_COMPILE(StringList)
        ADD_REFLEXIVE_START(tag.strings) {
            std::size_t data_size = reflexive.string.size;
            ASSERT_SIZE(data_size);
            ADD_POINTER_FROM_INT32(reflexive.string.pointer, compiled.data.size());
            reflexive.string.file_offset = static_cast<std::int32_t>(compiled.data.size());
            compiled.data.insert(compiled.data.end(), data, data + data_size);
            PAD_32_BIT
            INCREMENT_DATA_PTR(data_size);
        } ADD_REFLEXIVE_END
        FINISH_COMPILE
    }
}
