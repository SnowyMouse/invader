// SPDX-License-Identifier: GPL-3.0-only

#include "invader/tag/hek/compile.hpp"

#include "invader/tag/hek/class/equipment.hpp"

namespace Invader::HEK {
    void compile_equipment_tag(CompiledTag &compiled, const std::byte *data, std::size_t size) {
        BEGIN_COMPILE(Equipment)
        COMPILE_ITEM_DATA
        ADD_DEPENDENCY_ADJUST_SIZES(tag.pickup_sound);
        tag.object_type = OBJECT_TYPE_EQUIPMENT;
        FINISH_COMPILE
    }
}
