// SPDX-License-Identifier: GPL-3.0-only

#include <invader/tag/parser/parser.hpp>
#include <invader/build/build_workload.hpp>

namespace Invader::Parser {
    void ModelCollisionGeometry::pre_compile(BuildWorkload &, std::size_t , std::size_t, std::size_t) {
        if(this->recharge_time == 0.0) {
            this->shield_recharge_rate = 1.0F;
        }
        else {
            this->shield_recharge_rate = TICK_RATE_RECIPROCOL / this->recharge_time;
        }
    }
    void ModelCollisionGeometry::post_compile(BuildWorkload &workload, std::size_t tag_index, std::size_t struct_index, std::size_t offset) {
        auto &s = workload.structs[struct_index];
        auto &mcg = *reinterpret_cast<struct_little *>(s.data.data() + offset);
        std::size_t node_count = mcg.nodes.count;
        if(node_count != 0) {
            auto &resolved = workload.structs[*s.resolve_pointer(&mcg.nodes.pointer)];
            auto *nodes = reinterpret_cast<ModelCollisionGeometryNode::struct_little *>(resolved.data.data());
            
            for(std::size_t n = 0; n < node_count; n++) {
                auto *node_to_check = nodes + n;
                
                std::size_t iterations = 0;
                std::uint16_t thing = NULL_INDEX;
                
                // Find... whatever this is
                while(node_to_check && thing == NULL_INDEX) {
                    auto contain_set = [&node_to_check, &thing](const char *str, std::uint16_t set_value) {
                        std::size_t ilength = std::strlen(str);
                        std::size_t olength = std::strlen(node_to_check->name.string);
                        
                        if(olength >= ilength) {
                            // Check if substring
                            for(std::size_t i = 0; i <= (olength - ilength); i++) {
                                if(std::strncmp(str, node_to_check->name.string + i, ilength) == 0) {
                                    thing = set_value;
                                    break;
                                }
                            }
                        }
                    };
                    
                    contain_set("tail", 10);
                    contain_set("r foot", 10);
                    contain_set("r calf", 10);
                    contain_set("r horselink", 10);
                    contain_set("r thigh", 10);
                    contain_set("r hand", 8);
                    contain_set("r forearm", 8);
                    contain_set("r upperarm", 7);
                    contain_set("r clavicle", 7);
                    contain_set("l foot", 6);
                    contain_set("l calf", 6);
                    contain_set("l horselink", 6);
                    contain_set("l thigh", 6);
                    contain_set("l upperarm", 3);
                    contain_set("l clavicle", 3);
                    contain_set("spine", 0);
                    contain_set("spine1", 1);
                    contain_set("pelvis", 0);
                    contain_set("neck", 2);
                    contain_set("l hand", 4);
                    contain_set("l forearm", 4);
                    contain_set("head", 2);
                    
                    if(thing == NULL_INDEX) {
                        if(iterations++ > NULL_INDEX) {
                            REPORT_ERROR_PRINTF(workload, ERROR_TYPE_FATAL_ERROR, tag_index, "Infinite loop detected with node#%zu's parents", n);
                            throw InvalidTagDataException();
                        }
                        node_to_check = (node_to_check->parent_node == NULL_INDEX) ? nullptr : nodes + node_to_check->parent_node;
                    }
                }
                nodes[n].name_thing = thing;
            }
        }
    }
    void ModelCollisionGeometryBSP::pre_compile(BuildWorkload &workload, std::size_t tag_index, std::size_t, std::size_t) {
        std::size_t bsp3d_count = this->bsp3d_nodes.size();
        std::size_t bsp2d_count = this->bsp2d_nodes.size();
        std::size_t leaf_count = this->leaves.size();
        std::size_t plane_count = this->planes.size();
        std::size_t surface_count = this->surfaces.size();
        std::size_t edge_count = this->edges.size();
        std::size_t vertex_count = this->vertices.size();

        // Make sure each BSP3D node is valid
        for(std::size_t bsp3d_node_index = 0; bsp3d_node_index < bsp3d_count; bsp3d_node_index++) {
            auto &bsp3d = this->bsp3d_nodes[bsp3d_node_index];

            // Make sure the front and back children are valid
            auto validate_child = [&workload, &tag_index, &bsp3d_count, &leaf_count, &bsp3d_node_index](auto &child) {
                if(!child.is_null()) {
                    std::size_t int_value = child.int_value();
                    if(child.flag_value()) {
                        if(int_value >= leaf_count) {
                            REPORT_ERROR_PRINTF(workload, ERROR_TYPE_FATAL_ERROR, tag_index, "BSP3D node #%zu has an invalid leaf index (%zu >= %zu)", bsp3d_node_index, int_value, leaf_count);
                            throw InvalidTagDataException();
                        }
                    }
                    else {
                        if(int_value >= bsp3d_count) {
                            REPORT_ERROR_PRINTF(workload, ERROR_TYPE_FATAL_ERROR, tag_index, "BSP3D node #%zu has an invalid node index (%zu >= %zu)", bsp3d_node_index, int_value, bsp3d_count);
                            throw InvalidTagDataException();
                        }
                    }
                }
            };
            validate_child(bsp3d.front_child);
            validate_child(bsp3d.back_child);

            // Make sure the plane is valid too
            if(bsp3d.plane >= plane_count) {
                REPORT_ERROR_PRINTF(workload, ERROR_TYPE_FATAL_ERROR, tag_index, "BSP3D node #%zu has an invalid plane index (%zu >= %zu)", bsp3d_node_index, static_cast<std::size_t>(bsp3d.plane), plane_count);
                throw InvalidTagDataException();
            }
        }

        // Make sure the leaves are valid
        std::size_t bsp2d_reference_count = this->bsp2d_references.size();
        for(auto &leaf : this->leaves) {
            std::size_t leaf_first_bsp2d_reference = leaf.first_bsp2d_reference;
            std::size_t leaf_bsp2d_reference_count = leaf.bsp2d_reference_count;
            std::size_t leaf_end_bsp2d_reference = leaf_bsp2d_reference_count + leaf_first_bsp2d_reference;

            if(leaf_bsp2d_reference_count && (leaf_first_bsp2d_reference >= bsp2d_reference_count || leaf_bsp2d_reference_count > bsp2d_reference_count || leaf_end_bsp2d_reference > bsp2d_reference_count)) {
                REPORT_ERROR_PRINTF(workload, ERROR_TYPE_FATAL_ERROR, tag_index, "BSP leaf #%zu has an invalid BSP2D reference range (%zu - %zu / %zu)", &leaf - this->leaves.data(), leaf_first_bsp2d_reference, leaf_end_bsp2d_reference, bsp2d_reference_count);
                throw InvalidTagDataException();
            }
        }

        // Make sure the BSP2D references are valid
        for(auto &ref : this->bsp2d_references) {
            if(ref.plane >= plane_count) {
                REPORT_ERROR_PRINTF(workload, ERROR_TYPE_FATAL_ERROR, tag_index, "BSP2D reference #%zu has an invalid plane index (%zu >= %zu)", &ref - this->bsp2d_references.data(), static_cast<std::size_t>(ref.plane), plane_count);
                throw InvalidTagDataException();
            }
        }

        // Make sure the BSP2D nodes are valid
        for(std::size_t bsp2d_node_index = 0; bsp2d_node_index < bsp2d_count; bsp2d_node_index++) {
            auto &bsp2d = this->bsp2d_nodes[bsp2d_node_index];

            // Make sure the left and right children are valid
            auto validate_child = [&workload, &tag_index, &bsp2d_count, &surface_count, &bsp2d_node_index](auto &child) {
                if(!child.is_null()) {
                    std::size_t int_value = child.int_value();
                    if(child.flag_value()) {
                        if(int_value >= surface_count) {
                            REPORT_ERROR_PRINTF(workload, ERROR_TYPE_FATAL_ERROR, tag_index, "BSP2D node #%zu has an invalid surface index (%zu >= %zu)", bsp2d_node_index, int_value, surface_count);
                            throw InvalidTagDataException();
                        }
                    }
                    else {
                        if(int_value >= bsp2d_count) {
                            REPORT_ERROR_PRINTF(workload, ERROR_TYPE_FATAL_ERROR, tag_index, "BSP2D node #%zu has an invalid node index (%zu >= %zu)", bsp2d_node_index, int_value, bsp2d_count);
                            throw InvalidTagDataException();
                        }
                    }
                }
            };
            validate_child(bsp2d.left_child);
            validate_child(bsp2d.right_child);
        }

        // Make sure the surfaces are valid
        for(auto &surface : this->surfaces) {
            if(surface.first_edge >= edge_count) {
                REPORT_ERROR_PRINTF(workload, ERROR_TYPE_FATAL_ERROR, tag_index, "Surface #%zu has an invalid edge index (%zu >= %zu)", &surface - this->surfaces.data(), static_cast<std::size_t>(surface.first_edge), edge_count);
                throw InvalidTagDataException();
            }
            if(surface.plane >= plane_count) {
                REPORT_ERROR_PRINTF(workload, ERROR_TYPE_FATAL_ERROR, tag_index, "Surface #%zu has an invalid plane index (%zu >= %zu)", &surface - this->surfaces.data(), static_cast<std::size_t>(surface.plane), plane_count);
                throw InvalidTagDataException();
            }
        }

        // Make sure the edges are valid
        for(auto &edge : this->edges) {
            if(edge.start_vertex >= vertex_count || edge.end_vertex >= vertex_count) {
                REPORT_ERROR_PRINTF(workload, ERROR_TYPE_FATAL_ERROR, tag_index, "Edge #%zu has an invalid vertex index (%zu, %zu >= %zu)", &edge - this->edges.data(), static_cast<std::size_t>(edge.start_vertex), static_cast<std::size_t>(edge.end_vertex), vertex_count);
                throw InvalidTagDataException();
            }
            if(edge.left_surface >= surface_count || edge.right_surface >= surface_count) {
                REPORT_ERROR_PRINTF(workload, ERROR_TYPE_FATAL_ERROR, tag_index, "Edge #%zu has an invalid surface index (%zu, %zu >= %zu)", &edge - this->edges.data(), static_cast<std::size_t>(edge.left_surface), static_cast<std::size_t>(edge.right_surface), surface_count);
                throw InvalidTagDataException();
            }
        }
    }
}
