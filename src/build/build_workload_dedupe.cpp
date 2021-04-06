#include <invader/build/build_workload.hpp>

namespace Invader {
    bool BuildWorkload::BuildWorkloadStruct::can_dedupe(const BuildWorkload::BuildWorkloadStruct &other) const noexcept {
        if(this->unsafe_to_dedupe || other.unsafe_to_dedupe || (this->bsp.has_value() && this->bsp != other.bsp)) {
            return false;
        }

        std::size_t this_size = this->data.size();
        std::size_t other_size = other.data.size();

        if(this->dependencies == other.dependencies && this->pointers == other.pointers && this_size >= other_size) {
            return std::memcmp(this->data.data(), other.data.data(), other_size) == 0;
        }

        return false;
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
