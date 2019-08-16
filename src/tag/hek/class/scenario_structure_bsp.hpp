/*
 * Invader (c) 2018 Kavawuvi
 *
 * This program is free software under the GNU General Public License v3.0 or later. See LICENSE for more information.
 */

#ifndef INVADER__TAG__HEK__CLASS__SCENARIO_STRUCTURE_BSP_HPP
#define INVADER__TAG__HEK__CLASS__SCENARIO_STRUCTURE_BSP_HPP

#include "../../compiled_tag.hpp"
#include "../../../hek/data_type.hpp"
#include "../header.hpp"

#include "model_collision_geometry.hpp"

namespace Invader::HEK {
    ENDIAN_TEMPLATE(EndianType) struct ScenarioStructureBSPCollisionMaterial {
        TagDependency<EndianType> shader; // shader
        PAD(0x2);
        LittleEndian<MaterialType> material;

        ENDIAN_TEMPLATE(NewType) operator ScenarioStructureBSPCollisionMaterial<NewType>() const noexcept {
            ScenarioStructureBSPCollisionMaterial<NewType> copy = {};
            COPY_THIS(shader);
            COPY_THIS(material);
            return copy;
        }
    };
    static_assert(sizeof(ScenarioStructureBSPCollisionMaterial<BigEndian>) == 0x14);

    ENDIAN_TEMPLATE(EndianType) struct ScenarioStructureBSPNode {
        LittleEndian<std::uint16_t> node_stuff[3];

        ENDIAN_TEMPLATE(NewType) operator ScenarioStructureBSPNode<NewType>() const noexcept {
            ScenarioStructureBSPNode<NewType> copy;
            COPY_THIS_ARRAY(node_stuff);
            return copy;
        }
    };
    static_assert(sizeof(ScenarioStructureBSPNode<BigEndian>) == 0x6);

    ENDIAN_TEMPLATE(EndianType) struct ScenarioStructureBSPLeaf {
        LittleEndian<std::uint16_t> vertices[3];
        PAD(0x2);
        EndianType<std::int16_t> cluster;
        EndianType<std::int16_t> surface_reference_count;
        EndianType<std::int32_t> surface_references;

        ENDIAN_TEMPLATE(NewType) operator ScenarioStructureBSPLeaf<NewType>() const noexcept {
            ScenarioStructureBSPLeaf<NewType> copy = {};
            COPY_THIS_ARRAY(vertices);
            COPY_THIS(cluster);
            COPY_THIS(surface_reference_count);
            COPY_THIS(surface_references);
            return copy;
        }
    };
    static_assert(sizeof(ScenarioStructureBSPLeaf<BigEndian>) == 0x10);

    ENDIAN_TEMPLATE(EndianType) struct ScenarioStructureBSPSurfaceReference {
        EndianType<std::int32_t> surface;
        EndianType<std::int32_t> node;

        ENDIAN_TEMPLATE(NewType) operator ScenarioStructureBSPSurfaceReference<NewType>() const noexcept {
            ScenarioStructureBSPSurfaceReference<NewType> copy;
            COPY_THIS(surface);
            COPY_THIS(node);
            return copy;
        }
    };
    static_assert(sizeof(ScenarioStructureBSPSurfaceReference<BigEndian>) == 0x8);

    ENDIAN_TEMPLATE(EndianType) struct ScenarioStructureBSPArrayVertex {
        EndianType<std::int16_t> a;

        ENDIAN_TEMPLATE(NewType) operator ScenarioStructureBSPArrayVertex<NewType>() const noexcept {
            ScenarioStructureBSPArrayVertex<NewType> copy;
            COPY_THIS(a);
            return copy;
        }
    };
    ENDIAN_TEMPLATE(EndianType) struct ScenarioStructureBSPSurface {
        ScenarioStructureBSPArrayVertex<EndianType> vertices[3];

        ENDIAN_TEMPLATE(NewType) operator ScenarioStructureBSPSurface<NewType>() const noexcept {
            ScenarioStructureBSPSurface<NewType> copy;
            COPY_THIS_ARRAY(vertices);
            return copy;
        }
    };
    static_assert(sizeof(ScenarioStructureBSPSurface<BigEndian>) == 0x6);

    ENDIAN_TEMPLATE(EndianType) struct ScenarioStructureBSPArrayVertexBuffer {
        PAD(0x4);
        EndianType<std::int32_t> count;
        EndianType<std::int32_t> offset;
        PAD(0x8);

        ENDIAN_TEMPLATE(NewType) operator ScenarioStructureBSPArrayVertexBuffer<NewType>() const noexcept {
            ScenarioStructureBSPArrayVertexBuffer<NewType> copy = {};
            COPY_THIS(count);
            COPY_THIS(offset);
            return copy;
        }
    };
    static_assert(sizeof(ScenarioStructureBSPArrayVertexBuffer<BigEndian>) == 0x14);

    ENDIAN_TEMPLATE(EndianType) struct ScenarioStructureBSPMaterialUncompressedRenderedVertex {
        Point3D<EndianType> position;
        Vector3D<EndianType> normal;
        Vector3D<EndianType> binormal;
        Vector3D<EndianType> tangent;
        Point2D<EndianType> texture_coords;

        ENDIAN_TEMPLATE(NewType) operator ScenarioStructureBSPMaterialUncompressedRenderedVertex<NewType>() const noexcept {
            ScenarioStructureBSPMaterialUncompressedRenderedVertex<NewType> copy = {};
            COPY_THIS(position);
            COPY_THIS(normal);
            COPY_THIS(binormal);
            COPY_THIS(tangent);
            COPY_THIS(texture_coords);
            return copy;
        }
    };
    static_assert(sizeof(ScenarioStructureBSPMaterialUncompressedRenderedVertex<BigEndian>) == 0x38);

