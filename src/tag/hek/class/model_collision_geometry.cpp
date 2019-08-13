/*
 * Invader (c) 2018 Kavawuvi
 *
 * This program is free software under the GNU General Public License v3.0 or later. See LICENSE for more information.
 */

#include "../../../hek/constants.hpp"
#include "../compile.hpp"

#include "model_collision_geometry.hpp"

namespace Invader::HEK {
    void compile_model_collision_geometry_tag(CompiledTag &compiled, const std::byte *data, std::size_t size) {
        BEGIN_COMPILE(ModelCollisionGeometry);

        ADD_DEPENDENCY_ADJUST_SIZES(tag.localized_damage_effect);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.area_damage_effect);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.body_damaged_effect);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.body_depleted_effect);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.body_destroyed_effect);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.shield_damaged_effect);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.shield_depleted_effect);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.shield_recharging_effect);
        tag.shield_recharge_rate = 1.0f / tag.recharge_time / TICK_RATE;

        ADD_REFLEXIVE(tag.materials);
        ADD_REFLEXIVE_START(tag.regions) {
            ADD_DEPENDENCY_ADJUST_SIZES(reflexive.destroyed_effect);
            ADD_REFLEXIVE(reflexive.permutations);
        } ADD_REFLEXIVE_END;
        ADD_REFLEXIVE(tag.modifiers);
        ADD_REFLEXIVE(tag.pathfinding_spheres);
        ADD_REFLEXIVE_START(tag.nodes) {
            ADD_MODEL_COLLISION_BSP(reflexive.bsps)
        } ADD_REFLEXIVE_END

        FINISH_COMPILE
    }

    static inline bool point_in_front_of_plane(const Point3D<LittleEndian> &point, const ModelCollisionGeometryPlane<LittleEndian> *planes, std::uint32_t plane_count, std::uint32_t plane_index) {
        if(plane_index >= plane_count) {
            eprintf("Invalid plane index %u / %u in BSP.\n", plane_index, plane_count);
            throw OutOfBoundsException();
        }
        return point.distance_from_plane(planes[plane_index].plane) >= 0;
    }

    FlaggedInt<std::uint32_t> leaf_for_point_of_bsp_tree(const Point3D<LittleEndian> &point, const ModelCollisionGeometryBSP3DNode<LittleEndian> *bsp3d_nodes, std::uint32_t bsp3d_node_count, const ModelCollisionGeometryPlane<LittleEndian> *planes, std::uint32_t plane_count) {
        // Start with an initial index of 0
        FlaggedInt<std::uint32_t> node_index = {0};

        // Loop until we have a leaf or nothing
        while(!node_index.flag_value() && !node_index.is_null()) {
            // Make sure it's a valid index
            if(node_index >= bsp3d_node_count) {
                eprintf("Invalid BSP2D node %u / %u in BSP.\n", node_index.int_value(), bsp3d_node_count);
                throw OutOfBoundsException();
            }

            // Get the node as well as front/back child info
            auto &node = bsp3d_nodes[node_index];
            node_index = point_in_front_of_plane(point, planes, plane_count, node.plane) ? node.front_child : node.back_child;
        };

        return node_index;
    }

    class IntersectionCheck {
    public:
        static bool check_for_intersection(
            const Point3D<LittleEndian> &point_a,
            const Point3D<LittleEndian> &point_b,
            const ModelCollisionGeometryBSP3DNode<LittleEndian> *bsp3d_nodes,
            std::uint32_t bsp3d_node_count,
            const ModelCollisionGeometryPlane<LittleEndian> *planes,
            std::uint32_t plane_count,
            const ModelCollisionGeometryLeaf<LittleEndian> *leaves,
            std::uint32_t leaf_count,
            const ModelCollisionGeometryBSP2DNode<LittleEndian> *bsp2d_nodes,
            std::uint32_t bsp2d_node_count,
            const ModelCollisionGeometryBSP2DReference<LittleEndian> *bsp2d_references,
            std::uint32_t bsp2d_reference_count,
            const ModelCollisionGeometrySurface<LittleEndian> *surfaces,
            std::uint32_t surface_count,
            const ModelCollisionGeometryEdge<LittleEndian> *edges,
            std::uint32_t edge_count,
            const ModelCollisionGeometryVertex<LittleEndian> *vertices,
            std::uint32_t vertex_count,
            Point3D<LittleEndian> &intersection_point,
            std::uint32_t &surface_index,
            std::uint32_t &leaf_index
        ) {
            IntersectionCheck check(point_a, point_b, bsp3d_nodes, bsp3d_node_count, planes, plane_count, leaves, leaf_count, bsp2d_nodes, bsp2d_node_count, bsp2d_references, bsp2d_reference_count, surfaces, surface_count, edges, edge_count, vertices, vertex_count);
            return check.check_for_intersection_recursion(point_a, point_b, surface_index, leaf_index, intersection_point, {0});
        }

    private:
        const Point3D<LittleEndian> &original_point_a;
        const Point3D<LittleEndian> &original_point_b;
        const ModelCollisionGeometryBSP3DNode<LittleEndian> *bsp3d_nodes;
        std::uint32_t bsp3d_node_count;
        const ModelCollisionGeometryPlane<LittleEndian> *planes;
        std::uint32_t plane_count;
        const ModelCollisionGeometryLeaf<LittleEndian> *leaves;
        std::uint32_t leaf_count;
        const ModelCollisionGeometryBSP2DNode<LittleEndian> *bsp2d_nodes;
        std::uint32_t bsp2d_node_count;
        const ModelCollisionGeometryBSP2DReference<LittleEndian> *bsp2d_references;
        std::uint32_t bsp2d_reference_count;
        const ModelCollisionGeometrySurface<LittleEndian> *surfaces;
        std::uint32_t surface_count;
        const ModelCollisionGeometryEdge<LittleEndian> *edges;
        std::uint32_t edge_count;
        const ModelCollisionGeometryVertex<LittleEndian> *vertices;
        std::uint32_t vertex_count;

        bool check_for_intersection_bsp2d_node (
            FlaggedInt<std::uint32_t> node_index,
            const Point2D<LittleEndian> &point,
            std::uint32_t &surface_index
        ) {
            // If we fall out, fail
            if(node_index.is_null()) {
                return false;
            }

            // Until it's a surface, search
            while(!node_index.flag_value() && !node_index.is_null()) {
                if(node_index.int_value() >= this->bsp2d_node_count) {
                    eprintf("Invalid BSP2D node %u / %u in BSP.\n", node_index.int_value(), this->bsp2d_node_count);
                    throw OutOfBoundsException();
                }

                auto &bsp2d_node = this->bsp2d_nodes[node_index.int_value()];
                if(point.distance_from_plane(bsp2d_node.plane) > 0.0F) {
                    node_index = bsp2d_node.right_child.read();
                }
                else {
                    node_index = bsp2d_node.left_child.read();
                }
            }

            // If we fell out, return false
            if(node_index.is_null()) {
                return false;
            }

            if(node_index.int_value() >= this->surface_count) {
                eprintf("Invalid surface %u / %u in BSP.\n", node_index.int_value(), this->surface_count);
                throw OutOfBoundsException();
            }

            //auto &surface = this->surfaces[node_index.int_value()];
            surface_index = node_index.int_value();
            return true;
        }

        bool check_for_intersection_recursion (
            const Point3D<LittleEndian> &point_a,
            const Point3D<LittleEndian> &point_b,
            std::uint32_t &surface_index,
            std::uint32_t &leaf_index,
            Point3D<LittleEndian> &intersection_point,
            FlaggedInt<std::uint32_t> node_index)
        {
            // Check if they're equal. If so, there's no intersection
            if(point_a == point_b) {
                return false;
            }

            while(!node_index.flag_value() && !node_index.is_null()) {
                // Make sure it's a valid index
                if(node_index >= bsp3d_node_count) {
                    eprintf("Invalid BSP3D node %u / %u in BSP.\n", node_index.int_value(), this->bsp3d_node_count);
                    throw OutOfBoundsException();
                }

                // Get the node as well as front/back child info for each point
                auto &node = bsp3d_nodes[node_index];
                bool a_in_front_of_plane = point_in_front_of_plane(point_a, this->planes, plane_count, node.plane);
                bool b_in_front_of_plane = point_in_front_of_plane(point_b, this->planes, plane_count, node.plane);

                FlaggedInt<std::uint32_t> node_index_a = a_in_front_of_plane ? node.front_child : node.back_child;
                FlaggedInt<std::uint32_t> node_index_b = b_in_front_of_plane ? node.front_child : node.back_child;

                // If they're the same, set node_index to one of them and keep going
                if(node_index_a == node_index_b) {
                    node_index = node_index_a;
                }
                else {
                    // Calculate a point that's almost on the plane
                    auto &plane = planes[node.plane.read()].plane;
                    Point3D<LittleEndian> intersection_front;
                    intersect_plane_with_points(plane, point_a, point_b, &intersection_front);

                    Point3D<LittleEndian> point_a_intersection;
                    Point3D<LittleEndian> point_b_intersection;

                    std::uint32_t leaf_a_intersection, leaf_b_intersection, surface_a_intersection, surface_b_intersection;

                    // Can we get anything closer?
                    bool point_a_intersected = check_for_intersection_recursion(
                        point_a,
                        intersection_front,
                        surface_a_intersection,
                        leaf_a_intersection,
                        point_a_intersection,
                        node_index_a
                    );

                    bool point_b_intersected = check_for_intersection_recursion(
                        intersection_front,
                        point_b,
                        surface_b_intersection,
                        leaf_b_intersection,
                        point_b_intersection,
                        node_index_b
                    );

                    // If neither intersected, we don't have an intersection.
                    if(!point_a_intersected && !point_b_intersected) {
                        return false;
                    }

                    // If both intersected, invalidate the furthest one
                    if(point_a_intersected && point_b_intersected) {
                        float a_distance_squared = point_a_intersection.distance_from_point_squared(point_a);
                        float b_distance_squared = point_b_intersection.distance_from_point_squared(point_a);

                        if(a_distance_squared > b_distance_squared) {
                            point_a_intersected = false;
                        }
                        else {
                            point_b_intersected = false;
                        }
                    }

                    // Now that we have one, return it
                    if(point_a_intersected) {
                        intersection_point = point_a_intersection;
                        leaf_index = leaf_a_intersection;
                        surface_index = surface_a_intersection;
                    }
                    else {
                        intersection_point = point_b_intersection;
                        leaf_index = leaf_b_intersection;
                        surface_index = surface_b_intersection;
                    }

                    return true;
                }
            };

            // Fell out of the BSP; null
            if(node_index.is_null()) {
                return false;
            }

            // Make sure the leaf is valid
            std::uint32_t leaf_index_t = node_index.int_value();
            if(leaf_index_t >= leaf_count) {
                eprintf("invalid leaf index #%u / %u\n", leaf_index_t, leaf_count);
                throw OutOfBoundsException();
            }

            // Get the leaf
            auto &leaf = this->leaves[leaf_index_t];

            // Check if we have nil BSP2D references
            std::uint32_t leaf_bsp2d_reference_count = leaf.bsp2d_reference_count.read();
            std::uint32_t leaf_bsp2d_reference_index = leaf.first_bsp2d_reference.read();
            if(leaf_bsp2d_reference_count == 0) {
                return false;
            }

            // Make sure the BSP2D references are valid
            std::uint64_t bsp2d_end = static_cast<std::uint64_t>(leaf_bsp2d_reference_index + leaf_bsp2d_reference_count);
            if(bsp2d_end > this->bsp2d_reference_count) {
                eprintf("invalid bsp2d reference range #%u - %zu / %u\n", leaf_bsp2d_reference_count, static_cast<std::size_t>(leaf_bsp2d_reference_index + leaf_bsp2d_reference_count), this->bsp2d_reference_count);
                throw OutOfBoundsException();
            }

            // Go through each BSP2D reference
            bool ever_intersected = false;
            float closest_intersection_distance = 0.0F;
            for(std::uint32_t b = leaf_bsp2d_reference_index; b < bsp2d_end; b++) {
                auto &reference = this->bsp2d_references[b];
                auto node_index = reference.bsp2d_node.read();

                // Make sure the plane is valid
                auto plane = reference.plane.read().int_value();
                if(plane >= this->plane_count) {
                    eprintf("invalid plane range for BSP #%u / %u\n", plane, this->plane_count);
                    throw OutOfBoundsException();
                }

                // Make sure point a is in front and point b is behind
                Point3D<LittleEndian> intersection;
                if(!intersect_plane_with_points(this->planes[plane].plane, this->original_point_a, this->original_point_b, &intersection)) {
                    continue;
                }

                // Calculate the distance from the intersection to this. If it's further than what we got previously, disregard it
                float intersection_distance = point_a.distance_from_point_squared(intersection);
                if(ever_intersected && closest_intersection_distance < intersection_distance) {
                    continue;
                }

                // Okay, now let's see if we can get this going
                auto plane_ref = this->planes[plane].plane;
                float x = std::fabs(plane_ref.vector.i);
                float y = std::fabs(plane_ref.vector.j);
                float z = std::fabs(plane_ref.vector.k);
                int axis = 0;
                int sign = 0;

                // This is from <http://www.halomods.com/ips/index.php?/topic/357-collision-bsp-structure/>
                // Get the axis
                float highest;
                if (z < y || z < x) {
                    if (x > y) {
                        axis = 0;
                        highest = x;
                    }
                    else {
                        axis = 1;
                        highest = y;
                    }
                }
                else {
                    axis = 2;
                    highest = z;
                }

                if(highest > 0.0f) {
                    sign = 1;
                }

                // Projection plane
                static const short PLANE_INDICES[2][3][2] = {
                    {
                        {2, 1},
                        {0, 2},
                        {1, 0}
                    },
                    {
                         {1, 2},
                         {2, 0},
                         {0, 1}
                    }
                };

                Point2D<LittleEndian> point;
                point.x = (&plane_ref.vector.i)[PLANE_INDICES[sign][axis][0]];
                point.y = (&plane_ref.vector.i)[PLANE_INDICES[sign][axis][1]];

                if(check_for_intersection_bsp2d_node(node_index, point, surface_index)) {
                    ever_intersected = true;
                    intersection_point = intersection;
                    leaf_index = leaf_index_t;
                    closest_intersection_distance = intersection_distance;
                }
            }

            return ever_intersected;
        }

        IntersectionCheck(
            const Point3D<LittleEndian> &original_point_a,
            const Point3D<LittleEndian> &original_point_b,
            const ModelCollisionGeometryBSP3DNode<LittleEndian> *bsp3d_nodes,
            std::uint32_t bsp3d_node_count,
            const ModelCollisionGeometryPlane<LittleEndian> *planes,
            std::uint32_t plane_count,
            const ModelCollisionGeometryLeaf<LittleEndian> *leaves,
            std::uint32_t leaf_count,
            const ModelCollisionGeometryBSP2DNode<LittleEndian> *bsp2d_nodes,
            std::uint32_t bsp2d_node_count,
            const ModelCollisionGeometryBSP2DReference<LittleEndian> *bsp2d_references,
            std::uint32_t bsp2d_reference_count,
            const ModelCollisionGeometrySurface<LittleEndian> *surfaces,
            std::uint32_t surface_count,
            const ModelCollisionGeometryEdge<LittleEndian> *edges,
            std::uint32_t edge_count,
            const ModelCollisionGeometryVertex<LittleEndian> *vertices,
            std::uint32_t vertex_count
        ) :
        original_point_a(original_point_a),
        original_point_b(original_point_b),
        bsp3d_nodes(bsp3d_nodes),
        bsp3d_node_count(bsp3d_node_count),
        planes(planes),
        plane_count(plane_count),
        leaves(leaves),
        leaf_count(leaf_count),
        bsp2d_nodes(bsp2d_nodes),
        bsp2d_node_count(bsp2d_node_count),
        bsp2d_references(bsp2d_references),
        bsp2d_reference_count(bsp2d_reference_count),
        surfaces(surfaces),
        surface_count(surface_count),
        edges(edges),
        edge_count(edge_count),
        vertices(vertices),
        vertex_count(vertex_count) {}
    };

    bool check_for_intersection(
        const Point3D<LittleEndian> &point_a,
        const Point3D<LittleEndian> &point_b,
        const ModelCollisionGeometryBSP3DNode<LittleEndian> *bsp3d_nodes,
        std::uint32_t bsp3d_node_count,
        const ModelCollisionGeometryPlane<LittleEndian> *planes,
        std::uint32_t plane_count,
        const ModelCollisionGeometryLeaf<LittleEndian> *leaves,
        std::uint32_t leaf_count,
        const ModelCollisionGeometryBSP2DNode<LittleEndian> *bsp2d_nodes,
        std::uint32_t bsp2d_node_count,
        const ModelCollisionGeometryBSP2DReference<LittleEndian> *bsp2d_references,
        std::uint32_t bsp2d_reference_count,
        const ModelCollisionGeometrySurface<LittleEndian> *surfaces,
        std::uint32_t surface_count,
        const ModelCollisionGeometryEdge<LittleEndian> *edges,
        std::uint32_t edge_count,
        const ModelCollisionGeometryVertex<LittleEndian> *vertices,
        std::uint32_t vertex_count,
        Point3D<LittleEndian> &intersection_point,
        std::uint32_t &surface_index,
        std::uint32_t &leaf_index
    ) {
        return IntersectionCheck::check_for_intersection(point_a, point_b, bsp3d_nodes, bsp3d_node_count, planes, plane_count, leaves, leaf_count, bsp2d_nodes, bsp2d_node_count, bsp2d_references, bsp2d_reference_count, surfaces, surface_count, edges, edge_count, vertices, vertex_count, intersection_point, surface_index, leaf_index);
    }
}
