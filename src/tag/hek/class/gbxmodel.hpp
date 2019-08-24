/*
 * Invader (c) 2018 Kavawuvi
 *
 * This program is free software under the GNU General Public License v3.0 or later. See LICENSE for more information.
 */

#ifndef INVADER__TAG__HEK__CLASS__GBXMODEL_HPP
#define INVADER__TAG__HEK__CLASS__GBXMODEL_HPP

#include "../../compiled_tag.hpp"
#include "../../../hek/data_type.hpp"
#include "../header.hpp"

namespace Invader::HEK {
    ENDIAN_TEMPLATE(EndianType) struct GBXModelMarkerInstance {
        std::int8_t region_index;
        std::int8_t permutation_index;
        std::int8_t node_index;
        PAD(0x1);
        Point3D<EndianType> translation;
        Quaternion<EndianType> rotation;

        ENDIAN_TEMPLATE(NewType) operator GBXModelMarkerInstance<NewType>() const noexcept {
            GBXModelMarkerInstance<NewType> copy = {};
            COPY_THIS(region_index);
            COPY_THIS(permutation_index);
            COPY_THIS(node_index);
            COPY_THIS(translation);
            COPY_THIS(rotation);
            return copy;
        }
    };
    static_assert(sizeof(GBXModelMarkerInstance<BigEndian>) == 0x20);

    ENDIAN_TEMPLATE(EndianType) struct GBXModelMarker {
        TagString name;
        EndianType<std::int16_t> magic_identifier;
        PAD(0x2);
        PAD(0x10);
        TagReflexive<EndianType, GBXModelMarkerInstance> instances;

        ENDIAN_TEMPLATE(NewType) operator GBXModelMarker<NewType>() const noexcept {
            GBXModelMarker<NewType> copy = {};
            COPY_THIS(name);
            COPY_THIS(magic_identifier);
            COPY_THIS(instances);
            return copy;
        }
    };
    static_assert(sizeof(GBXModelMarker<BigEndian>) == 0x40 );

    ENDIAN_TEMPLATE(EndianType) struct GBXModelNode {
        TagString name;
        EndianType<std::int16_t> next_sibling_node_index;
        EndianType<std::int16_t> first_child_node_index;
        EndianType<std::int16_t> parent_node_index;
        PAD(0x2);
        Point3D<EndianType> default_translation;
        Quaternion<EndianType> default_rotation;
        EndianType<float> node_distance_from_parent;
        PAD(0x20);
        EndianType<float> scale;
        Matrix<EndianType> rotation;
        Point3D<EndianType> translation;

        ENDIAN_TEMPLATE(NewType) operator GBXModelNode<NewType>() const noexcept {
            GBXModelNode<NewType> copy = {};
            COPY_THIS(name);
            COPY_THIS(next_sibling_node_index);
            COPY_THIS(first_child_node_index);
            COPY_THIS(parent_node_index);
            COPY_THIS(default_translation);
            COPY_THIS(default_rotation);
            COPY_THIS(node_distance_from_parent);
            COPY_THIS(scale);
            COPY_THIS(rotation);
            COPY_THIS(translation);
            return copy;
        }
    };
    static_assert(sizeof(GBXModelNode<BigEndian>) == 0x9C);

    ENDIAN_TEMPLATE(EndianType) struct GBXModelRegionPermutationMarker {
        TagString name;
        EndianType<std::int16_t> node_index;
        PAD(0x2);
        Quaternion<EndianType> rotation;
        Point3D<EndianType> translation;
        PAD(0x10);

        ENDIAN_TEMPLATE(NewType) operator GBXModelRegionPermutationMarker<NewType>() const noexcept {
            GBXModelRegionPermutationMarker<NewType> copy = {};
            COPY_THIS(name);
            COPY_THIS(node_index);
            COPY_THIS(rotation);
            COPY_THIS(translation);
            return copy;
        }
    };
    static_assert(sizeof(GBXModelRegionPermutationMarker<BigEndian>) == 0x50);

