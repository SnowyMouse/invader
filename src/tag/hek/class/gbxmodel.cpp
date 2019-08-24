/*
 * Invader (c) 2018 Kavawuvi
 *
 * This program is free software under the GNU General Public License v3.0 or later. See LICENSE for more information.
 */

#include "../compile.hpp"

#include "gbxmodel.hpp"

namespace Invader::HEK {
    struct TempMarker {
        TagString name;
        std::vector<GBXModelMarkerInstance<LittleEndian>> instances;
    };

    void compile_gbxmodel_tag(CompiledTag &compiled, const std::byte *data, std::size_t size) {
        BEGIN_COMPILE(GBXModel)

        // Set to these by default
        DEFAULT_VALUE(tag.base_map_u_scale, 1.0f);
        DEFAULT_VALUE(tag.base_map_v_scale, 1.0f);

        // Apparently the cutoffs are stored in reverse order.
        float super_low = tag.super_low_detail_cutoff;
        float low = tag.low_detail_cutoff;
        float high = tag.high_detail_cutoff;
        float super_high = tag.super_high_detail_cutoff;

        tag.super_low_detail_cutoff = super_high;
        tag.low_detail_cutoff = high;
        tag.high_detail_cutoff = low;
        tag.super_high_detail_cutoff = super_low;

        // First, get existing markers
        std::vector<TempMarker> markers;
        std::size_t marker_count = tag.markers.count;
        const auto *markers_big = reinterpret_cast<const GBXModelMarker<BigEndian> *>(data);
        std::size_t marker_size = marker_count * sizeof(*markers_big);
        ASSERT_SIZE(marker_count * sizeof(*markers_big));
        INCREMENT_DATA_PTR(marker_size);
        for(std::size_t m = 0; m < marker_count; m++) {
            const auto &marker = markers_big[m];
            TempMarker marker_copy;
            marker_copy.name = marker.name;
            std::size_t instance_count = marker.instances.count;
            const auto *instances_big = reinterpret_cast<const GBXModelMarkerInstance<BigEndian> *>(data);
            std::size_t instance_size = instance_count * sizeof(*instances_big);
            INCREMENT_DATA_PTR(instance_size);
            marker_copy.instances.insert(marker_copy.instances.end(), instances_big, instances_big + instance_count);
            markers.emplace_back(std::move(marker_copy));
        }

        // Get nodes so we can add them after adding markers. Makers should appear first in this struct, so we want to be accurate here.
        std::size_t nodes_count = tag.nodes.count;
        auto *nodes_big = reinterpret_cast<const GBXModelNode<BigEndian> *>(data);
        std::size_t nodes_size = nodes_count * sizeof(*nodes_big);
        ASSERT_SIZE(nodes_size);
        INCREMENT_DATA_PTR(nodes_size);

        // Get markers before we add regions and region permutations.
        std::size_t region_count = tag.regions.count;
        const auto *regions_reflexive = reinterpret_cast<const GBXModelRegion<BigEndian> *>(data);
        std::size_t total_offset = sizeof(*regions_reflexive) * region_count;
        ASSERT_SIZE(total_offset);
        for(std::size_t r = 0; r < region_count; r++) {
            auto &region = regions_reflexive[r];
            std::size_t permutation_count = region.permutations.count;
            const auto *permutations_reflexive = reinterpret_cast<const GBXModelRegionPermutation<BigEndian> *>(data + total_offset);
            total_offset += sizeof(*permutations_reflexive) * permutation_count;
            ASSERT_SIZE(total_offset);
            for(std::size_t p = 0; p < permutation_count; p++) {
                auto &permutation = permutations_reflexive[p];
                std::size_t markers_count = permutation.markers.count;
                const auto *markers_reflexive = reinterpret_cast<const GBXModelRegionPermutationMarker<BigEndian> *>(data + total_offset);
                total_offset += sizeof(*markers_reflexive) * markers_count;
                ASSERT_SIZE(total_offset);
                for(std::size_t m = 0; m < markers_count; m++) {
                    bool found = false;
                    auto &reflexive = markers_reflexive[m];
                    GBXModelMarkerInstance<LittleEndian> instance = {};
                    instance.region_index = static_cast<std::int8_t>(r);
                    instance.permutation_index = static_cast<std::int8_t>(p);
                    instance.node_index = static_cast<std::int8_t>(reflexive.node_index);
                    instance.rotation = reflexive.rotation;
                    instance.translation = reflexive.translation;

                    // Add this marker instance to an existing marker
                    for(auto &marker : markers) {
                        if(std::strcmp(marker.name.string, reflexive.name.string) == 0) {
                            marker.instances.emplace_back(instance);
                            found = true;
                            break;
                        }
                    }

                    // Add this marker as a new marker
                    if(!found) {
                        TempMarker marker;
                        marker.name = reflexive.name;
                        marker.instances.emplace_back(instance);
                        markers.emplace_back(marker);
                    }
                }
            }
        }

        // Add the markers we just got
        auto markers_count = markers.size();
        tag.markers.count = static_cast<std::uint32_t>(markers_count);
        if(markers_count > 0) {
            auto markers_offset = compiled.data.size();
            ADD_POINTER_FROM_INT32(tag.markers.pointer, markers_offset);
            compiled.data.insert(compiled.data.end(), markers_count * sizeof(GBXModelMarker<LittleEndian>), std::byte());

            // Add all the markers in alphabetical order
            std::size_t marker = 0;
            while(markers_count != 0) {
                std::size_t first_marker = 0;
                const char *first_marker_name = markers[0].name.string;
                for(std::size_t i = 1; i < markers_count; i++) {
                    if(std::strcmp(first_marker_name, markers[i].name.string) > 0) {
                        first_marker = i;
                        first_marker_name = markers[i].name.string;
                    }
                }

                auto &marker_struct = reinterpret_cast<GBXModelMarker<LittleEndian> *>(compiled.data.data() + markers_offset)[marker];
                auto &temp_marker = markers[first_marker];
                marker_struct.name = temp_marker.name;

                auto instance_count = temp_marker.instances.size();
                auto instance_offset = compiled.data.size();
                marker_struct.instances.count = static_cast<std::int32_t>(instance_count);

                add_pointer(compiled, reinterpret_cast<std::uintptr_t>(marker_struct.instances.pointer.value) - reinterpret_cast<std::uintptr_t>(compiled.data.data()), instance_offset);
                compiled.data.insert(compiled.data.end(), instance_count * sizeof(GBXModelMarkerInstance<LittleEndian>), std::byte());
                for(std::size_t i = 0; i < instance_count; i++) {
                    reinterpret_cast<GBXModelMarkerInstance<LittleEndian> *>(compiled.data.data() + instance_offset)[i] = temp_marker.instances[i];
                }

                markers.erase(markers.begin() + first_marker);
                markers_count--;
                marker++;
            }
        }

        // Add the nodes we found before
        std::size_t nodes_offset = compiled.data.size();
        ADD_POINTER_FROM_INT32(tag.nodes.pointer, nodes_offset);
        compiled.data.insert(compiled.data.end(), nodes_size, std::byte());
        auto *nodes_little = reinterpret_cast<GBXModelNode<LittleEndian> *>(compiled.data.data() + nodes_offset);
        std::vector<bool> node_done(nodes_count, false);
        auto write_node_data = [&node_done, nodes_little, nodes_big, nodes_count](std::int16_t node_index, const auto &base_rotation, const auto &base_translation, const auto &recursion) {
            if(node_index == -1) {
                return;
            }
            if(node_index < 0 || static_cast<std::size_t>(node_index) >= nodes_count) {
                throw OutOfBoundsException();
            }
            if(node_done[node_index]) {
                return;
            }
            node_done[node_index] = true;

            auto &node = nodes_little[node_index];
            node = nodes_big[node_index];
            node.scale = 1.0f;

            auto node_rotation = quaternion_to_matrix(node.default_rotation);
            auto total_rotation = multiply_matrix(base_rotation, node_rotation);
            node.rotation = total_rotation;

            auto node_translation = multiply_vector(node.default_translation, -1.0);
            auto total_translation = rotate_vector(add_vector(node_translation, base_translation), node_rotation);
            node.translation = total_translation;

            recursion(node.next_sibling_node_index, base_rotation, base_translation, recursion);
            recursion(node.first_child_node_index, total_rotation, total_translation, recursion);
        };
        Matrix<LittleEndian> identity = {};
        for(int i = 0; i < 3; i++) {
            identity.matrix[i][i] = 1.0f;
        }
        Vector3D<LittleEndian> no_translation = {};
        write_node_data(0, identity, no_translation, write_node_data);

        // Make sure we don't have any stragglers
        for(std::size_t n = 0; n < nodes_count; n++) {
            if(!node_done[n]) {
                eprintf("orphaned model node %zu\n", n);
                throw OutOfBoundsException();
            }
        }

        // Now add regions and region permutations
        ADD_REFLEXIVE_START(tag.regions) {
            ADD_REFLEXIVE_START(reflexive.permutations) {
                std::size_t markers_count = reflexive.markers.count;
                INCREMENT_DATA_PTR(sizeof(const GBXModelRegionPermutationMarker<BigEndian>) * markers_count);
                reflexive.markers.count = 0;

                // Set this value
                reflexive.permutation_number = 0;
                const char *last_hyphen = nullptr;
                bool null_terminated = false;
                for(const char &c : reflexive.name.string) {
                    if(c == 0) {
                        null_terminated = true;
                        break;
                    }
                    else if(c == '-') {
                        last_hyphen = &c + 1;
                    }
                }

                // If null terminated, do this
                if(last_hyphen) {
                    if(null_terminated) {
                        reflexive.permutation_number = static_cast<std::uint16_t>(std::strtol(last_hyphen, nullptr, 10));
                    }
                    else {
                        eprintf("model permutation which contains a hyphen in its name is not null terminated\n");
                        throw OutOfBoundsException();
                    }
                }
            } ADD_REFLEXIVE_END
        } ADD_REFLEXIVE_END

        bool exodux_handler = false;
        bool exodux_parser = false;

        ADD_REFLEXIVE_START(tag.geometries) {
            ADD_REFLEXIVE_START(reflexive.parts) {
                auto vertex_count = static_cast<std::size_t>(reflexive.uncompressed_vertices.count);
                auto vertex_size = sizeof(GBXModelVertexUncompressed<BigEndian>) * vertex_count;
                ASSERT_SIZE(vertex_size);
                reflexive.uncompressed_vertices.count = 0;

                // Set vertex count and offsets
                reflexive.vertex_offset = static_cast<std::uint32_t>(compiled.asset_data.size());
                reflexive.vertex_count = static_cast<std::uint32_t>(vertex_count);

                // Allocate room for vertices
                auto *vertices_big = reinterpret_cast<const GBXModelVertexUncompressed<BigEndian> *>(data);
                compiled.asset_data.insert(compiled.asset_data.end(), vertex_size, std::byte());
                auto *vertices_little = reinterpret_cast<GBXModelVertexUncompressed<LittleEndian> *>(compiled.asset_data.data() + compiled.asset_data.size() - vertex_size);

                // Copy vertices
                std::copy(vertices_big, vertices_big + vertex_count, vertices_little);

                for(std::size_t v = 0; v < vertex_count; v++) {
                    vertices_little[v] = vertices_big[v];

                    if(vertices_little[v].node1_index == -1) {
                        vertices_little[v].node1_index = 0;
                    }
                }
                INCREMENT_DATA_PTR(vertex_size);

                // Skip compressed vertices
                auto compress_vertex_size = reflexive.compressed_vertices.count * sizeof(GBXModelVertexCompressed<BigEndian>);
                ASSERT_SIZE(compress_vertex_size);
                INCREMENT_DATA_PTR(compress_vertex_size);
                reflexive.compressed_vertices.count = 0;

                // Set triangle count and offsets
                auto index_little_offset = compiled.asset_data.size();
                auto *triangles_big = reinterpret_cast<const GBXModelTriangle<BigEndian> *>(data);
                reflexive.triangle_offset = static_cast<std::uint32_t>(index_little_offset);
                reflexive.triangle_offset_2 = static_cast<std::uint32_t>(index_little_offset);
                std::size_t triangle_count = reflexive.triangles.count;
                std::size_t additional_triangles_to_subtract = 0;
                if(triangle_count > 0) {
                    auto &last_triangle = triangles_big[triangle_count - 1];
                    if(last_triangle.vertex0_index == -1) {
                        additional_triangles_to_subtract += 3;
                    }
                    else if(last_triangle.vertex1_index == -1) {
                        additional_triangles_to_subtract += 2;
                    }
                    else if(last_triangle.vertex2_index == -1) {
                        additional_triangles_to_subtract += 1;
                    }
                }
                reflexive.triangle_count = static_cast<std::uint32_t>(triangle_count * 3 - 2 - additional_triangles_to_subtract);

                // Allocate room for triangles
                auto index_size = sizeof(GBXModelTriangle<LittleEndian>) * triangle_count;
                ASSERT_SIZE(index_size);
                compiled.asset_data.insert(compiled.asset_data.end(), index_size, std::byte());
                auto *triangles_little = reinterpret_cast<GBXModelTriangle<LittleEndian> *>(compiled.asset_data.data() + index_little_offset);

                // Copy triangles
                std::copy(triangles_big, triangles_big + triangle_count, triangles_little);

                // Delete excess indices
                for(std::size_t e = 0; e < additional_triangles_to_subtract * 2; e++) {
                    compiled.asset_data.erase(compiled.asset_data.end() - 1);
                }

                // This is apparently required so the game doesn't crash when the map is loaded, even if the model isn't referenced by anything?
                reflexive.do_not_crash_the_game = 1;

                // And this needs to be set to 4 so the model doesn't get screwed up?
                reflexive.do_not_screw_up_the_model = 4;

                // These are always -1?
                reflexive.next_filthy_part_index = -1;
                reflexive.prev_filthy_part_index = -1;

                INCREMENT_DATA_PTR(index_size);

                // exodux compatibility bit
                if(exodux_handler) {
                    reflexive.bullshit = 0x43687521;
                }
                else {
                    reflexive.bullshit = exodux_parser ? 0x52616921 : 0x56617021;
                    exodux_parser = !exodux_parser;
                }
                exodux_handler = !exodux_handler;

                reflexive.triangles.count = 0;

            } ADD_REFLEXIVE_END
        } ADD_REFLEXIVE_END

        ADD_BASIC_DEPENDENCY_REFLEXIVE(tag.shaders, shader);

        FINISH_COMPILE
    }
}
