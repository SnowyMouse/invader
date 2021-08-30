// SPDX-License-Identifier: GPL-3.0-only

#include <invader/build/build_workload.hpp>
#include <invader/tag/parser/definition/gbxmodel.hpp>
#include <invader/tag/parser/definition/model.hpp>
#include <invader/tag/parser/compile/model.hpp>
#include <invader/map/map.hpp>

namespace Invader::Parser {
    template<typename M> static void postprocess_hek_data_model(M &what) {
        what.super_high_detail_node_count = 0;
        what.high_detail_node_count = 0;
        what.medium_detail_node_count = 0;
        what.low_detail_node_count = 0;
        what.super_low_detail_node_count = 0;

        auto &geometries = what.geometries;
        auto find_highest = [&geometries, &what](std::uint16_t &c, Index what_index) {
            if(what_index >= geometries.size()) {
                c = 0;
            }
            else {
                for(auto &p : geometries[what_index].parts) {
                    bool use_local_nodes = what.flags & ModelFlagsFlag::MODEL_FLAGS_FLAG_PARTS_HAVE_LOCAL_NODES && sizeof(typename std::remove_reference<decltype(p)>::type::template C<LittleEndian>) == sizeof(GBXModelGeometryPart::C<LittleEndian>);
                
                    for(auto &v : p.uncompressed_vertices) {
                        std::uint16_t node0 = v.node0_index;
                        std::uint16_t node1 = v.node1_index;

                        // If zoner, use local nodes
                        if(use_local_nodes) {
                            auto &p_gbx = *reinterpret_cast<GBXModelGeometryPart *>(&p);
                            constexpr const std::size_t LOCAL_NODE_COUNT = sizeof(p_gbx.local_node_indices) / sizeof(p_gbx.local_node_indices[0]);
                            node0 = (node0 < LOCAL_NODE_COUNT) ? p_gbx.local_node_indices[node0] : NULL_INDEX;
                            node1 = (node1 < LOCAL_NODE_COUNT) ? p_gbx.local_node_indices[node1] : NULL_INDEX;
                        }

                        if(node0 != NULL_INDEX && c < node0) {
                            c = node0;
                        }

                        if(node1 != NULL_INDEX && c < node1) {
                            c = node1;
                        }
                    }
                }
            }
        };

        for(auto &r : what.regions) {
            for(auto &p : r.permutations) {
                find_highest(what.super_high_detail_node_count, p.super_high);
                find_highest(what.high_detail_node_count, p.high);
                find_highest(what.medium_detail_node_count, p.medium);
                find_highest(what.low_detail_node_count, p.low);
                find_highest(what.super_low_detail_node_count, p.super_low);
            }
        }
    }
    
    void GBXModel::postprocess_hek_data() {
        postprocess_hek_data_model(*this);
    }
    
    void Model::postprocess_hek_data() {
        postprocess_hek_data_model(*this);
    }
    
    template <class M> static void model_post_compile(M &what, BuildWorkload &workload, std::size_t struct_index, std::size_t offset, std::size_t) {
        // Put all of the markers in the marker array
        auto &markers = what.markers;
        auto region_count = what.regions.size();
        
        for(std::size_t ri = 0; ri < region_count; ri++) {
            auto &r = what.regions[ri];
            std::size_t permutation_count = r.permutations.size();
            for(std::size_t pi = 0; pi < permutation_count; pi++) {
                auto &p = r.permutations[pi];

                // Add the markers
                while(p.markers.size()) {
                    auto add_marker = [&p, &ri, &pi, &markers](std::size_t index) {
                        // Pop
                        auto m = p.markers[index];
                        p.markers.erase(p.markers.begin() + index);

                        // Make the instance
                        ModelMarkerInstance instance;
                        instance.node_index = m.node_index;
                        instance.permutation_index = pi;
                        instance.region_index = ri;
                        instance.rotation = m.rotation;
                        instance.translation = m.translation;

                        // Add it!
                        bool found = false;
                        for(auto &ma : markers) {
                            if(m.name == ma.name) {
                                ma.instances.push_back(instance);
                                found = true;
                                break;
                            }
                        }

                        // Add the marker and then add it!
                        if(!found) {
                            ModelMarker ma = {};

                            ma.name = m.name;
                            ma.instances.push_back(instance);

                            for(auto &marker_group : markers) {
                                if(std::strcmp(marker_group.name.string, m.name.string) > 0) {
                                    markers.insert(markers.begin() + (&marker_group - markers.data()), ma);
                                    found = true;
                                    break;
                                }
                            }

                            if(!found) {
                                markers.push_back(ma);
                            }
                        }
                    };

                    // Get the first instance
                    auto first_instance_name = p.markers[0].name;

                    // Add all of the instances after it, first
                    for(std::size_t f = 1; f < p.markers.size(); f++) {
                        if(p.markers[f].name == first_instance_name) {
                            add_marker(f);
                            f--;
                        }
                    }

                    // Lastly, add this one
                    add_marker(0);
                }
            }
        }

        // Generate it
        auto marker_count = what.markers.size();
        auto &gbxmodel_data = *reinterpret_cast<typename M::template C<LittleEndian> *>(workload.structs[struct_index].data.data() + offset);
        gbxmodel_data.markers.count = static_cast<std::uint32_t>(marker_count);

        if(marker_count > 0) {
            // Make the pointer
            auto &marker_ptr = workload.structs[struct_index].pointers.emplace_back();
            marker_ptr.offset = static_cast<std::uint32_t>(reinterpret_cast<std::byte *>(&gbxmodel_data.markers.pointer) - reinterpret_cast<std::byte *>(&gbxmodel_data));
            std::size_t marker_struct_index = workload.structs.size();
            marker_ptr.struct_index = marker_struct_index;

            // Make the struct
            auto &markers_struct = workload.structs.emplace_back();
            ModelMarker::C<LittleEndian> *markers_struct_arr;
            markers_struct.data = std::vector<std::byte>(marker_count * sizeof(*markers_struct_arr));
            markers_struct_arr = reinterpret_cast<decltype(markers_struct_arr)>(markers_struct.data.data());

            // Go through each marker
            for(std::size_t m = 0; m < marker_count; m++) {
                auto &marker_c = what.markers[m];
                auto &marker_l = markers_struct_arr[m];

                // Copy this stuff
                marker_l.name = marker_c.name;
                marker_l.magic_identifier = marker_c.magic_identifier;

                // Generate instances
                auto instance_count = static_cast<std::uint32_t>(marker_c.instances.size());
                marker_l.instances.count = instance_count;

                // Make the pointer
                auto &marker_ptr = workload.structs[marker_struct_index].pointers.emplace_back();
                marker_ptr.offset = static_cast<std::uint32_t>(reinterpret_cast<std::byte *>(&marker_l.instances.pointer) - reinterpret_cast<std::byte *>(markers_struct_arr));
                marker_ptr.struct_index = workload.structs.size();

                // Make the instances
                auto &instance_struct = workload.structs.emplace_back();
                ModelMarkerInstance::C<LittleEndian> *instances_struct_arr;
                instance_struct.data = std::vector<std::byte>(sizeof(*instances_struct_arr) * instance_count);
                instances_struct_arr = reinterpret_cast<decltype(instances_struct_arr)>(instance_struct.data.data());
                for(std::size_t i = 0; i < instance_count; i++) {
                    instances_struct_arr[i].node_index = marker_c.instances[i].node_index;
                    instances_struct_arr[i].permutation_index = marker_c.instances[i].permutation_index;
                    instances_struct_arr[i].region_index = marker_c.instances[i].region_index;
                    instances_struct_arr[i].rotation = marker_c.instances[i].rotation;
                    instances_struct_arr[i].translation = marker_c.instances[i].translation;
                }
            }
        }
    }

