// SPDX-License-Identifier: GPL-3.0-only

#ifndef INVADER__MODEL__JMS_HPP
#define INVADER__MODEL__JMS_HPP

#include <vector>
#include <map>

#include "../hek/data_type.hpp"
#include "../tag/parser/parser_struct.hpp"

namespace Invader {
    struct JMS {
        std::uint32_t node_list_checksum = 0;
        
        struct Node {
            std::string name;
            Parser::Index first_child;
            Parser::Index sibling_node;
            Parser::Quaternion<Parser::NativeEndian> rotation;
            Parser::Point3D<Parser::NativeEndian> position;
            
            std::string string() const;
            static Node from_string(const char *string, const char **end);
            
            bool operator ==(const Node &other) const noexcept {
                return other.name == this->name &&
                       other.first_child == this->first_child &&
                       other.sibling_node == this->sibling_node &&
                       other.rotation == this->rotation &&
                       other.position == this->position;
            }
            bool operator !=(const Node &other) const noexcept {
                return !(*this == other);
            }
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
            Parser::Index region;
            Parser::Index node;
            Parser::Quaternion<Parser::NativeEndian> rotation;
            Parser::Point3D<Parser::NativeEndian> position;
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
            Parser::Index node0;
            Parser::Point3D<Parser::NativeEndian> position;
            Parser::Vector3D<Parser::NativeEndian> normal;
            Parser::Index node1;
            float node1_weight;
            Parser::Point2D<Parser::NativeEndian> texture_coordinates;
            
            std::string string() const;
            static Vertex from_string(const char *string, const char **end);
            
            bool operator ==(const Vertex &other) const noexcept {
                return this->node0 == other.node0 &&
                       this->position == other.position &&
                       this->normal == other.normal &&
                       this->node1 == other.node1 &&
                       this->node1_weight == other.node1_weight &&
                       this->texture_coordinates == other.texture_coordinates;
            }
            
            bool operator !=(const Vertex &other) const noexcept {
                return !(*this == other);
            }
        };
        std::vector<Vertex> vertices;
        
        struct Triangle {
            Parser::Index region;
            Parser::Index shader;
            Parser::Index vertices[3];
            
            std::string string() const;
            static Triangle from_string(const char *string, const char **end);
        };
        std::vector<Triangle> triangles;
        
        std::string string() const;
        static JMS from_string(const char *string, const char **end = nullptr);
        
        /**
         * Optimize, removing duplicate vertices
         */
        void optimize();
    };
    
    using JMSMap = std::map<std::string, JMS>;
}

#endif
