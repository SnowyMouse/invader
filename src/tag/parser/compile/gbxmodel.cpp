// SPDX-License-Identifier: GPL-3.0-only

#include <invader/build2/build_workload.hpp>
#include <invader/tag/parser/parser.hpp>

namespace Invader::Parser {
    /*
    // Figure out the region
    std::size_t region_index = instance.region_index;
    std::size_t region_count = this->regions.size();
    if(region_index >= region_count) {
        eprintf_error("invalid region index %zu / %zu", region_index, region_count);
        throw OutOfBoundsException();
    }

    // Next, figure out the region permutation
    auto &region = this->regions[region_index];
    std::size_t permutation_count = region.permutations.size();
    std::size_t permutation_index = instance.permutation_index;
    if(permutation_index >= permutation_count) {
        eprintf_error("invalid permutation index %zu / %zu for region #%zu", permutation_index, permutation_count, region_index);
        throw OutOfBoundsException();
    }

    // Lastly
    auto &new_marker = region.permutations[permutation_index].markers.emplace_back();
    new_marker.name = marker.name;
    new_marker.node_index = instance.node_index;
    new_marker.rotation = instance.rotation;
    new_marker.translation = instance.translation;
     */

    void GBXModel::pre_compile(BuildWorkload2 &workload, std::size_t tag_index, std::size_t, std::size_t) {
        // Swap this stuff
        float super_low = this->super_low_detail_cutoff;
        float low = this->low_detail_cutoff;
        float high = this->high_detail_cutoff;
        float super_high = this->super_high_detail_cutoff;

        this->super_low_detail_cutoff = super_high;
        this->low_detail_cutoff = high;
        this->high_detail_cutoff = low;
        this->super_high_detail_cutoff = super_low;

        if(this->markers.size() == 0) {
            eprintf_warn("Markers array is populated.");
            eprintf_warn("This is DEPRECATED and will not be allowed in some future version of Invader.");
            workload.report_error(BuildWorkload2::ErrorType::ERROR_TYPE_WARNING, "To fix this, rebuild the model tag", tag_index);
        }

        // Put all of the markers in the marker array
        for(auto &r : this->regions) {
            for(auto &p : r.permutations) {
                for(auto &m : p.markers) {
                    GBXModelMarkerInstance instance;
                    instance.node_index = m.node_index;
                    instance.permutation_index = &p - r.permutations.data();
                    instance.region_index = &r - this->regions.data();
                    instance.rotation = m.rotation;
                    instance.translation = m.translation;

                    // Add it!
                    bool found = false;
                    for(auto &ma : this->markers) {
                        if(m.name == ma.name) {
                            ma.instances.push_back(instance);
                            found = true;
                            break;
                        }
                    }

                    // Add the marker and then add it!
                    if(!found) {
                        auto &ma = this->markers.emplace_back();
                        ma.name = m.name;
                        ma.instances.push_back(instance);
                    }
                }
            }
        }
    }
}
