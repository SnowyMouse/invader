// SPDX-License-Identifier: GPL-3.0-only

#ifndef INVADER__TAG__HEK__CLASS__MODEL_COLLISION_GEOMETRY_HPP
#define INVADER__TAG__HEK__CLASS__MODEL_COLLISION_GEOMETRY_HPP

#include "../../../hek/data_type.hpp"
#include "../definition.hpp"

namespace Invader::HEK {
    /**
     * Determine if a point is located inside of a BSP. If so, return the leaf.
     * @param  point            3D point reference
     * @param  bsp3d_nodes      pointer to the BSP3D nodes
     * @param  bsp3d_node_count number of BSP3D nodes
     * @param  planes           pointer to the BSP planes
     * @param  plane_count      number of BSP planes
     * @return                  leaf index if found; null index if not
     */
    FlaggedInt<std::uint32_t> leaf_for_point_of_bsp_tree(
        const Point3D<LittleEndian> &point,
        const ModelCollisionGeometryBSP3DNode<LittleEndian> *bsp3d_nodes,
        std::uint32_t bsp3d_node_count,
        const ModelCollisionGeometryBSPPlane<LittleEndian> *planes,
        std::uint32_t plane_count
    );


    bool check_for_intersection(
        const Point3D<LittleEndian> &point_a,
        const Point3D<LittleEndian> &point_b,
        const ModelCollisionGeometryBSP3DNode<LittleEndian> *bsp3d_nodes,
        std::uint32_t bsp3d_node_count,
        const ModelCollisionGeometryBSPPlane<LittleEndian> *planes,
        std::uint32_t plane_count,
        const ModelCollisionGeometryBSPLeaf<LittleEndian> *leaves,
        std::uint32_t leaf_count,
        const ModelCollisionGeometryBSP2DNode<LittleEndian> *bsp2d_nodes,
        std::uint32_t bsp2d_node_count,
        const ModelCollisionGeometryBSP2DReference<LittleEndian> *bsp2d_references,
        std::uint32_t bsp2d_reference_count,
        const ModelCollisionGeometryBSPSurface<LittleEndian> *surfaces,
        std::uint32_t surface_count,
        const ModelCollisionGeometryBSPEdge<LittleEndian> *edges,
        std::uint32_t edge_count,
        const ModelCollisionGeometryBSPVertex<LittleEndian> *vertices,
        std::uint32_t vertex_count,
        Point3D<LittleEndian> &intersection_point,
        std::uint32_t &surface_index,
        std::uint32_t &leaf_index
    );
}
#endif