    struct GBXModelRegionPermutationFlags {
        std::uint32_t cannot_be_chosen_randomly : 1;
    };
    static_assert(sizeof(GBXModelRegionPermutationFlags) == sizeof(std::uint32_t));

    ENDIAN_TEMPLATE(EndianType) struct GBXModelRegionPermutation {
        TagString name;
        EndianType<GBXModelRegionPermutationFlags> flags;
        LittleEndian<std::uint16_t> permutation_number;
        PAD(0x2);
        PAD(0x18);
        EndianType<std::int16_t> super_low;
        EndianType<std::int16_t> low;
        EndianType<std::int16_t> medium;
        EndianType<std::int16_t> high;
        EndianType<std::int16_t> super_high;
        PAD(0x2);
        TagReflexive<EndianType, GBXModelRegionPermutationMarker> markers;

        ENDIAN_TEMPLATE(NewType) operator GBXModelRegionPermutation<NewType>() const noexcept {
            GBXModelRegionPermutation<NewType> copy = {};
            COPY_THIS(name);
            COPY_THIS(flags);
            COPY_THIS(permutation_number);
            COPY_THIS(super_low);
            COPY_THIS(low);
            COPY_THIS(medium);
            COPY_THIS(high);
            COPY_THIS(super_high);
            COPY_THIS(markers);
            return copy;
        }
    };
    static_assert(sizeof(GBXModelRegionPermutation<BigEndian>) == 0x58);

    ENDIAN_TEMPLATE(EndianType) struct GBXModelRegion {
        TagString name;
        PAD(0x20);
        TagReflexive<EndianType, GBXModelRegionPermutation> permutations;

        ENDIAN_TEMPLATE(NewType) operator GBXModelRegion<NewType>() const noexcept {
            GBXModelRegion<NewType> copy = {};
            COPY_THIS(name);
            COPY_THIS(permutations);
            return copy;
        }
    };
    static_assert(sizeof(GBXModelRegion<BigEndian>) == 0x4C);

    ENDIAN_TEMPLATE(EndianType) struct GBXModelVertexUncompressed {
        Point3D<EndianType> position;
        Vector3D<EndianType> normal;
        Vector3D<EndianType> binormal;
        Vector3D<EndianType> tangent;
        Point2D<EndianType> texture_coords;
        EndianType<std::int16_t> node0_index;
        EndianType<std::int16_t> node1_index;
        EndianType<float> node0_weight;
        EndianType<float> node1_weight;

        ENDIAN_TEMPLATE(NewType) operator GBXModelVertexUncompressed<NewType>() const noexcept {
            GBXModelVertexUncompressed<NewType> copy;
            COPY_THIS(position);
            COPY_THIS(normal);
            COPY_THIS(binormal);
            COPY_THIS(tangent);
            COPY_THIS(texture_coords);
            COPY_THIS(node0_index);
            COPY_THIS(node1_index);
            COPY_THIS(node0_weight);
            COPY_THIS(node1_weight);
            return copy;
        }
    };
    static_assert(sizeof(GBXModelVertexUncompressed<BigEndian>) == 0x44 );

    ENDIAN_TEMPLATE(EndianType) struct GBXModelVertexCompressed {
        Point3D<EndianType> position;
        EndianType<std::int32_t> normal;
        EndianType<std::int32_t> binormal;
        EndianType<std::int32_t> tangent;
        EndianType<std::int16_t> texture_coordinate_u;
        EndianType<std::int16_t> texture_coordinate_v;
        std::int8_t node0_index;
        std::int8_t node1_index;
        EndianType<std::int16_t> node0_weight;

        ENDIAN_TEMPLATE(NewType) operator GBXModelVertexCompressed<NewType>() const noexcept {
            GBXModelVertexCompressed<NewType> copy;
            COPY_THIS(position);
            COPY_THIS(normal);
            COPY_THIS(binormal);
            COPY_THIS(tangent);
            COPY_THIS(texture_coordinate_u);
            COPY_THIS(texture_coordinate_v);
            COPY_THIS(node0_index);
            COPY_THIS(node1_index);
            COPY_THIS(node0_weight);
            return copy;
        }
    };
    static_assert(sizeof(GBXModelVertexCompressed<BigEndian>) == 0x20 );