    ENDIAN_TEMPLATE(EndianType) struct ScenarioStructureBSPMaterialUncompressedLightmapVertex {
        Vector3D<EndianType> normal;
        Point2D<EndianType> texture_coords;

        ENDIAN_TEMPLATE(NewType) operator ScenarioStructureBSPMaterialUncompressedLightmapVertex<NewType>() const noexcept {
            ScenarioStructureBSPMaterialUncompressedLightmapVertex<NewType> copy = {};
            COPY_THIS(normal);
            COPY_THIS(texture_coords);
            return copy;
        }
    };
    static_assert(sizeof(ScenarioStructureBSPMaterialUncompressedLightmapVertex<BigEndian>) == 0x14);

    struct ScenarioStructureBSPMaterialFlags {
        std::uint16_t coplanar : 1;
        std::uint16_t fog_plane : 1;
    };

    ENDIAN_TEMPLATE(EndianType) struct ScenarioStructureBSPMaterial {
        TagDependency<EndianType> shader; // shader
        EndianType<std::int16_t> shader_permutation;
        EndianType<ScenarioStructureBSPMaterialFlags> flags;
        EndianType<std::int32_t> surfaces;
        EndianType<std::int32_t> surface_count;
        Point3D<EndianType> centroid;
        ColorRGB<EndianType> ambient_color;
        EndianType<std::int16_t> distant_light_count;
        PAD(0x2);
        ColorRGB<EndianType> distant_light_0_color;
        Vector3D<EndianType> distant_light_0_direction;
        ColorRGB<EndianType> distant_light_1_color;
        Vector3D<EndianType> distant_light_1_direction;
        PAD(0xC);
        ColorARGB<EndianType> reflection_tint;
        Vector3D<EndianType> shadow_vector;
        ColorRGB<EndianType> shadow_color;
        Plane3D<EndianType> plane;
        EndianType<std::int16_t> breakable_surface;
        PAD(0x2);
        ScenarioStructureBSPArrayVertexBuffer<EndianType> rendered_vertices;
        ScenarioStructureBSPArrayVertexBuffer<EndianType> lightmap_vertices;
        TagDataOffset<EndianType> uncompressed_vertices;
        TagDataOffset<EndianType> compressed_vertices;

        ENDIAN_TEMPLATE(NewType) operator ScenarioStructureBSPMaterial<NewType>() const noexcept {
            ScenarioStructureBSPMaterial<NewType> copy = {};
            COPY_THIS(shader);
            COPY_THIS(shader_permutation);
            COPY_THIS(flags);
            COPY_THIS(surfaces);
            COPY_THIS(surface_count);
            COPY_THIS(centroid);
            COPY_THIS(ambient_color);
            COPY_THIS(distant_light_count);
            COPY_THIS(distant_light_0_color);
            COPY_THIS(distant_light_0_direction);
            COPY_THIS(distant_light_1_color);
            COPY_THIS(distant_light_1_direction);
            COPY_THIS(reflection_tint);
            COPY_THIS(shadow_vector);
            COPY_THIS(shadow_color);
            COPY_THIS(plane);
            COPY_THIS(breakable_surface);
            COPY_THIS(rendered_vertices);
            COPY_THIS(lightmap_vertices);
            COPY_THIS(uncompressed_vertices);
            COPY_THIS(compressed_vertices);
            return copy;
        }
    };
    static_assert(sizeof(ScenarioStructureBSPMaterial<BigEndian>) == 0x100);

    ENDIAN_TEMPLATE(EndianType) struct ScenarioStructureBSPLightmap {
        EndianType<std::int16_t> bitmap;
        PAD(0x2);
        PAD(0x10);
        TagReflexive<EndianType, ScenarioStructureBSPMaterial> materials;

        ENDIAN_TEMPLATE(NewType) operator ScenarioStructureBSPLightmap<NewType>() const noexcept {
            ScenarioStructureBSPLightmap<NewType> copy = {};
            COPY_THIS(bitmap);
            COPY_THIS(materials);
            return copy;
        }
    };
    static_assert(sizeof(ScenarioStructureBSPLightmap<BigEndian>) == 0x20);

    SINGLE_DEPENDENCY_STRUCT(ScenarioStructureBSPLensFlare, lens);

    ENDIAN_TEMPLATE(EndianType) struct ScenarioStructureBSPLensFlareMarker {
        Point3D<EndianType> position;
        std::int8_t direction_i_component;
        std::int8_t direction_j_component;
        std::int8_t direction_k_component;
        std::int8_t lens_flare_index;

        ENDIAN_TEMPLATE(NewType) operator ScenarioStructureBSPLensFlareMarker<NewType>() const noexcept {
            ScenarioStructureBSPLensFlareMarker<NewType> copy = {};
            COPY_THIS(position);
            COPY_THIS(direction_i_component);
            COPY_THIS(direction_j_component);
            COPY_THIS(direction_k_component);
            COPY_THIS(lens_flare_index);
            return copy;
        }
    };
    static_assert(sizeof(ScenarioStructureBSPLensFlareMarker<BigEndian>) == 0x10);

