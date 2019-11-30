// SPDX-License-Identifier: GPL-3.0-only

#include <invader/tag/parser/parser.hpp>
#include <invader/tag/hek/class/bitmap.hpp>
#include <invader/build2/build_workload.hpp>

namespace Invader::Parser {
    void ScenarioStructureBSPCollisionMaterial::post_compile(BuildWorkload2 &workload, std::size_t, std::size_t struct_index, std::size_t offset) {
        auto *data = workload.structs[struct_index].data.data();
        auto &material = *reinterpret_cast<struct_little *>(data + offset);
        this->material = reinterpret_cast<Shader::struct_little *>(workload.structs[(*workload.tags[this->shader.tag_id.index].base_struct)].data.data())->material_type;
        material.material = this->material;
    }
    void ScenarioStructureBSPMaterial::pre_compile(BuildWorkload2 &workload, std::size_t tag_index, std::size_t, std::size_t) {
        this->compressed_vertices.clear();

        if(this->lightmap_vertices_count != 0) {
            if(this->lightmap_vertices_count != this->rendered_vertices_count) {
                REPORT_ERROR_PRINTF(workload, ERROR_TYPE_ERROR, tag_index, "BSP lightmap material doesn't have equal # of lightmap and rendered vertices");
            }

            auto *vertices = this->uncompressed_vertices.data();

            auto *lightmap_rendered_vertices_big = reinterpret_cast<ScenarioStructureBSPMaterialUncompressedRenderedVertex::struct_big *>(vertices);
            auto *lightmap_rendered_vertices_little = reinterpret_cast<ScenarioStructureBSPMaterialUncompressedRenderedVertex::struct_little *>(lightmap_rendered_vertices_big);

            auto *lightmap_lightmap_vertices_big = reinterpret_cast<ScenarioStructureBSPMaterialUncompressedLightmapVertex::struct_big *>(lightmap_rendered_vertices_big + this->rendered_vertices_count);
            auto *lightmap_lightmap_vertices_little = reinterpret_cast<ScenarioStructureBSPMaterialUncompressedLightmapVertex::struct_little *>(lightmap_lightmap_vertices_big);

            auto *lightmap_vertices_end = lightmap_lightmap_vertices_big + this->lightmap_vertices_count;
            std::size_t expected_size = reinterpret_cast<std::byte *>(lightmap_vertices_end) - vertices;
            if(expected_size) {
                REPORT_ERROR_PRINTF(workload, ERROR_TYPE_FATAL_ERROR, tag_index, "BSP lightmap material lightmap vertices size is wrong (%zu gotten, %zu expected)", expected_size, this->uncompressed_vertices.size());
                throw InvalidTagDataException();
            }

            for(std::size_t v = 0; v < this->rendered_vertices_count; v++) {
                lightmap_rendered_vertices_little[v] = lightmap_rendered_vertices_big[v];
            }
            for(std::size_t v = 0; v < this->lightmap_vertices_count; v++) {
                lightmap_lightmap_vertices_little[v] = lightmap_lightmap_vertices_big[v];
            }
        }
    }

    void ScenarioStructureBSP::post_compile(BuildWorkload2 &workload, std::size_t tag_index, std::size_t struct_index, std::size_t offset) {
        // Check lightmaps
        bool lightmaps_present = false;
        bool only_transparent = true;
        for(auto &lm : this->lightmaps) {
            for(auto &mat : lm.materials) {
                if(mat.lightmap_vertices_count) {
                    lightmaps_present = true;
                    break;
                }
                else {
                    auto &shader = mat.shader;
                    if(!shader.tag_id.is_null()) {
                        auto shader_type = reinterpret_cast<Shader::struct_little *>(workload.structs[*workload.tags[mat.shader.tag_id.index].base_struct].data.data())->shader_type.read();
                        if(shader_type < HEK::ShaderType::SHADER_TYPE_SHADER_TRANSPARENT_GENERIC) {
                            only_transparent = false;
                        }
                    }
                }
            }
            if(lightmaps_present) {
                break;
            }
        }

        if(!lightmaps_present && !only_transparent) {
            REPORT_ERROR_PRINTF(workload, ERROR_TYPE_WARNING, tag_index, "BSP has no lightmaps baked, so parts of it will not render");
        }

        // Handle the fog palette
        auto &tag_struct = workload.structs[struct_index];
        auto &tag_data = *reinterpret_cast<struct_little *>(tag_struct.data.data() + offset);

        std::size_t fog_plane_count = this->fog_planes.size();
        std::size_t fog_region_count = this->fog_regions.size();
        std::size_t fog_palette_count = this->fog_palette.size();

        if(fog_palette_count && fog_region_count && fog_plane_count) {
            auto fog_palette_index = *tag_struct.resolve_pointer(&tag_data.fog_palette.pointer);
            auto fog_region_index = *tag_struct.resolve_pointer(&tag_data.fog_regions.pointer);
            auto fog_plane_index = *tag_struct.resolve_pointer(&tag_data.fog_planes.pointer);
            auto *fog_planes = reinterpret_cast<ScenarioStructureBSPFogPlane::struct_little *>(workload.structs[fog_plane_index].data.data());
            auto *fog_regions = reinterpret_cast<ScenarioStructureBSPFogRegion::struct_little *>(workload.structs[fog_region_index].data.data());
            auto *fog_palette = reinterpret_cast<ScenarioStructureBSPFogPalette::struct_little *>(workload.structs[fog_palette_index].data.data());

            // Go through each fog plane
            for(std::size_t i = 0; i < fog_plane_count; i++) {
                auto &plane = fog_planes[i];

                // Find what region this fog is in
                std::size_t region_index = plane.front_region;
                if(region_index > fog_region_count) {
                    continue;
                }
                auto &region = fog_regions[region_index];

                // Lastly get the fog tag
                std::size_t palette_index = region.fog_palette;
                if(palette_index >= fog_palette_count) {
                    REPORT_ERROR_PRINTF(workload, ERROR_TYPE_ERROR, tag_index, "BSP fog palette index exceeds fog palette count (%zu >= %zu)", palette_index, fog_palette_count);
                    continue;
                }
                const auto &fog_id = fog_palette[palette_index].fog.tag_id.read();
                if(fog_id.is_null()) {
                    continue;
                }
                auto &fog = *reinterpret_cast<Fog::struct_little *>(workload.structs[*workload.tags[fog_id.index].base_struct].data.data());
                if(fog.flags.read().is_water) {
                    plane.material_type = HEK::MaterialType::MATERIAL_TYPE_WATER;
                }
            }
        }
    }
}
