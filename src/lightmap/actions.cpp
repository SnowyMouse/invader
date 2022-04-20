#include "actions.hpp"

#include <invader/file/file.hpp>
#include <invader/build/build_workload.hpp>
#include <invader/map/map.hpp>

#include <cstdio>
#include <cstdlib>
#include <map>

using namespace Invader;

static constexpr const std::size_t MESH_FORMAT_VERSION = 1;

struct ExportedVertex {
    float x, y, z;
    float i, j, k;
};

struct ExportedTriangle {
    std::size_t material;
    std::size_t a, b, c;
};

const char *ExportedMaterialTypeStr[] = {
    "opaque",
    "invisible"
};

enum ExportedMaterialType {
    EXPORTED_MATERIAL_TYPE_OPAQUE,
    EXPORTED_MATERIAL_TYPE_INVISIBLE
};

struct ExportedMaterial {
    std::string path;
    ExportedMaterialType type;
    float power;
    float emission_red, emission_green, emission_blue;
};

struct ExportedLightmap {
    std::size_t first_triangle_index;
    std::size_t triangle_count;
};

struct ExportedModel {
    std::string path;
    std::vector<ExportedMaterial> materials;
    std::vector<ExportedTriangle> triangles;
    std::vector<ExportedVertex> vertices;
    std::vector<ExportedLightmap> lightmaps;
};

struct ExportedSkyLight {
    float power;
    float red, green, blue;
    float yaw, pitch;
};

struct ExportedSky {
    std::string path;
    std::vector<ExportedSkyLight> lights;
    float outdoor_power;
    float outdoor_red, outdoor_green, outdoor_blue;
    float indoor_power;
    float indoor_red, indoor_green, indoor_blue;
};

struct ExportedObject {
    std::size_t model;
    float x;
    float y;
    float z;
    float yaw;
    float pitch;
    float roll;
};

static std::size_t add_shader_to_materials(const Tag &tag, std::vector<ExportedMaterial> &materials) {
    auto fourcc = tag.get_tag_fourcc();
    auto full_path = tag.get_path() + "." + HEK::tag_fourcc_to_extension(fourcc);
    auto mat_count = materials.size();
    for(std::size_t m = 0; m < mat_count; m++) {
        if(materials[m].path == full_path) {
            return m;
        }
    }
    
    // Get the shader of fun
    auto &shader = tag.get_base_struct<HEK::Shader>();
    bool opaque = fourcc == HEK::TagFourCC::TAG_FOURCC_SHADER_MODEL || fourcc == HEK::TagFourCC::TAG_FOURCC_SHADER_ENVIRONMENT;
    
    // Add the material
    auto &material = materials.emplace_back();
    material.path = std::move(full_path);
    material.power = shader.power;
    material.emission_red = shader.color_of_emitted_light.red;
    material.emission_green = shader.color_of_emitted_light.green;
    material.emission_blue = shader.color_of_emitted_light.blue;
    
    if(material.power <= 0.0F) {
        material.power = 0.0F;
    }
    material.type = opaque ? ExportedMaterialType::EXPORTED_MATERIAL_TYPE_OPAQUE : ExportedMaterialType::EXPORTED_MATERIAL_TYPE_INVISIBLE;
    
    return mat_count;
}