    void GBXModel::post_compile(BuildWorkload &workload, std::size_t tag_index, std::size_t struct_index, std::size_t offset) {
        model_post_compile(*this, workload, struct_index, offset, tag_index);
    }

    void Model::post_compile(BuildWorkload &workload, std::size_t tag_index, std::size_t struct_index, std::size_t offset) {
        if(this->flags & ModelFlagsFlag::MODEL_FLAGS_FLAG_PARTS_HAVE_LOCAL_NODES) {
            workload.report_error(BuildWorkload::ErrorType::ERROR_TYPE_FATAL_ERROR, "Non-Gearbox models are not allowed to have \"parts have local nodes\" set but this model does", tag_index);
            throw InvalidTagDataException();
        }
        
        model_post_compile(*this, workload, struct_index, offset, tag_index);
    }
    
    template <class P, class M> static bool regenerate_missing_model_vertices_part(P &part, M &model, bool fix) {
        // If we have no uncompressed vertices, try to regenerate those
        if(part.uncompressed_vertices.size() == 0 && part.compressed_vertices.size() > 0) {
            if(!fix) {
                return true;
            }
            
            part.uncompressed_vertices.reserve(part.compressed_vertices.size());
            for(auto &v : part.compressed_vertices) {
                auto before_data = v.generate_hek_tag_data();
                auto after_data = decompress_model_vertex(*reinterpret_cast<ModelVertexCompressed::C<BigEndian> *>(before_data.data()));
                auto &after_data_write = part.uncompressed_vertices.emplace_back();
                after_data_write.binormal = after_data.binormal;
                after_data_write.normal = after_data.normal;
                after_data_write.position = after_data.position;
                after_data_write.tangent = after_data.tangent;
                after_data_write.node0_index = after_data.node0_index;
                after_data_write.node0_weight = after_data.node0_weight;
                after_data_write.node1_index = after_data.node1_index;
                after_data_write.node1_weight = after_data.node1_weight;
                after_data_write.texture_coords = after_data.texture_coords;
            }
        }
        else if(part.compressed_vertices.size() == 0 && part.uncompressed_vertices.size() > 0) {
            auto resolve_local_node = [&part, &model](Index local_index) -> Index {
                // If we don't have any local nodes, just passthrough
                if((sizeof(typename P::template C<LittleEndian>) != sizeof(GBXModelGeometryPart::C<LittleEndian>)) || !(model.flags & ModelFlagsFlag::MODEL_FLAGS_FLAG_PARTS_HAVE_LOCAL_NODES)) {
                    return local_index;
                }
                
                // And if the local node passed is null, just return it
                if(local_index == NULL_INDEX) {
                    return NULL_INDEX;
                }
                
                // Do it
                auto *part_gbx = reinterpret_cast<GBXModelGeometryPart *>(&part);
                if(local_index >= part_gbx->local_node_count) {
                    eprintf_error("Can't fix: Index is out-of-bounds for local node indices (%u vs %u)", local_index, part_gbx->local_node_count);
                    throw InvalidTagDataException();
                }
                return part_gbx->local_node_indices[local_index];
            };
            
            // Can we even do this?
            for(auto &v : part.uncompressed_vertices) {
                auto n0 = resolve_local_node(v.node0_index);
                auto n1 = resolve_local_node(v.node1_index);
                if((n0 > MaxCompressedModelNodeIndex::MAX_COMPRESSED_MODEL_NODE_INDEX && n0 < NULL_INDEX) || (n1 > MaxCompressedModelNodeIndex::MAX_COMPRESSED_MODEL_NODE_INDEX && n1 < NULL_INDEX)) {
                    return false;
                }
            }
            
            if(!fix) {
                return true;
            }
            
            part.compressed_vertices.reserve(part.uncompressed_vertices.size());
            
            for(auto &v : part.uncompressed_vertices) {
                // Resolve local nodes before throwing them into the compressor
                auto before_data_copy = *reinterpret_cast<ModelVertexUncompressed::C<BigEndian> *>(v.generate_hek_tag_data().data());
                before_data_copy.node0_index = resolve_local_node(before_data_copy.node0_index);
                before_data_copy.node1_index = resolve_local_node(before_data_copy.node1_index);
                
                // Done
                auto after_data = compress_model_vertex(before_data_copy);
                auto &after_data_write = part.compressed_vertices.emplace_back();
                after_data_write.binormal = after_data.binormal;
                after_data_write.normal = after_data.normal;
                after_data_write.position = after_data.position;
                after_data_write.tangent = after_data.tangent;
                after_data_write.node0_index = after_data.node0_index;
                after_data_write.node0_weight = after_data.node0_weight;
                after_data_write.node1_index = after_data.node1_index;
                after_data_write.texture_coordinate_u = after_data.texture_coordinate_u;
                after_data_write.texture_coordinate_v = after_data.texture_coordinate_v;
            }
        }
        else if(part.compressed_vertices.size() != part.uncompressed_vertices.size()) {
            eprintf_error("Can't fix: Compressed vertex and uncompressed vertex counts don't match");
            throw InvalidTagDataException();
        }
        else {
            return false;
        }
        return true;
    }

    bool regenerate_missing_model_vertices(GBXModelGeometryPart &part, GBXModel &model, bool fix) {
        return regenerate_missing_model_vertices_part(part, model, fix);
    }

