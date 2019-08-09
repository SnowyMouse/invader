/*
 * Invader (c) 2018 Kavawuvi
 *
 * This program is free software under the GNU General Public License v3.0 or later. See LICENSE for more information.
 */

#ifndef INVADER__TAG__HEK__CLASS__MODEL_COLLISION_GEOMETRY_HPP
#define INVADER__TAG__HEK__CLASS__MODEL_COLLISION_GEOMETRY_HPP

#include "../../compiled_tag.hpp"
#include "../../../hek/data_type.hpp"
#include "../header.hpp"

namespace Invader::HEK {
    struct ModelCollisionGeometryMaterialFlags {
        std::uint32_t head : 1;
    };
    ENDIAN_TEMPLATE(EndianType) struct ModelCollisionGeometryMaterial {
        TagString name;
        EndianType<ModelCollisionGeometryMaterialFlags> flags;
        EndianType<MaterialType> material_type;
        PAD(0x2);
        EndianType<Fraction> shield_leak_percentage;
        EndianType<float> shield_damage_multiplier;
        PAD(0xC);
        EndianType<float> body_damage_multiplier;
        PAD(0x8);

        ENDIAN_TEMPLATE(NewType) operator ModelCollisionGeometryMaterial<NewType>() const noexcept {
            ModelCollisionGeometryMaterial<NewType> copy = {};
            COPY_THIS(name);
            COPY_THIS(flags);
            COPY_THIS(material_type);
            COPY_THIS(shield_leak_percentage);
            COPY_THIS(shield_damage_multiplier);
            COPY_THIS(body_damage_multiplier);
            return copy;
        }
    };
    static_assert(sizeof(ModelCollisionGeometryMaterial<BigEndian>) == 0x48);

    ENDIAN_TEMPLATE(EndianType) struct ModelCollisionGeometryPermutation {
        TagString name;

        ENDIAN_TEMPLATE(NewType) operator ModelCollisionGeometryPermutation<NewType>() const noexcept {
            ModelCollisionGeometryPermutation<NewType> copy;
            COPY_THIS(name);
            return copy;
        }
    };
    static_assert(sizeof(ModelCollisionGeometryPermutation<BigEndian>) == 0x20);

    struct ModelCollisionGeometryRegionFlags {
        std::uint32_t lives_until_object_dies : 1;
        std::uint32_t forces_object_to_die : 1;
        std::uint32_t dies_when_object_dies : 1;
        std::uint32_t dies_when_object_is_damaged : 1;
        std::uint32_t disappears_when_shield_is_off : 1;
        std::uint32_t inhibits_melee_attack : 1;
        std::uint32_t inhibits_weapon_attack : 1;
        std::uint32_t inhibits_walking : 1;
        std::uint32_t forces_drop_weapon : 1;
        std::uint32_t causes_head_maimed_scream : 1;
    };

    ENDIAN_TEMPLATE(EndianType) struct ModelCollisionGeometryRegion {
        TagString name;
        EndianType<ModelCollisionGeometryRegionFlags> flags;
        PAD(0x4);
        EndianType<float> damage_threshold;
        PAD(0xC);
        TagDependency<EndianType> destroyed_effect; // effect
        TagReflexive<EndianType, ModelCollisionGeometryPermutation> permutations;

        ENDIAN_TEMPLATE(NewType) operator ModelCollisionGeometryRegion<NewType>() const noexcept {
            ModelCollisionGeometryRegion<NewType> copy = {};
            COPY_THIS(name);
            COPY_THIS(flags);
            COPY_THIS(damage_threshold);
            COPY_THIS(destroyed_effect);
            COPY_THIS(permutations);
            return copy;
        }
    };
    static_assert(sizeof(ModelCollisionGeometryRegion<BigEndian>) == 0x54);

    ENDIAN_TEMPLATE(EndianType) struct ModelCollisionGeometryModifier {
        PAD(0x34);

        ENDIAN_TEMPLATE(NewType) operator ModelCollisionGeometryModifier<NewType>() const noexcept {
            ModelCollisionGeometryModifier<NewType> copy = {};
            return copy;
        }
    };
    static_assert(sizeof(ModelCollisionGeometryModifier<BigEndian>) == 0x34);

    ENDIAN_TEMPLATE(EndianType) struct ModelCollisionGeometrySphere {
        EndianType<std::int16_t> node;
        PAD(0x2);
        PAD(0xC);
        Point3D<EndianType> center;
        EndianType<float> radius;

        ENDIAN_TEMPLATE(NewType) operator ModelCollisionGeometrySphere<NewType>() const noexcept {
            ModelCollisionGeometrySphere<NewType> copy = {};
            COPY_THIS(node);
            COPY_THIS(center);
            COPY_THIS(radius);
            return copy;
        }
    };
    static_assert(sizeof(ModelCollisionGeometrySphere<BigEndian>) == 0x20);

