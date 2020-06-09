// SPDX-License-Identifier: GPL-3.0-only

#include <invader/build/build_workload.hpp>
#include <invader/tag/parser/parser.hpp>
#include <invader/tag/parser/compile/gbxmodel.hpp>

namespace Invader::Parser {
    void GBXModel::postprocess_hek_data() {
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
                        if(p.flags & HEK::GBXModelGeometryPartFlagsFlag::GBX_MODEL_GEOMETRY_PART_FLAGS_FLAG_ZONER) {
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

    void GBXModel::post_compile(BuildWorkload &workload, std::size_t, std::size_t struct_index, std::size_t offset) {
        // Put all of the markers in the marker array
        auto &markers = this->markers;
        auto region_count = regions.size();
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

        // Generate it
        auto marker_count = this->markers.size();
        auto &gbxmodel_data = *reinterpret_cast<struct_little *>(workload.structs[struct_index].data.data() + offset);
        gbxmodel_data.markers.count = static_cast<std::uint32_t>(marker_count);

        if(marker_count > 0) {
            // Make the pointer
            auto &marker_ptr = workload.structs[struct_index].pointers.emplace_back();
            marker_ptr.offset = static_cast<std::uint32_t>(reinterpret_cast<std::byte *>(&gbxmodel_data.markers.pointer) - reinterpret_cast<std::byte *>(&gbxmodel_data));
            std::size_t marker_struct_index = workload.structs.size();
            marker_ptr.struct_index = marker_struct_index;

            // Make the struct
            auto &markers_struct = workload.structs.emplace_back();
            GBXModelMarker::struct_little *markers_struct_arr;
            markers_struct.data = std::vector<std::byte>(marker_count * sizeof(*markers_struct_arr));
            markers_struct_arr = reinterpret_cast<decltype(markers_struct_arr)>(markers_struct.data.data());

            // Go through each marker
            for(std::size_t m = 0; m < marker_count; m++) {
                auto &marker_c = this->markers[m];
                auto &marker_l = markers_struct_arr[m];

                // Copy this stuff
                marker_l.name = marker_c.name;
                marker_l.magic_identifier = marker_c.magic_identifier;

                // Generate instances
                auto instance_count = static_cast<std::uint32_t>(marker_c.instances.size());
                marker_l.instances.count = instance_count;

                // Make the pointer
                auto &marker_ptr = workload.structs[marker_struct_index].pointers.emplace_back();
                marker_ptr.offset = static_cast<std::uint32_t>(reinterpret_cast<std::byte *>(&marker_l.instances.pointer) - reinterpret_cast<std::byte *>(markers_struct_arr));
                marker_ptr.struct_index = workload.structs.size();

                // Make the instances
                auto &instance_struct = workload.structs.emplace_back();
                GBXModelMarkerInstance::struct_little *instances_struct_arr;
                instance_struct.data = std::vector<std::byte>(sizeof(*instances_struct_arr) * instance_count);
                instances_struct_arr = reinterpret_cast<decltype(instances_struct_arr)>(instance_struct.data.data());
                for(std::size_t i = 0; i < instance_count; i++) {
                    instances_struct_arr[i].node_index = marker_c.instances[i].node_index;
                    instances_struct_arr[i].permutation_index = marker_c.instances[i].permutation_index;
                    instances_struct_arr[i].region_index = marker_c.instances[i].region_index;
                    instances_struct_arr[i].rotation = marker_c.instances[i].rotation;
                    instances_struct_arr[i].translation = marker_c.instances[i].translation;
                }
            }
        }
    }

    bool regenerate_missing_model_vertices(GBXModelGeometryPart &part, bool fix) {
        if(part.uncompressed_vertices.size() == 0) {
            if(!fix) {
                return true;
            }
            part.uncompressed_vertices.reserve(part.compressed_vertices.size());
            for(auto &v : part.compressed_vertices) {
                auto before_data = v.generate_hek_tag_data();
                auto after_data = HEK::decompress_model_vertex(*reinterpret_cast<GBXModelVertexCompressed::struct_big *>(before_data.data()));
                auto &after_data_write = part.uncompressed_vertices.emplace_back();
                after_data_write.binormal = after_data.binormal;
                after_data_write.normal = after_data.normal;
                after_data_write.position = after_data.position;
                after_data_write.tangent = after_data.tangent;
                after_data_write.node0_index = after_data.node0_index;
                after_data_write.node0_weight = after_data.node0_weight;
                after_data_write.node1_index = after_data.node1_index;
                after_data_write.node1_weight = after_data.node1_weight;
                after_data_write.texture_coords = after_data.texture_coords;
            }
        }
        else if(part.compressed_vertices.size() == 0) {
            if(!fix) {
                return true;
            }
            part.compressed_vertices.reserve(part.uncompressed_vertices.size());
            for(auto &v : part.uncompressed_vertices) {
                auto before_data = v.generate_hek_tag_data();
                auto after_data = HEK::compress_model_vertex(*reinterpret_cast<GBXModelVertexUncompressed::struct_big *>(before_data.data()));
                auto &after_data_write = part.compressed_vertices.emplace_back();
                after_data_write.binormal = after_data.binormal;
                after_data_write.normal = after_data.normal;
                after_data_write.position = after_data.position;
                after_data_write.tangent = after_data.tangent;
                after_data_write.node0_index = after_data.node0_index;
                after_data_write.node0_weight = after_data.node0_weight;
                after_data_write.node1_index = after_data.node1_index;
                after_data_write.texture_coordinate_u = after_data.texture_coordinate_u;
                after_data_write.texture_coordinate_v = after_data.texture_coordinate_v;
            }
        }
        else if(part.compressed_vertices.size() != part.uncompressed_vertices.size()) {
            eprintf_error("Can't fix: Compressed vertex and uncompressed vertex counts don't match");
            throw InvalidTagDataException();
        }
        else {
            return false;
        }
        return true;
    }

    bool regenerate_missing_model_vertices(GBXModel &model, bool fix) {
        bool result = false;
        for(auto &g : model.geometries) {
            for(auto &p : g.parts) {
                result = regenerate_missing_model_vertices(p, fix) || result;
            }
        }
        return result;
    }

    void GBXModelGeometryPart::post_cache_parse(const Invader::Tag &tag, std::optional<HEK::Pointer> pointer) {
        const auto &part = tag.get_struct_at_pointer<HEK::GBXModelGeometryPart>(*pointer);
        const auto &map = tag.get_map();

        // Get model vertices
        std::size_t vertex_count = part.vertex_count.read();
        auto model_data_offset = map.get_model_data_offset();
        auto model_index_offset = map.get_model_index_offset() + model_data_offset;

        // Repopulate vertices - PC only has uncompressed vertices
        if(vertex_count > 0 && tag.get_map().get_engine() != HEK::CacheFileEngine::CACHE_FILE_XBOX) {
            const auto *vertices = reinterpret_cast<const GBXModelVertexUncompressed::struct_little *>(map.get_data_at_offset(model_data_offset + part.vertex_offset.read(), sizeof(GBXModelVertexUncompressed::struct_little) * vertex_count));
            for(std::size_t v = 0; v < vertex_count; v++) {
                std::size_t data_read;
                GBXModelVertexUncompressed::struct_big vertex_uncompressed = vertices[v];
                this->uncompressed_vertices.emplace_back(GBXModelVertexUncompressed::parse_hek_tag_data(reinterpret_cast<const std::byte *>(&vertex_uncompressed), sizeof(vertex_uncompressed), data_read, true));
            }
            if(!regenerate_missing_model_vertices(*this, true)) {
                eprintf_error("Failed to regenerate compressed vertices");
                throw InvalidTagDataException();
            }
        }

        // Get model indices
        std::size_t index_count = part.triangle_count.read() + 2;

        std::size_t triangle_count = (index_count) / 3;
        std::size_t triangle_modulo = index_count % 3;
        const auto *indices = reinterpret_cast<const HEK::LittleEndian<HEK::Index> *>(map.get_data_at_offset(model_index_offset + part.triangle_offset.read(), sizeof(std::uint16_t) * index_count));

        for(std::size_t t = 0; t < triangle_count; t++) {
            auto &triangle = this->triangles.emplace_back();
            auto *triangle_indices = indices + t * 3;
            triangle.vertex0_index = triangle_indices[0];
            triangle.vertex1_index = triangle_indices[1];
            triangle.vertex2_index = triangle_indices[2];
        }

        if(triangle_modulo) {
            auto &straggler_triangle = this->triangles.emplace_back();
            auto *triangle_indices = indices + triangle_count * 3;
            straggler_triangle.vertex0_index = triangle_indices[0];
            straggler_triangle.vertex1_index = triangle_modulo > 1 ? triangle_indices[1].read() : NULL_INDEX;
            straggler_triangle.vertex2_index = NULL_INDEX;
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

                    if(p.flags & HEK::GBXModelGeometryPartFlagsFlag::GBX_MODEL_GEOMETRY_PART_FLAGS_FLAG_ZONER) {
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

        // Make sure the node indices in the markers we added are valid
        std::size_t errors_given = 0;
        static constexpr std::size_t MAX_ERRORS = 5;
        for(auto &m : this->markers) {
            for(auto &i : m.instances) {
                if(i.node_index >= node_count) {
                    if(++errors_given != MAX_ERRORS) {
                        REPORT_ERROR_PRINTF(workload, ERROR_TYPE_ERROR, tag_index, "Instance #%zu of marker #%zu has an invalid node index (%zu >= %zu)", &m - this->markers.data(), &i - m.instances.data(), static_cast<std::size_t>(i.node_index), node_count);
                        errors_given++;
                    }
                    else {
                        eprintf_error("... and more errors. Suffice it to say, the model needs recompiled");
                        break;
                    }
                }
            }
            if(errors_given == MAX_ERRORS) {
                break;
            }
        }
        if(errors_given > 0 && errors_given < MAX_ERRORS) {
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
                std::uint32_t zoner = p.flags & HEK::GBXModelGeometryPartFlagsFlag::GBX_MODEL_GEOMETRY_PART_FLAGS_FLAG_ZONER;
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
            triangle_indices.emplace_back(t.vertex0_index);
            triangle_indices.emplace_back(t.vertex1_index);
            triangle_indices.emplace_back(t.vertex2_index);
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
        
        // Not sure what this does, but it keeps the needler from being broken
        this->prev_filthy_part_index = this->prev_filthy_part_index == 0 ? -1 : this->prev_filthy_part_index;
        this->next_filthy_part_index = this->next_filthy_part_index == 0 ? -1 : this->next_filthy_part_index;
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

    void Invader::Parser::GBXModel::post_cache_deformat() {
        this->flags &= ~HEK::GBXModelFlagsFlag::GBX_MODEL_FLAGS_FLAG_BLEND_SHARED_NORMALS; // prevent generational loss

        uncache_model_markers(*this, true);

        float super_low = this->super_low_detail_cutoff;
        float low = this->low_detail_cutoff;
        float high = this->high_detail_cutoff;
        float super_high = this->super_high_detail_cutoff;

        this->super_low_detail_cutoff = super_high;
        this->low_detail_cutoff = high;
        this->high_detail_cutoff = low;
        this->super_high_detail_cutoff = super_low;

        this->postprocess_hek_data();
    }

    bool uncache_model_markers(GBXModel &model, bool fix) {
        if(model.markers.size() == 0) {
            return false;
        }
        else if(!fix) {
            return true;
        }

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
        return true;
    }
}