static ExportedModel read_bsp(const Tag &tag, std::vector<ExportedMaterial> &materials_arr, std::vector<ExportedSky> &skies_arr) {
    ExportedModel exported_model;
    exported_model.path = tag.get_path() + "." + HEK::tag_fourcc_to_extension(tag.get_tag_fourcc());
    
    auto &base_struct = tag.get_base_struct<HEK::ScenarioStructureBSP>();
    
    std::size_t triangle_count = base_struct.surfaces.count;
    const Parser::ScenarioStructureBSPSurface::struct_little *triangles = reinterpret_cast<const Parser::ScenarioStructureBSPSurface::struct_little *>(tag.data(base_struct.surfaces.pointer, sizeof(*triangles) * triangle_count));
    
    std::size_t lightmaps_count = base_struct.lightmaps.count;
    const Parser::ScenarioStructureBSPLightmap::struct_little *lightmaps = reinterpret_cast<const Parser::ScenarioStructureBSPLightmap::struct_little *>(tag.data(base_struct.lightmaps.pointer, sizeof(*lightmaps) * lightmaps_count));
    
    auto &map = tag.get_map();
    
    for(std::size_t l = 0; l < lightmaps_count; l++) {
        auto &lightmap = lightmaps[l];
        std::size_t materials_count = lightmap.materials.count;
        const Parser::ScenarioStructureBSPMaterial::struct_little *materials = reinterpret_cast<const Parser::ScenarioStructureBSPMaterial::struct_little *>(tag.data(lightmap.materials.pointer, sizeof(*materials) * materials_count));
        auto first_triangle_index_this_lightmap = exported_model.triangles.size();
        
        // Go through each lightmap
        for(std::size_t m = 0; m < materials_count; m++) {
            auto &material = materials[m];
            std::size_t rendered_vertices_count = material.rendered_vertices_count;
            const Parser::ScenarioStructureBSPMaterialUncompressedRenderedVertex::struct_little *uncompressed_vertices = reinterpret_cast<const Parser::ScenarioStructureBSPMaterialUncompressedRenderedVertex::struct_little *>(tag.data(material.uncompressed_vertices.pointer, sizeof(*uncompressed_vertices) * rendered_vertices_count));
            
            std::size_t material_index = add_shader_to_materials(map.get_tag(material.shader.tag_id.read().index), materials_arr);
            std::size_t offset = exported_model.vertices.size();
            for(std::size_t v = 0; v < rendered_vertices_count; v++) {
                auto &vertex = exported_model.vertices.emplace_back();
                auto &uv = uncompressed_vertices[v];
                vertex.x = uv.position.x;
                vertex.y = uv.position.y;
                vertex.z = uv.position.z;
                vertex.i = uv.normal.i;
                vertex.j = uv.normal.j;
                vertex.k = uv.normal.k;
            }
            
            std::size_t initial_surface = material.surfaces;
            std::size_t surface_count = material.surface_count;
            
            for(std::size_t t = 0; t < surface_count; t++) {
                auto &triangle = exported_model.triangles.emplace_back();
                auto &surface = triangles[t + initial_surface];
                triangle.material = material_index;
                
                triangle.a = offset + surface.vertex0_index;
                triangle.b = offset + surface.vertex1_index;
                triangle.c = offset + surface.vertex2_index;
            }
        }
            
        // Add it as a thing
        if(lightmap.bitmap != NULL_INDEX) {
            auto &lm = exported_model.lightmaps.emplace_back();
            lm.first_triangle_index = first_triangle_index_this_lightmap;
            lm.triangle_count = exported_model.triangles.size() - first_triangle_index_this_lightmap;
        }
    }
    
    // Clusters
    std::size_t cluster_count = base_struct.clusters.count;
    const Parser::ScenarioStructureBSPCluster::struct_little *clusters = reinterpret_cast<const Parser::ScenarioStructureBSPCluster::struct_little *>(tag.data(base_struct.clusters.pointer, sizeof(*clusters) * cluster_count));
    auto &scenario_tag = map.get_tag(map.get_scenario_tag_id());
    auto &scenario_tag_struct = scenario_tag.get_base_struct<HEK::Scenario>();
    
    std::map<std::size_t, bool> sky_tag_added;
    
    // Go through each cluster. Add all skyboxes
    for(std::size_t c = 0; c < cluster_count; c++) {
        std::size_t sky = clusters[c].sky.read();
        if(sky == NULL_INDEX || sky_tag_added[sky]) {
            continue;
        }
        
        sky_tag_added[sky] = true;
        auto &sky_in_array = skies_arr.emplace_back();
        
        // Add the sky
        auto &sky_entry = scenario_tag.get_struct_from_reflexive(scenario_tag_struct.skies, sky);
        auto &sky_tag = map.get_tag(sky_entry.sky.tag_id.read().index);
        auto &sky_tag_struct = sky_tag.get_base_struct<HEK::Sky>();
        sky_in_array.path = sky_tag.get_path() + "." + HEK::tag_fourcc_to_extension(sky_tag.get_tag_fourcc());
        
        sky_in_array.outdoor_power = sky_tag_struct.outdoor_ambient_radiosity_power;
        sky_in_array.outdoor_red = sky_tag_struct.outdoor_ambient_radiosity_color.red;
        sky_in_array.outdoor_green = sky_tag_struct.outdoor_ambient_radiosity_color.green;
        sky_in_array.outdoor_blue = sky_tag_struct.outdoor_ambient_radiosity_color.blue;
        
        sky_in_array.indoor_power = sky_tag_struct.indoor_ambient_radiosity_power;
        sky_in_array.indoor_red = sky_tag_struct.indoor_ambient_radiosity_color.red;
        sky_in_array.indoor_green = sky_tag_struct.indoor_ambient_radiosity_color.green;
        sky_in_array.indoor_blue = sky_tag_struct.indoor_ambient_radiosity_color.blue;
        
        // Add these lights
        std::size_t light_count = sky_tag_struct.lights.count;
        auto *lights = sky_tag.resolve_reflexive(sky_tag_struct.lights);
        for(std::size_t l = 0; l < light_count; l++) {
            auto &light_to_add = sky_in_array.lights.emplace_back();
            auto &light_being_added = lights[l];
            light_to_add.power = light_being_added.power;
            light_to_add.red = light_being_added.color.red;
            light_to_add.green = light_being_added.color.green;
            light_to_add.blue = light_being_added.color.blue;
            light_to_add.yaw = light_being_added.direction.yaw;
            light_to_add.pitch = light_being_added.direction.pitch;
        }
    }
    
    return exported_model;
}

