// SPDX-License-Identifier: GPL-3.0-only

#include <invader/tag/parser/definition/lightning.hpp>

namespace Invader::Parser {
    void LightningShader::pre_compile(BuildWorkload &, std::size_t, std::size_t, std::size_t) {
        this->make_it_work = 1;
        this->some_more_stuff_that_should_be_set_for_some_reason = -1;
    }
}