    ENDIAN_TEMPLATE(EndianType) struct ScenarioStructureBSPSubclusterSurfaceIndex {
        EndianType<std::int32_t> index;

        ENDIAN_TEMPLATE(NewType) operator ScenarioStructureBSPSubclusterSurfaceIndex<NewType>() const noexcept {
            ScenarioStructureBSPSubclusterSurfaceIndex<NewType> copy;
            COPY_THIS(index);
            return copy;
        }
    };
    static_assert(sizeof(ScenarioStructureBSPSubclusterSurfaceIndex<BigEndian>) == 0x4);

    ENDIAN_TEMPLATE(EndianType) struct ScenarioStructureBSPSubcluster {
        Bounds<EndianType<float>> world_bounds_x;
        Bounds<EndianType<float>> world_bounds_y;
        Bounds<EndianType<float>> world_bounds_z;
        TagReflexive<EndianType, ScenarioStructureBSPSubclusterSurfaceIndex> surface_indices;

        ENDIAN_TEMPLATE(NewType) operator ScenarioStructureBSPSubcluster<NewType>() const noexcept {
            ScenarioStructureBSPSubcluster<NewType> copy;
            COPY_THIS(world_bounds_x);
            COPY_THIS(world_bounds_y);
            COPY_THIS(world_bounds_z);
            COPY_THIS(surface_indices);
            return copy;
        }
    };
    static_assert(sizeof(ScenarioStructureBSPSubcluster<BigEndian>) == 0x24);

    ENDIAN_TEMPLATE(EndianType) struct ScenarioStructureBSPClusterSurfaceIndex {
        EndianType<std::int32_t> index;

        ENDIAN_TEMPLATE(NewType) operator ScenarioStructureBSPClusterSurfaceIndex<NewType>() const noexcept {
            ScenarioStructureBSPClusterSurfaceIndex<NewType> copy;
            COPY_THIS(index);
            return copy;
        }
    };
    static_assert(sizeof(ScenarioStructureBSPClusterSurfaceIndex<BigEndian>) == 0x4);

    ENDIAN_TEMPLATE(EndianType) struct ScenarioStructureBSPMirrorVertex {
        Point3D<EndianType> point;

        ENDIAN_TEMPLATE(NewType) operator ScenarioStructureBSPMirrorVertex<NewType>() const noexcept {
            ScenarioStructureBSPMirrorVertex<NewType> copy;
            COPY_THIS(point);
            return copy;
        }
    };
    static_assert(sizeof(ScenarioStructureBSPMirrorVertex<BigEndian>) == 0xC);

    ENDIAN_TEMPLATE(EndianType) struct ScenarioStructureBSPMirror {
        Plane3D<EndianType> plane;
        PAD(0x14);
        TagDependency<EndianType> shader; // shader
        TagReflexive<EndianType, ScenarioStructureBSPMirrorVertex> vertices;

        ENDIAN_TEMPLATE(NewType) operator ScenarioStructureBSPMirror<NewType>() const noexcept {
            ScenarioStructureBSPMirror<NewType> copy = {};
            COPY_THIS(plane);
            COPY_THIS(shader);
            COPY_THIS(vertices);
            return copy;
        }
    };
    static_assert(sizeof(ScenarioStructureBSPMirror<BigEndian>) == 0x40);

    ENDIAN_TEMPLATE(EndianType) struct ScenarioStructureBSPClusterPortalIndex {
        EndianType<std::int16_t> portal;

        ENDIAN_TEMPLATE(NewType) operator ScenarioStructureBSPClusterPortalIndex<NewType>() const noexcept {
            ScenarioStructureBSPClusterPortalIndex<NewType> copy;
            COPY_THIS(portal);
            return copy;
        }
    };
    static_assert(sizeof(ScenarioStructureBSPClusterPortalIndex<BigEndian>) == 0x2);

    ENDIAN_TEMPLATE(EndianType) struct ScenarioStructureBSPCluster {
        EndianType<std::int16_t> sky;
        EndianType<std::int16_t> fog;
        EndianType<std::int16_t> background_sound;
        EndianType<std::int16_t> sound_environment;
        EndianType<std::int16_t> weather;
        EndianType<std::int16_t> transition_structure_bsp;
        LittleEndian<std::int16_t> first_decal_index;
        LittleEndian<std::int16_t> decal_count;
        PAD(0x18);
        TagReflexive<EndianType, PredictedResource> predicted_resources;
        TagReflexive<EndianType, ScenarioStructureBSPSubcluster> subclusters;
        EndianType<std::int16_t> first_lens_flare_marker_index;
        EndianType<std::int16_t> lens_flare_marker_count;
        TagReflexive<EndianType, ScenarioStructureBSPClusterSurfaceIndex> surface_indices;
        TagReflexive<EndianType, ScenarioStructureBSPMirror> mirrors;
        TagReflexive<EndianType, ScenarioStructureBSPClusterPortalIndex> portals;

        ENDIAN_TEMPLATE(NewType) operator ScenarioStructureBSPCluster<NewType>() const noexcept {
            ScenarioStructureBSPCluster<NewType> copy = {};
            COPY_THIS(sky);
            COPY_THIS(fog);
            COPY_THIS(background_sound);
            COPY_THIS(sound_environment);
            COPY_THIS(weather);
            COPY_THIS(transition_structure_bsp);
            COPY_THIS(predicted_resources);
            COPY_THIS(subclusters);
            COPY_THIS(first_lens_flare_marker_index);
            COPY_THIS(lens_flare_marker_count);
            COPY_THIS(surface_indices);
            COPY_THIS(mirrors);
            COPY_THIS(portals);
            return copy;
        }
    };
    static_assert(sizeof(ScenarioStructureBSPCluster<BigEndian>) == 0x68);

