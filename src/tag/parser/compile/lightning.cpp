// SPDX-License-Identifier: GPL-3.0-only

#include <invader/tag/parser/parser.hpp>

namespace Invader::Parser {
    void LightningShader::pre_compile(BuildWorkload &, std::size_t, std::size_t, std::size_t) {
        this->shader_type = HEK::ShaderType::SHADER_TYPE_EFFECT;
        this->some_more_stuff_that_should_be_set_for_some_reason = -1;
    }
}
