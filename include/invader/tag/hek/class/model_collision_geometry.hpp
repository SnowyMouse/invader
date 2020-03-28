// SPDX-License-Identifier: GPL-3.0-only

#ifndef INVADER__TAG__HEK__CLASS__MODEL_COLLISION_GEOMETRY_HPP
#define INVADER__TAG__HEK__CLASS__MODEL_COLLISION_GEOMETRY_HPP

#include "../../../hek/data_type.hpp"
#include "../definition.hpp"

namespace Invader::HEK {
    /**
     * Struct for containing all information required to find intersections among other things
     */
    struct BSPData {
        const ModelCollisionGeometryBSP3DNode<LittleEndian> *bsp3d_nodes = nullptr;
        std::uint32_t bsp3d_node_count = 0;
        const ModelCollisionGeometryBSPPlane<LittleEndian> *planes = nullptr;
        std::uint32_t plane_count = 0;
        const ModelCollisionGeometryBSPLeaf<LittleEndian> *leaves = nullptr;
        std::uint32_t leaf_count = 0;
        const ModelCollisionGeometryBSP2DNode<LittleEndian> *bsp2d_nodes = nullptr;
        std::uint32_t bsp2d_node_count = 0;
        const ModelCollisionGeometryBSP2DReference<LittleEndian> *bsp2d_references = nullptr;
        std::uint32_t bsp2d_reference_count = 0;
        const ModelCollisionGeometryBSPSurface<LittleEndian> *surfaces = nullptr;
        std::uint32_t surface_count = 0;
        const ModelCollisionGeometryBSPEdge<LittleEndian> *edges = nullptr;
        std::uint32_t edge_count = 0;
        const ModelCollisionGeometryBSPVertex<LittleEndian> *vertices = nullptr;
        std::uint32_t vertex_count = 0;
        const ScenarioStructureBSPLeaf<LittleEndian> *render_leaves = nullptr;
        std::uint32_t render_leaf_count = 0;
        
        /**
         * Determine if a point intersects vertically with the BSP.
         * @param point_a            one point in the line to check
         * @param point_b            the other point in the line to check
         * @param intersection_point if non-null and this function returns true, this will be set to the point where an intersection was found
         * @param surface_index      if non-null and this function returns true, this will be set to the surface index where the intersection was found
         * @param leaf_index         if non-null and this function returns true, this will be set to the leaf index where the intersection was found
         * @return                   true if an intersection was found
         */
        bool check_for_intersection(const Point3D<LittleEndian> &point_a, const Point3D<LittleEndian> &point_b, Point3D<LittleEndian> *intersection_point = nullptr, std::uint32_t *surface_index = nullptr, std::uint32_t *leaf_index = nullptr) const;
        
        /**
         * Determine if a point intersects vertically with the BSP.
         * @param point              point to check
         * @param range              range up-and-down to check
         * @param intersection_point if non-null and this function returns true, this will be set to the point where an intersection was found
         * @param surface_index      if non-null and this function returns true, this will be set to the surface index where the intersection was found
         * @param leaf_index         if non-null and this function returns true, this will be set to the leaf index where the intersection was found
         * @return                   true if an intersection was found
         */
        bool check_for_intersection(const Point3D<LittleEndian> &point, float range, Point3D<LittleEndian> *intersection_point = nullptr, std::uint32_t *surface_index = nullptr, std::uint32_t *leaf_index = nullptr) const;
        
        /**
         * Determine if a point lays inside of a BSP.
         * @param point      point to check
         * @param leaf_index if non-null and this function returns true, this will be set to the leaf index where the point is located
         */
        bool check_if_point_inside_bsp(const Point3D<LittleEndian> &point, std::uint32_t *leaf_index = nullptr) const;
    };
}
#endif