    bool regenerate_missing_model_vertices(ModelGeometryPart &part, Model &model, bool fix) {
        return regenerate_missing_model_vertices_part(part, model, fix);
    }

    template <class M> static bool regenerate_missing_model_vertices_model(M &model, bool fix) {
        bool result = false;
        for(auto &g : model.geometries) {
            for(auto &p : g.parts) {
                result = regenerate_missing_model_vertices(p, model, fix) || result;
            }
        }
        
        return result;
    }

    bool regenerate_missing_model_vertices(GBXModel &model, bool fix) {
        return regenerate_missing_model_vertices_model(model, fix);
    }

    bool regenerate_missing_model_vertices(Model &model, bool fix) {
        return regenerate_missing_model_vertices_model(model, fix);
    }
    
    template <class P, typename PS> static void post_cache_parse_model_part(P &what, const PS &part, const Tag &tag) {
        const auto &map = tag.get_map();

        // Get model vertices and indices count
        std::size_t vertex_count = part.vertex_count.read();
        std::size_t index_count = part.triangle_count.read() + 2;
        std::size_t triangle_count = (index_count) / 3;
        std::size_t triangle_modulo = index_count % 3;
        const LittleEndian<Index> *indices = nullptr;

        // Get vertices
        if(vertex_count > 0) {
            // Xbox maps use compressed vertices at a given address
            if(tag.get_map().get_engine() == CacheFileEngine::CACHE_FILE_XBOX) {
                const auto vertex_pointer = reinterpret_cast<const CacheFileModelPartVerticesXbox *>(tag.data(what.vertex_offset, sizeof(CacheFileModelPartVerticesXbox)))->vertices;
                const auto *vertices = reinterpret_cast<const ModelVertexCompressed::C<LittleEndian> *>(tag.data(vertex_pointer, sizeof(ModelVertexCompressed::C<LittleEndian>) * vertex_count));
                for(std::size_t v = 0; v < vertex_count; v++) {
                    std::size_t cursor = 0;
                    ModelVertexCompressed::C<BigEndian> vertex_compressed = vertices[v];
                    what.compressed_vertices.emplace_back(ModelVertexCompressed::parse_hek_tag_data(reinterpret_cast<const std::byte *>(&vertex_compressed), sizeof(vertex_compressed), cursor, vertex_compressed));
                }
                indices = reinterpret_cast<const LittleEndian<Index> *>(tag.data(what.triangle_offset, sizeof(std::uint16_t) * index_count));
            }
            // Other maps use uncompressed vertices at an offset
            else {
                auto model_data_offset = map.get_model_data_offset();
                auto model_index_offset = map.get_model_index_offset() + model_data_offset;
                const auto *vertices = reinterpret_cast<const ModelVertexUncompressed::C<LittleEndian> *>(map.get_data_at_offset(model_data_offset + part.vertex_offset.read(), sizeof(ModelVertexUncompressed::C<LittleEndian>) * vertex_count));
                for(std::size_t v = 0; v < vertex_count; v++) {
                    std::size_t cursor = 0;
                    ModelVertexUncompressed::C<BigEndian> vertex_uncompressed = vertices[v];
                    what.uncompressed_vertices.emplace_back(ModelVertexUncompressed::parse_hek_tag_data(reinterpret_cast<const std::byte *>(&vertex_uncompressed), sizeof(vertex_uncompressed), cursor, vertex_uncompressed));
                }
                indices = reinterpret_cast<const LittleEndian<Index> *>(map.get_data_at_offset(model_index_offset + part.triangle_offset.read(), sizeof(std::uint16_t) * index_count));
            }
        }

        // Get model indices
        for(std::size_t t = 0; t < triangle_count; t++) {
            auto &triangle = what.triangles.emplace_back();
            auto *triangle_indices = indices + t * 3;
            triangle.vertex0_index = triangle_indices[0];
            triangle.vertex1_index = triangle_indices[1];
            triangle.vertex2_index = triangle_indices[2];
        }

        if(triangle_modulo) {
            auto &straggler_triangle = what.triangles.emplace_back();
            auto *triangle_indices = indices + triangle_count * 3;
            straggler_triangle.vertex0_index = triangle_indices[0];
            straggler_triangle.vertex1_index = triangle_modulo > 1 ? triangle_indices[1].read() : NULL_INDEX;
            straggler_triangle.vertex2_index = NULL_INDEX;
        }
    }

    void GBXModelGeometryPart::post_cache_parse(const Tag &tag, std::optional<Pointer> pointer) {
        post_cache_parse_model_part(*this, tag.get_struct_at_pointer<GBXModelGeometryPart::C>(*pointer), tag);
    }

    void ModelGeometryPart::post_cache_parse(const Tag &tag, std::optional<Pointer> pointer) {
        post_cache_parse_model_part(*this, tag.get_struct_at_pointer<ModelGeometryPart::C>(*pointer), tag);
    }

