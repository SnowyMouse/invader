// SPDX-License-Identifier: GPL-3.0-only

#include <invader/tag/parser/parser.hpp>
#include <invader/tag/hek/class/bitmap.hpp>
#include <invader/build/build_workload.hpp>

namespace Invader::Parser {
    void ScenarioStructureBSP::pre_compile(BuildWorkload &, std::size_t, std::size_t, std::size_t) {
        this->runtime_decals.clear(); // delete these in case this tag was extracted improperly
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

        // If we're building anniversary maps, we handle vertices a lil' differently o-o
        if(workload.engine_target == HEK::CacheFileEngine::CACHE_FILE_ANNIVERSARY) {
            std::vector<std::byte> *vertices_vec = nullptr;
            for(auto &v : workload.bsp_vertices) {
                if(v.first == tag_index) {
                    vertices_vec = &v.second;
                }
            }
            if(!vertices_vec) {
                vertices_vec = &workload.bsp_vertices.emplace_back(tag_index, std::vector<std::byte>()).second;
            }

            this->rendered_vertices_offset = vertices_vec->size();
            vertices_vec->insert(vertices_vec->end(), reinterpret_cast<std::byte *>(lightmap_rendered_vertices), reinterpret_cast<std::byte *>(lightmap_rendered_vertices + this->rendered_vertices_count));
            this->lightmap_vertices_offset = vertices_vec->size();
            vertices_vec->insert(vertices_vec->end(), reinterpret_cast<std::byte *>(lightmap_lightmap_vertices), reinterpret_cast<std::byte *>(lightmap_lightmap_vertices + this->lightmap_vertices_count));

            this->uncompressed_vertices.clear();
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
                if(region_index > fog_region_count) {
                    continue;
                }
                auto &region = fog_regions[region_index];

                // Lastly get the fog tag
                std::size_t palette_index = region.fog_palette;
                if(palette_index == NULL_INDEX) {
                    continue;
                }
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

    void Invader::Parser::ScenarioStructureBSPMaterial::post_cache_parse(const Invader::Tag &tag, std::optional<HEK::Pointer> pointer) {
        // Do nothing if there is nothing to do
        if(this->rendered_vertices_count == 0) {
            this->lightmap_vertices_count = 0;
            return;
        }

        // Material
        auto &bsp_material = tag.get_struct_at_pointer<HEK::ScenarioStructureBSPMaterial>(*pointer);

        // If it's Xbox, it's compressed
        if(tag.get_map().get_cache_file_header().engine == HEK::CacheFileEngine::CACHE_FILE_XBOX) {
            // Extract vertices
            std::size_t compressed_vertices_size = this->rendered_vertices_count * sizeof(ScenarioStructureBSPMaterialCompressedRenderedVertex::struct_little);
            std::size_t lightmap_vertices_size = this->lightmap_vertices_count * sizeof(ScenarioStructureBSPMaterialCompressedLightmapVertex::struct_little);
            std::size_t total_vertices_size = lightmap_vertices_size + compressed_vertices_size;

            const std::byte *compressed_bsp_vertices_start = tag.data(bsp_material.compressed_vertices.pointer, total_vertices_size);
            const std::byte *compressed_lightmap_vertices_start = compressed_bsp_vertices_start + this->rendered_vertices_count * sizeof(ScenarioStructureBSPMaterialCompressedRenderedVertex::struct_little);
            const auto *compressed_bsp_vertices = reinterpret_cast<const ScenarioStructureBSPMaterialCompressedRenderedVertex::struct_little *>(compressed_bsp_vertices_start);

            auto *new_uncompressed_bsp_vertices = reinterpret_cast<ScenarioStructureBSPMaterialUncompressedRenderedVertex::struct_little *>(
                this->uncompressed_vertices.insert(
                    this->uncompressed_vertices.end(),
                    this->rendered_vertices_count * sizeof(ScenarioStructureBSPMaterialUncompressedRenderedVertex::struct_little),
                    std::byte()
                ).base()
            );
            for(std::size_t v = 0; v < this->rendered_vertices_count; v++) {
                new_uncompressed_bsp_vertices[v] = HEK::decompress_sbsp_rendered_vertex(compressed_bsp_vertices[v]);
            }

            // Add lightmap vertices
            if(this->lightmap_vertices_count == this->rendered_vertices_count) {
                const auto *compressed_bsp_lightmap_vertices = reinterpret_cast<const ScenarioStructureBSPMaterialCompressedLightmapVertex::struct_little *>(compressed_lightmap_vertices_start);
                this->compressed_vertices.insert(this->compressed_vertices.end(), reinterpret_cast<const std::byte *>(compressed_bsp_lightmap_vertices), reinterpret_cast<const std::byte *>(compressed_bsp_lightmap_vertices + this->lightmap_vertices_count));

                // Decompress them as well
                auto *new_uncompressed_bsp_lightmap_vertices = reinterpret_cast<ScenarioStructureBSPMaterialUncompressedLightmapVertex::struct_little *>(
                    this->uncompressed_vertices.insert(
                        this->uncompressed_vertices.end(),
                        this->lightmap_vertices_count * sizeof(ScenarioStructureBSPMaterialUncompressedLightmapVertex::struct_little),
                        std::byte()
                    ).base()
                );
                for(std::size_t v = 0; v < this->lightmap_vertices_count; v++) {
                    new_uncompressed_bsp_lightmap_vertices[v] = HEK::decompress_sbsp_lightmap_vertex(compressed_bsp_lightmap_vertices[v]);
                }
            }
            else if(this->lightmap_vertices_count != 0) {
                eprintf_error("non-zero lightmap vertex count (%zu) != rendered vertex count (%zu)", static_cast<std::size_t>(this->lightmap_vertices_count), static_cast<std::size_t>(this->rendered_vertices_count));
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
            if(tag.get_map().get_cache_file_header().engine == HEK::CacheFileEngine::CACHE_FILE_ANNIVERSARY) {
                auto &base_struct = tag.get_base_struct<HEK::ScenarioStructureBSPCompiledHeader>();

                uncompressed_bsp_vertices_start = tag.get_map().get_data_at_offset(base_struct.lightmap_vertices_start + this->rendered_vertices_offset, uncompressed_vertices_size);
                uncompressed_lightmap_vertices_start = tag.get_map().get_data_at_offset(base_struct.lightmap_vertices_start + this->lightmap_vertices_offset, lightmap_vertices_size);
            }
            else {
                uncompressed_bsp_vertices_start = tag.data(bsp_material.uncompressed_vertices.pointer, total_vertices_size);
                uncompressed_lightmap_vertices_start = uncompressed_bsp_vertices_start + this->rendered_vertices_count * sizeof(ScenarioStructureBSPMaterialUncompressedRenderedVertex::struct_little);
            }

            const auto *uncompressed_bsp_vertices = reinterpret_cast<const ScenarioStructureBSPMaterialUncompressedRenderedVertex::struct_little *>(uncompressed_bsp_vertices_start);
            this->uncompressed_vertices.insert(this->uncompressed_vertices.end(), reinterpret_cast<const std::byte *>(uncompressed_bsp_vertices), reinterpret_cast<const std::byte *>(uncompressed_bsp_vertices + this->rendered_vertices_count));

            // Compress the vertices too
            auto *new_compressed_bsp_vertices = reinterpret_cast<ScenarioStructureBSPMaterialCompressedRenderedVertex::struct_little *>(
                this->compressed_vertices.insert(
                    this->compressed_vertices.end(),
                    this->rendered_vertices_count * sizeof(ScenarioStructureBSPMaterialCompressedRenderedVertex::struct_little),
                    std::byte()
                ).base()
            );
            for(std::size_t v = 0; v < this->rendered_vertices_count; v++) {
                new_compressed_bsp_vertices[v] = HEK::compress_sbsp_rendered_vertex(uncompressed_bsp_vertices[v]);
            }

            // Add lightmap vertices
            if(this->lightmap_vertices_count == this->rendered_vertices_count) {
                const auto *uncompressed_bsp_lightmap_vertices = reinterpret_cast<const ScenarioStructureBSPMaterialUncompressedLightmapVertex::struct_little *>(uncompressed_lightmap_vertices_start);
                this->uncompressed_vertices.insert(this->uncompressed_vertices.end(), reinterpret_cast<const std::byte *>(uncompressed_bsp_lightmap_vertices), reinterpret_cast<const std::byte *>(uncompressed_bsp_lightmap_vertices + this->lightmap_vertices_count));

                // Compress them as well
                auto *new_compressed_bsp_lightmap_vertices = reinterpret_cast<ScenarioStructureBSPMaterialCompressedLightmapVertex::struct_little *>(
                    this->compressed_vertices.insert(
                        this->compressed_vertices.end(),
                        this->lightmap_vertices_count * sizeof(ScenarioStructureBSPMaterialCompressedLightmapVertex::struct_little),
                        std::byte()
                    ).base()
                );
                for(std::size_t v = 0; v < this->lightmap_vertices_count; v++) {
                    new_compressed_bsp_lightmap_vertices[v] = HEK::compress_sbsp_lightmap_vertex(uncompressed_bsp_lightmap_vertices[v]);
                }
            }
            else if(this->lightmap_vertices_count != 0) {
                eprintf_error("non-zero lightmap vertex count (%zu) != rendered vertex count (%zu)", static_cast<std::size_t>(this->lightmap_vertices_count), static_cast<std::size_t>(this->rendered_vertices_count));
                throw InvalidTagDataException();
            }
        }
    }
}
