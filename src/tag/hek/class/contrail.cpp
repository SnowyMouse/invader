// SPDX-License-Identifier: GPL-3.0-only

#include "invader/tag/hek/compile.hpp"

#include "invader/tag/hek/class/contrail.hpp"

namespace Invader::HEK {
    void compile_contrail_tag(CompiledTag &compiled, const std::byte *data, std::size_t size) {
        BEGIN_COMPILE(Contrail)
        ADD_DEPENDENCY_ADJUST_SIZES(tag.bitmap);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.secondary_bitmap);
        ADD_BASIC_DEPENDENCY_REFLEXIVE(tag.point_states, physics);
        tag.unknown_int = 1;
        FINISH_COMPILE
    }
}