    template<typename M> static void pre_compile_model(M &what, BuildWorkload &workload, std::size_t tag_index) {
        std::size_t region_count = what.regions.size();
        std::size_t geometries_count = what.geometries.size();
        std::size_t node_count = what.nodes.size();
        
        // Make sure we HAVE nodes
        if(node_count == 0) {
            REPORT_ERROR_PRINTF(workload, ERROR_TYPE_FATAL_ERROR, tag_index, "Model has no nodes");
            throw InvalidTagDataException();
        }
        
        for(std::size_t ri = 0; ri < region_count; ri++) {
            auto &r = what.regions[ri];
            std::size_t permutation_count = r.permutations.size();
            for(std::size_t pi = 0; pi < permutation_count; pi++) {
                auto &p = r.permutations[pi];

                #define DO_GEO_CHECK(what, what_str) \
                if(p.what >= geometries_count) { \
                    REPORT_ERROR_PRINTF(workload, ERROR_TYPE_FATAL_ERROR, tag_index, what_str " geometry of permutation %zu of region %zu is incorrect (%zu >= %zu)", pi, ri, static_cast<std::size_t>(p.what), geometries_count); \
                    throw InvalidTagDataException(); \
                }
                DO_GEO_CHECK(super_high, "Super high");
                DO_GEO_CHECK(high, "High");
                DO_GEO_CHECK(medium, "Medium");
                DO_GEO_CHECK(low, "Low");
                DO_GEO_CHECK(super_low, "Super low");
                #undef DO_GEO_CHECK
            }
        }
        
        // Handle blended normals
        if(what.flags & ModelFlagsFlag::MODEL_FLAGS_FLAG_BLEND_SHARED_NORMALS) {
            std::vector<Point3D<NativeEndian>> positions;
            
            // Get all of the possible vertex positions
            for(auto &g : what.geometries) {
                for(auto &p : g.parts) {
                    for(auto &v : p.uncompressed_vertices) {
                        bool present = false;
                        for(auto &pos : positions) {
                            if(pos == v.position) {
                                present = true;
                                break;
                            }
                        }
                        if(!present) {
                            positions.emplace_back(v.position);
                        }
                    }
                    
                }
            }
            
            // Do we have anything?
            if(positions.size() == 0) {
                REPORT_ERROR_PRINTF(workload, ERROR_TYPE_FATAL_ERROR, tag_index, "Unable to blend shared normals due to missing uncompressed vertices");
                throw InvalidTagDataException();
            }
            else {
                // Add all the stuff (idk why it does this but lol)
                for(auto &pos : positions) {
                    // Add up all of the fun stuff
                    Vector3D<NativeEndian> vectors = {};
                    for(auto &g : what.geometries) {
                        for(auto &p : g.parts) {
                            for(auto &v : p.uncompressed_vertices) {
                                if(pos == v.position) {
                                    vectors = vectors + v.normal;
                                }
                            }
                        }
                    }
                    
                    // Account for memed stuff
                    if(vectors.i == 0.0F && vectors.j == 0.0F && vectors.k == 0.0F) {
                        vectors.i = 1.0F;
                    }
                    
                    // Normalize
                    auto normalized = vectors.normalize();
                    auto normalized_compressed = compress_vector(normalized.i, normalized.j, normalized.k);
                    
                    // Store the stuff
                    for(auto &g : what.geometries) {
                        for(auto &p : g.parts) {
                            for(auto &v : p.uncompressed_vertices) {
                                if(pos == v.position) {
                                    v.normal = normalized;
                                }
                            }
                            for(auto &v : p.compressed_vertices) {
                                if(pos == v.position) {
                                    v.normal = normalized_compressed;
                                }
                            }
                        }
                    }
                }
            }
        }

        bool model_part_warned = false;
        bool uses_local_part_nodes = what.flags & ModelFlagsFlag::MODEL_FLAGS_FLAG_PARTS_HAVE_LOCAL_NODES;
        for(std::size_t gi = 0; gi < geometries_count; gi++) {
            auto &g = what.geometries[gi];
            std::size_t parts_count = g.parts.size();
            for(std::size_t pi = 0; pi < parts_count; pi++) {
                auto &p = g.parts[pi];
                
                // Check this stuff!
                std::size_t compressed_vertex_count = p.compressed_vertices.size();
                std::size_t uncompressed_vertex_count = p.uncompressed_vertices.size();
                if(!model_part_warned && uncompressed_vertex_count != compressed_vertex_count && compressed_vertex_count != 0) {
                    REPORT_ERROR_PRINTF(workload, ERROR_TYPE_WARNING, tag_index, "Compressed vertex count (%zu) is not equal to uncompressed (%zu)", compressed_vertex_count, uncompressed_vertex_count)
                    eprintf_warn("To fix this, rebuild the model tag");
                    model_part_warned = true;
                }

                // Set these
                p.centroid_primary_node = 0;
                p.centroid_secondary_node = 0;
                p.centroid_primary_weight = 1.0F;
                p.centroid_secondary_weight = 0.0F;
                
                // CAN we use local part nodes
                if(uses_local_part_nodes && sizeof(typename std::remove_reference<decltype(p)>::type::template C<LittleEndian>) != sizeof(GBXModelGeometryPart::C<LittleEndian>)) {
                    REPORT_ERROR_PRINTF(workload, ERROR_TYPE_FATAL_ERROR, tag_index, "Part #%zu of geometry #%zu uses local part nodes, but they aren't supported for non-gbxmodels", pi, gi);
                    eprintf_warn("To fix this, rebuild the model tag");
                    throw InvalidTagDataException();
                }

                // Calculate weight
                std::size_t node_count = what.nodes.size();
                std::vector<float> node_weight(node_count, 0.0F);
                std::size_t vertices_count = p.uncompressed_vertices.size();
                for(std::size_t vi = 0; vi < vertices_count; vi++) {
                    auto &v = p.uncompressed_vertices[vi];
                    std::size_t node0_index = static_cast<std::size_t>(v.node0_index);
                    std::size_t node1_index = static_cast<std::size_t>(v.node1_index);
                    
                    if(uses_local_part_nodes) {
                        // using reinterpret cast here because we don't want the compiler to complain about ModelPart not having local nodes
                        auto &p_gbx = *reinterpret_cast<GBXModelGeometryPart *>(&p);
                        auto get_local_node = [&workload, &tag_index, &p_gbx, &vi, &pi, &gi](std::size_t index) -> std::size_t {
                            if(index != NULL_INDEX) {
                                constexpr const std::size_t LOCAL_NODE_COUNT = sizeof(p_gbx.local_node_indices) / sizeof(p_gbx.local_node_indices[0]);
                                if(index > LOCAL_NODE_COUNT) {
                                    REPORT_ERROR_PRINTF(workload, ERROR_TYPE_FATAL_ERROR, tag_index, "Vertex #%zu for part %zu of geometry %zu has an incorrect local node (%zu > %zu)", vi, pi, gi, index, LOCAL_NODE_COUNT);
                                    eprintf_warn("To fix this, rebuild the model tag");
                                    throw InvalidTagDataException();
                                }
                                return p_gbx.local_node_indices[index];
                            }
                            else {
                                return NULL_INDEX;
                            }
                        };

                        node0_index = get_local_node(node0_index);
                        node1_index = get_local_node(node1_index);
                    }

                    if(v.node0_index != NULL_INDEX) {
                        if(node0_index > node_count) {
                            REPORT_ERROR_PRINTF(workload, ERROR_TYPE_FATAL_ERROR, tag_index, "Vertex #%zu for part %zu of geometry %zu has an incorrect node0 (%zu > %zu)", vi, pi, gi, node0_index, node_count);
                            eprintf_warn("To fix this, rebuild the model tag");
                            throw InvalidTagDataException();
                        }
                        node_weight[node0_index] += v.node0_weight;
                    }

                    if(v.node1_index != NULL_INDEX) {
                        if(node1_index > node_count) {
                            REPORT_ERROR_PRINTF(workload, ERROR_TYPE_FATAL_ERROR, tag_index, "Vertex #%zu for part %zu of geometry %zu has an incorrect node1 (%zu > %zu)", vi, pi, gi, node1_index, node_count);
                            eprintf_warn("To fix this, rebuild the model tag");
                            throw InvalidTagDataException();
                        }
                        node_weight[node1_index] += v.node1_weight;
                    }
                }

                // Sort nodes from highest to lowest if we have more than one node
                if(node_count > 1) {
                    std::vector<std::size_t> highest_nodes;
                    highest_nodes.reserve(node_count);
                    for(std::size_t n = 0; n < node_count; n++) {
                        bool added = false;
                        for(std::size_t n2 = 0; n2 < highest_nodes.size(); n2++) {
                            if(node_weight[n] > node_weight[highest_nodes[n2]]) {
                                highest_nodes.insert(highest_nodes.begin() + n2, n);
                                added = true;
                                break;
                            }
                        }
                        if(!added) {
                            highest_nodes.emplace_back(n);
                        }
                    }

                    // Check the top two
                    std::size_t first_highest = highest_nodes[0];
                    std::size_t second_highest = highest_nodes[1];
                    float first_highest_weight = node_weight[first_highest];
                    float second_highest_weight = node_weight[second_highest];

                    // If we have a centroid primary node, let's hear it
                    if(first_highest_weight > 0.0F) {
                        // Set the centroid primary node
                        p.centroid_primary_node = static_cast<Index>(first_highest);

                        // Next, do we have a secondary node? If so, divide the weight between the two and set the secondary node
                        if(second_highest_weight > 0.0F) {
                            p.centroid_secondary_node = static_cast<Index>(second_highest);
                            float total_weight = first_highest_weight + second_highest_weight;
                            p.centroid_primary_weight = first_highest_weight / total_weight;
                            p.centroid_secondary_weight = second_highest_weight / total_weight;
                        }
                    }
                }
            }
        }

        // Swap this stuff
        float super_low = what.super_low_detail_cutoff;
        float low = what.low_detail_cutoff;
        float high = what.high_detail_cutoff;
        float super_high = what.super_high_detail_cutoff;

        what.super_low_detail_cutoff = super_high;
        what.low_detail_cutoff = high;
        what.high_detail_cutoff = low;
        what.super_high_detail_cutoff = super_low;

        // Make sure the node indices in the markers we added are valid
        std::size_t errors_given = 0;
        static constexpr std::size_t MAX_ERRORS = 5;
        for(auto &m : what.markers) {
            for(auto &i : m.instances) {
                if(i.node_index >= node_count) {
                    if(++errors_given != MAX_ERRORS) {
                        REPORT_ERROR_PRINTF(workload, ERROR_TYPE_FATAL_ERROR, tag_index, "Instance #%zu of marker #%zu has an invalid node index (%zu >= %zu)", &m - what.markers.data(), &i - m.instances.data(), static_cast<std::size_t>(i.node_index), node_count);
                        errors_given++;
                    }
                    else {
                        REPORT_ERROR_PRINTF(workload, ERROR_TYPE_FATAL_ERROR, tag_index, "... and more errors");
                        break;
                    }
                }
            }
            if(errors_given == MAX_ERRORS) {
                break;
            }
        }
        if(errors_given > 0) {
            eprintf("This can be fixed by recompiling the model");
            throw InvalidTagDataException();
        }

        // Set node stuff
        std::vector<bool> node_done(node_count);
        auto *nodes = what.nodes.data();

        auto write_node_data = [&node_done, &nodes, &node_count, &workload, &tag_index](Index node_index, const auto &base_rotation, const auto &base_translation, const auto &recursion) {
            if(node_index == NULL_INDEX) {
                return;
            }
            if(node_done[node_index]) {
                return;
            }
            node_done[node_index] = true;

            auto &node = nodes[node_index];
            node.scale = 1.0f;

            auto node_rotation = quaternion_to_matrix(node.default_rotation);
            auto total_rotation = multiply_matrix(base_rotation, node_rotation);
            node.rotation = total_rotation;

            auto node_translation = multiply_vector(node.default_translation, -1.0);
            auto total_translation = rotate_vector(add_vector(node_translation, base_translation), node_rotation);
            node.translation = total_translation;

            bool fatal = false;
            if(node.next_sibling_node_index != NULL_INDEX && node.next_sibling_node_index >= node_count) {
                REPORT_ERROR_PRINTF(workload, ERROR_TYPE_FATAL_ERROR, tag_index, "Next sibling node index of node %u is invalid (%u >= %zu)", node_index, node.next_sibling_node_index, node_count);
                fatal = true;
            }
            if(node.first_child_node_index != NULL_INDEX && node.first_child_node_index >= node_count) {
                REPORT_ERROR_PRINTF(workload, ERROR_TYPE_FATAL_ERROR, tag_index, "First child node index of node %u is invalid (%u >= %zu)", node_index, node.first_child_node_index, node_count);
                fatal = true;
            }
            if(fatal) {
                throw InvalidTagDataException();
            }

            recursion(node.next_sibling_node_index, base_rotation, base_translation, recursion);
            recursion(node.first_child_node_index, total_rotation, total_translation, recursion);
        };

        Matrix<LittleEndian> identity = {};
        for(int i = 0; i < 3; i++) {
            identity.matrix[i][i] = 1.0f;
        }
        Vector3D<LittleEndian> no_translation = {};
        write_node_data(0, identity, no_translation, write_node_data);

        // exodux compatibility - recalibrate the bitmask using a high pass filter on the exodux compatibility bit
        bool exodux_handler = false;
        bool exodux_parser = false;

        for(auto &g : what.geometries) {
            for(auto &p : g.parts) {
                // exodux compatibility bit; AND zoner flag with the value from the tag data and XOR with the auxiliary rainbow bitmask
                std::uint32_t zoner = p.flags & ModelGeometryPartFlagsFlag::MODEL_GEOMETRY_PART_FLAGS_FLAG_ZONER;
                std::uint32_t exodux_value = (p.bullshit & zoner) ^ 0x7F7F7F7F;
                if(exodux_handler) {
                    // Since the exodux handler is active, we don't need to deobfuscate the binary rainbow table for this value due to Penn's theory
                    exodux_value ^= 0x3C170A5E;
                }
                else {
                    // Remodulate the upper 16 bits of the control magic since the exodux handler is not active
                    exodux_value <<= 16;

                    // Depending on if the parser is active, use the necessary precalculated bitmask from the binary rainbow table
                    exodux_value ^= exodux_parser ? 0x2D1E6921 : 0x291E7021;
                    exodux_parser = !exodux_parser;
                }

                // Invert the handler for the next part
                exodux_handler = !exodux_handler;

                // Do an endian swap of the exodux rainbow table checksum hash
                p.bullshit = (exodux_value & 0xFF000000) >> 24 |
                             (exodux_value & 0xFF0000) >> 8 |
                             (exodux_value & 0xFF00) << 8 |
                             (exodux_value & 0xFF) << 24;
            }
        }
    }