    ENDIAN_TEMPLATE(EndianType) struct ScenarioStructureBSPClusterPortalVertex {
        Point3D<EndianType> point;

        ENDIAN_TEMPLATE(NewType) operator ScenarioStructureBSPClusterPortalVertex<NewType>() const noexcept {
            ScenarioStructureBSPClusterPortalVertex<NewType> copy;
            COPY_THIS(point);
            return copy;
        }
    };
    static_assert(sizeof(ScenarioStructureBSPClusterPortalVertex<BigEndian>) == 0xC);

    struct ScenarioStructureBSPClusterPortalFlags {
        std::uint32_t ai_can_simply_not_hear_through_all_this_amazing_stuff_darn_it : 1;
    };

    ENDIAN_TEMPLATE(EndianType) struct ScenarioStructureBSPClusterPortal {
        EndianType<std::int16_t> front_cluster;
        EndianType<std::int16_t> back_cluster;
        EndianType<std::int32_t> plane_index;
        Point3D<EndianType> centroid;
        EndianType<float> bounding_radius;
        EndianType<ScenarioStructureBSPClusterPortalFlags> flags;
        PAD(0x18);
        TagReflexive<EndianType, ScenarioStructureBSPClusterPortalVertex> vertices;

        ENDIAN_TEMPLATE(NewType) operator ScenarioStructureBSPClusterPortal<NewType>() const noexcept {
            ScenarioStructureBSPClusterPortal<NewType> copy = {};
            COPY_THIS(front_cluster);
            COPY_THIS(back_cluster);
            COPY_THIS(plane_index);
            COPY_THIS(centroid);
            COPY_THIS(bounding_radius);
            COPY_THIS(flags);
            COPY_THIS(vertices);
            return copy;
        }
    };
    static_assert(sizeof(ScenarioStructureBSPClusterPortal<BigEndian>) == 0x40);

    ENDIAN_TEMPLATE(EndianType) struct ScenarioStructureBSPBreakableSurface {
        Point3D<EndianType> centroid;
        EndianType<float> radius;
        EndianType<std::int32_t> collision_surface_index;
        PAD(0x1C);

        ENDIAN_TEMPLATE(NewType) operator ScenarioStructureBSPBreakableSurface<NewType>() const noexcept {
            ScenarioStructureBSPBreakableSurface<NewType> copy = {};
            COPY_THIS(centroid);
            COPY_THIS(radius);
            COPY_THIS(collision_surface_index);
            return copy;
        }
    };
    static_assert(sizeof(ScenarioStructureBSPBreakableSurface<BigEndian>) == 0x30);

    ENDIAN_TEMPLATE(EndianType) struct ScenarioStructureBSPFogPlaneVertex {
        Point3D<EndianType> point;

        ENDIAN_TEMPLATE(NewType) operator ScenarioStructureBSPFogPlaneVertex<NewType>() const noexcept {
            ScenarioStructureBSPFogPlaneVertex<NewType> copy;
            COPY_THIS(point);
            return copy;
        }
    };
    static_assert(sizeof(ScenarioStructureBSPFogPlaneVertex<BigEndian>) == 0xC);

    ENDIAN_TEMPLATE(EndianType) struct ScenarioStructureBSPFogPlane {
        EndianType<std::int16_t> front_region;
        LittleEndian<std::int16_t> material_type;
        Plane3D<EndianType> plane;
        TagReflexive<EndianType, ScenarioStructureBSPFogPlaneVertex> vertices;

        ENDIAN_TEMPLATE(NewType) operator ScenarioStructureBSPFogPlane<NewType>() const noexcept {
            ScenarioStructureBSPFogPlane<NewType> copy = {};
            COPY_THIS(front_region);
            COPY_THIS(material_type);
            COPY_THIS(plane);
            COPY_THIS(vertices);
            return copy;
        }
    };
    static_assert(sizeof(ScenarioStructureBSPFogPlane<BigEndian>) == 0x20);

    ENDIAN_TEMPLATE(EndianType) struct ScenarioStructureBSPFogRegion {
        PAD(0x24);
        EndianType<std::int16_t> fog_palette;
        EndianType<std::int16_t> weather_palette;

        ENDIAN_TEMPLATE(NewType) operator ScenarioStructureBSPFogRegion<NewType>() const noexcept {
            ScenarioStructureBSPFogRegion<NewType> copy = {};
            COPY_THIS(fog_palette);
            COPY_THIS(weather_palette);
            return copy;
        }
    };
    static_assert(sizeof(ScenarioStructureBSPFogRegion<BigEndian>) == 0x28);

    ENDIAN_TEMPLATE(EndianType) struct ScenarioStructureBSPFogPalette {
        TagString name;
        TagDependency<EndianType> fog; // fog
        PAD(0x4);
        TagString fog_scale_function;
        PAD(0x34);

        ENDIAN_TEMPLATE(NewType) operator ScenarioStructureBSPFogPalette<NewType>() const noexcept {
            ScenarioStructureBSPFogPalette<NewType> copy = {};
            COPY_THIS(name);
            COPY_THIS(fog);
            COPY_THIS(fog_scale_function);
            return copy;
        }
    };
    static_assert(sizeof(ScenarioStructureBSPFogPalette<BigEndian>) == 0x88);

