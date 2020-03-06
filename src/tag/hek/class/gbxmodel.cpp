// SPDX-License-Identifier: GPL-3.0-only

#include <invader/tag/parser/parser.hpp>
#include <invader/tag/hek/class/gbxmodel.hpp>

namespace Invader::HEK {
    void uncache_model_markers(Parser::GBXModel &model) {
        auto &regions = model.regions;
        for(auto &marker : model.markers) {
            auto add_marker_instance = [&marker, &regions](std::size_t instance_index) {
                // Get it
                auto instance = marker.instances[instance_index];
                marker.instances.erase(marker.instances.begin() + instance_index);

                // Figure out the region
                std::size_t region_index = instance.region_index;
                std::size_t region_count = regions.size();
                if(region_index >= region_count) {
                    eprintf_error("invalid region index (%zu >= %zu) in marker %s #%zu", region_index, region_count, marker.name.string, instance_index);
                    throw OutOfBoundsException();
                }

                // Next, figure out the region permutation
                auto &region = regions[region_index];
                std::size_t permutation_count = region.permutations.size();
                std::size_t permutation_index = instance.permutation_index;
                if(permutation_index >= permutation_count) {
                    eprintf_error("invalid permutation index (%zu >= %zu) for region #%zu in marker %s #%zu", permutation_index, permutation_count, region_index, marker.name.string, instance_index);
                    throw OutOfBoundsException();
                }

                // Lastly, add it to the beginning
                auto &new_marker = region.permutations[permutation_index].markers.emplace_back();
                new_marker.node_index = instance.node_index;
                new_marker.rotation = instance.rotation;
                new_marker.translation = instance.translation;
                new_marker.name = marker.name;
            };

            while(marker.instances.size()) {
                // Add the last one first
                std::size_t last_instance = marker.instances.size() - 1;
                auto &last_index_ref = marker.instances[last_instance];
                std::size_t permutation_index = last_index_ref.permutation_index;
                std::size_t region_index = last_index_ref.region_index;
                add_marker_instance(last_instance);

                // Add all ones like it in order
                for(std::size_t i = 0; i < last_instance; i++) {
                    auto &this_instance_ref = marker.instances[i];
                    if(this_instance_ref.permutation_index == permutation_index && this_instance_ref.region_index == region_index) {
                        add_marker_instance(i);
                        i--;
                        last_instance--;
                    }
                }
            }
        }
        model.markers.clear();
    }
}
