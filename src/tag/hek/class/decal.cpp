// SPDX-License-Identifier: GPL-3.0-only

#include <invader/tag/hek/compile.hpp>
#include <invader/tag/hek/definition.hpp>

namespace Invader::HEK {
    void compile_decal_tag(CompiledTag &compiled, const std::byte *data, std::size_t size) {
        BEGIN_COMPILE(Decal);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.next_decal_in_chain);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.map);
        DEFAULT_VALUE(tag.animation_speed, 1);
        FINISH_COMPILE
    }
}
