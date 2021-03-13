// SPDX-License-Identifier: GPL-3.0-only

#ifndef INVADER__MODEL__JMS_HPP
#define INVADER__MODEL__JMS_HPP

#include <vector>
#include <map>

#include "../hek/data_type.hpp"
#include "../tag/parser/parser_struct.hpp"

namespace Invader {
    struct JMS {
        struct Node {
            std::string name;
            HEK::Index first_child;
            HEK::Index sibling_node;
            HEK::Quaternion<HEK::NativeEndian> rotation;
            HEK::Point3D<HEK::NativeEndian> position;
            
            std::string string() const;
            static Node from_string(const char *string, const char **end);
        };
        std::vector<Node> nodes;
        
        struct Material {
            std::string name;
            std::string tif_path;
            
            std::string string() const;
            static Material from_string(const char *string, const char **end);
        };
        std::vector<Material> materials;
        
        struct Marker {
            std::string name;
            HEK::Index region;
            HEK::Index node;
            HEK::Quaternion<HEK::NativeEndian> rotation;
            HEK::Point3D<HEK::NativeEndian> position;
            float radius;
            
            std::string string() const;
            static Marker from_string(const char *string, const char **end);
        };
        std::vector<Marker> markers;
        
        struct Region {
            std::string name;
            
            std::string string() const;
            static Region from_string(const char *string, const char **end);
        };
        std::vector<Region> regions;
        
        struct Vertex {
            HEK::Index node0;
            HEK::Point3D<HEK::NativeEndian> position;
            HEK::Vector3D<HEK::NativeEndian> normal;
            HEK::Index node1;
            float node1_weight;
            HEK::Point2D<HEK::NativeEndian> texture_coordinates;
            
            std::string string() const;
            static Vertex from_string(const char *string, const char **end);
        };
        std::vector<Vertex> vertices;
        
        struct Triangle {
            HEK::Index region;
            HEK::Index shader;
            HEK::Index vertices[3];
            
            std::string string() const;
            static Triangle from_string(const char *string, const char **end);
        };
        std::vector<Triangle> triangles;
        
        std::string string() const;
        static JMS from_string(const char *string, const char **end = nullptr);
    };
    
    using JMSMap = std::map<std::string, JMS>;
}

#endif