    ENDIAN_TEMPLATE(EndianType) struct ModelCollisionGeometryBSP3DNode {
        EndianType<std::int32_t> plane;
        EndianType<FlaggedInt<std::uint32_t>> back_child;
        EndianType<FlaggedInt<std::uint32_t>> front_child;

        ENDIAN_TEMPLATE(NewType) operator ModelCollisionGeometryBSP3DNode<NewType>() const noexcept {
            ModelCollisionGeometryBSP3DNode<NewType> copy = {};
            COPY_THIS(plane);
            COPY_THIS(back_child);
            COPY_THIS(front_child);
            return copy;
        }
    };
    static_assert(sizeof(ModelCollisionGeometryBSP3DNode<BigEndian>) == 0xC);

    ENDIAN_TEMPLATE(EndianType) struct ModelCollisionGeometryPlane {
        Plane3D<EndianType> plane;

        ENDIAN_TEMPLATE(NewType) operator ModelCollisionGeometryPlane<NewType>() const noexcept {
            ModelCollisionGeometryPlane<NewType> copy;
            COPY_THIS(plane);
            return copy;
        }
    };
    static_assert(sizeof(ModelCollisionGeometryPlane<BigEndian>) == 0x10);

    struct ModelCollisionGeometryLeafFlags {
        std::uint16_t contains_double_sided_surfaces : 1;
    };

    ENDIAN_TEMPLATE(EndianType) struct ModelCollisionGeometryLeaf {
        EndianType<ModelCollisionGeometryLeafFlags> flags;
        EndianType<std::int16_t> bsp2d_reference_count;
        EndianType<std::int32_t> first_bsp2d_reference;

        ENDIAN_TEMPLATE(NewType) operator ModelCollisionGeometryLeaf<NewType>() const noexcept {
            ModelCollisionGeometryLeaf<NewType> copy;
            COPY_THIS(flags);
            COPY_THIS(bsp2d_reference_count);
            COPY_THIS(first_bsp2d_reference);
            return copy;
        }
    };
    static_assert(sizeof(ModelCollisionGeometryLeaf<BigEndian>) == 0x8);

    ENDIAN_TEMPLATE(EndianType) struct ModelCollisionGeometryBSP2DReference {
        EndianType<FlaggedInt<std::uint32_t>> plane;
        EndianType<FlaggedInt<std::uint32_t>> bsp2d_node;

        ENDIAN_TEMPLATE(NewType) operator ModelCollisionGeometryBSP2DReference<NewType>() const noexcept {
            ModelCollisionGeometryBSP2DReference<NewType> copy;
            COPY_THIS(plane);
            COPY_THIS(bsp2d_node);
            return copy;
        }
    };
    static_assert(sizeof(ModelCollisionGeometryBSP2DReference<BigEndian>) == 0x8);

    ENDIAN_TEMPLATE(EndianType) struct ModelCollisionGeometryBSP2DNode {
        Plane2D<EndianType> plane;
        EndianType<FlaggedInt<std::uint32_t>> left_child;
        EndianType<FlaggedInt<std::uint32_t>> right_child;

        ENDIAN_TEMPLATE(NewType) operator ModelCollisionGeometryBSP2DNode<NewType>() const noexcept {
            ModelCollisionGeometryBSP2DNode<NewType> copy;
            COPY_THIS(plane);
            COPY_THIS(left_child);
            COPY_THIS(right_child);
            return copy;
        }
    };
    static_assert(sizeof(ModelCollisionGeometryBSP2DNode<BigEndian>) == 0x14);

    struct ModelCollisionGeometrySurfaceFlags {
        std::uint8_t two_sided : 1;
        std::uint8_t invisible : 1;
        std::uint8_t climbable : 1;
        std::uint8_t breakable : 1;
    };

    ENDIAN_TEMPLATE(EndianType) struct ModelCollisionGeometrySurface {
        EndianType<FlaggedInt<std::uint32_t>> plane;
        EndianType<std::int32_t> first_edge;
        ModelCollisionGeometrySurfaceFlags flags;
        std::int8_t breakable_surface;
        EndianType<std::int16_t> material;

        ENDIAN_TEMPLATE(NewType) operator ModelCollisionGeometrySurface<NewType>() const noexcept {
            ModelCollisionGeometrySurface<NewType> copy;
            COPY_THIS(plane);
            COPY_THIS(first_edge);
            COPY_THIS(flags);
            COPY_THIS(breakable_surface);
            COPY_THIS(material);
            return copy;
        }
    };
    static_assert(sizeof(ModelCollisionGeometrySurface<BigEndian>) == 0xC);