    ENDIAN_TEMPLATE(EndianType) struct GBXModelTriangle {
        EndianType<std::int16_t> vertex0_index;
        EndianType<std::int16_t> vertex1_index;
        EndianType<std::int16_t> vertex2_index;

        ENDIAN_TEMPLATE(NewType) operator GBXModelTriangle<NewType>() const noexcept {
            GBXModelTriangle<NewType> copy;
            COPY_THIS(vertex0_index);
            COPY_THIS(vertex1_index);
            COPY_THIS(vertex2_index);
            return copy;
        }
    };
    static_assert(sizeof(GBXModelTriangle<BigEndian>) == 0x6 );

    struct GBXModelGeometryPartFlags {
        std::uint32_t stripped_internal : 1;
        std::uint32_t zoner : 1;
    };

    ENDIAN_TEMPLATE(EndianType) struct GBXModelGeometryPart {
        EndianType<GBXModelGeometryPartFlags> flags;
        EndianType<std::int16_t> shader_index;
        std::int8_t prev_filthy_part_index;
        std::int8_t next_filthy_part_index;
        EndianType<std::int16_t> centroid_primary_node;
        EndianType<std::int16_t> centroid_secondary_node;
        EndianType<Fraction> centroid_primary_weight;
        EndianType<Fraction> centroid_secondary_weight;
        Point3D<EndianType> centroid;
        TagReflexive<EndianType, GBXModelVertexUncompressed> uncompressed_vertices;
        TagReflexive<EndianType, GBXModelVertexCompressed> compressed_vertices;
        TagReflexive<EndianType, GBXModelTriangle> triangles;

        /** If this isn't 1, crash the game on map load (even when the model isn't visible)? */
        EndianType<std::uint32_t> do_not_crash_the_game;

        /** number of triangles when compiled */
        EndianType<std::uint32_t> triangle_count;

        /** offset to triangles from end of vertices when compiled */
        EndianType<std::uint32_t> triangle_offset;

        /** another offset to triangles from end of vertices when compiled */
        EndianType<std::uint32_t> triangle_offset_2;

        /** If this isn't 4, the model gets screwed up? */
        EndianType<std::uint32_t> do_not_screw_up_the_model;

        /** number of vertices when compiled */
        EndianType<std::uint32_t> vertex_count;

        /** actual padding */
        PAD(0x4);

        /** This value is complete and utter bullshit. Tool changes this arbitrarily, resulting in different values when built on different OS's, shells, etc. And it probably doesn't even do anything. */
        BigEndian<std::uint32_t> bullshit;

        /** offset to vertices from beginning of vertices when compiled */
        EndianType<std::uint32_t> vertex_offset;

        PAD(0x1);
        PAD(0x1);
        PAD(0x1);
        std::int8_t num_nodes;
        std::int8_t local_node_indices[24];

        ENDIAN_TEMPLATE(NewType) operator GBXModelGeometryPart<NewType>() const noexcept {
            GBXModelGeometryPart<NewType> copy = {};
            COPY_THIS(flags);
            COPY_THIS(shader_index);
            COPY_THIS(prev_filthy_part_index);
            COPY_THIS(next_filthy_part_index);
            COPY_THIS(centroid_primary_node);
            COPY_THIS(centroid_secondary_node);
            COPY_THIS(centroid_primary_weight);
            COPY_THIS(centroid_secondary_weight);
            COPY_THIS(centroid);
            COPY_THIS(uncompressed_vertices);
            COPY_THIS(compressed_vertices);
            COPY_THIS(triangles);
            COPY_THIS(num_nodes);
            COPY_THIS(triangle_count);
            COPY_THIS(triangle_offset);
            COPY_THIS(triangle_offset_2);
            COPY_THIS(vertex_count);
            COPY_THIS(vertex_offset);
            COPY_THIS(bullshit);
            COPY_THIS_ARRAY(local_node_indices);
            return copy;
        }
    };
    static_assert(sizeof(GBXModelGeometryPart<BigEndian>) == 0x84);

