// SPDX-License-Identifier: GPL-3.0-only

#include <invader/tag/parser/parser.hpp>
#include <invader/tag/hek/class/bitmap.hpp>
#include <invader/tag/parser/compile/scenario_structure_bsp.hpp>
#include <invader/build/build_workload.hpp>

namespace Invader::Parser {
    void ScenarioStructureBSP::pre_compile(BuildWorkload &, std::size_t, std::size_t, std::size_t) {
        this->runtime_decals.clear(); // delete these in case this tag was extracted improperly
    }
    
    bool ScenarioStructureBSPMaterial::check_for_nonnormal_vectors_more(bool normalize) {
        auto *vertices = this->uncompressed_vertices.data();
        auto uncompressed_vertices_size = this->uncompressed_vertices.size();

        auto *lightmap_rendered_vertices = reinterpret_cast<ScenarioStructureBSPMaterialUncompressedRenderedVertex::struct_little *>(vertices);
        auto *lightmap_lightmap_vertices = reinterpret_cast<ScenarioStructureBSPMaterialUncompressedLightmapVertex::struct_little *>(lightmap_rendered_vertices + this->rendered_vertices_count);
        
        auto *lightmap_vertices_end = lightmap_lightmap_vertices + this->lightmap_vertices_count;
        std::size_t expected_size = reinterpret_cast<std::byte *>(lightmap_vertices_end) - vertices;
        if(expected_size != uncompressed_vertices_size) {
            return false; // stuff's messed up. we can't do anything
        }
        
        bool return_value = false;
        
        if(this->rendered_vertices_count) {
            for(std::size_t i = 0; i < this->rendered_vertices_count; i++) {
                auto &rv = lightmap_rendered_vertices[i];
                if(!rv.normal.is_normalized()) {
                    if(!normalize) {
                        return true;
                    }
                    else {
                        return_value = true;
                        rv.normal = rv.normal.normalize();
                    }
                }
            }
        }
        
        return return_value;
    }

    void ScenarioStructureBSPCollisionMaterial::post_compile(BuildWorkload &workload, std::size_t, std::size_t struct_index, std::size_t offset) {
        auto *data = workload.structs[struct_index].data.data();
        auto &material = *reinterpret_cast<struct_little *>(data + offset);

        if(workload.disable_recursion) {
            material.material = static_cast<HEK::MaterialType>(0xFFFF);
            return;
        }

        this->material = reinterpret_cast<Shader::struct_little *>(workload.structs[(*workload.tags[this->shader.tag_id.index].base_struct)].data.data())->material_type;
        material.material = this->material;
    }
    void ScenarioStructureBSPMaterial::pre_compile(BuildWorkload &workload, std::size_t tag_index, std::size_t, std::size_t) {
        if(this->lightmap_vertices_count != 0 && this->lightmap_vertices_count != this->rendered_vertices_count) {
            REPORT_ERROR_PRINTF(workload, ERROR_TYPE_ERROR, tag_index, "BSP lightmap material doesn't have equal # of lightmap and rendered vertices");
        }

        auto *vertices = this->uncompressed_vertices.data();
        auto uncompressed_vertices_size = this->uncompressed_vertices.size();

        auto *lightmap_rendered_vertices = reinterpret_cast<ScenarioStructureBSPMaterialUncompressedRenderedVertex::struct_little *>(vertices);
        auto *lightmap_lightmap_vertices = reinterpret_cast<ScenarioStructureBSPMaterialUncompressedLightmapVertex::struct_little *>(lightmap_rendered_vertices + this->rendered_vertices_count);

        auto *lightmap_vertices_end = lightmap_lightmap_vertices + this->lightmap_vertices_count;
        std::size_t expected_size = reinterpret_cast<std::byte *>(lightmap_vertices_end) - vertices;
        if(expected_size != uncompressed_vertices_size) {
            REPORT_ERROR_PRINTF(workload, ERROR_TYPE_FATAL_ERROR, tag_index, "BSP lightmap material lightmap vertices size is wrong (%zu gotten, %zu expected)", expected_size, uncompressed_vertices_size);
            throw InvalidTagDataException();
        }
    }