    ENDIAN_TEMPLATE(EndianType) struct ScenarioStructureBSPWeatherPalette {
        TagString name;
        TagDependency<EndianType> particle_system; // weather_system
        PAD(0x4);
        TagString particle_system_scale_function;
        PAD(0x2C);
        TagDependency<EndianType> wind; // wind
        Vector3D<EndianType> wind_direction;
        EndianType<float> wind_magnitude;
        PAD(0x4);
        TagString wind_scale_function;
        PAD(0x2C);

        ENDIAN_TEMPLATE(NewType) operator ScenarioStructureBSPWeatherPalette<NewType>() const noexcept {
            ScenarioStructureBSPWeatherPalette<NewType> copy = {};
            COPY_THIS(name);
            COPY_THIS(particle_system);
            COPY_THIS(particle_system_scale_function);
            COPY_THIS(wind);
            COPY_THIS(wind_direction);
            COPY_THIS(wind_magnitude);
            COPY_THIS(wind_scale_function);
            return copy;
        }
    };
    static_assert(sizeof(ScenarioStructureBSPWeatherPalette<BigEndian>) == 0xF0);

    ENDIAN_TEMPLATE(EndianType) struct ScenarioStructureBSPWeatherPolyhedronPlane {
        Plane3D<EndianType> plane;

        ENDIAN_TEMPLATE(NewType) operator ScenarioStructureBSPWeatherPolyhedronPlane<NewType>() const noexcept {
            ScenarioStructureBSPWeatherPolyhedronPlane<NewType> copy;
            COPY_THIS(plane);
            return copy;
        }
    };
    static_assert(sizeof(ScenarioStructureBSPWeatherPolyhedronPlane<BigEndian>) == 0x10);

    ENDIAN_TEMPLATE(EndianType) struct ScenarioStructureBSPWeatherPolyhedron {
        Point3D<EndianType> bounding_sphere_center;
        EndianType<float> bounding_sphere_radius;
        PAD(0x4);
        TagReflexive<EndianType, ScenarioStructureBSPWeatherPolyhedronPlane> planes;

        ENDIAN_TEMPLATE(NewType) operator ScenarioStructureBSPWeatherPolyhedron<NewType>() const noexcept {
            ScenarioStructureBSPWeatherPolyhedron<NewType> copy = {};
            COPY_THIS(bounding_sphere_center);
            COPY_THIS(bounding_sphere_radius);
            COPY_THIS(planes);
            return copy;
        }
    };
    static_assert(sizeof(ScenarioStructureBSPWeatherPolyhedron<BigEndian>) == 0x20);

    ENDIAN_TEMPLATE(EndianType) struct ScenarioStructureBSPPathfindingSurface {
        std::int8_t data;

        ENDIAN_TEMPLATE(NewType) operator ScenarioStructureBSPPathfindingSurface<NewType>() const noexcept {
            ScenarioStructureBSPPathfindingSurface<NewType> copy;
            COPY_THIS(data);
            return copy;
        }
    };
    static_assert(sizeof(ScenarioStructureBSPPathfindingSurface<BigEndian>) == 0x1);

    ENDIAN_TEMPLATE(EndianType) struct ScenarioStructureBSPPathfindingEdge {
        std::int8_t midpoint;

        ENDIAN_TEMPLATE(NewType) operator ScenarioStructureBSPPathfindingEdge<NewType>() const noexcept {
            ScenarioStructureBSPPathfindingEdge<NewType> copy;
            COPY_THIS(midpoint);
            return copy;
        }
    };
    static_assert(sizeof(ScenarioStructureBSPPathfindingEdge<BigEndian>) == 0x1);

    ENDIAN_TEMPLATE(EndianType) struct ScenarioStructureBSPBackgroundSoundPalette {
        TagString name;
        TagDependency<EndianType> background_sound; // sound_looping
        PAD(0x4);
        TagString scale_function;
        PAD(0x20);

        ENDIAN_TEMPLATE(NewType) operator ScenarioStructureBSPBackgroundSoundPalette<NewType>() const noexcept {
            ScenarioStructureBSPBackgroundSoundPalette<NewType> copy = {};
            COPY_THIS(name);
            COPY_THIS(background_sound);
            COPY_THIS(scale_function);
            return copy;
        }
    };
    static_assert(sizeof(ScenarioStructureBSPBackgroundSoundPalette<BigEndian>) == 0x74);

    ENDIAN_TEMPLATE(EndianType) struct ScenarioStructureBSPSoundEnvironmentPalette {
        TagString name;
        TagDependency<EndianType> sound_environment; // sound_environment
        PAD(0x20);

        ENDIAN_TEMPLATE(NewType) operator ScenarioStructureBSPSoundEnvironmentPalette<NewType>() const noexcept {
            ScenarioStructureBSPSoundEnvironmentPalette<NewType> copy = {};
            COPY_THIS(name);
            COPY_THIS(sound_environment);
            return copy;
        }
    };
    static_assert(sizeof(ScenarioStructureBSPSoundEnvironmentPalette<BigEndian>) == 0x50);

