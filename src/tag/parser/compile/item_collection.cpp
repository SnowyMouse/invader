// SPDX-License-Identifier: GPL-3.0-only

#include <invader/tag/parser/parser.hpp>
#include <invader/build/build_workload.hpp>

namespace Invader::Parser {
    void ItemCollectionPermutation::pre_compile(BuildWorkload &workload, std::size_t tag_index, std::size_t, std::size_t offset) {
        if(this->weight < 0.0F) {
            REPORT_ERROR_PRINTF(workload, ERROR_TYPE_WARNING, tag_index, "Permutation #%zu will not be used (weight < 1)", offset / sizeof(struct_little));
        }

        float weight_float = this->weight;
        std::int32_t weight_integer = static_cast<std::int32_t>(weight_float);
        float weight_float_again = static_cast<float>(weight_integer);
        if(weight_float != weight_float_again) {
            REPORT_ERROR_PRINTF(workload, ERROR_TYPE_WARNING, tag_index, "Permutation #%zu has a fractional weight. This will be rounded down to %i in runtime.", offset / sizeof(struct_little), weight_integer);
        }
    }

    void ItemCollection::pre_compile(BuildWorkload &workload, std::size_t tag_index, std::size_t, std::size_t) {
        std::uint64_t total_weight = 0;
        for(auto &p : this->permutations) {
            if(p.weight >= 0.0 && p.weight <= 32768.0) {
                total_weight += static_cast<std::uint64_t>(p.weight);
            }
            if(total_weight > 32768) {
                REPORT_ERROR_PRINTF(workload, ERROR_TYPE_FATAL_ERROR, tag_index, "Total weight sum exceeds 32768. This can crash the game.");
                throw InvalidTagDataException();
            }
        }
        if(total_weight == 0) {
            REPORT_ERROR_PRINTF(workload, ERROR_TYPE_FATAL_ERROR, tag_index, "Total weight sum is 0. This can crash the game.");
            throw InvalidTagDataException();
        }
    }
}