template<typename GeometryStruct, typename PartStruct, typename ModelStruct> static ExportedModel read_model(const Tag &tag, const ModelStruct &base_struct, std::vector<ExportedMaterial> &materials) {
    ExportedModel exported_model;
    exported_model.path = tag.get_path() + "." + HEK::tag_fourcc_to_extension(tag.get_tag_fourcc());
    
    // Are we doing memes?
    // bool use_local_nodes = tag.get_tag_fourcc() == HEK::TagFourCC::TAG_FOURCC_GBXMODEL && (base_struct.flags & HEK::ModelFlagsFlag::MODEL_FLAGS_FLAG_PARTS_HAVE_LOCAL_NODES);
    
    // Add all materials first
    std::size_t shader_count = base_struct.shaders.count.read();
    const Parser::ModelShaderReference::struct_little *shaders = reinterpret_cast<decltype(shaders)>(tag.data(base_struct.shaders.pointer, sizeof(*shaders) * shader_count));
    
    std::map<std::size_t, std::size_t> material_map; // map local shader A to material B
    for(std::size_t s = 0; s < shader_count; s++) {
        material_map[s] = add_shader_to_materials(tag.get_map().get_tag(shaders[s].shader.tag_id.read().index), materials);
    }
    
    // Hold geometry information
    std::size_t geometry_count = base_struct.geometries.count.read();
    const GeometryStruct *geometries = reinterpret_cast<decltype(geometries)>(tag.data(base_struct.geometries.pointer, sizeof(*geometries) * geometry_count));
    
    // Did we add this stuff already? (in case the model was deduped on generation, we don't *need* to add the same geometry over and over then)
    std::map<HEK::Index, bool> added_already;
    added_already[NULL_INDEX] = true;
    
    auto model_data_offset = tag.get_map().get_model_data_offset();
    auto model_index_offset = model_data_offset + tag.get_map().get_model_index_offset();
    
    // Next, add the geometries
    std::size_t region_count = base_struct.regions.count.read();
    const Parser::ModelRegion::struct_little *regions = reinterpret_cast<const Parser::ModelRegion::struct_little *>(tag.data(base_struct.regions.pointer, sizeof(*regions) * region_count));
    for(std::size_t r = 0; r < region_count; r++) {
        auto &region = regions[r];
        std::size_t permutation_count = region.permutations.count.read();
        const Parser::ModelRegionPermutation::struct_little *permutations = reinterpret_cast<decltype(permutations)>(tag.data(region.permutations.pointer, sizeof(*permutations) * permutation_count));
        if(permutation_count == 0) {
            continue;
        }
        
        // First permutation
        auto &permutation = permutations[0];
        
        // Check if we added this already!
        auto &super_high_added = added_already[permutation.super_high];
        if(super_high_added) {
            continue;
        }
        super_high_added = true;
        
        // Get the geometry
        auto &geometry = geometries[permutation.super_high];
        std::size_t part_count = geometry.parts.count.read();
        const PartStruct *parts = reinterpret_cast<const PartStruct *>(tag.data(geometry.parts.pointer, sizeof(*parts) * part_count));
        
        // Go through each part. Add it!
        for(std::size_t a = 0; a < part_count; a++) {
            const auto &part = parts[a];
            // const std::uint8_t *local_nodes = use_local_nodes ? reinterpret_cast<const Parser::GBXModelGeometryPart::struct_little *>(&part)->local_node_indices : nullptr;
            
            auto vertex_count = static_cast<std::size_t>(part.vertex_count);
            const Parser::ModelVertexUncompressed::struct_little *vertices = reinterpret_cast<decltype(vertices)>(tag.get_map().get_data_at_offset(model_data_offset + part.vertex_offset, vertex_count * sizeof(*vertices)));
            
            std::size_t offset = exported_model.vertices.size();
            
            // Add vertices
            for(std::size_t v = 0; v < vertex_count; v++) {
                auto &vertex = vertices[v];
                auto &vertex_added = exported_model.vertices.emplace_back();
                vertex_added.x = vertex.position.x;
                vertex_added.y = vertex.position.y;
                vertex_added.z = vertex.position.z;
                vertex_added.i = vertex.normal.i;
                vertex_added.j = vertex.normal.j;
                vertex_added.k = vertex.normal.k;
            }
            
            // Now indices
            auto triangle_count = static_cast<std::size_t>(part.triangle_count);
            const HEK::LittleEndian<HEK::Index> *triangles = reinterpret_cast<decltype(triangles)>(tag.get_map().get_data_at_offset(model_index_offset + part.triangle_offset, (triangle_count + 2) * sizeof(*triangles)));
            bool flipped = false;
            auto material = material_map[part.shader_index];
            
            for(std::size_t t = 0; t < triangle_count; t++, flipped = !flipped) {
                // Skip degenerate triangle
                if(triangles[t+0] == triangles[t+1] || triangles[t+1] == triangles[t+2] || triangles[t+2] == triangles[t+0]) {
                    continue;
                }
                if(triangles[t+0] == NULL_INDEX || triangles[t+1] == NULL_INDEX || triangles[t+2] == NULL_INDEX) {
                    continue;
                }
                
                // Let's add it
                auto &triangle = exported_model.triangles.emplace_back();
                triangle.material = material;
                triangle.a = offset + triangles[t];
                triangle.b = offset + triangles[t + (flipped ? 2 : 1)];
                triangle.c = offset + triangles[t + (flipped ? 1 : 2)];
            }
        }
    }
    
    return exported_model;
}

