/*
 * Invader (c) 2018 Kavawuvi
 *
 * This program is free software under the GNU General Public License v3.0 or later. See LICENSE for more information.
 */

#include "../compile.hpp"
#include "scenario_structure_bsp.hpp"

namespace Invader::HEK {
    void compile_scenario_structure_bsp_tag(CompiledTag &compiled, const std::byte *data, std::size_t size) {
        // Allocate data for the header
        compiled.data.insert(compiled.data.end(), sizeof(ScenarioStructureBSPCompiledHeader), std::byte());

        // Initialize the header
        ScenarioStructureBSPCompiledHeader header = {};
        header.signature = TagClassInt::TAG_CLASS_SCENARIO_STRUCTURE_BSP;

        compiled.pointers.emplace_back(CompiledTagPointer { 0x0, sizeof(ScenarioStructureBSPCompiledHeader) });

        BEGIN_COMPILE(ScenarioStructureBSP)

        ADD_DEPENDENCY_ADJUST_SIZES(tag.lightmaps);
        ADD_BASIC_DEPENDENCY_REFLEXIVE(tag.collision_materials, shader);
        ADD_MODEL_COLLISION_BSP(tag.collision_bsp); // Add collision BSP (same as the model_collision_geometry collsion BSP)
        ADD_REFLEXIVE(tag.nodes);
        ADD_REFLEXIVE(tag.leaves);
        ADD_REFLEXIVE(tag.leaf_surfaces);
        ADD_REFLEXIVE(tag.surfaces);
        ADD_REFLEXIVE_START(tag.lightmaps_1) {
            ADD_REFLEXIVE_START(reflexive.materials) {
                ADD_DEPENDENCY_ADJUST_SIZES(reflexive.shader);

                std::size_t uncompressed_vertices_size = reflexive.uncompressed_vertices.size;
                ASSERT_SIZE(uncompressed_vertices_size);

                std::size_t rendered_vertex_count = reflexive.rendered_vertices.count;
                std::size_t lightmap_vertex_count = reflexive.lightmap_vertices.count;

                std::size_t total_uncompressed_vertex_size = rendered_vertex_count * sizeof(ScenarioStructureBSPMaterialUncompressedRenderedVertex<LittleEndian>) + lightmap_vertex_count * sizeof(ScenarioStructureBSPMaterialUncompressedLightmapVertex<LittleEndian>);
                ASSERT_SIZE(total_uncompressed_vertex_size);

                ASSERT_SIZE(uncompressed_vertices_size);
                ADD_POINTER_FROM_INT32(reflexive.uncompressed_vertices.pointer, compiled.data.size());

                std::size_t uncompressed_offset = compiled.data.size();
                compiled.data.insert(compiled.data.end(), total_uncompressed_vertex_size, std::byte());

                const auto *rendered_vertices_from = reinterpret_cast<const ScenarioStructureBSPMaterialUncompressedRenderedVertex<LittleEndian> *>(data);
                const auto *lightmap_vertices_from = reinterpret_cast<const ScenarioStructureBSPMaterialUncompressedLightmapVertex<LittleEndian> *>(data + rendered_vertex_count * sizeof(*rendered_vertices_from));

                auto *rendered_vertices_to = reinterpret_cast<ScenarioStructureBSPMaterialUncompressedRenderedVertex<LittleEndian> *>(compiled.data.data() + uncompressed_offset);
                auto *lightmap_vertices_to = reinterpret_cast<ScenarioStructureBSPMaterialUncompressedLightmapVertex<LittleEndian> *>(compiled.data.data() + uncompressed_offset + rendered_vertex_count * sizeof(*rendered_vertices_from));

                std::copy(rendered_vertices_from, rendered_vertices_from + rendered_vertex_count, rendered_vertices_to);
                std::copy(lightmap_vertices_from, lightmap_vertices_from + lightmap_vertex_count, lightmap_vertices_to);

                INCREMENT_DATA_PTR(uncompressed_vertices_size);
                INCREMENT_DATA_PTR(reflexive.compressed_vertices.size);
            } ADD_REFLEXIVE_END
        } ADD_REFLEXIVE_END
        ADD_BASIC_DEPENDENCY_REFLEXIVE(tag.lens_flares, lens);
        ADD_REFLEXIVE(tag.lens_flare_markers);
        ADD_REFLEXIVE_START(tag.clusters) {
            reflexive.first_decal_index = -1;
            reflexive.decal_count = 0;
            ADD_REFLEXIVE(reflexive.predicted_resources);
            ADD_REFLEXIVE_START(reflexive.subclusters) {
                ADD_REFLEXIVE(reflexive.surface_indices);
            } ADD_REFLEXIVE_END
            ADD_REFLEXIVE(reflexive.surface_indices);
            ADD_REFLEXIVE_START(reflexive.mirrors) {
                ADD_DEPENDENCY_ADJUST_SIZES(reflexive.shader);
                ADD_REFLEXIVE(reflexive.vertices);
            } ADD_REFLEXIVE_END
            ADD_REFLEXIVE(reflexive.portals);
        } ADD_REFLEXIVE_END

        std::size_t cluster_data_size = tag.cluster_data.size;
        ADD_POINTER_FROM_INT32(tag.cluster_data.pointer, compiled.data.size());
        ASSERT_SIZE(cluster_data_size);
        compiled.data.insert(compiled.data.end(), data, data + cluster_data_size);
        INCREMENT_DATA_PTR(cluster_data_size);
        PAD_32_BIT

        ADD_REFLEXIVE_START(tag.cluster_portals) {
            ADD_REFLEXIVE(reflexive.vertices);
        } ADD_REFLEXIVE_END
        ADD_REFLEXIVE(tag.breakable_surfaces);
        ADD_REFLEXIVE_START(tag.fog_planes) {
            DEFAULT_VALUE(reflexive.material_type, -1);
            ADD_REFLEXIVE(reflexive.vertices);
        } ADD_REFLEXIVE_END
        ADD_REFLEXIVE(tag.fog_regions);
        ADD_BASIC_DEPENDENCY_REFLEXIVE(tag.fog_palette, fog);
        ADD_REFLEXIVE_START(tag.weather_palette) {
            ADD_DEPENDENCY_ADJUST_SIZES(reflexive.particle_system);
            ADD_DEPENDENCY_ADJUST_SIZES(reflexive.wind);
        } ADD_REFLEXIVE_END
        ADD_REFLEXIVE_START(tag.weather_polyhedra) {
            ADD_REFLEXIVE(reflexive.planes);
        } ADD_REFLEXIVE_END
        ADD_REFLEXIVE(tag.pathfinding_surfaces);
        ADD_REFLEXIVE(tag.pathfinding_edges);
        ADD_BASIC_DEPENDENCY_REFLEXIVE(tag.background_sound_palette, background_sound);
        ADD_BASIC_DEPENDENCY_REFLEXIVE(tag.sound_environment_palette, sound_environment);

        std::size_t sound_pas_size = tag.sound_pas_data.size;
        ADD_POINTER_FROM_INT32(tag.sound_pas_data.pointer, compiled.data.size());
        ASSERT_SIZE(sound_pas_size);
        compiled.data.insert(compiled.data.end(), data, data + sound_pas_size);
        INCREMENT_DATA_PTR(sound_pas_size);
        PAD_32_BIT

        ADD_REFLEXIVE(tag.markers);
        ADD_REFLEXIVE_START(tag.detail_objects) {
            ADD_REFLEXIVE(reflexive.cells);
            ADD_REFLEXIVE(reflexive.instances);
            ADD_REFLEXIVE(reflexive.counts);
            ADD_REFLEXIVE(reflexive.z_reference_vectors);
            reflexive.bullshit = reflexive.instances.count.read() != 0;
        } ADD_REFLEXIVE_END

        skip_data = true;
        ADD_REFLEXIVE(tag.runtime_decals);
        tag.runtime_decals.count = 0;
        skip_data = false;

        ADD_REFLEXIVE_START(tag.leaf_map_leaves) {
            ADD_REFLEXIVE_START(reflexive.faces) {
                ADD_REFLEXIVE(reflexive.vertices);
            } ADD_REFLEXIVE_END
            ADD_REFLEXIVE(reflexive.portal_indices);
        } ADD_REFLEXIVE_END
        ADD_REFLEXIVE_START(tag.leaf_map_portals) {
            ADD_REFLEXIVE(reflexive.vertices);
        } ADD_REFLEXIVE_END

        *reinterpret_cast<ScenarioStructureBSPCompiledHeader *>(compiled.data.data()) = header;

        FINISH_COMPILE
    }
}
