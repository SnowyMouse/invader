// SPDX-License-Identifier: GPL-3.0-only

#include <invader/build/build_workload.hpp>

using namespace Invader::Parser;

namespace Invader {
    bool BuildWorkload::BuildWorkloadStruct::can_dedupe(const BuildWorkload::BuildWorkloadStruct &other) const noexcept {
        std::size_t this_size = this->data.size();
        std::size_t other_size = other.data.size();
        
        if(this->unsafe_to_dedupe || other.unsafe_to_dedupe || this->bsp != other.bsp || other_size > this_size) {
            return false;
        }
        
        // Make sure dependencies match
        if(this->dependencies != other.dependencies) {
            std::vector<BuildWorkloadDependency> this_dep_small;
            for(auto &td : this->dependencies) {
                if(td.offset < other_size) {
                    if(td.offset + sizeof(TagDependency<LittleEndian>) > other_size) { // other struct only contains part of the dependency
                        return false;
                    }
                    this_dep_small.emplace_back(td);
                }
            }
            if(this_dep_small != other.dependencies) {
                return false;
            }
        }
        
        // And now pointers
        if(this->pointers != other.pointers) {
            std::vector<BuildWorkloadStructPointer> this_ptr_small;
            for(auto &ptr : this->pointers) {
                if(ptr.offset < other_size) {
                    this_ptr_small.emplace_back(ptr);
                }
            }
            if(this_ptr_small != other.pointers) {
                return false;
            }
        }
        
        return std::memcmp(this->data.data(), other.data.data(), other_size) == 0;
    }

    void BuildWorkload::dedupe_structs() {
        bool found_something = true;
        std::size_t total_savings = 0;
        std::size_t struct_count = this->structs.size();

        oprintf("Optimizing tag space...");
        oflush();

        while(found_something) {
            found_something = false;
            for(std::size_t i = 0; i < struct_count; i++) {
                if(this->structs[i].unsafe_to_dedupe) {
                    continue;
                }
                for(std::size_t j = i + 1; j < struct_count; j++) {
                    if(this->structs[j].unsafe_to_dedupe) {
                        continue;
                    }

                    // Check if the structs are the same
                    if(this->structs[i].can_dedupe(this->structs[j])) {
                        // If so, go through every struct pointer. If they equal j, set to i. If they're greater than j, decrement
                        for(std::size_t k = 0; k < struct_count; k++) {
                            for(auto &pointer : this->structs[k].pointers) {
                                auto &struct_index = pointer.struct_index;
                                if(struct_index == j) {
                                    struct_index = i;
                                }
                            }
                        }

                        // Also go through every tag, too
                        for(auto &tag : this->tags) {
                            if(tag.base_struct.has_value()) {
                                auto &base_struct = tag.base_struct.value();
                                if(base_struct == j) {
                                    base_struct = i;
                                }
                            }
                        }

                        total_savings += this->structs[j].data.size();
                        this->structs[j].unsafe_to_dedupe = true;

                        found_something = true;
                    }
                }
            }
        }
        oprintf(" done; reduced tag space usage by %.02f MiB\n", total_savings / 1024.0 / 1024.0);
    }
}