std::string Invader::Lightmap::export_lightmap_mesh(const char *scenario, const char *bsp_name, const std::vector<std::filesystem::path> &tags_directories) {
    BuildWorkload::BuildParameters parameters;
    parameters.verbosity = BuildWorkload::BuildParameters::BuildVerbosity::BUILD_VERBOSITY_QUIET;
    parameters.tags_directories = tags_directories;
    parameters.use_tags_for_script_data = true;
    parameters.scenario = scenario;
    parameters.details.build_compress = false;
    
    std::vector<std::string> lines;
    std::vector<ExportedMaterial> materials;
    std::vector<ExportedModel> models;
    std::vector<ExportedModel> bsps;
    std::vector<ExportedObject> objects;
    std::vector<ExportedSky> skies;
    
#define ADD_LINE(...) lines.emplace_back(__VA_ARGS__)
    
    try {
        auto map = Map::map_with_move(BuildWorkload::compile_map(parameters));
        auto &scenario_tag = map.get_tag(map.get_scenario_tag_id());
        auto &scenario_struct = scenario_tag.get_base_struct<HEK::Scenario>();
        
        // Find the BSP
        std::optional<std::size_t> bsp_index;
        std::size_t bsp_count = scenario_struct.structure_bsps.count;
        const Parser::ScenarioBSP::struct_little *scenario_bsps = reinterpret_cast<decltype(scenario_bsps)>(scenario_tag.data(scenario_struct.structure_bsps.pointer, sizeof(*scenario_bsps) * bsp_count));
        for(std::size_t b = 0; b < bsp_count; b++) {
            auto tag_id = scenario_bsps[b].structure_bsp.tag_id.read();
            if(tag_id.is_null()) {
                continue;
            }
            
            // Find the tag name
            auto &bsp_tag = map.get_tag(tag_id.index);
            const char *bsp_tag_name = bsp_tag.get_path().c_str();
            for(const char *q = bsp_tag_name; *q; q++) {
                if(*q == '\\') {
                    bsp_tag_name = q + 1;
                }
            }
            
            // Do it
            if(std::strcmp(bsp_tag_name, bsp_name) == 0) {
                bsp_index = b;
                bsps.emplace_back(read_bsp(bsp_tag, materials, skies)); // add the BSP
                break;
            }
        }
        
        // Did we get it?
        if(!bsp_index.has_value()) {
            eprintf_error("No such BSP %s referenced by the scenario", bsp_name);
            throw std::exception();
        }
        
        // Now add all the sceneries in the BSP
        std::uint16_t bsp_bit = 1 << *bsp_index;
        
        // Start getting scenery
        std::size_t scenery_count = scenario_struct.scenery.count;
        std::size_t scenery_palette_count = scenario_struct.scenery_palette.count;
        if(scenery_count && scenery_palette_count) {
            std::map<std::size_t, std::optional<std::size_t>> model_tag_id_to_exported_model_id;
            
            const Parser::ScenarioScenery::struct_little *scenery = reinterpret_cast<decltype(scenery)>(scenario_tag.data(scenario_struct.scenery.pointer, sizeof(*scenery) * scenery_count));
            const Parser::ScenarioSceneryPalette::struct_little *scenery_palette = reinterpret_cast<decltype(scenery_palette)>(scenario_tag.data(scenario_struct.scenery_palette.pointer, sizeof(*scenery_palette) * scenery_palette_count));
            
            for(std::size_t s = 0; s < scenery_count; s++) {
                auto &scenery_entry = scenery[s];
                if(scenery_entry.bsp_indices & bsp_bit) {
                    // Is the type set?
                    auto type = scenery_entry.type.read();
                    if(type == NULL_INDEX) {
                        continue;
                    }
                    
                    // Is the palette entry valid? If not... how did the map even compile? We check for that. Anyway, skip if not.
                    auto scenery_tag_id = scenery_palette[type].name.tag_id.read();
                    if(scenery_tag_id.is_null()) {
                        continue;
                    }
                    
                    // Check the model of the scenery
                    auto &scenery_tag = map.get_tag(scenery_tag_id.index);
                    auto &scenery_base_struct = scenery_tag.get_base_struct<HEK::Scenery>();
                    auto model_tag_id = scenery_base_struct.model.tag_id.read();
                    if(model_tag_id.is_null()) {
                        continue;
                    }
                    
                    // Add the model if we haven't already
                    auto &model_id = model_tag_id_to_exported_model_id[model_tag_id.index];
                    if(!model_id.has_value()) {
                        model_id = models.size();
                        auto &model_tag = map.get_tag(model_tag_id.index);
                        
                        switch(model_tag.get_tag_fourcc()) {
                            case HEK::TagFourCC::TAG_FOURCC_GBXMODEL:
                                models.emplace_back(read_model<Parser::GBXModelGeometry::struct_little, Parser::GBXModelGeometryPart::struct_little>(model_tag, model_tag.get_base_struct<HEK::GBXModel>(), materials));
                                break;
                            case HEK::TagFourCC::TAG_FOURCC_MODEL:
                                models.emplace_back(read_model<Parser::ModelGeometry::struct_little, Parser::ModelGeometryPart::struct_little>(model_tag, model_tag.get_base_struct<HEK::Model>(), materials));
                                break;
                            default:
                                std::fprintf(stderr, "Unknown model fourcc");
                                throw std::exception();
                        }
                    }
                    
                    // Add the Add the object now
                    auto &object = objects.emplace_back();
                    object.model = *model_id;
                    object.x = scenery_entry.position.x.read();
                    object.y = scenery_entry.position.y.read();
                    object.z = scenery_entry.position.z.read();
                    object.yaw = scenery_entry.rotation.yaw.read();
                    object.pitch = scenery_entry.rotation.pitch.read();
                    object.roll = scenery_entry.rotation.roll.read();
                }
            }
        }
    }
    catch (std::exception &e) {
        eprintf_error("Failed to build/parse map: %s", e.what());
        std::exit(EXIT_FAILURE);
    }
    
    auto float_to_str = [](const auto &f) -> std::string {
        std::string fstr = std::to_string(f);
        
        while(fstr[fstr.size() - 1] == '0') {
            fstr.resize(fstr.size() - 1);
        }
        if(fstr[fstr.size() - 1] == '.') {
            fstr.resize(fstr.size() - 1);
        }
        
        return fstr;
    };
    
    // Put the version in it
    ADD_LINE("version 1 unbaked");
    
    // Check skies
    if(skies.size() > 1) {
        eprintf_error("Only 1 sky per BSP is currently allowed maximum for this operation");
        std::exit(EXIT_FAILURE);
    }
    
    // Add skies
    for(auto &s : skies) {
        ADD_LINE(std::string("sky \"") + s.path + "\" " + float_to_str(s.outdoor_power) + " " + float_to_str(s.outdoor_red) + " " + float_to_str(s.outdoor_green) + " " + float_to_str(s.outdoor_blue) + " {");
        for(auto &l : s.lights) {
            ADD_LINE(std::string(" light ") + float_to_str(l.power) + " " + float_to_str(l.red) + " " + float_to_str(l.green) + " " + float_to_str(l.blue) + " " + float_to_str(l.yaw) + " " + float_to_str(l.pitch));
        }
        ADD_LINE("}");
    }
    
    // Add materials
    for(auto &mat : materials) {
        ADD_LINE(std::string("material \"") + mat.path + "\" " + ExportedMaterialTypeStr[mat.type] + " " + float_to_str(mat.power) + " rgb " + float_to_str(mat.emission_red) + " " + float_to_str(mat.emission_green) + " " + float_to_str(mat.emission_blue)); // todo: add image sampling (base64 of pixel data maybe - `image <base64>` vs `rgb <red> <green> <blue>`)
    }
    
    // Add models
    auto write_model = [&lines, &float_to_str](auto &m) {
        ADD_LINE(std::string(m.lightmaps.size() ? "scenario_structure_bsp" : "model") + " \"" + m.path + "\" {");
        for(auto &v : m.vertices) {
            ADD_LINE(std::string(" vertex ") + float_to_str(v.x) + " " + float_to_str(v.y) + " " + float_to_str(v.z));
        }
        for(auto &t : m.triangles) {
            ADD_LINE(std::string(" triangle ") + std::to_string(t.a) + " " + std::to_string(t.b) + " " + std::to_string(t.c) + " " + std::to_string(t.material));
        }
        for(auto &l : m.lightmaps) {
            ADD_LINE(std::string(" lightmap ") + std::to_string(l.first_triangle_index) + " " + std::to_string(l.triangle_count));
        }
        ADD_LINE("}");
    };
    
    for(auto &m : bsps) {
        write_model(m);
    }
    
    for(auto &m : models) {
        write_model(m);
    }
    
    // Add objects
    for(auto &o : objects) {
        ADD_LINE(std::string("object ") + std::to_string(o.model) + " " + float_to_str(o.x) + " " + float_to_str(o.y) + " " + float_to_str(o.z) + " " + float_to_str(o.yaw) + " " + float_to_str(o.pitch) + " " + float_to_str(o.roll));
    }
    
#undef ADD_LINE

    // Concatenate into a string
    std::string str;
    for(auto &l : lines) {
        str += std::move(l) + "\n";
    }
    return str;
}

