// SPDX-License-Identifier: GPL-3.0-only

#include <invader/tag/parser/parser.hpp>

namespace Invader::Parser {
    void Meter::pre_compile(BuildWorkload &, std::size_t, std::size_t, std::size_t) {
        this->postprocess_hek_data();
    }
    void Meter::postprocess_hek_data() {
        this->source_bitmap = {};
        this->stencil_bitmaps = {};
    }
}