    ENDIAN_TEMPLATE(EndianType) struct ScenarioStructureBSPMarker {
        TagString name;
        Quaternion<EndianType> rotation;
        Point3D<EndianType> position;

        ENDIAN_TEMPLATE(NewType) operator ScenarioStructureBSPMarker<NewType>() const noexcept {
            ScenarioStructureBSPMarker<NewType> copy;
            COPY_THIS(name);
            COPY_THIS(rotation);
            COPY_THIS(position);
            return copy;
        }
    };
    static_assert(sizeof(ScenarioStructureBSPMarker<BigEndian>) == 0x3C);

    ENDIAN_TEMPLATE(EndianType) struct ScenarioStructureBSPGlobalDetailObjectCell {
        EndianType<std::int16_t> no_name;
        EndianType<std::int16_t> no_name_1;
        EndianType<std::int16_t> no_name_2;
        EndianType<std::int16_t> no_name_3;
        EndianType<std::int32_t> no_name_4;
        EndianType<std::int32_t> no_name_5;
        EndianType<std::int32_t> no_name_6;
        PAD(0xC);

        ENDIAN_TEMPLATE(NewType) operator ScenarioStructureBSPGlobalDetailObjectCell<NewType>() const noexcept {
            ScenarioStructureBSPGlobalDetailObjectCell<NewType> copy = {};
            COPY_THIS(no_name);
            COPY_THIS(no_name_1);
            COPY_THIS(no_name_2);
            COPY_THIS(no_name_3);
            COPY_THIS(no_name_4);
            COPY_THIS(no_name_5);
            COPY_THIS(no_name_6);
            return copy;
        }
    };
    static_assert(sizeof(ScenarioStructureBSPGlobalDetailObjectCell<BigEndian>) == 0x20);

    ENDIAN_TEMPLATE(EndianType) struct ScenarioStructureBSPGlobalDetailObject {
        std::int8_t no_name;
        std::int8_t no_name_1;
        std::int8_t no_name_2;
        std::int8_t no_name_3;
        EndianType<std::int16_t> no_name_4;

        ENDIAN_TEMPLATE(NewType) operator ScenarioStructureBSPGlobalDetailObject<NewType>() const noexcept {
            ScenarioStructureBSPGlobalDetailObject<NewType> copy;
            COPY_THIS(no_name);
            COPY_THIS(no_name_1);
            COPY_THIS(no_name_2);
            COPY_THIS(no_name_3);
            COPY_THIS(no_name_4);
            return copy;
        }
    };
    static_assert(sizeof(ScenarioStructureBSPGlobalDetailObject<BigEndian>) == 0x6);

    ENDIAN_TEMPLATE(EndianType) struct ScenarioStructureBSPGlobalDetailObjectCount {
        EndianType<std::int16_t> no_name;

        ENDIAN_TEMPLATE(NewType) operator ScenarioStructureBSPGlobalDetailObjectCount<NewType>() const noexcept {
            ScenarioStructureBSPGlobalDetailObjectCount<NewType> copy;
            COPY_THIS(no_name);
            return copy;
        }
    };
    static_assert(sizeof(ScenarioStructureBSPGlobalDetailObjectCount<BigEndian>) == 0x2);

    ENDIAN_TEMPLATE(EndianType) struct ScenarioStructureBSPGlobalZReferenceVector {
        EndianType<float> no_name;
        EndianType<float> no_name_1;
        EndianType<float> no_name_2;
        EndianType<float> no_name_3;

        ENDIAN_TEMPLATE(NewType) operator ScenarioStructureBSPGlobalZReferenceVector<NewType>() const noexcept {
            ScenarioStructureBSPGlobalZReferenceVector<NewType> copy;
            COPY_THIS(no_name);
            COPY_THIS(no_name_1);
            COPY_THIS(no_name_2);
            COPY_THIS(no_name_3);
            return copy;
        }
    };
    static_assert(sizeof(ScenarioStructureBSPGlobalZReferenceVector<BigEndian>) == 0x10);

    ENDIAN_TEMPLATE(EndianType) struct ScenarioStructureBSPDetailObjectData {
        TagReflexive<EndianType, ScenarioStructureBSPGlobalDetailObjectCell> cells;
        TagReflexive<EndianType, ScenarioStructureBSPGlobalDetailObject> instances;
        TagReflexive<EndianType, ScenarioStructureBSPGlobalDetailObjectCount> counts;
        TagReflexive<EndianType, ScenarioStructureBSPGlobalZReferenceVector> z_reference_vectors;
        std::uint8_t bullshit; // 1 = enabled lol
        PAD(0x3);
        PAD(0xC);

        ENDIAN_TEMPLATE(NewType) operator ScenarioStructureBSPDetailObjectData<NewType>() const noexcept {
            ScenarioStructureBSPDetailObjectData<NewType> copy = {};
            COPY_THIS(cells);
            COPY_THIS(instances);
            COPY_THIS(counts);
            COPY_THIS(z_reference_vectors);
            COPY_THIS(bullshit);
            return copy;
        }
    };
    static_assert(sizeof(ScenarioStructureBSPDetailObjectData<BigEndian>) == 0x40);

