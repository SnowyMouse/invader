// SPDX-License-Identifier: GPL-3.0-only

#include <invader/tag/hek/compile.hpp>
#include <invader/tag/hek/definition.hpp>

namespace Invader::HEK {
    void compile_item_collection_tag(CompiledTag &compiled, const std::byte *data, std::size_t size) {
        BEGIN_COMPILE(ItemCollection);
        ADD_BASIC_DEPENDENCY_REFLEXIVE(tag.permutations, item);
        FINISH_COMPILE
    }
}