    void ScenarioStructureBSP::post_compile(BuildWorkload &workload, std::size_t tag_index, std::size_t struct_index, std::size_t offset) {
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
                plane.material_type = static_cast<HEK::MaterialType>(0xFFFF);

                if(workload.disable_recursion) {
                    continue;
                }

                // Find what region this fog is in
                std::size_t region_index = plane.front_region;
                if(region_index >= fog_region_count) {
                    continue;
                }
                auto &region = fog_regions[region_index];

                // Lastly get the fog tag
                std::size_t palette_index = region.fog;
                if(palette_index >= fog_palette_count) {
                    continue;
                }

                const auto &fog_id = fog_palette[palette_index].fog.tag_id.read();
                if(fog_id.is_null()) {
                    continue;
                }

                auto &fog = *reinterpret_cast<Fog::struct_little *>(workload.structs[*workload.tags[fog_id.index].base_struct].data.data());
                if(fog.flags & HEK::FogFlagsFlag::FOG_FLAGS_FLAG_IS_WATER) {
                    plane.material_type = HEK::MaterialType::MATERIAL_TYPE_WATER;
                }
            }
        }
    }

    bool regenerate_missing_bsp_vertices(ScenarioStructureBSPMaterial &material, bool fix) {
        // Lightmap vertices size is incorrect
        if(material.lightmap_vertices_count != material.rendered_vertices_count && material.lightmap_vertices_count != 0) {
            eprintf_error("Can't fix: Non-zero lightmap vertex count is wrong (%zu != %zu)", static_cast<std::size_t>(material.lightmap_vertices_count), static_cast<std::size_t>(material.rendered_vertices_count));
            return false;
        }

        #define PROCESS_VERTICES(from,to,rendered_type_from,rendered_type_to,lightmap_type_from,lightmap_type_to,convert_rendered,convert_lightmap) \
            /* Extract vertices */ \
            std::size_t vertices_size = material.rendered_vertices_count * sizeof(rendered_type_from::struct_little); \
            std::size_t lightmap_vertices_size = material.lightmap_vertices_count * sizeof(lightmap_type_from::struct_little); \
            std::size_t total_vertices_size = lightmap_vertices_size + vertices_size; \
     \
            /* Make sure it isn't bullshit */ \
            if(total_vertices_size != material.from.size()) { \
                eprintf_error("Can't fix: Vertices is an invalid size (%zu != %zu)", total_vertices_size, material.from.size()); \
                return false; \
            } \
            \
            if(!fix) { \
                return true; \
            } \
     \
            const std::byte *bsp_vertices_start = material.from.data(); \
            const std::byte *lightmap_vertices_start = bsp_vertices_start + material.rendered_vertices_count * sizeof(rendered_type_from::struct_little); \
            const auto *bsp_vertices = reinterpret_cast<const rendered_type_from::struct_little *>(bsp_vertices_start); \
     \
            auto *new_bsp_vertices = reinterpret_cast<rendered_type_to::struct_little *>( \
                material.to.insert( \
                    material.to.end(), \
                    material.rendered_vertices_count * sizeof(rendered_type_to::struct_little), \
                    std::byte() \
                ).base() \
            ); \
            for(std::size_t v = 0; v < material.rendered_vertices_count; v++) { \
                new_bsp_vertices[v] = convert_rendered(bsp_vertices[v]); \
            } \
     \
            /* Add lightmap vertices */ \
            if(material.lightmap_vertices_count == material.rendered_vertices_count) { \
                const auto *bsp_lightmap_vertices = reinterpret_cast<const lightmap_type_from::struct_little *>(lightmap_vertices_start); \
     \
                /* Decompress them as well */ \
                auto *new_bsp_lightmap_vertices = reinterpret_cast<lightmap_type_to::struct_little *>( \
                    material.to.insert( \
                        material.to.end(), \
                        material.lightmap_vertices_count * sizeof(lightmap_type_to::struct_little), \
                        std::byte() \
                    ).base() \
                ); \
                for(std::size_t v = 0; v < material.lightmap_vertices_count; v++) { \
                    new_bsp_lightmap_vertices[v] = convert_lightmap(bsp_lightmap_vertices[v]); \
                } \
            }

        if(material.uncompressed_vertices.size() == 0 && material.compressed_vertices.size() != 0) {
            PROCESS_VERTICES(compressed_vertices,uncompressed_vertices,ScenarioStructureBSPMaterialCompressedRenderedVertex,ScenarioStructureBSPMaterialUncompressedRenderedVertex,ScenarioStructureBSPMaterialCompressedLightmapVertex,ScenarioStructureBSPMaterialUncompressedLightmapVertex,decompress_sbsp_rendered_vertex,decompress_sbsp_lightmap_vertex)
        }
        else if(material.uncompressed_vertices.size() != 0 && material.compressed_vertices.size() == 0) {
            PROCESS_VERTICES(uncompressed_vertices,compressed_vertices,ScenarioStructureBSPMaterialUncompressedRenderedVertex,ScenarioStructureBSPMaterialCompressedRenderedVertex,ScenarioStructureBSPMaterialUncompressedLightmapVertex,ScenarioStructureBSPMaterialCompressedLightmapVertex,compress_sbsp_rendered_vertex,compress_sbsp_lightmap_vertex)
        }
        else {
            return false;
        }
        return true;
    }

    bool regenerate_missing_bsp_vertices(ScenarioStructureBSP &bsp, bool fix) {
        bool return_value = false;
        for(auto &lightmap : bsp.lightmaps) {
            for(auto &material : lightmap.materials) {
                return_value = regenerate_missing_bsp_vertices(material, fix) || return_value;
            }
        }
        return return_value;
    }

    void ScenarioStructureBSPMaterial::post_cache_parse(const Invader::Tag &tag, std::optional<HEK::Pointer> pointer) {
        // Do nothing if there is nothing to do
        if(this->rendered_vertices_count == 0) {
            this->lightmap_vertices_count = 0;
            return;
        }

        // Material
        auto &bsp_material = tag.get_struct_at_pointer<HEK::ScenarioStructureBSPMaterial>(*pointer);

        // If it's Xbox, it's compressed
        auto engine = tag.get_map().get_engine();
        if(engine == HEK::CacheFileEngine::CACHE_FILE_XBOX) {
            // Extract vertices
            std::size_t compressed_vertices_size = this->rendered_vertices_count * sizeof(ScenarioStructureBSPMaterialCompressedRenderedVertex::struct_little);
            std::size_t lightmap_vertices_size = this->lightmap_vertices_count * sizeof(ScenarioStructureBSPMaterialCompressedLightmapVertex::struct_little);
            std::size_t total_vertices_size = lightmap_vertices_size + compressed_vertices_size;
            const std::byte *compressed_bsp_vertices_start = tag.data(bsp_material.compressed_vertices.pointer, total_vertices_size);
            this->compressed_vertices = std::vector<std::byte>(compressed_bsp_vertices_start, compressed_bsp_vertices_start + total_vertices_size);
            if(!regenerate_missing_bsp_vertices(*this, true)) {
                eprintf_error("Failed to decompress vertices");
                throw InvalidTagDataException();
            }
        }
        else {
            // Extract vertices
            std::size_t uncompressed_vertices_size = this->rendered_vertices_count * sizeof(ScenarioStructureBSPMaterialUncompressedRenderedVertex::struct_little);
            std::size_t lightmap_vertices_size = this->lightmap_vertices_count * sizeof(ScenarioStructureBSPMaterialUncompressedLightmapVertex::struct_little);
            std::size_t total_vertices_size = lightmap_vertices_size + uncompressed_vertices_size;

            const std::byte *uncompressed_bsp_vertices_start;
            const std::byte *uncompressed_lightmap_vertices_start;

            // CE Anniversary stuff!
            if(bsp_material.uncompressed_vertices.pointer == 0) {
                auto &base_struct = tag.get_base_struct<HEK::ScenarioStructureBSPCompiledHeader>();
                uncompressed_bsp_vertices_start = tag.get_map().get_data_at_offset(base_struct.lightmap_vertices_start + this->rendered_vertices_offset, uncompressed_vertices_size);
                uncompressed_lightmap_vertices_start = tag.get_map().get_data_at_offset(base_struct.lightmap_vertices_start + this->lightmap_vertices_offset, lightmap_vertices_size);
            }
            // Not CE Anniversary stuff
            else {
                uncompressed_bsp_vertices_start = tag.data(bsp_material.uncompressed_vertices.pointer, total_vertices_size);
                uncompressed_lightmap_vertices_start = uncompressed_bsp_vertices_start + this->rendered_vertices_count * sizeof(ScenarioStructureBSPMaterialUncompressedRenderedVertex::struct_little);
            }

            this->uncompressed_vertices = std::vector<std::byte>(uncompressed_bsp_vertices_start, uncompressed_bsp_vertices_start + uncompressed_vertices_size);
            this->uncompressed_vertices.insert(this->uncompressed_vertices.end(), uncompressed_lightmap_vertices_start, uncompressed_lightmap_vertices_start + lightmap_vertices_size);

            if(!regenerate_missing_bsp_vertices(*this, true)) {
                eprintf_error("Failed to compress vertices");
                throw InvalidTagDataException();
            }
        }
    }
}