    ENDIAN_TEMPLATE(EndianType) struct ModelCollisionGeometryEdge {
        EndianType<std::int32_t> start_vertex;
        EndianType<std::int32_t> end_vertex;
        EndianType<std::int32_t> forward_edge;
        EndianType<std::int32_t> reverse_edge;
        EndianType<std::int32_t> left_surface;
        EndianType<std::int32_t> right_surface;

        ENDIAN_TEMPLATE(NewType) operator ModelCollisionGeometryEdge<NewType>() const noexcept {
            ModelCollisionGeometryEdge<NewType> copy;
            COPY_THIS(start_vertex);
            COPY_THIS(end_vertex);
            COPY_THIS(forward_edge);
            COPY_THIS(reverse_edge);
            COPY_THIS(left_surface);
            COPY_THIS(right_surface);
            return copy;
        }
    };
    static_assert(sizeof(ModelCollisionGeometryEdge<BigEndian>) == 0x18);

    ENDIAN_TEMPLATE(EndianType) struct ModelCollisionGeometryVertex {
        Point3D<EndianType> point;
        EndianType<std::int32_t> first_edge;

        ENDIAN_TEMPLATE(NewType) operator ModelCollisionGeometryVertex<NewType>() const noexcept {
            ModelCollisionGeometryVertex<NewType> copy;
            COPY_THIS(point);
            COPY_THIS(first_edge);
            return copy;
        }
    };
    static_assert(sizeof(ModelCollisionGeometryVertex<BigEndian>) == 0x10);

    ENDIAN_TEMPLATE(EndianType) struct ModelCollisionGeometryBSP {
        TagReflexive<EndianType, ModelCollisionGeometryBSP3DNode> bsp3d_nodes;
        TagReflexive<EndianType, ModelCollisionGeometryPlane> planes;
        TagReflexive<EndianType, ModelCollisionGeometryLeaf> leaves;
        TagReflexive<EndianType, ModelCollisionGeometryBSP2DReference> bsp2d_references;
        TagReflexive<EndianType, ModelCollisionGeometryBSP2DNode> bsp2d_nodes;
        TagReflexive<EndianType, ModelCollisionGeometrySurface> surfaces;
        TagReflexive<EndianType, ModelCollisionGeometryEdge> edges;
        TagReflexive<EndianType, ModelCollisionGeometryVertex> vertices;

        ENDIAN_TEMPLATE(NewType) operator ModelCollisionGeometryBSP<NewType>() const noexcept {
            ModelCollisionGeometryBSP<NewType> copy;
            COPY_THIS(bsp3d_nodes);
            COPY_THIS(planes);
            COPY_THIS(leaves);
            COPY_THIS(bsp2d_references);
            COPY_THIS(bsp2d_nodes);
            COPY_THIS(surfaces);
            COPY_THIS(edges);
            COPY_THIS(vertices);
            return copy;
        }
    };
    static_assert(sizeof(ModelCollisionGeometryBSP<BigEndian>) == 0x60);

    ENDIAN_TEMPLATE(EndianType) struct ModelCollisionGeometryNode {
        TagString name;
        EndianType<std::int16_t> region;
        EndianType<std::int16_t> parent_node;
        EndianType<std::int16_t> next_sibling_node;
        EndianType<std::int16_t> first_child_node;
        PAD(0xC);
        TagReflexive<EndianType, ModelCollisionGeometryBSP> bsps;

        ENDIAN_TEMPLATE(NewType) operator ModelCollisionGeometryNode<NewType>() const noexcept {
            ModelCollisionGeometryNode<NewType> copy = {};
            COPY_THIS(name);
            COPY_THIS(region);
            COPY_THIS(parent_node);
            COPY_THIS(next_sibling_node);
            COPY_THIS(first_child_node);
            COPY_THIS(bsps);
            return copy;
        }
    };
    static_assert(sizeof(ModelCollisionGeometryNode<BigEndian>) == 0x40);

    struct ModelCollisionGeometryFlags {
        std::uint32_t takes_shield_damage_for_children : 1;
        std::uint32_t takes_body_damage_for_children : 1;
        std::uint32_t always_shields_friendly_damage : 1;
        std::uint32_t passes_area_damage_to_children : 1;
        std::uint32_t parent_never_takes_body_damage_for_us : 1;
        std::uint32_t only_damaged_by_explosives : 1;
        std::uint32_t only_damaged_while_occupied : 1;
    };

