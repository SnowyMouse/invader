// SPDX-License-Identifier: GPL-3.0-only

#include <invader/tag/hek/compile.hpp>
#include <invader/tag/hek/definition.hpp>
#include "compile.hpp"

namespace Invader::HEK {
    void compile_object_tag(CompiledTag &compiled, const std::byte *data, std::size_t size, ObjectType type) {
        BEGIN_COMPILE(BasicObject);
        COMPILE_OBJECT_DATA
        tag.object_type = type;
        FINISH_COMPILE
    }
}
