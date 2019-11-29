// SPDX-License-Identifier: GPL-3.0-only

#include <invader/tag/parser/parser.hpp>
#include <invader/build2/build_workload.hpp>

namespace Invader::Parser {
    void ModelCollisionGeometry::pre_compile(BuildWorkload2 &, std::size_t , std::size_t, std::size_t) {
        this->shield_recharge_rate = 1.0F / this->recharge_time / TICK_RATE;
    }
    void ModelCollisionGeometryBSP::pre_compile(BuildWorkload2 &workload, std::size_t tag_index, std::size_t, std::size_t) {
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
                        if(int_value > leaf_count) {
                            REPORT_ERROR_PRINTF(workload, ERROR_TYPE_FATAL_ERROR, tag_index, "BSP3D node #%zu has an invalid leaf index (%zu / %zu)", bsp3d_node_index, int_value, leaf_count);
                            throw InvalidTagDataException();
                        }
                    }
                    else {
                        if(int_value > bsp3d_count) {
                            REPORT_ERROR_PRINTF(workload, ERROR_TYPE_FATAL_ERROR, tag_index, "BSP3D node #%zu has an invalid node index (%zu / %zu)", bsp3d_node_index, int_value, bsp3d_count);
                            throw InvalidTagDataException();
                        }
                    }
                }
            };
            validate_child(bsp3d.front_child);
            validate_child(bsp3d.back_child);

            // Make sure the plane is valid too
            if(bsp3d.plane > plane_count) {
                REPORT_ERROR_PRINTF(workload, ERROR_TYPE_FATAL_ERROR, tag_index, "BSP3D node #%zu has an invalid plane index (%zu / %zu)", bsp3d_node_index, static_cast<std::size_t>(bsp3d.plane), plane_count);
                throw InvalidTagDataException();
            }
        }

        // Make sure the leaves are valid
        std::size_t bsp2d_reference_count = this->bsp2d_references.size();
        for(auto &leaf : this->leaves) {
            std::size_t leaf_first_bsp2d_reference = leaf.first_bsp2d_reference;
            std::size_t leaf_bsp2d_reference_count = leaf.bsp2d_reference_count;
            std::size_t leaf_end_bsp2d_reference = leaf_bsp2d_reference_count + leaf_first_bsp2d_reference;

            if(bsp2d_reference_count && (leaf_first_bsp2d_reference >= bsp2d_reference_count || leaf_bsp2d_reference_count > bsp2d_reference_count || leaf_end_bsp2d_reference > bsp2d_reference_count)) {
                REPORT_ERROR_PRINTF(workload, ERROR_TYPE_FATAL_ERROR, tag_index, "BSP leaf #%zu has an invalid BSP2D reference range (%zu - %zu / %zu)", &leaf - this->leaves.data(), leaf_first_bsp2d_reference, leaf_end_bsp2d_reference, bsp2d_reference_count);
                throw InvalidTagDataException();
            }
        }

        // Make sure the BSP2D references are valid
        for(auto &ref : this->bsp2d_references) {
            if(ref.plane > plane_count) {
                REPORT_ERROR_PRINTF(workload, ERROR_TYPE_FATAL_ERROR, tag_index, "BSP2D reference #%zu has an invalid plane index (%zu / %zu)", &ref - this->bsp2d_references.data(), static_cast<std::size_t>(ref.plane), plane_count);
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
                        if(int_value > surface_count) {
                            REPORT_ERROR_PRINTF(workload, ERROR_TYPE_FATAL_ERROR, tag_index, "BSP2D node #%zu has an invalid surface index (%zu / %zu)", bsp2d_node_index, int_value, surface_count);
                            throw InvalidTagDataException();
                        }
                    }
                    else {
                        if(int_value > bsp2d_count) {
                            REPORT_ERROR_PRINTF(workload, ERROR_TYPE_FATAL_ERROR, tag_index, "BSP2D node #%zu has an invalid node index (%zu / %zu)", bsp2d_node_index, int_value, bsp2d_count);
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
            if(surface.first_edge > edge_count) {
                REPORT_ERROR_PRINTF(workload, ERROR_TYPE_FATAL_ERROR, tag_index, "Surface #%zu has an invalid edge index (%zu / %zu)", &surface - this->surfaces.data(), static_cast<std::size_t>(surface.first_edge), edge_count);
                throw InvalidTagDataException();
            }
            if(surface.plane > plane_count) {
                REPORT_ERROR_PRINTF(workload, ERROR_TYPE_FATAL_ERROR, tag_index, "Surface #%zu has an invalid plane index (%zu / %zu)", &surface - this->surfaces.data(), static_cast<std::size_t>(surface.plane), plane_count);
                throw InvalidTagDataException();
            }
        }

        // Make sure the edges are valid
        for(auto &edge : this->edges) {
            if(edge.start_vertex > vertex_count || edge.end_vertex > vertex_count) {
                REPORT_ERROR_PRINTF(workload, ERROR_TYPE_FATAL_ERROR, tag_index, "Edge #%zu has an invalid vertex index (%zu, %zu / %zu)", &edge - this->edges.data(), static_cast<std::size_t>(edge.start_vertex), static_cast<std::size_t>(edge.end_vertex), vertex_count);
                throw InvalidTagDataException();
            }
            if(edge.left_surface > surface_count || edge.right_surface > surface_count) {
                REPORT_ERROR_PRINTF(workload, ERROR_TYPE_FATAL_ERROR, tag_index, "Edge #%zu has an invalid surface index (%zu, %zu / %zu)", &edge - this->edges.data(), static_cast<std::size_t>(edge.left_surface), static_cast<std::size_t>(edge.right_surface), surface_count);
                throw InvalidTagDataException();
            }
        }
    }
}