    void GBXModel::pre_compile(BuildWorkload &workload, std::size_t tag_index, std::size_t, std::size_t) {
        if(workload.get_build_parameters()->details.build_cache_file_engine == CacheFileEngine::CACHE_FILE_XBOX) {
            workload.report_error(BuildWorkload::ErrorType::ERROR_TYPE_FATAL_ERROR, "Gearbox model tags do not exist on the target engine", tag_index);
            throw InvalidTagDataException();
        }
        
        pre_compile_model(*this, workload, tag_index);
    }

    void Model::pre_compile(BuildWorkload &workload, std::size_t tag_index, std::size_t, std::size_t) {
        switch(workload.get_build_parameters()->details.build_cache_file_engine) {
            case CacheFileEngine::CACHE_FILE_DEMO:
            case CacheFileEngine::CACHE_FILE_RETAIL:
            case CacheFileEngine::CACHE_FILE_CUSTOM_EDITION:
            case CacheFileEngine::CACHE_FILE_MCC_CEA:
                workload.report_error(BuildWorkload::ErrorType::ERROR_TYPE_FATAL_ERROR, "Non-Gearbox model tags do not work on the target engine", tag_index);
                throw InvalidTagDataException();
            default: break;
        }
        
        pre_compile_model(*this, workload, tag_index);
    }
    
