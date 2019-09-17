/*
 * Invader (c) 2018 Kavawuvi
 *
 * This program is free software under the GNU General Public License v3.0. See LICENSE for more information.
 */

#include "../compile.hpp"

#include "garbage.hpp"

namespace Invader::HEK {
    void compile_garbage_tag(CompiledTag &compiled, const std::byte *data, std::size_t size) {
        BEGIN_COMPILE(Garbage)
        COMPILE_ITEM_DATA
        tag.object_type = OBJECT_TYPE_GARBAGE;
        FINISH_COMPILE
    }
}
