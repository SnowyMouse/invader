// SPDX-License-Identifier: GPL-3.0-only

#include <invader/tag/parser/parser.hpp>
#include <invader/build/build_workload.hpp>

#include "hud_interface.hpp"

namespace Invader::Parser {
    void Contrail::pre_compile(BuildWorkload &, std::size_t , std::size_t, std::size_t) {
        this->unknown_int = 1;
    }

    void Contrail::post_compile(BuildWorkload &workload, std::size_t tag_index, std::size_t, std::size_t) {
        CHECK_BITMAP_SEQUENCE(workload, this->bitmap, this->first_sequence_index, "contrail");
    }
}
