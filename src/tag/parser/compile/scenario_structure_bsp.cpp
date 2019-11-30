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

    void ScenarioStructureBSP::post_compile(BuildWorkload2 &workload, std::size_t tag_index, std::size_t, std::size_t) {
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
    }
}
