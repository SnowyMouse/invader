// SPDX-License-Identifier: GPL-3.0-only

#ifndef INVADER__TAG__HEK__CLASS__MODEL_COLLISION_GEOMETRY__INTERSECTION_CHECK_HPP
#define INVADER__TAG__HEK__CLASS__MODEL_COLLISION_GEOMETRY__INTERSECTION_CHECK_HPP

#include <invader/tag/hek/class/model_collision_geometry.hpp>

namespace Invader::HEK {
    struct IntersectionCheck {
    public:
        static bool check_for_intersection(
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

        const Point3D<LittleEndian> &original_point_a;
        const Point3D<LittleEndian> &original_point_b;
        const ModelCollisionGeometryBSP3DNode<LittleEndian> *bsp3d_nodes;
        std::uint32_t bsp3d_node_count;
        const ModelCollisionGeometryBSPPlane<LittleEndian> *planes;
        std::uint32_t plane_count;
        const ModelCollisionGeometryBSPLeaf<LittleEndian> *leaves;
        std::uint32_t leaf_count;
        const ModelCollisionGeometryBSP2DNode<LittleEndian> *bsp2d_nodes;
        std::uint32_t bsp2d_node_count;
        const ModelCollisionGeometryBSP2DReference<LittleEndian> *bsp2d_references;
        std::uint32_t bsp2d_reference_count;
        const ModelCollisionGeometryBSPSurface<LittleEndian> *surfaces;
        std::uint32_t surface_count;
        const ModelCollisionGeometryBSPEdge<LittleEndian> *edges;
        std::uint32_t edge_count;
        const ModelCollisionGeometryBSPVertex<LittleEndian> *vertices;
        std::uint32_t vertex_count;

        bool check_for_intersection_bsp2d_node (
            FlaggedInt<std::uint32_t> node_index,
            const Point2D<LittleEndian> &point,
            std::uint32_t &surface_index
        );

        bool check_for_intersection_recursion (
            const Point3D<LittleEndian> &point_a,
            const Point3D<LittleEndian> &point_b,
            std::uint32_t &surface_index,
            std::uint32_t &leaf_index,
            Point3D<LittleEndian> &intersection_point,
            FlaggedInt<std::uint32_t> node_index
        );

        IntersectionCheck(
            const Point3D<LittleEndian> &original_point_a,
            const Point3D<LittleEndian> &original_point_b,
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
            std::uint32_t vertex_count
        );
    };
    
    FlaggedInt<std::uint32_t> leaf_for_point_of_bsp_tree(const Point3D<LittleEndian> &point, const ModelCollisionGeometryBSP3DNode<LittleEndian> *bsp3d_nodes, std::uint32_t bsp3d_node_count, const ModelCollisionGeometryBSPPlane<LittleEndian> *planes, std::uint32_t plane_count);
}

#endif
