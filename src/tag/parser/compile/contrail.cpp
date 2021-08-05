// SPDX-License-Identifier: GPL-3.0-only

#include <invader/tag/parser/definition/contrail.hpp>
#include <invader/build/build_workload.hpp>

namespace Invader::Parser {
    void Contrail::pre_compile(BuildWorkload &, std::size_t , std::size_t, std::size_t) {
        this->unknown_int = 1;
    }
}