struct ImportedBSPVertex {
    float u, v;
};

struct ImportedBSPTriangle {
    std::size_t vertices[3];
};

struct ImportedBSPLightmap {
    std::size_t first_triangle, triangle_count;
    std::string image_filename;
};

struct ImportedBSP {
    std::string path;
    std::vector<ImportedBSPVertex> vertices;
    std::vector<ImportedBSPTriangle> triangles;
    std::vector<ImportedBSPLightmap> lightmaps;
};
    
void Invader::Lightmap::import_lightmap_mesh(const std::string &mesh_data, const std::filesystem::path &mesh_path, const char *scenario, const char *bsp_name, const std::vector<std::filesystem::path> &tags_directories) {
    auto size = mesh_data.size();
    auto *data = mesh_data.data();
    auto *data_end = data + size;
    
    const char *token_start = nullptr;
    std::vector<std::string> tokens;
    bool in_quote = false;
    
    // Split whitespace
    for(const char *a = data; a < data_end; a++) {
        bool whitespace = !in_quote && (*a == ' ' || *a == '\n' || *a == '\t' || *a == '\r');
        if(whitespace) {
            if(!token_start) {
                continue;
            }
            else {
                auto &str = tokens.emplace_back(token_start, a);
                
                // Remove quotes from string
                for(std::size_t t = 0; t < str.size(); t++) {
                    if(str[t] == '"') {
                        str.erase(str.begin() + t);
                        t--;
                    }
                }
                
                token_start = nullptr;
                continue;
            }
        }
        
        if(!token_start) {
            token_start = a;
        }
        
        if(*a == '"') {
            in_quote = !in_quote;
        }
    }
    
    // If we still are in the middle of a token, error
    if(in_quote) {
        eprintf_error("Failed to parse %s: Unterminated quote", mesh_path.string().c_str());
        std::exit(EXIT_FAILURE);
    }
    if(token_start || (data_end != data && data_end[-1] != '\n')) {
        eprintf_error("Failed to parse %s: It does not end with a newline", mesh_path.string().c_str());
        std::exit(EXIT_FAILURE);
    }
    
    // Is it empty
    if(tokens.empty()) {
        eprintf_error("Failed to parse %s: No tokens were read", mesh_path.string().c_str());
        std::exit(EXIT_FAILURE);
    }
    
    std::size_t next_token = 0;
    auto extract_next_token = [&next_token, &tokens]() -> std::optional<std::string> {
        if(next_token == tokens.size()) {
            return std::nullopt;
        }
        else {
            return tokens[next_token++];
        }
    };
    
    // Version
    if(extract_next_token() != "version") {
        eprintf_error("Input mesh does not start with a version");
        std::exit(EXIT_FAILURE);
    }
    if(extract_next_token() != std::to_string(MESH_FORMAT_VERSION)) {
        eprintf_error("Input mesh does not have a supported version");
        std::exit(EXIT_FAILURE);
    }
    if(extract_next_token() != "baked") {
        eprintf_error("Input mesh is not baked");
        std::exit(EXIT_FAILURE);
    }
    
    // Hold the format here
    std::optional<std::size_t> format_length, format_bpp;
    std::vector<ImportedBSP> bsps;
    
    while(true) {
        auto command_maybe = extract_next_token();
        if(!command_maybe.has_value()) {
            break; // done
        }
        auto &command = *command_maybe;
        
        // Format
        if(command == "format") {
            try {
                format_length = std::stoul(extract_next_token().value());
                format_bpp = std::stoul(extract_next_token().value());
            }
            catch(...) {
                eprintf_error("Invalid format specified.");
                std::exit(EXIT_FAILURE);
            }
        }
        // BSP
        else if(command == "scenario_structure_bsp") {
            try {
                auto &bsp = bsps.emplace_back();
                bsp.path = extract_next_token().value();
                
                // Block
                if(extract_next_token() != "{") {
                    throw std::exception();
                }
                
                while(true) {
                    auto subcommand_maybe = extract_next_token();
                    if(!subcommand_maybe.has_value()) {
                        throw std::exception();
                    }
                    auto &subcommand = *subcommand_maybe;
                    
                    // Done
                    if(subcommand == "}") {
                        break;
                    }
                    else if(subcommand == "vertex") {
                        try {
                            bsp.vertices.emplace_back() = { std::stof(extract_next_token().value()), std::stof(extract_next_token().value()) };
                        }
                        catch(...) {
                            eprintf_error("Invalid vertex specified.");
                            std::exit(EXIT_FAILURE);
                        }
                    }
                    else if(subcommand == "triangle") {
                        try {
                            bsp.triangles.emplace_back() = { std::stoul(extract_next_token().value()), std::stoul(extract_next_token().value()), std::stoul(extract_next_token().value()) };
                        }
                        catch(...) {
                            eprintf_error("Invalid triangle specified.");
                            std::exit(EXIT_FAILURE);
                        }
                    }
                    else if(subcommand == "lightmap") {
                        try {
                            bsp.lightmaps.emplace_back() = { std::stoul(extract_next_token().value()), std::stoul(extract_next_token().value()), extract_next_token().value() };
                        }
                        catch(...) {
                            eprintf_error("Invalid lightmap specified.");
                            std::exit(EXIT_FAILURE);
                        }
                    }
                    else {
                        eprintf_error("Unknown %s command %s", command.c_str(), subcommand.c_str());
                        std::exit(EXIT_FAILURE);
                    }
                }
            }
            catch(...) {
                eprintf_error("Invalid BSP specified.");
                std::exit(EXIT_FAILURE);
            }
        }
        else {
            eprintf_error("Unrecognized command \"%s\"", command.c_str());
            std::exit(EXIT_FAILURE);
        }
    }
    
    // Check the format
    if(!format_length.has_value() || !format_bpp.has_value()) {
        eprintf_error("Input mesh does not specify a format.");
        std::exit(EXIT_FAILURE);
    }
    
    if(bsps.empty()) {
        eprintf_error("Input mesh does have any BSPs.");
        std::exit(EXIT_FAILURE);
    }
    
    if(bsps.size() > 1) {
        eprintf_error("Input mesh has more than 1 BSP (only 1 is supported at this time).");
        std::exit(EXIT_FAILURE);
    }
    
    // Bounds checking
    auto &bsp = bsps[0];
    for(auto &t : bsp.triangles) {
        for(auto &v : t.vertices) {
            if(v >= bsp.vertices.size()) {
                eprintf_error("Input mesh has an out-of-bounds vertex");
                std::exit(EXIT_FAILURE);
            }
        }
    }
    for(auto &l : bsp.lightmaps) {
        if(l.first_triangle >= bsp.triangles.size() || bsp.triangles.size() - l.first_triangle < l.triangle_count) {
            eprintf_error("Input mesh has an out-of-bounds lightmap");
            std::exit(EXIT_FAILURE);
        }
    }
    
    // Make sure the scenario path is here
    auto scenario_path = File::tag_path_to_file_path(std::string(scenario) + ".scenario", tags_directories);
    if(!scenario_path.has_value()) {
        eprintf_error("Cannot find scenario tag %s", scenario);
        std::exit(EXIT_FAILURE);
    }
    
    // Open that
    auto scenario_tag_data = File::open_file(*scenario_path);
    if(!scenario_tag_data.has_value()) {
        eprintf_error("Failed to open scenario tag. Make sure you have read permission!");
        std::exit(EXIT_FAILURE);
    }
    
    // Parse that shit
    std::unique_ptr<Parser::ParserStruct> scenario_tag_data_struct;
    Parser::Scenario *scenario_tag;
    try {
        scenario_tag_data_struct = Parser::ParserStruct::parse_hek_tag_file(scenario_tag_data->data(), scenario_tag_data->size());
        scenario_tag = dynamic_cast<decltype(scenario_tag)>(scenario_tag_data_struct.get());
    }
    catch(std::exception &e) {
        eprintf_error("Failed to parse scenario tag: %s", e.what());
        std::exit(EXIT_FAILURE);
    }
    
    // Go through each BSP
    std::optional<std::string> bsp_path;
    for(auto &b : scenario_tag->structure_bsps) {
        auto &path = b.structure_bsp.path;
        if(path.empty()) {
            continue;
        }
        auto base_name = File::base_name(path);
        if(base_name == bsp_name) {
            bsp_path = path + "." + HEK::tag_fourcc_to_extension(b.structure_bsp.tag_fourcc);
            break;
        }
    }
    
    if(!bsp_path.has_value()) {
        eprintf_error("Scenario tag does not have a BSP named %s", bsp_name);
        std::exit(EXIT_FAILURE);
    }
    if(bsp_path != bsp.path) {
        eprintf_error("Input mesh refers to a different BSP tag (\"%s\", not \"%s\")", bsp.path.c_str(), bsp_path->c_str());
        std::exit(EXIT_FAILURE);
    }
    
    // BSP
    auto bsp_path_file = File::tag_path_to_file_path(*bsp_path, tags_directories);
    if(!bsp_path_file.has_value()) {
        eprintf_error("Cannot find scenario BSP tag %s", bsp_path->c_str());
        std::exit(EXIT_FAILURE);
    }
    
    auto scenario_bsp_data = File::open_file(*bsp_path_file);
    if(!scenario_bsp_data.has_value()) {
        eprintf_error("Failed to open scenario BSP. Make sure you have read permission!");
        std::exit(EXIT_FAILURE);
    }
    
    std::unique_ptr<Parser::ParserStruct> scenario_bsp_struct;
    Parser::ScenarioStructureBSP *scenario_bsp;
    try {
        scenario_bsp_struct = Parser::ParserStruct::parse_hek_tag_file(scenario_bsp_data->data(), scenario_bsp_data->size());
        scenario_bsp = dynamic_cast<decltype(scenario_bsp)>(scenario_bsp_struct.get());
    }
    catch(std::exception &e) {
        eprintf_error("Failed to parse scenario tag: %s", e.what());
        std::exit(EXIT_FAILURE);
    }
    
    // UVs
    auto *uvs = bsp.vertices.data();
    auto *tris = bsp.triangles.data();
    
    // Check this stuff
    auto *bsp_surfaces = scenario_bsp->surfaces.data();
    auto bsp_surface_count = scenario_bsp->surfaces.size();
    if(bsp.triangles.size() > bsp_surface_count) {
        eprintf_error("BSP mismatch: Incorrect number of triangles");
        std::exit(EXIT_FAILURE);
    }
    
    // Go through each one!
    for(auto &lm : scenario_bsp->lightmaps) {
        if(lm.bitmap == NULL_INDEX) {
            continue;
        }
        if(lm.bitmap >= bsp.lightmaps.size()) {
            eprintf_error("BSP mismatch: Incorrect number of lightmaps");
            std::exit(EXIT_FAILURE);
        }
        
        for(auto &mat : lm.materials) {
            std::vector<Parser::ScenarioStructureBSPMaterialUncompressedRenderedVertex::struct_little> uncompressed_vertices_duped;
            std::vector<Parser::ScenarioStructureBSPMaterialUncompressedLightmapVertex::struct_little> uncompressed_lightmap_vertices_duped;
            
            auto &rendered_vertices_count = mat.rendered_vertices_count;
            auto *uncompressed_vertices = reinterpret_cast<decltype(uncompressed_vertices_duped.data())>(mat.uncompressed_vertices.data());
            auto expected_size = sizeof(uncompressed_vertices_duped[0]) * rendered_vertices_count;
            if(mat.uncompressed_vertices.size() < expected_size) {
                eprintf_error("BSP uncompressed vertices size is wrong");
                std::exit(EXIT_FAILURE);
            }
            
            // Bounds check
            auto first_surface = static_cast<std::size_t>(mat.surfaces);
            auto last_surface = first_surface + mat.surface_count;
            if(last_surface > bsp_surface_count) {
                eprintf_error("BSP surfaces are out of bounds");
                std::exit(EXIT_FAILURE);
            }
            
            auto *triangles = bsp_surfaces + first_surface;
            auto *triangles_end = bsp_surfaces + last_surface;
            auto *imported_triangles = tris + first_surface;
            
            for(auto *t = triangles; t < triangles_end; t++, imported_triangles++) {
                // Blow it up
                auto dupe_it_all_to_hell = [&uncompressed_vertices_duped, &uncompressed_vertices, &rendered_vertices_count, &uncompressed_lightmap_vertices_duped, &uvs](HEK::Index &index, std::size_t vertex_index) {
                    if(index >= rendered_vertices_count) {
                        eprintf_error("BSP surface vertices are out of bounds");
                        std::exit(EXIT_FAILURE);
                    }
                    uncompressed_vertices_duped.emplace_back(uncompressed_vertices[index]);
                    index = uncompressed_vertices_duped.size() - 1;
                    
                    auto &lm_vertex = uncompressed_lightmap_vertices_duped.emplace_back();
                    lm_vertex.normal.i = 1.0F;
                    lm_vertex.normal.j = 0.0F;
                    lm_vertex.normal.k = 0.0F;
                    lm_vertex.texture_coords.x = uvs[vertex_index].u;
                    lm_vertex.texture_coords.y = uvs[vertex_index].v;
                };
                
                // Do this
                dupe_it_all_to_hell(t->vertex0_index, imported_triangles->vertices[0]);
                dupe_it_all_to_hell(t->vertex1_index, imported_triangles->vertices[1]);
                dupe_it_all_to_hell(t->vertex2_index, imported_triangles->vertices[2]);
            }
            
            // Insert the stuff
            mat.rendered_vertices_count = uncompressed_vertices_duped.size();
            mat.rendered_vertices_offset = 0;
            mat.uncompressed_vertices = std::vector<std::byte>(reinterpret_cast<std::byte *>(uncompressed_vertices_duped.data()), reinterpret_cast<std::byte *>(uncompressed_vertices_duped.data() + mat.rendered_vertices_count));
            
            mat.lightmap_vertices_count = uncompressed_lightmap_vertices_duped.size();
            mat.lightmap_vertices_offset = mat.uncompressed_vertices.size();
            mat.uncompressed_vertices.insert(mat.uncompressed_vertices.end(), reinterpret_cast<std::byte *>(uncompressed_lightmap_vertices_duped.data()), reinterpret_cast<std::byte *>(uncompressed_lightmap_vertices_duped.data() + mat.lightmap_vertices_count));
        }
    }
    
    File::save_file(*bsp_path_file, scenario_bsp->generate_hek_tag_data(HEK::TagFourCC::TAG_FOURCC_SCENARIO_STRUCTURE_BSP));
}
