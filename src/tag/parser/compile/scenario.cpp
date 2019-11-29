// SPDX-License-Identifier: GPL-3.0-only

#include <invader/map/map.hpp>
#include <invader/map/tag.hpp>
#include <invader/tag/parser/parser.hpp>
#include <invader/build2/build_workload.hpp>

namespace Invader::Parser {
    void Scenario::pre_compile(BuildWorkload2 &workload, std::size_t tag_index, std::size_t, std::size_t) {
        if(!workload.cache_file_type.has_value()) {
            workload.cache_file_type = this->type;
        }

        // TODO: Swap endianness for script node table, adding a script syntax data and script string data block if none is present
    }
}
