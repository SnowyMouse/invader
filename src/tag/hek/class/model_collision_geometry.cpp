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
        FlaggedInt<std::uint32_t> node_index = {0};

        // Loop until we get outta here
        while(true) {
            // If it's a leaf or it's null, return it
            if(node_index.flag_value() || node_index.is_null()) {
                return node_index;
            }

            // Make sure it's a valid index
            if(node_index >= bsp3d_node_count) {
                eprintf("Invalid BSP2D node %u / %u in BSP.\n", node_index.int_value(), bsp3d_node_count);
                throw OutOfBoundsException();
            }

            // Get the node as well as front/back child info
            auto &node = bsp3d_nodes[node_index];

            // Let's see if it's in front of the plane
            if(point_in_front_of_plane(point, planes, plane_count, node.plane)) {
                node_index = node.front_child;
            }

            // If not, keep going
            else {
                node_index = node.back_child;
            }
        };
    }
}