    template<class P, class PartVertex, class CacheVertex> static void pre_compile_model_geometry_part(P &what, BuildWorkload &workload, std::size_t tag_index, std::size_t struct_index, std::size_t struct_offset, const std::vector<PartVertex> &part_vertices, std::vector<CacheVertex> &workload_vertices) {
        auto uncompressed_vertices = sizeof(CacheVertex) == sizeof(ModelVertexUncompressed::C<LittleEndian>);
        
        std::vector<Index> triangle_indices;

        // Add it all
        triangle_indices.reserve(what.triangles.size() * 3);
        for(auto &t : what.triangles) {
            triangle_indices.emplace_back(t.vertex0_index);
            triangle_indices.emplace_back(t.vertex1_index);
            triangle_indices.emplace_back(t.vertex2_index);
        }

        // Remove excess NULL_INDEX values
        std::size_t triangle_indices_size;
        while(true) {
            // Make sure we have enough indices for a triangle
            triangle_indices_size = triangle_indices.size();
            if(triangle_indices_size < 3) {
                REPORT_ERROR_PRINTF(workload, ERROR_TYPE_FATAL_ERROR, tag_index, "Triangle index count is invalid (%zu < 3)", triangle_indices_size);
                throw InvalidTagDataException();
            }

            // Here we go again
            auto &index = triangle_indices[triangle_indices_size - 1];
            if(index == NULL_INDEX) {
                triangle_indices.erase(triangle_indices.begin() + (&index - triangle_indices.data()));
            }
            else {
                break;
            }
        }

        // Subtract two (since each index is technically an individual triangle, minus the last two indices since you need three indices to make a triangle)
        what.triangle_count = triangle_indices_size - 2;

        // Make sure every triangle is valid
        std::size_t part_vertices_count = part_vertices.size();
        for(auto &t : triangle_indices) {
            if(t >= part_vertices_count) {
                REPORT_ERROR_PRINTF(workload, ERROR_TYPE_FATAL_ERROR, tag_index, "Index #%zu in triangle indices is invalid (%zu >= %zu)", &t - triangle_indices.data(), static_cast<std::size_t>(t), part_vertices_count);
                throw InvalidTagDataException();
            }
        }

        // See if we can find a copy of this
        std::size_t this_indices_count = triangle_indices_size;
        std::size_t indices_count = workload.model_indices.size();
        bool found = false;

        if(indices_count >= this_indices_count) {
            auto &first = triangle_indices[0];
            auto &last = triangle_indices[triangle_indices_size - 1];
            std::size_t check_size = this_indices_count - 1;

            for(std::size_t i = 0; i <= indices_count - this_indices_count; i++) {
                auto *model_data = workload.model_indices.data() + i;

                // Check the last index, first, since it's most likely to be different
                if(model_data[triangle_indices_size - 1] != last) {
                    continue;
                }

                // If triangles match, set the triangle offset to this instead
                if(std::memcmp(&first, model_data, sizeof(workload.model_indices[0]) * check_size) == 0) {
                    found = true;
                    what.triangle_offset = i * sizeof(workload.model_indices[0]);
                    break;
                }
            }
        }

        if(!found) {
            what.triangle_offset = indices_count * sizeof(workload.model_indices[0]);
            workload.model_indices.insert(workload.model_indices.end(), triangle_indices.begin(), triangle_indices.end());
        }
        what.triangle_offset_2 = what.triangle_offset;

        // Add the vertices next
        what.vertex_count = part_vertices.size();
        std::vector<CacheVertex> vertices_of_fun;
        vertices_of_fun.reserve(what.vertex_count);
        
        for(auto &v : part_vertices) {
            auto &mv = vertices_of_fun.emplace_back();
            mv.binormal = v.binormal;
            mv.node0_index = v.node0_index;
            if(v.node1_index == NULL_INDEX) {
                mv.node1_index = 0;
            }
            else {
                mv.node1_index = v.node1_index;
            }
            mv.node0_weight = v.node0_weight;
            mv.normal = v.normal;
            mv.position = v.position;
            mv.tangent = v.tangent;
            
            if(uncompressed_vertices) {
                auto &uncompressed_cache_vertex = *reinterpret_cast<ModelVertexUncompressed::C<LittleEndian> *>(&mv);
                const auto &uncompressed_parser_vertex = *reinterpret_cast<const ModelVertexUncompressed *>(&v);
                uncompressed_cache_vertex.node1_weight = uncompressed_parser_vertex.node1_weight;
                uncompressed_cache_vertex.texture_coords = uncompressed_parser_vertex.texture_coords;
            }
            else {
                auto &compressed_cache_vertex = *reinterpret_cast<ModelVertexCompressed::C<LittleEndian> *>(&mv);
                const auto &compressed_parser_vertex = *reinterpret_cast<const ModelVertexCompressed *>(&v);
                compressed_cache_vertex.texture_coordinate_u = compressed_parser_vertex.texture_coordinate_u;
                compressed_cache_vertex.texture_coordinate_v = compressed_parser_vertex.texture_coordinate_v;
            }
        }
        
        // Part thingy
        workload.model_parts.emplace_back(BuildWorkload::BuildWorkloadModelPart { struct_index, struct_offset });

        // Let's see if we can also dedupe this
        std::size_t this_vertices_count = vertices_of_fun.size();
        std::size_t vertices_count = workload_vertices.size();
        found = false;

        if(vertices_count >= this_vertices_count) {
            for(std::size_t i = 0; i <= vertices_count - this_vertices_count; i++) {
                // If vertices match, set the vertices offset to this instead
                if(std::memcmp(workload_vertices.data() + i, vertices_of_fun.data(), sizeof(CacheVertex) * this_vertices_count) == 0) {
                    found = true;
                    what.vertex_offset = i * sizeof(workload_vertices[0]);
                    break;
                }
            }
        }

        if(!found) {
            what.vertex_offset = vertices_count * sizeof(CacheVertex);
            workload_vertices.insert(workload_vertices.end(), vertices_of_fun.begin(), vertices_of_fun.end());
        }

        // Don't forget to set these memes
        what.do_not_crash_the_game = 1;
        what.do_not_screw_up_the_model = uncompressed_vertices ? 4 : 5;
    }

