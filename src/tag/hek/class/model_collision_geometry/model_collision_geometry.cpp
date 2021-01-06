// SPDX-License-Identifier: GPL-3.0-only

#include <invader/tag/hek/class/model_collision_geometry.hpp>
#include "intersection_check.hpp"

namespace Invader::HEK {
    bool BSPData::check_for_intersection(const Point3D<LittleEndian> &point_a, const Point3D<LittleEndian> &point_b, Point3D<LittleEndian> *intersection_point, std::uint32_t *surface_index, std::uint32_t *leaf_index) const {
        // Set our variables up
        Point3D<LittleEndian> new_intersection_point;
        std::uint32_t new_surface_index, new_leaf_index;
        
        if(IntersectionCheck::check_for_intersection(
            point_a,
            point_b,
            this->bsp3d_nodes, this->bsp3d_node_count,
            this->planes, this->plane_count,
            this->leaves, this->leaf_count,
            this->bsp2d_nodes, this->bsp2d_node_count,
            this->bsp2d_references, this->bsp2d_reference_count,
            this->surfaces, this->surface_count,
            this->edges, this->edge_count, 
            this->vertices, this->vertex_count,
            new_intersection_point, new_surface_index, new_leaf_index
        )) {
            if(intersection_point) *intersection_point = new_intersection_point;
            if(surface_index) *surface_index = new_surface_index;
            if(leaf_index) *leaf_index = new_leaf_index;
            return true;
        }
        
        return false;
    }
    
    bool BSPData::check_for_intersection(const Point3D<LittleEndian> &point, float range, Point3D<LittleEndian> *intersection_point, std::uint32_t *surface_index, std::uint32_t *leaf_index) const {
        std::printf("Point: %f %f %f\n", point.x.read(), point.y.read(), point.z.read());
        
        // Plus or minus distance it
        auto position_above = point;
        position_above.z = position_above.z + range;
        auto position_below = point;
        position_below.z = position_below.z - range;
        
        // Hold any results in a struct
        struct PositionFound {
            std::uint32_t leaf_index_found;
            std::uint32_t surface_index_found;
            HEK::Point3D<HEK::LittleEndian> intersection_point_found;
        };
        std::vector<PositionFound> positions_found;
        
        // Start with the top position and work our way down
        auto current_position = position_above;
        
        // Keep doing this until we can't anymore
        while(current_position.z.read() > position_below.z.read()) {
            std::uint32_t leaf_index_found;
            std::uint32_t surface_index_found;
            HEK::Point3D<HEK::LittleEndian> intersection_point_found;
            auto val = this->check_for_intersection(
                current_position,
                position_below,
                &intersection_point_found,
                &surface_index_found,
                &leaf_index_found
            );
            
            // We got it!
            if(val) {
                positions_found.emplace_back(PositionFound { leaf_index_found, surface_index_found, intersection_point_found });
                current_position.z = intersection_point_found.z - 0.01F; // subtract a lil' bit so we don't loop forever
            }
            
            // No intersection found; no point continuing then
            else {
                break;
            }
        }
        
        for(auto &i : positions_found) {
            std::printf("%f %f %f, %zu\n", i.intersection_point_found.x.read(), i.intersection_point_found.y.read(), i.intersection_point_found.z.read(), static_cast<std::size_t>(i.surface_index_found));
        }
        
        // Find the closest intersection to our input point
        PositionFound *closest = nullptr;
        float closest_distance_squared = range;
        for(auto &i : positions_found) {
            float new_distance = i.intersection_point_found.distance_from_point_squared(point);
            if(!closest || new_distance < closest_distance_squared) {
                closest_distance_squared = new_distance;
                closest = &i;
            }
        }
        
        // If we found the closest point, keep going
        if(closest) {
            if(intersection_point) {
                *intersection_point = closest->intersection_point_found;
            }
            
            if(surface_index) {
                *surface_index = closest->surface_index_found;
            }
            
            if(leaf_index) {
                *leaf_index = closest->leaf_index_found;
            }
            
            return true;
        }
        return false;
    }
    
    bool BSPData::check_if_point_inside_bsp(const Point3D<LittleEndian> &point, std::uint32_t *leaf_index) const {
        auto result = HEK::leaf_for_point_of_bsp_tree(point, this->bsp3d_nodes, this->bsp3d_node_count, this->planes, this->plane_count);
        
        // If null, then we don't have anything
        if(result.is_null()) {
            return false;
        }
        
        // Otherwise, yay!
        if(leaf_index) {
            *leaf_index = result.int_value();
        }
        
        return true;
    }
}