    ENDIAN_TEMPLATE(EndianType) struct ScenarioStructureBSPRuntimeDecal {
        Point3D<EndianType> position;
        EndianType<std::int16_t> decal_type;
        std::int8_t yaw;
        std::int8_t pitch;

        ENDIAN_TEMPLATE(NewType) operator ScenarioStructureBSPRuntimeDecal<NewType>() const noexcept {
            ScenarioStructureBSPRuntimeDecal<NewType> copy;
            COPY_THIS(decal_type);
            COPY_THIS(yaw);
            COPY_THIS(pitch);
            COPY_THIS(position);
            return copy;
        }
    };
    static_assert(sizeof(ScenarioStructureBSPRuntimeDecal<BigEndian>) == 0x10);

    ENDIAN_TEMPLATE(EndianType) struct ScenarioStructureBSPMapLeafFaceVertex {
        Point2D<EndianType> vertex;

        ENDIAN_TEMPLATE(NewType) operator ScenarioStructureBSPMapLeafFaceVertex<NewType>() const noexcept {
            ScenarioStructureBSPMapLeafFaceVertex<NewType> copy;
            COPY_THIS(vertex);
            return copy;
        }
    };
    static_assert(sizeof(ScenarioStructureBSPMapLeafFaceVertex<BigEndian>) == 0x8);

    ENDIAN_TEMPLATE(EndianType) struct ScenarioStructureBSPMapLeafFace {
        EndianType<std::int32_t> node_index;
        TagReflexive<EndianType, ScenarioStructureBSPMapLeafFaceVertex> vertices;

        ENDIAN_TEMPLATE(NewType) operator ScenarioStructureBSPMapLeafFace<NewType>() const noexcept {
            ScenarioStructureBSPMapLeafFace<NewType> copy;
            COPY_THIS(node_index);
            COPY_THIS(vertices);
            return copy;
        }
    };
    static_assert(sizeof(ScenarioStructureBSPMapLeafFace<BigEndian>) == 0x10);

    ENDIAN_TEMPLATE(EndianType) struct ScenarioStructureBSPMapLeafPortalIndex {
        EndianType<std::int32_t> portal_index;

        ENDIAN_TEMPLATE(NewType) operator ScenarioStructureBSPMapLeafPortalIndex<NewType>() const noexcept {
            ScenarioStructureBSPMapLeafPortalIndex<NewType> copy;
            COPY_THIS(portal_index);
            return copy;
        }
    };
    static_assert(sizeof(ScenarioStructureBSPMapLeafPortalIndex<BigEndian>) == 0x4);

    ENDIAN_TEMPLATE(EndianType) struct ScenarioStructureBSPGlobalMapLeaf {
        TagReflexive<EndianType, ScenarioStructureBSPMapLeafFace> faces;
        TagReflexive<EndianType, ScenarioStructureBSPMapLeafPortalIndex> portal_indices;

        ENDIAN_TEMPLATE(NewType) operator ScenarioStructureBSPGlobalMapLeaf<NewType>() const noexcept {
            ScenarioStructureBSPGlobalMapLeaf<NewType> copy;
            COPY_THIS(faces);
            COPY_THIS(portal_indices);
            return copy;
        }
    };
    static_assert(sizeof(ScenarioStructureBSPGlobalMapLeaf<BigEndian>) == 0x18);

    ENDIAN_TEMPLATE(EndianType) struct ScenarioStructureBSPLeafPortalVertex {
        Point3D<EndianType> point;

        ENDIAN_TEMPLATE(NewType) operator ScenarioStructureBSPLeafPortalVertex<NewType>() const noexcept {
            ScenarioStructureBSPLeafPortalVertex<NewType> copy;
            COPY_THIS(point);
            return copy;
        }
    };
    static_assert(sizeof(ScenarioStructureBSPLeafPortalVertex<BigEndian>) == 0xC);

    ENDIAN_TEMPLATE(EndianType) struct ScenarioStructureBSPGlobalLeafPortal {
        EndianType<std::int32_t> plane_index;
        EndianType<std::int32_t> back_leaf_index;
        EndianType<std::int32_t> front_leaf_index;
        TagReflexive<EndianType, ScenarioStructureBSPLeafPortalVertex> vertices;

        ENDIAN_TEMPLATE(NewType) operator ScenarioStructureBSPGlobalLeafPortal<NewType>() const noexcept {
            ScenarioStructureBSPGlobalLeafPortal<NewType> copy;
            COPY_THIS(plane_index);
            COPY_THIS(back_leaf_index);
            COPY_THIS(front_leaf_index);
            COPY_THIS(vertices);
            return copy;
        }
    };
    static_assert(sizeof(ScenarioStructureBSPGlobalLeafPortal<BigEndian>) == 0x18);