    void GBXModelGeometryPart::pre_compile(BuildWorkload &workload, std::size_t tag_index, std::size_t struct_index, std::size_t offset) {
        pre_compile_model_geometry_part(*this, workload, tag_index, struct_index, offset, this->uncompressed_vertices, workload.uncompressed_model_vertices);
    }

    void ModelGeometryPart::pre_compile(BuildWorkload &workload, std::size_t tag_index, std::size_t struct_index, std::size_t offset) {
        if(workload.get_build_parameters()->details.build_cache_file_engine == CacheFileEngine::CACHE_FILE_XBOX) {
            pre_compile_model_geometry_part(*this, workload, tag_index, struct_index, offset, this->compressed_vertices, workload.compressed_model_vertices);
        }
        else {
            pre_compile_model_geometry_part(*this, workload, tag_index, struct_index, offset, this->uncompressed_vertices, workload.uncompressed_model_vertices);
        }
    }

    void ModelRegionPermutation::pre_compile(BuildWorkload &workload, std::size_t tag_index, std::size_t, std::size_t) {
        const char *last_hyphen = nullptr;
        for(const char &c : this->name.string) {
            if(c == '-') {
                last_hyphen = &c + 1;
            }
        }
        this->permutation_number = 0;
        if(last_hyphen && *last_hyphen) {
            unsigned long permutation_number = ~0;
            try {
                permutation_number = std::stoul(last_hyphen, nullptr, 10);
                if(permutation_number < static_cast<std::size_t>(NULL_INDEX)) {
                    this->permutation_number = static_cast<Index>(permutation_number);
                }
            }
            catch(std::out_of_range &) {
                permutation_number = ~0;
            }
            catch(std::exception &) {
                return;
            }
            if(permutation_number >= NULL_INDEX) {
                REPORT_ERROR_PRINTF(workload, ERROR_TYPE_WARNING, tag_index, "Permutation %s has an index that is out of range (%lu >= %zu)", this->name.string, permutation_number, static_cast<std::size_t>(NULL_INDEX));
            }
        }
    }
    
    template <typename M> static void post_cache_deformat_model(M &what) {
        what.flags &= ~ModelFlagsFlag::MODEL_FLAGS_FLAG_BLEND_SHARED_NORMALS; // prevent generational loss

        uncache_model_markers(what, true);

        float super_low = what.super_low_detail_cutoff;
        float low = what.low_detail_cutoff;
        float high = what.high_detail_cutoff;
        float super_high = what.super_high_detail_cutoff;

        what.super_low_detail_cutoff = super_high;
        what.low_detail_cutoff = high;
        what.high_detail_cutoff = low;
        what.super_high_detail_cutoff = super_low;

        regenerate_missing_model_vertices(what, true); // regenerate if needed
        
        what.postprocess_hek_data();
    }

    void GBXModel::post_cache_deformat() {
        post_cache_deformat_model(*this);
    }

    void Model::post_cache_deformat() {
        post_cache_deformat_model(*this);
    }
    
    template<typename M> static bool uncache_model_markers_model(M &model, bool fix) {
        if(model.markers.size() == 0) {
            return false;
        }
        else if(!fix) {
            return true;
        }

        auto &regions = model.regions;
        for(auto &marker : model.markers) {
            auto add_marker_instance = [&marker, &regions](std::size_t instance_index) {
                // Get it
                auto instance = marker.instances[instance_index];
                marker.instances.erase(marker.instances.begin() + instance_index);

                // Figure out the region
                std::size_t region_index = instance.region_index;
                std::size_t region_count = regions.size();
                if(region_index >= region_count) {
                    eprintf_error("invalid region index (%zu >= %zu) in marker %s #%zu", region_index, region_count, marker.name.string, instance_index);
                    throw OutOfBoundsException();
                }

                // Next, figure out the region permutation
                auto &region = regions[region_index];
                std::size_t permutation_count = region.permutations.size();
                std::size_t permutation_index = instance.permutation_index;
                if(permutation_index >= permutation_count) {
                    eprintf_error("invalid permutation index (%zu >= %zu) for region #%zu in marker %s #%zu", permutation_index, permutation_count, region_index, marker.name.string, instance_index);
                    throw OutOfBoundsException();
                }

                // Lastly, add it to the beginning
                auto &new_marker = region.permutations[permutation_index].markers.emplace_back();
                new_marker.node_index = instance.node_index;
                new_marker.rotation = instance.rotation;
                new_marker.translation = instance.translation;
                new_marker.name = marker.name;
            };

            while(marker.instances.size()) {
                // Add the last one first
                std::size_t last_instance = marker.instances.size() - 1;
                auto &last_index_ref = marker.instances[last_instance];
                std::size_t permutation_index = last_index_ref.permutation_index;
                std::size_t region_index = last_index_ref.region_index;
                add_marker_instance(last_instance);

                // Add all ones like it in order
                for(std::size_t i = 0; i < last_instance; i++) {
                    auto &this_instance_ref = marker.instances[i];
                    if(this_instance_ref.permutation_index == permutation_index && this_instance_ref.region_index == region_index) {
                        add_marker_instance(i);
                        i--;
                        last_instance--;
                    }
                }
            }
        }
        model.markers.clear();
        return true;
    }