    struct GBXModelGeometryFlags {
        std::uint32_t unused : 1;
    };
    ENDIAN_TEMPLATE(EndianType) struct GBXModelGeometry {
        EndianType<GBXModelGeometryFlags> flags;
        PAD(0x20);
        TagReflexive<EndianType, GBXModelGeometryPart> parts;

        ENDIAN_TEMPLATE(NewType) operator GBXModelGeometry<NewType>() const noexcept {
            GBXModelGeometry<NewType> copy = {};
            COPY_THIS(flags);
            COPY_THIS(parts);
            return copy;
        }
    };
    static_assert(sizeof(GBXModelGeometry<BigEndian>) == 0x30 );

    ENDIAN_TEMPLATE(EndianType) struct GBXModelShaderReference {
        TagDependency<EndianType> shader; // shader
        EndianType<std::int16_t> permutation;
        PAD(0x2);
        PAD(0xC);

        ENDIAN_TEMPLATE(NewType) operator GBXModelShaderReference<NewType>() const noexcept {
            GBXModelShaderReference<NewType> copy = {};
            COPY_THIS(shader);
            COPY_THIS(permutation);
            return copy;
        }
    };
    static_assert(sizeof(GBXModelShaderReference<BigEndian>) == 0x20 );

    struct GBXModelFlags {
        std::uint32_t blend_shared_normals : 1;
        std::uint32_t parts_have_local_nodes : 1;
        std::uint32_t ignore_skinning : 1;
    };

    ENDIAN_TEMPLATE(EndianType) struct GBXModel {
        EndianType<GBXModelFlags> flags;
        EndianType<std::int32_t> node_list_checksum;
        EndianType<float> super_high_detail_cutoff;
        EndianType<float> high_detail_cutoff;
        EndianType<float> medium_detail_cutoff;
        EndianType<float> low_detail_cutoff;
        EndianType<float> super_low_detail_cutoff;
        EndianType<std::int16_t> super_high_detail_node_count;
        EndianType<std::int16_t> high_detail_node_count;
        EndianType<std::int16_t> medium_detail_node_count;
        EndianType<std::int16_t> low_detail_node_count;
        EndianType<std::int16_t> super_low_detail_node_count;
        PAD(0x2);
        PAD(0x8);
        EndianType<float> base_map_u_scale;
        EndianType<float> base_map_v_scale;
        PAD(0x74);
        TagReflexive<EndianType, GBXModelMarker> markers;
        TagReflexive<EndianType, GBXModelNode> nodes;
        TagReflexive<EndianType, GBXModelRegion> regions;
        TagReflexive<EndianType, GBXModelGeometry> geometries;
        TagReflexive<EndianType, GBXModelShaderReference> shaders;

        ENDIAN_TEMPLATE(NewType) operator GBXModel<NewType>() const noexcept {
            GBXModel<NewType> copy = {};
            COPY_THIS(flags);
            COPY_THIS(node_list_checksum);
            COPY_THIS(super_high_detail_cutoff);
            COPY_THIS(high_detail_cutoff);
            COPY_THIS(medium_detail_cutoff);
            COPY_THIS(low_detail_cutoff);
            COPY_THIS(super_low_detail_cutoff);
            COPY_THIS(super_high_detail_node_count);
            COPY_THIS(high_detail_node_count);
            COPY_THIS(medium_detail_node_count);
            COPY_THIS(low_detail_node_count);
            COPY_THIS(super_low_detail_node_count);
            COPY_THIS(base_map_u_scale);
            COPY_THIS(base_map_v_scale);
            COPY_THIS(markers);
            COPY_THIS(nodes);
            COPY_THIS(regions);
            COPY_THIS(geometries);
            COPY_THIS(shaders);
            return copy;
        }
    };
    static_assert(sizeof(GBXModel<BigEndian>) == 0xE8);

    void compile_gbxmodel_tag(CompiledTag &compiled, const std::byte *data, std::size_t size);
}
#endif
