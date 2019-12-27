// SPDX-License-Identifier: GPL-3.0-only

#include <invader/tag/parser/parser.hpp>

namespace Invader::Parser {
    void LensFlare::pre_compile(BuildWorkload &, std::size_t, std::size_t, std::size_t) {
        this->cos_falloff_angle = std::cos(this->falloff_angle);
        this->cos_cutoff_angle = std::cos(this->cutoff_angle);
    }
}