    bool uncache_model_markers(GBXModel &model, bool fix) {
        return uncache_model_markers_model(model, fix);
    }

    bool uncache_model_markers(Model &model, bool fix) {
        return uncache_model_markers_model(model, fix);
    }
    
    template <class MFrom, class MTo> static MTo begin_converting_model(const MFrom &model) {
        MTo output = {};
        
        output.flags = model.flags;
        output.node_list_checksum = model.node_list_checksum;
        
        output.super_high_detail_cutoff = model.super_high_detail_cutoff;
        output.high_detail_cutoff = model.high_detail_cutoff;
        output.medium_detail_cutoff = model.medium_detail_cutoff;
        output.low_detail_cutoff = model.low_detail_cutoff;
        output.super_low_detail_cutoff = model.super_low_detail_cutoff;
        
        output.super_high_detail_node_count = model.super_high_detail_node_count;
        output.high_detail_node_count = model.high_detail_node_count;
        output.medium_detail_node_count = model.medium_detail_node_count;
        output.low_detail_node_count = model.low_detail_node_count;
        output.super_low_detail_node_count = model.super_low_detail_node_count;
        
        output.base_map_u_scale = model.base_map_u_scale;
        output.base_map_v_scale = model.base_map_v_scale;
        output.markers = model.markers;
        output.nodes = model.nodes;
        output.regions = model.regions;
        output.shaders = model.shaders;
        
        return output;
    }
    
    template <class PFrom, class PTo> static PTo begin_converting_part(const PFrom &part) {
        PTo output = {};
        
        output.flags = part.flags;
        output.shader_index = part.shader_index;
        output.prev_filthy_part_index = part.prev_filthy_part_index;
        output.next_filthy_part_index = part.next_filthy_part_index;
        output.centroid = part.centroid;
        output.uncompressed_vertices = part.uncompressed_vertices;
        output.compressed_vertices = part.compressed_vertices;
        output.triangles = part.triangles;
        
        return output;
    }
    
    GBXModel convert_model_to_gbxmodel(const Model &model) {
        auto output = begin_converting_model<Model, GBXModel>(model);
        
        for(auto &gi : model.geometries) {
            auto &go = output.geometries.emplace_back();
            go.flags = gi.flags;
            for(auto &pi : gi.parts) {
                go.parts.emplace_back(begin_converting_part<ModelGeometryPart, GBXModelGeometryPart>(pi));
            }
        }
        
        return output;
    }
    
    Model convert_gbxmodel_to_model(const GBXModel &model) {
        auto output = begin_converting_model<GBXModel, Model>(model);
        
        // Check if we have local nodes
        auto local_nodes = model.flags & ModelFlagsFlag::MODEL_FLAGS_FLAG_PARTS_HAVE_LOCAL_NODES;
        if(local_nodes) {
            // Local nodes don't exist on model tags
            output.flags = output.flags & ~ModelFlagsFlag::MODEL_FLAGS_FLAG_PARTS_HAVE_LOCAL_NODES;
        }
        
        // Do this
        for(auto &gi : model.geometries) {
            auto &go = output.geometries.emplace_back();
            go.flags = gi.flags;
            for(auto &pi : gi.parts) {
                auto &po = go.parts.emplace_back(begin_converting_part<GBXModelGeometryPart, ModelGeometryPart>(pi));
                
                auto node_count = sizeof(pi.local_node_indices) / sizeof(*pi.local_node_indices);
                
                if(local_nodes) {
                    for(auto &v : po.uncompressed_vertices) {
                        if(v.node0_index != NULL_INDEX) {
                            v.node0_index = pi.local_node_indices[v.node0_index % node_count];
                        }
                        if(v.node1_index != NULL_INDEX) {
                            v.node1_index = pi.local_node_indices[v.node1_index % node_count];
                        }
                    }
                }
            }
        }
        
        return output;
    }
    
    ModelVertexCompressed::C<NativeEndian> compress_model_vertex(const ModelVertexUncompressed::C<NativeEndian> &vertex) noexcept {
        ModelVertexCompressed::C<NativeEndian> r;
        r.position = vertex.position;
        r.node0_index = vertex.node0_index > Parser::MaxCompressedModelNodeIndex::MAX_COMPRESSED_MODEL_NODE_INDEX ? -3 : vertex.node0_index * 3;
        r.node0_weight = static_cast<std::int16_t>(compress_float<16>(vertex.node0_weight));
        r.node1_index = vertex.node1_index > Parser::MaxCompressedModelNodeIndex::MAX_COMPRESSED_MODEL_NODE_INDEX ? -3 : vertex.node1_index * 3;
        r.texture_coordinate_u = static_cast<std::int16_t>(compress_float<16>(vertex.texture_coords.x));
        r.texture_coordinate_v = static_cast<std::int16_t>(compress_float<16>(vertex.texture_coords.y));
        r.normal = compress_vector(vertex.normal.i, vertex.normal.j, vertex.normal.k);
        r.binormal = compress_vector(vertex.binormal.i, vertex.binormal.j, vertex.binormal.k);
        r.tangent = compress_vector(vertex.tangent.i, vertex.tangent.j, vertex.tangent.k);
        return r;
    }

    ModelVertexUncompressed::C<NativeEndian> decompress_model_vertex(const ModelVertexCompressed::C<NativeEndian> &vertex) noexcept {
        ModelVertexUncompressed::C<NativeEndian> r;
        r.position = vertex.position;
        r.node0_index = vertex.node0_index < 0 ? 65535 : vertex.node0_index / 3;
        r.node0_weight = decompress_float<16>(vertex.node0_weight);
        r.node1_index = vertex.node1_index < 0 ? 65535 : vertex.node1_index / 3;
        r.node1_weight = 1.0F - r.node0_weight; // this is just derived from node0_weight
        r.texture_coords.x = decompress_float<16>(vertex.texture_coordinate_u);
        r.texture_coords.y = decompress_float<16>(vertex.texture_coordinate_v);

        float normal_i, normal_j, normal_k;

        decompress_vector(vertex.normal, normal_i, normal_j, normal_k);
        r.normal.i = normal_i;
        r.normal.j = normal_j;
        r.normal.k = normal_k;

        decompress_vector(vertex.binormal, normal_i, normal_j, normal_k);
        r.binormal.i = normal_i;
        r.binormal.j = normal_j;
        r.binormal.k = normal_k;

        decompress_vector(vertex.tangent, normal_i, normal_j, normal_k);
        r.tangent.i = normal_i;
        r.tangent.j = normal_j;
        r.tangent.k = normal_k;
        return r;
    }
}
