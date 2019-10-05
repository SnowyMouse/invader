// SPDX-License-Identifier: GPL-3.0-only

#include "../compile.hpp"
#include "tag_collection.hpp"

namespace Invader::HEK {
    void compile_tag_collection_tag(CompiledTag &compiled, const std::byte *data, std::size_t size) {
        BEGIN_COMPILE(TagCollection);
        ADD_BASIC_DEPENDENCY_REFLEXIVE(tag.tags, reference);
        FINISH_COMPILE
    }
}