    ENDIAN_TEMPLATE(EndianType) struct ScenarioStructureBSP {
        TagDependency<EndianType> lightmaps; // bitmap
        EndianType<float> vehicle_floor;
        EndianType<float> vehicle_ceiling;
        PAD(0x14);
        ColorRGB<EndianType> default_ambient_color;
        PAD(0x4);
        ColorRGB<EndianType> default_distant_light_0_color;
        Vector3D<EndianType> default_distant_light_0_direction;
        ColorRGB<EndianType> default_distant_light_1_color;
        Vector3D<EndianType> default_distant_light_1_direction;
        PAD(0xC);
        ColorARGB<EndianType> default_reflection_tint;
        Vector3D<EndianType> default_shadow_vector;
        ColorRGB<EndianType> default_shadow_color;
        PAD(0x4);
        TagReflexive<EndianType, ScenarioStructureBSPCollisionMaterial> collision_materials;
        TagReflexive<EndianType, ModelCollisionGeometryBSP> collision_bsp;
        TagReflexive<EndianType, ScenarioStructureBSPNode> nodes;
        Bounds<EndianType<float>> world_bounds_x;
        Bounds<EndianType<float>> world_bounds_y;
        Bounds<EndianType<float>> world_bounds_z;
        TagReflexive<EndianType, ScenarioStructureBSPLeaf> leaves;
        TagReflexive<EndianType, ScenarioStructureBSPSurfaceReference> leaf_surfaces;
        TagReflexive<EndianType, ScenarioStructureBSPSurface> surfaces;
        TagReflexive<EndianType, ScenarioStructureBSPLightmap> lightmaps_1;
        PAD(0xC);
        TagReflexive<EndianType, ScenarioStructureBSPLensFlare> lens_flares;
        TagReflexive<EndianType, ScenarioStructureBSPLensFlareMarker> lens_flare_markers;
        TagReflexive<EndianType, ScenarioStructureBSPCluster> clusters;
        TagDataOffset<EndianType> cluster_data;
        TagReflexive<EndianType, ScenarioStructureBSPClusterPortal> cluster_portals;
        PAD(0xC);
        TagReflexive<EndianType, ScenarioStructureBSPBreakableSurface> breakable_surfaces;
        TagReflexive<EndianType, ScenarioStructureBSPFogPlane> fog_planes;
        TagReflexive<EndianType, ScenarioStructureBSPFogRegion> fog_regions;
        TagReflexive<EndianType, ScenarioStructureBSPFogPalette> fog_palette;
        PAD(0x18);
        TagReflexive<EndianType, ScenarioStructureBSPWeatherPalette> weather_palette;
        TagReflexive<EndianType, ScenarioStructureBSPWeatherPolyhedron> weather_polyhedra;
        PAD(0x18);
        TagReflexive<EndianType, ScenarioStructureBSPPathfindingSurface> pathfinding_surfaces;
        TagReflexive<EndianType, ScenarioStructureBSPPathfindingEdge> pathfinding_edges;
        TagReflexive<EndianType, ScenarioStructureBSPBackgroundSoundPalette> background_sound_palette;
        TagReflexive<EndianType, ScenarioStructureBSPSoundEnvironmentPalette> sound_environment_palette;
        TagDataOffset<EndianType> sound_pas_data;
        PAD(0x18);
        TagReflexive<EndianType, ScenarioStructureBSPMarker> markers;
        TagReflexive<EndianType, ScenarioStructureBSPDetailObjectData> detail_objects;
        TagReflexive<EndianType, ScenarioStructureBSPRuntimeDecal> runtime_decals;
        PAD(0x8);
        PAD(0x4);
        TagReflexive<EndianType, ScenarioStructureBSPGlobalMapLeaf> leaf_map_leaves;
        TagReflexive<EndianType, ScenarioStructureBSPGlobalLeafPortal> leaf_map_portals;

        ENDIAN_TEMPLATE(NewType) operator ScenarioStructureBSP<NewType>() const noexcept {
            ScenarioStructureBSP<NewType> copy = {};
            COPY_THIS(lightmaps);
            COPY_THIS(vehicle_floor);
            COPY_THIS(vehicle_ceiling);
            COPY_THIS(default_ambient_color);
            COPY_THIS(default_distant_light_0_color);
            COPY_THIS(default_distant_light_0_direction);
            COPY_THIS(default_distant_light_1_color);
            COPY_THIS(default_distant_light_1_direction);
            COPY_THIS(default_reflection_tint);
            COPY_THIS(default_shadow_vector);
            COPY_THIS(default_shadow_color);
            COPY_THIS(collision_materials);
            COPY_THIS(collision_bsp);
            COPY_THIS(nodes);
            COPY_THIS(world_bounds_x);
            COPY_THIS(world_bounds_y);
            COPY_THIS(world_bounds_z);
            COPY_THIS(leaves);
            COPY_THIS(leaf_surfaces);
            COPY_THIS(surfaces);
            COPY_THIS(lightmaps_1);
            COPY_THIS(lens_flares);
            COPY_THIS(lens_flare_markers);
            COPY_THIS(clusters);
            COPY_THIS(cluster_data);
            COPY_THIS(cluster_portals);
            COPY_THIS(breakable_surfaces);
            COPY_THIS(fog_planes);
            COPY_THIS(fog_regions);
            COPY_THIS(fog_palette);
            COPY_THIS(weather_palette);
            COPY_THIS(weather_polyhedra);
            COPY_THIS(pathfinding_surfaces);
            COPY_THIS(pathfinding_edges);
            COPY_THIS(background_sound_palette);
            COPY_THIS(sound_environment_palette);
            COPY_THIS(sound_pas_data);
            COPY_THIS(markers);
            COPY_THIS(detail_objects);
            COPY_THIS(runtime_decals);
            COPY_THIS(leaf_map_leaves);
            COPY_THIS(leaf_map_portals);
            return copy;
        }
    };
    static_assert(sizeof(ScenarioStructureBSP<BigEndian>) == 0x288);

    struct ScenarioStructureBSPCompiledHeader {
        LittleEndian<HEK::Pointer> pointer;
        PAD(0x10);
        LittleEndian<HEK::TagClassInt> signature;
    };
    static_assert(sizeof(ScenarioStructureBSPCompiledHeader) == 0x18);

    void compile_scenario_structure_bsp_tag(CompiledTag &compiled, const std::byte *data, std::size_t size);
}
#endif
