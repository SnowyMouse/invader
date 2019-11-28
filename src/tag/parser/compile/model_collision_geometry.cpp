// SPDX-License-Identifier: GPL-3.0-only

#include <invader/map/map.hpp>
#include <invader/map/tag.hpp>
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

        // Make sure each BSP3D node is valid
        for(auto &bsp3d : this->bsp3d_nodes) {
            std::size_t bsp3d_node_index = &bsp3d - this->bsp3d_nodes.data();

            // Make sure the front and back children are valid
            auto validate_child = [&workload, &tag_index, &bsp3d_count, &leaf_count, &bsp3d_node_index](auto &child) {
                if(!child.is_null()) {
                    auto int_value = child.int_value();
                    if(child.flag_value()) {
                        if(int_value > leaf_count) {
                            REPORT_ERROR_PRINTF(workload, ERROR_TYPE_FATAL_ERROR, tag_index, "BSP node #%zu has an invalid leaf index", bsp3d_node_index);
                            throw InvalidTagDataException();
                        }
                    }
                    else {
                        if(int_value > bsp3d_count) {
                            REPORT_ERROR_PRINTF(workload, ERROR_TYPE_FATAL_ERROR, tag_index, "BSP node #%zu has an invalid node index", bsp3d_node_index);
                            throw InvalidTagDataException();
                        }
                    }
                }
            };
            validate_child(bsp3d.front_child);
            validate_child(bsp3d.back_child);

            // Make sure the plane is valid too
            if(bsp3d.plane > plane_count) {
                REPORT_ERROR_PRINTF(workload, ERROR_TYPE_FATAL_ERROR, tag_index, "BSP node #%zu has an invalid plane index", bsp3d_node_index);
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
                REPORT_ERROR_PRINTF(workload, ERROR_TYPE_FATAL_ERROR, tag_index, "BSP leaf #%zu has an invalid BSP2D reference range (%zu - %zu)", &leaf - this->leaves.data(), leaf_first_bsp2d_reference, leaf_end_bsp2d_reference);
                throw InvalidTagDataException();
            }
        }

        // Make sure the BSP2D references are valid
        for(auto &ref : this->bsp2d_references) {
            if(ref.plane > plane_count) {
                REPORT_ERROR_PRINTF(workload, ERROR_TYPE_FATAL_ERROR, tag_index, "BSP2D reference #%zu has an invalid plane index", &ref - this->bsp2d_references.data());
                throw InvalidTagDataException();
            }
        }

        // Make sure the BSP2D nodes are valid
        for(auto &bsp2d : this->bsp2d_nodes) {
            std::size_t bsp2d_node_index = &bsp2d - this->bsp2d_nodes.data();

            // Make sure the front and back children are valid
            auto validate_child = [&workload, &tag_index, &bsp2d_count, &surface_count, &bsp2d_node_index](auto &child) {
                if(!child.is_null()) {
                    auto int_value = child.int_value();
                    if(child.flag_value()) {
                        if(int_value > surface_count) {
                            REPORT_ERROR_PRINTF(workload, ERROR_TYPE_FATAL_ERROR, tag_index, "BSP node #%zu has an invalid surface index", bsp2d_node_index);
                            throw InvalidTagDataException();
                        }
                    }
                    else {
                        if(int_value > bsp2d_count) {
                            REPORT_ERROR_PRINTF(workload, ERROR_TYPE_FATAL_ERROR, tag_index, "BSP node #%zu has an invalid node index", bsp2d_node_index);
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
            // TODO
        }
    }
}
