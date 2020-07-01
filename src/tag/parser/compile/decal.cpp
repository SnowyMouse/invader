// SPDX-License-Identifier: GPL-3.0-only

#include <invader/tag/parser/parser.hpp>

namespace Invader::Parser {
    void Decal::postprocess_hek_data() {
        this->radius.to = std::max(this->radius.from, this->radius.to);
    }
}
