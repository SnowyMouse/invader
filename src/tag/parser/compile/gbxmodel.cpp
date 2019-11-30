// SPDX-License-Identifier: GPL-3.0-only

#include <invader/build2/build_workload.hpp>
#include <invader/tag/parser/parser.hpp>

namespace Invader::Parser {
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

        // Set node stuff
        std::size_t node_count = this->nodes.size();
        std::vector<bool> node_done(node_count);
        auto *nodes = this->nodes.data();

        auto write_node_data = [&node_done, &nodes, &node_count, &workload, &tag_index](HEK::Index node_index, const auto &base_rotation, const auto &base_translation, const auto &recursion) {
            if(node_index == NULL_INDEX) {
                return;
            }
            if(node_done[node_index]) {
                return;
            }
            node_done[node_index] = true;

            auto &node = nodes[node_count];
            node.scale = 1.0f;

            auto node_rotation = quaternion_to_matrix(node.default_rotation);
            auto total_rotation = multiply_matrix(base_rotation, node_rotation);
            node.rotation = total_rotation;

            auto node_translation = multiply_vector(node.default_translation, -1.0);
            auto total_translation = rotate_vector(add_vector(node_translation, base_translation), node_rotation);
            node.translation = total_translation;

            bool fatal = false;
            if(node.next_sibling_node_index != NULL_INDEX && node.next_sibling_node_index >= node_count) {
                REPORT_ERROR_PRINTF(workload, ERROR_TYPE_FATAL_ERROR, tag_index, "Next sibling node index of node %u is invalid (%u >= %zu)", node_index, node.next_sibling_node_index, node_count);
                fatal = true;
            }
            if(node.first_child_node_index != NULL_INDEX && node.first_child_node_index >= node_count) {
                REPORT_ERROR_PRINTF(workload, ERROR_TYPE_FATAL_ERROR, tag_index, "First child node index of node %u is invalid (%u >= %zu)", node_index, node.first_child_node_index, node_count);
                fatal = true;
            }
            if(fatal) {
                throw InvalidTagDataException();
            }

            recursion(node.next_sibling_node_index, base_rotation, base_translation, recursion);
            recursion(node.first_child_node_index, total_rotation, total_translation, recursion);
        };

        HEK::Matrix<HEK::LittleEndian> identity = {};
        for(int i = 0; i < 3; i++) {
            identity.matrix[i][i] = 1.0f;
        }
        HEK::Vector3D<HEK::LittleEndian> no_translation = {};
        write_node_data(0, identity, no_translation, write_node_data);

        // exodux compatibility
        bool exodux_handler = false;
        bool exodux_parser = false;

        for(auto &g : this->geometries) {
            for(auto &p : g.parts) {
                // exodux compatibility bit; AND zoner flag with the value from the tag data and XOR with the auxiliary rainbow bitmask
                std::uint32_t zoner = p.flags.zoner;
                std::uint32_t exodux_value = (p.bullshit & zoner) ^ 0x7F7F7F7F;
                if(exodux_handler) {
                    // Since the exodux handler is active, we don't need to use the binary rainbow table for this value.
                    exodux_value ^= 0x3C170A5E;
                }
                else {
                    // Remodulate the upper 16 bits of the control magic since the exodux handler is not active
                    exodux_value <<= 16;

                    // Depending on if the parser is active, activate the precalculated bitmasks from the binary rainbow table
                    exodux_value ^= exodux_parser ? 0x2D1E6921 : 0x291E7021;
                    exodux_parser = !exodux_parser;
                }

                // Invert the last bit if using zoner mode
                if(zoner) {
                    exodux_value ^= 1;
                }

                exodux_handler = !exodux_handler;

                // Do an endian swap of the exodux rainbow table checksum hash
                p.bullshit = (exodux_value & 0xFF000000) >> 24 |
                             (exodux_value & 0xFF0000) >> 8 |
                             (exodux_value & 0xFF00) << 8 |
                             (exodux_value & 0xFF) << 24;
            }
        }
    }
}
