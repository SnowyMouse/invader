// SPDX-License-Identifier: GPL-3.0-only

#include <invader/build/build_workload.hpp>
#include <invader/tag/parser/parser.hpp>

namespace Invader::Parser {
    void GBXModel::post_hek_parse() {
        this->super_high_detail_node_count = 0;
        this->high_detail_node_count = 0;
        this->medium_detail_node_count = 0;
        this->low_detail_node_count = 0;
        this->super_low_detail_node_count = 0;

        auto &geometries = this->geometries;
        auto find_highest = [&geometries](std::uint16_t &c, HEK::Index what) {
            if(what >= geometries.size()) {
                c = 0;
            }
            else {
                for(auto &p : geometries[what].parts) {
                    for(auto &v : p.uncompressed_vertices) {
                        std::uint16_t node0 = v.node0_index;
                        std::uint16_t node1 = v.node1_index;

                        // If zoner, use local nodes
                        if(p.flags.zoner) {
                            constexpr const std::size_t LOCAL_NODE_COUNT = sizeof(p.local_node_indices) / sizeof(p.local_node_indices[0]);
                            node0 = (node0 < LOCAL_NODE_COUNT) ? p.local_node_indices[node0] : NULL_INDEX;
                            node1 = (node1 < LOCAL_NODE_COUNT) ? p.local_node_indices[node1] : NULL_INDEX;
                        }

                        if(node0 != NULL_INDEX && c < node0) {
                            c = node0;
                        }

                        if(node1 != NULL_INDEX && c < node1) {
                            c = node1;
                        }
                    }
                }
            }
        };

        for(auto &r : this->regions) {
            for(auto &p : r.permutations) {
                find_highest(this->super_high_detail_node_count, p.super_high);
                find_highest(this->high_detail_node_count, p.high);
                find_highest(this->medium_detail_node_count, p.medium);
                find_highest(this->low_detail_node_count, p.low);
                find_highest(this->super_low_detail_node_count, p.super_low);
            }
        }
    }

