// SPDX-License-Identifier: GPL-3.0-only

#include <invader/tag/parser/definition/glow.hpp>

namespace Invader::Parser {
    void Glow::pre_compile(BuildWorkload &, std::size_t, std::size_t, std::size_t) {
        this->attachment_0 = static_cast<FunctionOut>(this->attachment_0 - 1);
        this->attachment_1 = static_cast<FunctionOut>(this->attachment_1 - 1);
        this->attachment_2 = static_cast<FunctionOut>(this->attachment_2 - 1);
        this->attachment_3 = static_cast<FunctionOut>(this->attachment_3 - 1);
        this->attachment_4 = static_cast<FunctionOut>(this->attachment_4 - 1);
        this->attachment_5 = static_cast<FunctionOut>(this->attachment_5 - 1);
    }
}
