// SPDX-License-Identifier: GPL-3.0-only

#include <invader/tag/parser/definition/item_collection.hpp>
#include <invader/build/build_workload.hpp>

namespace Invader::Parser {
    void ItemCollectionPermutation::pre_compile(BuildWorkload &workload, std::size_t tag_index, std::size_t, std::size_t offset) {
        if(this->weight <= 0.0F) {
            REPORT_ERROR_PRINTF(workload, ERROR_TYPE_WARNING, tag_index, "Permutation #%zu will not be used (no weight set)", offset / sizeof(C<LittleEndian>));
        }
    }
}