    void GBXModel::pre_compile(BuildWorkload &workload, std::size_t tag_index, std::size_t, std::size_t) {
        std::size_t region_count = this->regions.size();
        std::size_t geometries_count = this->geometries.size();
        std::size_t node_count = this->nodes.size();
        for(std::size_t ri = 0; ri < region_count; ri++) {
            auto &r = this->regions[ri];
            std::size_t permutation_count = r.permutations.size();
            for(std::size_t pi = 0; pi < permutation_count; pi++) {
                auto &p = r.permutations[pi];

                #define DO_GEO_CHECK(what, what_str) \
                if(p.what >= geometries_count) { \
                    REPORT_ERROR_PRINTF(workload, ERROR_TYPE_FATAL_ERROR, tag_index, what_str " geometry of permutation %zu of region %zu is incorrect (%zu >= %zu)", pi, ri, static_cast<std::size_t>(p.what), geometries_count); \
                    throw InvalidTagDataException(); \
                }
                DO_GEO_CHECK(super_high, "Super high");
                DO_GEO_CHECK(high, "High");
                DO_GEO_CHECK(medium, "Medium");
                DO_GEO_CHECK(low, "Low");
                DO_GEO_CHECK(super_low, "Super low");
                #undef DO_GEO_CHECK
            }
        }

        bool model_part_warned = false;
        for(auto &g : this->geometries) {
            for(auto &p : g.parts) {
                // Check this stuff!
                std::size_t compressed_vertex_count = p.compressed_vertices.size();
                std::size_t uncompressed_vertex_count = p.uncompressed_vertices.size();
                if(!model_part_warned && uncompressed_vertex_count != compressed_vertex_count && compressed_vertex_count != 0) {
                    REPORT_ERROR_PRINTF(workload, ERROR_TYPE_WARNING, tag_index, "Compressed vertex count (%zu) is not equal to uncompressed (%zu)", compressed_vertex_count, uncompressed_vertex_count)
                    eprintf_warn("To fix this, rebuild the model tag");
                    model_part_warned = true;
                }

                // Set these
                p.centroid_primary_node = 0;
                p.centroid_secondary_node = 0;
                p.centroid_primary_weight = 1.0F;
                p.centroid_secondary_weight = 0.0F;

                // Calculate weight
                std::size_t node_count = this->nodes.size();
                std::vector<float> node_weight(node_count, 0.0F);
                for(auto &v : p.uncompressed_vertices) {
                    std::size_t node0_index = static_cast<std::size_t>(v.node0_index);
                    std::size_t node1_index = static_cast<std::size_t>(v.node1_index);

                    if(p.flags.zoner) {
                        auto get_local_node = [&workload, &tag_index, &p](std::size_t index) -> std::size_t {
                            if(index != NULL_INDEX) {
                                constexpr const std::size_t LOCAL_NODE_COUNT = sizeof(p.local_node_indices) / sizeof(p.local_node_indices[0]);
                                if(index > LOCAL_NODE_COUNT) {
                                    REPORT_ERROR_PRINTF(workload, ERROR_TYPE_FATAL_ERROR, tag_index, "Vertex has an incorrect local node (%zu > %zu)", index, LOCAL_NODE_COUNT);
                                    eprintf_warn("To fix this, rebuild the model tag");
                                    throw InvalidTagDataException();
                                }
                                return p.local_node_indices[index];
                            }
                            else {
                                return NULL_INDEX;
                            }
                        };

                        node0_index = get_local_node(node0_index);
                        node1_index = get_local_node(node1_index);
                    }

                    if(v.node0_index != NULL_INDEX) {
                        if(node0_index > node_count) {
                            REPORT_ERROR_PRINTF(workload, ERROR_TYPE_FATAL_ERROR, tag_index, "Vertex has an incorrect node0 (%zu > %zu)", node0_index, node_count);
                            eprintf_warn("To fix this, rebuild the model tag");
                            throw InvalidTagDataException();
                        }
                        node_weight[node0_index] += v.node0_weight;
                    }

                    if(v.node1_index != NULL_INDEX) {
                        if(node1_index > node_count) {
                            REPORT_ERROR_PRINTF(workload, ERROR_TYPE_FATAL_ERROR, tag_index, "Vertex has an incorrect node1 (%zu > %zu)", node1_index, node_count);
                            eprintf_warn("To fix this, rebuild the model tag");
                            throw InvalidTagDataException();
                        }
                        node_weight[node1_index] += v.node1_weight;
                    }
                }

                // Sort nodes from highest to lowest if we have more than one node
                if(node_count > 1) {
                    std::vector<std::size_t> highest_nodes;
                    highest_nodes.reserve(node_count);
                    for(std::size_t n = 0; n < node_count; n++) {
                        bool added = false;
                        for(std::size_t n2 = 0; n2 < highest_nodes.size(); n2++) {
                            if(node_weight[n] > node_weight[highest_nodes[n2]]) {
                                highest_nodes.insert(highest_nodes.begin() + n2, n);
                                added = true;
                                break;
                            }
                        }
                        if(!added) {
                            highest_nodes.emplace_back(n);
                        }
                    }

                    // Check the top two
                    std::size_t first_highest = highest_nodes[0];
                    std::size_t second_highest = highest_nodes[1];
                    float first_highest_weight = node_weight[first_highest];
                    float second_highest_weight = node_weight[second_highest];

                    // If we have a centroid primary node, let's hear it
                    if(first_highest_weight > 0.0F) {
                        // Set the centroid primary node
                        p.centroid_primary_node = static_cast<HEK::Index>(first_highest);

                        // Next, do we have a secondary node? If so, divide the weight between the two and set the secondary node
                        if(second_highest_weight > 0.0F) {
                            p.centroid_secondary_node = static_cast<HEK::Index>(second_highest);
                            float total_weight = first_highest_weight + second_highest_weight;
                            p.centroid_primary_weight = first_highest_weight / total_weight;
                            p.centroid_secondary_weight = second_highest_weight / total_weight;
                        }
                    }
                }
            }
        }

        // Swap this stuff
        float super_low = this->super_low_detail_cutoff;
        float low = this->low_detail_cutoff;
        float high = this->high_detail_cutoff;
        float super_high = this->super_high_detail_cutoff;

        this->super_low_detail_cutoff = super_high;
        this->low_detail_cutoff = high;
        this->high_detail_cutoff = low;
        this->super_high_detail_cutoff = super_low;

        if(this->markers.size() > 0) {
            workload.report_error(BuildWorkload::ErrorType::ERROR_TYPE_WARNING, "Markers array is populated, but this array should be empty", tag_index);
            eprintf_warn("To fix this, rebuild the model tag");
        }

        // Put all of the markers in the marker array
        auto &markers = this->markers;
        for(std::size_t ri = 0; ri < region_count; ri++) {
            auto &r = this->regions[ri];
            std::size_t permutation_count = r.permutations.size();
            for(std::size_t pi = 0; pi < permutation_count; pi++) {
                auto &p = r.permutations[pi];

                // Add the markers
                while(p.markers.size()) {
                    auto add_marker = [&p, &ri, &pi, &markers](std::size_t index) {
                        // Pop
                        auto m = p.markers[index];
                        p.markers.erase(p.markers.begin() + index);

                        // Make the instance
                        GBXModelMarkerInstance instance;
                        instance.node_index = m.node_index;
                        instance.permutation_index = pi;
                        instance.region_index = ri;
                        instance.rotation = m.rotation;
                        instance.translation = m.translation;

                        // Add it!
                        bool found = false;
                        for(auto &ma : markers) {
                            if(m.name == ma.name) {
                                ma.instances.push_back(instance);
                                found = true;
                                break;
                            }
                        }

                        // Add the marker and then add it!
                        if(!found) {
                            GBXModelMarker ma = {};

                            ma.name = m.name;
                            ma.instances.push_back(instance);

                            for(auto &marker_group : markers) {
                                if(std::strcmp(marker_group.name.string, m.name.string) > 0) {
                                    markers.insert(markers.begin() + (&marker_group - markers.data()), ma);
                                    found = true;
                                    break;
                                }
                            }

                            if(!found) {
                                markers.push_back(ma);
                            }
                        }
                    };

                    // Get the first instance
                    auto first_instance_name = p.markers[0].name;

                    // Add all of the instances after it, first
                    for(std::size_t f = 1; f < p.markers.size(); f++) {
                        if(p.markers[f].name == first_instance_name) {
                            add_marker(f);
                            f--;
                        }
                    }

                    // Lastly, add this one
                    add_marker(0);
                }
            }
        }

        // Make sure the node indices in the markers we added are valid
        std::size_t errors_given = 0;
        for(auto &m : this->markers) {
            for(auto &i : m.instances) {
                if(i.node_index >= node_count) {
                    if(++errors_given == 5) {
                        REPORT_ERROR_PRINTF(workload, ERROR_TYPE_ERROR, tag_index, "Instance #%zu of marker #%zu has an invalid node index (%zu >= %zu)", &m - this->markers.data(), &i - m.instances.data(), static_cast<std::size_t>(i.node_index), node_count);
                        errors_given++;
                    }
                    else {
                        eprintf_error("... and more errors. Suffice it to say, the model needs recompiled");
                        break;
                    }
                }
            }
            if(errors_given == 5) {
                break;
            }
        }
        if(errors_given > 0 && errors_given < 5) {
            eprintf("This can be fixed by recompiling the model");
        }

        // Set node stuff
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

            auto &node = nodes[node_index];
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

        // exodux compatibility - recalibrate the bitmask using a high pass filter on the exodux compatibility bit
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

    void GBXModelGeometryPart::pre_compile(BuildWorkload &workload, std::size_t tag_index, std::size_t, std::size_t) {
        std::vector<HEK::Index> triangle_indices;

        // Add 1 to this
        workload.part_count++;

        // Add it all
        triangle_indices.reserve(this->triangles.size() * 3);
        for(auto &t : this->triangles) {
            triangle_indices.push_back(t.vertex0_index);
            triangle_indices.push_back(t.vertex1_index);
            triangle_indices.push_back(t.vertex2_index);
        }

        // Remove excess NULL_INDEX values
        std::size_t triangle_indices_size;
        while(true) {
            // Make sure we have enough indices for a triangle
            triangle_indices_size = triangle_indices.size();
            if(triangle_indices_size < 3) {
                REPORT_ERROR_PRINTF(workload, ERROR_TYPE_FATAL_ERROR, tag_index, "Triangle index count is invalid (%zu < 3)", triangle_indices_size);
                throw InvalidTagDataException();
            }

            // Here we go again
            auto &index = triangle_indices[triangle_indices_size - 1];
            if(index == NULL_INDEX) {
                triangle_indices.erase(triangle_indices.begin() + (&index - triangle_indices.data()));
            }
            else {
                break;
            }
        }

        // Subtract two (since each index is technically an individual triangle, minus the last two indices since you need three indices to make a triangle)
        this->triangle_count = triangle_indices_size - 2;

        // Make sure every triangle is valid
        std::size_t uncompressed_vertices_count = this->uncompressed_vertices.size();
        for(auto &t : triangle_indices) {
            if(t >= uncompressed_vertices_count) {
                REPORT_ERROR_PRINTF(workload, ERROR_TYPE_FATAL_ERROR, tag_index, "Index #%zu in triangle indices is invalid (%zu >= %zu)", &t - triangle_indices.data(), static_cast<std::size_t>(t), uncompressed_vertices_count);
                throw InvalidTagDataException();
            }
        }

        // See if we can find a copy of this
        std::size_t this_indices_count = triangle_indices_size;
        std::size_t indices_count = workload.model_indices.size();
        bool found = false;

        if(indices_count >= this_indices_count) {
            auto &first = triangle_indices[0];
            auto &last = triangle_indices[triangle_indices_size - 1];
            std::size_t check_size = this_indices_count - 1;

            for(std::size_t i = 0; i <= indices_count - this_indices_count; i++) {
                auto *model_data = workload.model_indices.data() + i;

                // Check the last index, first, since it's most likely to be different
                if(model_data[triangle_indices_size - 1] != last) {
                    continue;
                }

                // If triangles match, set the triangle offset to this instead
                if(std::memcmp(&first, model_data, sizeof(workload.model_indices[0]) * check_size) == 0) {
                    found = true;
                    this->triangle_offset = i * sizeof(workload.model_indices[0]);
                    break;
                }
            }
        }

        if(!found) {
            this->triangle_offset = indices_count * sizeof(workload.model_indices[0]);
            workload.model_indices.insert(workload.model_indices.end(), triangle_indices.begin(), triangle_indices.end());
        }
        this->triangle_offset_2 = this->triangle_offset;

        // Add the vertices, next
        this->vertex_count = this->uncompressed_vertices.size();
        std::vector<GBXModelVertexUncompressed::struct_little> vertices_of_fun;
        vertices_of_fun.reserve(this->vertex_count);
        for(auto &v : this->uncompressed_vertices) {
            auto &mv = vertices_of_fun.emplace_back();
            mv.binormal = v.binormal;
            mv.node0_index = v.node0_index;
            if(v.node1_index == NULL_INDEX) {
                mv.node1_index = 0;
            }
            else {
                mv.node1_index = v.node1_index;
            }
            mv.node0_weight = v.node0_weight;
            mv.node1_weight = v.node1_weight;
            mv.normal = v.normal;
            mv.position = v.position;
            mv.tangent = v.tangent;
            mv.texture_coords = v.texture_coords;
        }

        // Let's see if we can also dedupe this
        std::size_t this_vertices_count = vertices_of_fun.size();
        std::size_t vertices_count = workload.model_vertices.size();
        found = false;

        if(vertices_count >= this_vertices_count) {
            for(std::size_t i = 0; i <= vertices_count - this_vertices_count; i++) {
                // If vertices match, set the vertices offset to this instead
                if(std::memcmp(workload.model_vertices.data() + i, vertices_of_fun.data(), sizeof(workload.model_vertices[0]) * this_vertices_count) == 0) {
                    found = true;
                    this->vertex_offset = i * sizeof(workload.model_vertices[0]);
                    break;
                }
            }
        }

        if(!found) {
            this->vertex_offset = vertices_count * sizeof(workload.model_vertices[0]);
            workload.model_vertices.insert(workload.model_vertices.end(), vertices_of_fun.begin(), vertices_of_fun.end());
        }

        // Don't forget to set these memes
        this->do_not_crash_the_game = 1;
        this->do_not_screw_up_the_model = 4;
        this->prev_filthy_part_index = -1;
        this->next_filthy_part_index = -1;
    }

    void GBXModelRegionPermutation::pre_compile(BuildWorkload &workload, std::size_t tag_index, std::size_t, std::size_t) {
        const char *last_hyphen = nullptr;
        for(const char &c : this->name.string) {
            if(c == '-') {
                last_hyphen = &c + 1;
            }
        }
        this->permutation_number = 0;
        if(last_hyphen && *last_hyphen) {
            unsigned long permutation_number = ~0;
            try {
                permutation_number = std::stoul(last_hyphen, nullptr, 10);
                if(permutation_number < static_cast<std::size_t>(NULL_INDEX)) {
                    this->permutation_number = static_cast<HEK::Index>(permutation_number);
                }
            }
            catch(std::out_of_range &) {
                permutation_number = ~0;
            }
            catch(std::exception &) {
                return;
            }
            if(permutation_number >= NULL_INDEX) {
                REPORT_ERROR_PRINTF(workload, ERROR_TYPE_WARNING, tag_index, "Permutation %s has an index that is out of range (%lu >= %zu)", this->name.string, permutation_number, static_cast<std::size_t>(NULL_INDEX));
            }
        }
    }
}