    ENDIAN_TEMPLATE(EndianType) struct ModelCollisionGeometry {
        EndianType<ModelCollisionGeometryFlags> flags;
        EndianType<std::int16_t> indirect_damage_material;
        PAD(0x2);
        EndianType<float> maximum_body_vitality;
        EndianType<float> body_system_shock;
        PAD(0x18);
        PAD(0x1C);
        EndianType<Fraction> friendly_damage_resistance;
        PAD(0x8);
        PAD(0x20);
        TagDependency<EndianType> localized_damage_effect; // effect
        EndianType<float> area_damage_effect_threshold;
        TagDependency<EndianType> area_damage_effect; // effect
        EndianType<float> body_damaged_threshold;
        TagDependency<EndianType> body_damaged_effect; // effect
        TagDependency<EndianType> body_depleted_effect; // effect
        EndianType<float> body_destroyed_threshold;
        TagDependency<EndianType> body_destroyed_effect; // effect
        EndianType<float> maximum_shield_vitality;
        PAD(0x2);
        EndianType<MaterialType> shield_material_type;
        PAD(0x18);
        EndianType<FunctionType> shield_failure_function;
        PAD(0x2);
        EndianType<Fraction> shield_failure_threshold;
        EndianType<Fraction> failing_shield_leak_fraction;
        PAD(0x10);
        EndianType<float> minimum_stun_damage;
        EndianType<float> stun_time;
        EndianType<float> recharge_time;
        PAD(0x10);
        PAD(0x60);
        EndianType<float> shield_damaged_threshold;
        TagDependency<EndianType> shield_damaged_effect; // effect
        TagDependency<EndianType> shield_depleted_effect; // effect
        TagDependency<EndianType> shield_recharging_effect; // effect
        PAD(0x8);
        LittleEndian<float> shield_recharge_rate;
        PAD(0x70);
        TagReflexive<EndianType, ModelCollisionGeometryMaterial> materials;
        TagReflexive<EndianType, ModelCollisionGeometryRegion> regions;
        TagReflexive<EndianType, ModelCollisionGeometryModifier> modifiers;
        PAD(0x10);
        Bounds<EndianType<float>> x;
        Bounds<EndianType<float>> y;
        Bounds<EndianType<float>> z;
        TagReflexive<EndianType, ModelCollisionGeometrySphere> pathfinding_spheres;
        TagReflexive<EndianType, ModelCollisionGeometryNode> nodes;

        ENDIAN_TEMPLATE(NewType) operator ModelCollisionGeometry<NewType>() const noexcept {
            ModelCollisionGeometry<NewType> copy = {};
            COPY_THIS(flags);
            COPY_THIS(indirect_damage_material);
            COPY_THIS(maximum_body_vitality);
            COPY_THIS(body_system_shock);
            COPY_THIS(friendly_damage_resistance);
            COPY_THIS(localized_damage_effect);
            COPY_THIS(area_damage_effect_threshold);
            COPY_THIS(area_damage_effect);
            COPY_THIS(body_damaged_threshold);
            COPY_THIS(body_damaged_effect);
            COPY_THIS(body_depleted_effect);
            COPY_THIS(body_destroyed_threshold);
            COPY_THIS(body_destroyed_effect);
            COPY_THIS(maximum_shield_vitality);
            COPY_THIS(shield_material_type);
            COPY_THIS(shield_failure_function);
            COPY_THIS(shield_failure_threshold);
            COPY_THIS(failing_shield_leak_fraction);
            COPY_THIS(minimum_stun_damage);
            COPY_THIS(stun_time);
            COPY_THIS(recharge_time);
            COPY_THIS(shield_damaged_threshold);
            COPY_THIS(shield_damaged_effect);
            COPY_THIS(shield_depleted_effect);
            COPY_THIS(shield_recharging_effect);
            COPY_THIS(shield_recharge_rate);
            COPY_THIS(materials);
            COPY_THIS(regions);
            COPY_THIS(modifiers);
            COPY_THIS(x);
            COPY_THIS(y);
            COPY_THIS(z);
            COPY_THIS(pathfinding_spheres);
            COPY_THIS(nodes);
            return copy;
        }
    };
    static_assert(sizeof(ModelCollisionGeometry<BigEndian>) == 0x298);

    #define ADD_MODEL_COLLISION_BSP(reflexive_struct) ADD_REFLEXIVE_START(reflexive_struct) { \
                                                             ADD_REFLEXIVE(reflexive.bsp3d_nodes); \
                                                             ADD_REFLEXIVE(reflexive.planes); \
                                                             ADD_REFLEXIVE(reflexive.leaves); \
                                                             ADD_REFLEXIVE(reflexive.bsp2d_references); \
                                                             ADD_REFLEXIVE(reflexive.bsp2d_nodes); \
                                                             ADD_REFLEXIVE(reflexive.surfaces); \
                                                             ADD_REFLEXIVE(reflexive.edges); \
                                                             ADD_REFLEXIVE(reflexive.vertices); \
                                                      } ADD_REFLEXIVE_END;

    void compile_model_collision_geometry_tag(CompiledTag &compiled, const std::byte *data, std::size_t size);

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
        const ModelCollisionGeometryPlane<LittleEndian> *planes,
        std::uint32_t plane_count
    );


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
    );
}
#endif
