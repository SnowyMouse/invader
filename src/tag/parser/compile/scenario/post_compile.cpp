// SPDX-License-Identifier: GPL-3.0-only

#include <invader/tag/parser/parser.hpp>
#include <invader/build/build_workload.hpp>
#include <invader/file/file.hpp>
#include <invader/tag/hek/class/model_collision_geometry.hpp>
#include <invader/script/compiler.hpp>
#include <invader/tag/parser/compile/scenario.hpp>

namespace Invader::Parser {
    using BSPData = HEK::BSPData;
    
    // Functions for finding stuff
    static std::vector<BSPData> get_bsp_data(const Scenario &scenario, BuildWorkload &workload);
    static void find_encounters(Scenario &scenario, BuildWorkload &workload, std::size_t tag_index, const std::vector<BSPData> &bsp_data, BuildWorkload::BuildWorkloadStruct &scenario_struct, const Scenario::struct_little &scenario_data, std::size_t &bsp_find_warnings);
    static void find_command_lists(Scenario &scenario, BuildWorkload &workload, std::size_t tag_index, const std::vector<BSPData> &bsp_data, BuildWorkload::BuildWorkloadStruct &scenario_struct, const Scenario::struct_little &scenario_data, std::size_t &bsp_find_warnings);
    static void find_decals(Scenario &scenario, BuildWorkload &workload, const std::vector<BSPData> &bsp_data);
    static void find_conversations(Scenario &scenario, BuildWorkload &workload, std::size_t tag_index, BuildWorkload::BuildWorkloadStruct &scenario_struct, const Scenario::struct_little &scenario_data);

    void Scenario::post_compile(BuildWorkload &workload, std::size_t tag_index, std::size_t struct_index, std::size_t struct_offset) {
        if(workload.disable_recursion) {
            return; // if recursion is disabled, doing any of this will be a meme
        }

        // Get the struct information here
        auto &scenario_struct = workload.structs[struct_index];
        const auto &scenario_data = *reinterpret_cast<const struct_little *>(scenario_struct.data.data() + struct_offset);

        // Get the bsp data; saves us from having to get it again
        auto bsp_data = get_bsp_data(*this, workload);
        auto bsp_count = bsp_data.size();

        // Determine which BSP light fixtures and scenery are in
        #define FIND_BSP_INDICES_FOR_OBJECT_ARRAY(array_type, objects, palette_type, palette, type_name) { \
            std::size_t object_count = this->objects.size(); \
            if(object_count) { \
                auto &object_struct = workload.structs[*scenario_struct.resolve_pointer(&scenario_data.objects.pointer)]; \
                auto *object_array = reinterpret_cast<array_type::struct_little *>(object_struct.data.data()); \
                for(std::size_t o = 0; o < object_count; o++) { \
                    std::uint32_t bsp_indices = 0; \
                    auto &object = object_array[o]; \
                    if(object.type != NULL_INDEX) { \
                        auto &type = this->palette[object.type].name; \
                        auto &object_bounding_offset = reinterpret_cast<Object::struct_little *>(workload.structs[*workload.tags[type.tag_id.index].base_struct].data.data())->bounding_offset; \
                        auto position_to_check = object.position + object_bounding_offset; \
                        for(std::size_t b = 0; b < bsp_count; b++) { \
                            /* Check if we're inside this BSP */ \
                            auto &bsp = bsp_data[b]; \
                            if(bsp.check_if_point_inside_bsp(position_to_check)) { \
                                bsp_indices |= 1 << b; \
                            } \
                        } \
                        if(bsp_indices == 0) { \
                            REPORT_ERROR_PRINTF(workload, ERROR_TYPE_WARNING, tag_index, type_name " spawn #%zu was found in 0 BSPs, so it will not spawn", o); \
                        } \
                    } \
                    /* Set to the result */ \
                    object.bsp_indices = bsp_indices; \
                } \
            } \
        }

        FIND_BSP_INDICES_FOR_OBJECT_ARRAY(ScenarioScenery, scenery, ScenarioSceneryPalette, scenery_palette, "Scenery");
        FIND_BSP_INDICES_FOR_OBJECT_ARRAY(ScenarioLightFixture, light_fixtures, ScenarioLightFixturePalette, light_fixture_palette, "Light fixture");
        
        #undef FIND_BSP_INDICES_FOR_OBJECT_ARRAY

        // Find what we need
        std::size_t bsp_find_warnings = 0;
        find_encounters(*this, workload, tag_index, bsp_data, scenario_struct, scenario_data, bsp_find_warnings);
        find_command_lists(*this, workload, tag_index, bsp_data, scenario_struct, scenario_data, bsp_find_warnings);
        find_decals(*this, workload, bsp_data);
        find_conversations(*this, workload, tag_index, scenario_struct, scenario_data);
        
        if(bsp_find_warnings) {
            eprintf_warn_lesser("Use manual BSP indices to silence %s.", bsp_find_warnings == 1 ? "this warning" : "these warnings");
        }
    }
    
    static std::vector<BSPData> get_bsp_data(const Scenario &scenario, BuildWorkload &workload) {
        std::vector<BSPData> bsp_data;
        std::size_t bsp_count = scenario.structure_bsps.size();
        bsp_data.reserve(bsp_count);
        for(auto &b : scenario.structure_bsps) {
            auto &bsp_data_s = bsp_data.emplace_back();
            if(b.structure_bsp.tag_id.is_null()) {
                continue;
            }
            
            // Figure out the base tag struct thing
            auto *bsp_tag_struct = &workload.structs[workload.tags[b.structure_bsp.tag_id.index].base_struct.value()];
            
            // If we're not on dark circlet, we need to read the pointer at the beginning of the struct
            if(workload.engine_target != HEK::CacheFileEngine::CACHE_FILE_DARK_CIRCLET) {
                bsp_tag_struct = &workload.structs[bsp_tag_struct->resolve_pointer(static_cast<std::size_t>(0)).value()];
            }

            auto &bsp_tag_data = *reinterpret_cast<const ScenarioStructureBSP::struct_little *>(bsp_tag_struct->data.data());
            if(bsp_tag_data.collision_bsp.count == 0) {
                continue;
            }

            auto &collision_bsp_struct = workload.structs[bsp_tag_struct->resolve_pointer(&bsp_tag_data.collision_bsp.pointer).value()];
            auto &collision_bsp_data = *reinterpret_cast<const ModelCollisionGeometryBSP::struct_little *>(collision_bsp_struct.data.data());

            bsp_data_s.bsp3d_node_count = collision_bsp_data.bsp3d_nodes.count;
            bsp_data_s.plane_count = collision_bsp_data.planes.count;
            bsp_data_s.leaf_count = collision_bsp_data.leaves.count;
            bsp_data_s.bsp2d_node_count = collision_bsp_data.bsp2d_nodes.count;
            bsp_data_s.bsp2d_reference_count = collision_bsp_data.bsp2d_references.count;
            bsp_data_s.surface_count = collision_bsp_data.surfaces.count;
            bsp_data_s.edge_count = collision_bsp_data.edges.count;
            bsp_data_s.vertex_count = collision_bsp_data.vertices.count;
            bsp_data_s.render_leaf_count = bsp_tag_data.leaves.count;

            if(bsp_data_s.bsp3d_node_count) {
                bsp_data_s.bsp3d_nodes = reinterpret_cast<const ModelCollisionGeometryBSP3DNode::struct_little *>(workload.structs[*collision_bsp_struct.resolve_pointer(&collision_bsp_data.bsp3d_nodes.pointer)].data.data());
            }
            if(bsp_data_s.plane_count) {
                bsp_data_s.planes = reinterpret_cast<const ModelCollisionGeometryBSPPlane::struct_little *>(workload.structs[*collision_bsp_struct.resolve_pointer(&collision_bsp_data.planes.pointer)].data.data());
            }
            if(bsp_data_s.leaf_count) {
                bsp_data_s.leaves = reinterpret_cast<const ModelCollisionGeometryBSPLeaf::struct_little *>(workload.structs[*collision_bsp_struct.resolve_pointer(&collision_bsp_data.leaves.pointer)].data.data());
            }
            if(bsp_data_s.bsp2d_node_count) {
                bsp_data_s.bsp2d_nodes = reinterpret_cast<const ModelCollisionGeometryBSP2DNode::struct_little *>(workload.structs[*collision_bsp_struct.resolve_pointer(&collision_bsp_data.bsp2d_nodes.pointer)].data.data());
            }
            if(bsp_data_s.bsp2d_reference_count) {
                bsp_data_s.bsp2d_references = reinterpret_cast<const ModelCollisionGeometryBSP2DReference::struct_little *>(workload.structs[*collision_bsp_struct.resolve_pointer(&collision_bsp_data.bsp2d_references.pointer)].data.data());
            }
            if(bsp_data_s.surface_count) {
                bsp_data_s.surfaces = reinterpret_cast<const ModelCollisionGeometryBSPSurface::struct_little *>(workload.structs[*collision_bsp_struct.resolve_pointer(&collision_bsp_data.surfaces.pointer)].data.data());
            }
            if(bsp_data_s.edge_count) {
                bsp_data_s.edges = reinterpret_cast<const ModelCollisionGeometryBSPEdge::struct_little *>(workload.structs[*collision_bsp_struct.resolve_pointer(&collision_bsp_data.edges.pointer)].data.data());
            }
            if(bsp_data_s.vertex_count) {
                bsp_data_s.vertices = reinterpret_cast<const ModelCollisionGeometryBSPVertex::struct_little *>(workload.structs[*collision_bsp_struct.resolve_pointer(&collision_bsp_data.vertices.pointer)].data.data());
            }
            if(bsp_data_s.render_leaf_count) {
                bsp_data_s.render_leaves = reinterpret_cast<const ScenarioStructureBSPLeaf::struct_little *>(workload.structs[*bsp_tag_struct->resolve_pointer(&bsp_tag_data.leaves.pointer)].data.data());
            }
        }
        
        return bsp_data;
    }
    
    
    
    static void find_encounters(Scenario &scenario, BuildWorkload &workload, std::size_t tag_index, const std::vector<BSPData> &bsp_data, BuildWorkload::BuildWorkloadStruct &scenario_struct, const Scenario::struct_little &scenario_data, std::size_t &bsp_find_warnings) {
        // Determine which BSP the encounters fall in
        std::size_t encounter_list_count = scenario.encounters.size();
        if(encounter_list_count != 0) {
            auto &encounter_struct = workload.structs[*scenario_struct.resolve_pointer(&scenario_data.encounters.pointer)];
            auto *encounter_array = reinterpret_cast<ScenarioEncounter::struct_little *>(encounter_struct.data.data());
            auto bsp_count = bsp_data.size();
            
            for(std::size_t i = 0; i < encounter_list_count; i++) {
                auto &encounter = scenario.encounters[i];
                auto &encounter_data = encounter_array[i];

                // Set this to 1 because memes
                encounter_data.one = 1;

                // If we have a manual BSP index, set this stuff here
                std::size_t start_bsp = 0;
                bool manual_bsp_index_specified = encounter.flags & HEK::ScenarioEncounterFlagsFlag::SCENARIO_ENCOUNTER_FLAGS_FLAG_MANUAL_BSP_INDEX_SPECIFIED;
                if(manual_bsp_index_specified) {
                    encounter_data.precomputed_bsp_index = encounter_data.manual_bsp_index;
                    start_bsp = encounter_data.manual_bsp_index;
                }

                // Otherwise, we need to look for the best BSP
                std::size_t best_bsp = NULL_INDEX;
                std::size_t best_bsp_firing_position_hits = 0;
                std::size_t best_bsp_squad_hits = 0;
                std::size_t best_bsp_total_hits = 0;
                std::size_t total_best_bsps = 0;

                // We also need to find firing position indices
                struct FiringPositionIndex {
                    HEK::Index cluster_index = NULL_INDEX;
                    std::uint32_t surface_index = 0;
                    bool found = false;
                };
                std::size_t firing_position_count = encounter.firing_positions.size();
                std::vector<FiringPositionIndex> best_firing_positions_indices(firing_position_count);
                
                // And we'll hold onto this, too
                struct SquadPositionFound {
                    std::size_t squad = ~0;
                    std::size_t starting_position = ~0;
                    HEK::Index cluster_index = NULL_INDEX;
                    bool found = false;
                };
                
                // Get some default data
                std::size_t squad_count = encounter.squads.size();
                std::vector<SquadPositionFound> best_squad_positions_found;
                for(std::size_t s = 0; s < squad_count; s++) {
                    auto position_count = encounter.squads[s].starting_locations.size();
                    for(std::size_t p = 0; p < position_count; p++) {
                        best_squad_positions_found.emplace_back(SquadPositionFound { s, p });
                    }
                }
                std::size_t squad_position_count = best_squad_positions_found.size();

                // Also, are we raycasting?
                bool raycast = !(encounter.flags & HEK::ScenarioEncounterFlagsFlag::SCENARIO_ENCOUNTER_FLAGS_FLAG__3D_FIRING_POSITIONS);

                // Go through each BSP
                std::vector<FiringPositionIndex> firing_positions_indices = best_firing_positions_indices;
                std::vector<SquadPositionFound> squad_positions_found = best_squad_positions_found;
                for(std::size_t b = start_bsp; b < bsp_count; b++) {
                    firing_positions_indices.clear();
                    squad_positions_found.clear();
                    auto &bsp = bsp_data[b];

                    // Go through each squad; add 1 to hits for every squad we find in the BSP
                    std::size_t squad_hits = 0;
                    for(std::size_t s = 0; s < squad_count; s++) {
                        auto &squad = encounter.squads[s];
                        std::size_t location_count = squad.starting_locations.size();
                        
                        for(std::size_t l = 0; l < location_count; l++) {
                            auto &location = squad.starting_locations[l];
                            
                            // If raycasting check for a surface that is 0.5 world units above/below it
                            bool found;
                            std::optional<std::uint32_t> leaf_index;
                            
                            if(raycast) {
                                std::uint32_t leaf;
                                if((found = bsp.check_for_intersection(location.position, 0.5F, nullptr, nullptr, &leaf))) {
                                    leaf_index = leaf;
                                }
                            }
                            else {
                                std::uint32_t leaf;
                                if((found = bsp.check_if_point_inside_bsp(location.position, &leaf))) {
                                    leaf_index = leaf; 
                                }
                            }
                            
                            // Set the cluster index
                            HEK::Index cluster_index;
                            if(leaf_index.has_value()) {
                                cluster_index = bsp.render_leaves[*leaf_index].cluster;
                            }
                            else {
                                cluster_index = NULL_INDEX;
                            }
                            
                            squad_hits += found;
                            squad_positions_found.emplace_back(SquadPositionFound { s, l, cluster_index, found });
                        }
                    }
                    
                    // Go through each firing position
                    std::size_t firing_position_hits = 0;
                    for(auto &f : encounter.firing_positions) {
                        // Here are the indices we'll set
                        bool in_bsp;
                        std::uint32_t surface_index = 0;
                        std::uint32_t leaf_index = 0;
                        
                        // Raycast stuff
                        if(raycast) {
                            in_bsp = bsp.check_for_intersection(f.position, 0.5F, nullptr, &surface_index, &leaf_index);
                        }
                        else {
                            std::uint32_t leaf;
                            if((in_bsp = bsp.check_if_point_inside_bsp(f.position, &leaf))) {
                                leaf_index = leaf;
                            }
                        }
                        
                        // If we're in the BSP, add it
                        if(in_bsp) {
                            firing_positions_indices.emplace_back(FiringPositionIndex {bsp.render_leaves[leaf_index].cluster, surface_index, true});
                            firing_position_hits++;
                        }
                        else {
                            firing_positions_indices.emplace_back();
                        }
                    }
                    
                    // Add it up
                    auto total_hits = squad_hits + firing_position_hits;

                    // If this is the next best BSP, write data
                    if(total_hits > best_bsp_total_hits) {
                        best_bsp_total_hits = total_hits;
                        best_bsp_firing_position_hits = firing_position_hits;
                        best_bsp_squad_hits = squad_hits;
                        best_firing_positions_indices = firing_positions_indices;
                        best_squad_positions_found = squad_positions_found;
                        best_bsp = b;
                        total_best_bsps = 1;
                    }
                    else if(total_hits && total_hits == best_bsp_total_hits) {
                        total_best_bsps++;
                    }
                    
                    // If we have a manual index set, break early
                    if(manual_bsp_index_specified) {
                        total_best_bsps = 1;
                        break;
                    }
                }

                // Are we doing the thing?
                if(!manual_bsp_index_specified) {
                    encounter_data.precomputed_bsp_index = static_cast<HEK::Index>(best_bsp);
                }
                
                auto best_possible_hits = squad_position_count + firing_position_count;
                
                // Ambiguous?
                if(total_best_bsps > 1) {
                    REPORT_ERROR_PRINTF(workload, ERROR_TYPE_WARNING, tag_index, "Encounter #%zu (%s) was found in %zu BSPs (will place in BSP #%zu)", i, encounter.name.string, total_best_bsps, best_bsp);
                    bsp_find_warnings++;
                }
                
                // Are we missing stuff?
                if(total_best_bsps == 0) {
                    REPORT_ERROR_PRINTF(workload, ERROR_TYPE_WARNING, tag_index, "Encounter #%zu (%s) was found in 0 BSPs", i, encounter.name.string);
                }
                else if(best_bsp_total_hits != best_possible_hits) {
                    REPORT_ERROR_PRINTF(workload, ERROR_TYPE_WARNING, tag_index, "Encounter #%zu (%s) is partially outside of BSP #%zu", i, encounter.name.string, best_bsp);
                    
                    // Show the firing positions and squad positions that are missing
                    auto missing_firing_positions = firing_position_count - best_bsp_firing_position_hits;
                    if(missing_firing_positions) {
                        int offset = 0;
                        char missing_firing_positions_list[256] = {};
                        unsigned int listed = 0;
                        for(std::size_t fp = 0; fp < firing_position_count; fp++) {
                            // Look for anything we didn't find
                            if(!best_firing_positions_indices[fp].found) {
                                offset += std::snprintf(missing_firing_positions_list + offset, sizeof(missing_firing_positions_list) - offset, "%s%zu", listed == 0 ? "" : " ", fp);
                                
                                // If we're going past 7, we shouldn't list anymore as it's a bit spammy
                                if(++listed == 7) {
                                    if(missing_firing_positions > listed) {
                                        std::snprintf(missing_firing_positions_list + offset, sizeof(missing_firing_positions_list) - offset, " ...");
                                    }
                                    break;
                                }
                            }
                        }
                        
                        eprintf_warn_lesser("    - %zu firing position%s fell out: [%s]", missing_firing_positions, missing_firing_positions == 1 ? "" : "s", missing_firing_positions_list);
                    }
                    
                    auto missing_squad_positions = squad_position_count - best_bsp_squad_hits;
                    if(missing_squad_positions) {
                        int offset = 0;
                        char missing_squad_positions_list[256] = {};
                        unsigned int listed = 0;
                        for(auto &sp : best_squad_positions_found) {
                            // Look for anything we didn't find
                            if(!sp.found) {
                                offset += std::snprintf(missing_squad_positions_list + offset, sizeof(missing_squad_positions_list) - offset, "%s%zu-%zu", listed == 0 ? "" : " ", sp.squad, sp.starting_position);
                                
                                // If we're going past 5, we shouldn't list anymore as it's a bit spammy
                                if(++listed == 5) {
                                    if(missing_firing_positions > listed) {
                                        std::snprintf(missing_squad_positions_list + offset, sizeof(missing_squad_positions_list) - offset, " ...");
                                    }
                                    break;
                                }
                            }
                        }
                        
                        eprintf_warn_lesser("    - %zu squad position%s fell out: [%s]", missing_squad_positions, missing_squad_positions == 1 ? "" : "s", missing_squad_positions_list);
                    }
                }

                // Set all the cluster and surface indices
                if(firing_position_count > 0) {
                    auto *firing_positions_data = reinterpret_cast<HEK::ScenarioFiringPosition<HEK::LittleEndian> *>(workload.structs[*encounter_struct.resolve_pointer(&encounter_data.firing_positions.pointer)].data.data());
                    for(std::size_t fp = 0; fp < firing_position_count; fp++) {
                        auto &f = firing_positions_data[fp];
                        auto &indices = best_firing_positions_indices[fp];
                        
                        f.surface_index = indices.surface_index;
                        f.cluster_index = indices.cluster_index;
                    }
                }
                
                // Same with squads
                if(squad_position_count > 0) {
                    std::size_t position_index_found = 0;
                    auto &squad_struct = workload.structs[*encounter_struct.resolve_pointer(&encounter_data.squads.pointer)];
                    auto *squad_data = reinterpret_cast<Parser::ScenarioSquad::struct_little *>(squad_struct.data.data());
                    for(std::size_t s = 0; s < squad_count; s++) {
                        auto &squad = squad_data[s];
                        std::size_t position_count = squad.starting_locations.count.read();
                        if(position_count) {
                            auto *position_data = reinterpret_cast<Parser::ScenarioActorStartingLocation::struct_little *>(workload.structs[*squad_struct.resolve_pointer(&squad.starting_locations.pointer)].data.data());
                            for(std::size_t p = 0; p < position_count; p++) {
                                auto &found_index = best_squad_positions_found[position_index_found++];
                                position_data[p].cluster_index = found_index.cluster_index;
                            }
                        }
                    }
                }
            }
        }
    }
    
    static void find_command_lists(Scenario &scenario, BuildWorkload &workload, std::size_t tag_index, const std::vector<BSPData> &bsp_data, BuildWorkload::BuildWorkloadStruct &scenario_struct, const Scenario::struct_little &scenario_data, std::size_t &bsp_find_warnings) {
        std::size_t command_list_count = scenario.command_lists.size();
        if(command_list_count != 0) {
            auto &command_list_struct = workload.structs[*scenario_struct.resolve_pointer(&scenario_data.command_lists.pointer)];
            auto *command_list_array = reinterpret_cast<ScenarioCommandList::struct_little *>(command_list_struct.data.data());
            auto bsp_count = bsp_data.size();
            for(std::size_t i = 0; i < command_list_count; i++) {
                auto &command_list = scenario.command_lists[i];
                auto &command_list_data = command_list_array[i];

                // If there are no points, set to a null BSP
                if(command_list.points.size() == 0) {
                    command_list_data.precomputed_bsp_index = NULL_INDEX;
                    continue;
                }

                // Go through each BSP
                std::size_t best_bsp = NULL_INDEX;
                std::size_t best_bsp_hits = 0;
                std::size_t total_best_bsps = 0;
                std::size_t max_hits = 0;
                std::size_t start = 0;
                bool manual_bsp_index_specified = command_list.flags & HEK::ScenarioCommandListFlagsFlag::SCENARIO_COMMAND_LIST_FLAGS_FLAG_MANUAL_BSP_INDEX;

                // If manually specifying a BSP, don't bother checking every BSP
                if(manual_bsp_index_specified) {
                    start = command_list.manual_bsp_index;
                }
                
                auto point_count = command_list.points.size();
                std::vector<std::optional<std::uint32_t>> best_surface_indices(point_count);
                
                for(std::size_t b = start; b < bsp_count; b++) {
                    auto &bsp = bsp_data[b];
                    std::size_t hits = 0;
                    std::size_t total_hits = 0;
                    std::vector<std::optional<std::uint32_t>> surface_indices;
                    surface_indices.reserve(point_count);

                    // Basically, add 1 for every time we find it in here
                    for(auto &p : command_list.points) {
                        total_hits++;
                        
                        std::uint32_t surface_index = 0;
                        if(bsp.check_for_intersection(p.position, 0.5F, nullptr, &surface_index, nullptr)) {
                            hits++;
                            surface_indices.emplace_back(surface_index);
                        }
                        else {
                            surface_indices.emplace_back();
                        }
                    }

                    if(hits > best_bsp_hits) {
                        best_bsp_hits = hits;
                        best_bsp = b;
                        total_best_bsps = 1;
                        max_hits = total_hits;
                        best_surface_indices = surface_indices;
                    }
                    else if(hits && hits == best_bsp_hits) {
                        total_best_bsps++;
                    }
                    
                    // Since we're just checking one BSP's things, break
                    if(manual_bsp_index_specified) {
                        total_best_bsps = 1;
                        break;
                    }
                }
                
                // Write the surface indices we found
                if(point_count > 0) {
                    auto &points_struct = workload.structs[*command_list_struct.resolve_pointer(&command_list_data.points.pointer)];
                    auto *points_array = reinterpret_cast<ScenarioCommandPoint::struct_little *>(points_struct.data.data());
                    for(std::size_t p = 0; p < point_count; p++) {
                        points_array[p].surface_index = best_surface_indices[p].value_or(0);
                    }
                }
                
                // Command lists are all-or-nothing here
                if(manual_bsp_index_specified || max_hits == best_bsp_hits) {
                    command_list_data.precomputed_bsp_index = static_cast<HEK::Index>(best_bsp);
                }
                else {
                    command_list_data.precomputed_bsp_index = NULL_INDEX;
                }
                
                // Show warnings if needed
                if(total_best_bsps == 0) {
                    REPORT_ERROR_PRINTF(workload, ERROR_TYPE_WARNING, tag_index, "Command list #%zu (%s) was found in 0 BSPs", i, command_list.name.string);
                }
                else if(best_bsp_hits != max_hits) {
                    REPORT_ERROR_PRINTF(workload, ERROR_TYPE_WARNING, tag_index, "Command list #%zu (%s) is partially outside of BSP #%zu (%zu / %zu hit%s)", i, command_list.name.string, best_bsp, best_bsp_hits, max_hits, max_hits == 1 ? "" : "s");
                    
                    auto missing_points = max_hits - best_bsp_hits;
                    int offset = 0;
                    char missing_points_list[256] = {};
                    unsigned int listed = 0;
                    for(std::size_t p = 0; p < point_count; p++) {
                        // Look for anything we didn't find
                        if(!best_surface_indices[p].has_value()) {
                            offset += std::snprintf(missing_points_list + offset, sizeof(missing_points_list) - offset, "%s%zu", listed == 0 ? "" : " ", p);
                            
                            // If we're going past 7, we shouldn't list anymore as it's a bit spammy
                            if(++listed == 7) {
                                if(missing_points > listed) {
                                    std::snprintf(missing_points_list + offset, sizeof(missing_points_list) - offset, " ...");
                                }
                                break;
                            }
                        }
                    }
                    
                    eprintf_warn_lesser("    - %zu point%s fell out: [%s]", missing_points, missing_points == 1 ? "" : "s", missing_points_list);
                    
                }
                else if(total_best_bsps > 1) {
                    REPORT_ERROR_PRINTF(workload, ERROR_TYPE_WARNING, tag_index, "Command list #%zu (%s) was found in %zu BSP%s (will place in BSP #%zu)", i, command_list.name.string, total_best_bsps, total_best_bsps == 1 ? "" : "s", best_bsp);
                    bsp_find_warnings++;
                }
            }
        }
    }
    
    static void find_decals(Scenario &scenario, BuildWorkload &workload, const std::vector<BSPData> &bsp_data) {
        std::size_t decal_count = scenario.decals.size();
        if(decal_count > 0) {
            for(std::size_t bsp = 0; bsp < scenario.structure_bsps.size(); bsp++) {
                auto &b = scenario.structure_bsps[bsp];
                auto &bsp_id = b.structure_bsp.tag_id;
                if(!bsp_id.is_null()) {
                    // Figure out the base tag struct thing
                    auto *bsp_tag_struct = &workload.structs[workload.tags[bsp_id.index].base_struct.value()];
                    
                    // If we're not on dark circlet, we need to read the pointer at the beginning of the struct
                    if(workload.engine_target != HEK::CacheFileEngine::CACHE_FILE_DARK_CIRCLET) {
                        bsp_tag_struct = &workload.structs[bsp_tag_struct->resolve_pointer(static_cast<std::size_t>(0)).value()];
                    }
                    
                    auto &bsp_tag_data = *reinterpret_cast<ScenarioStructureBSP::struct_little *>(bsp_tag_struct->data.data());
                    std::size_t bsp_cluster_count = bsp_tag_data.clusters.count.read();

                    if(bsp_cluster_count == 0) {
                        continue;
                    }

                    // Make sure we know which decals we used for this BSP so we don't reuse them
                    std::vector<bool> used(decal_count, false);

                    // Now let's go do stuff
                    auto &bsp_cluster_struct = workload.structs[*bsp_tag_struct->resolve_pointer(&bsp_tag_data.clusters.pointer)];
                    auto *clusters = reinterpret_cast<ScenarioStructureBSPCluster::struct_little *>(bsp_cluster_struct.data.data());

                    // Go through each decal; see what we can come up with
                    std::vector<std::pair<std::size_t, std::size_t>> cluster_decals;
                    for(std::size_t d = 0; d < decal_count; d++) {
                        auto &decal = scenario.decals[d];
                        auto &bd = bsp_data[bsp];
                        std::uint32_t leaf;
                        if(!bd.check_if_point_inside_bsp(decal.position, &leaf)) {
                            continue;
                        }
                        cluster_decals.emplace_back(bd.render_leaves[leaf].cluster, d);
                    }

                    // Get clusters
                    std::vector<ScenarioStructureBSPRuntimeDecal::struct_little> runtime_decals;

                    for(std::size_t c = 0; c < bsp_cluster_count; c++) {
                        auto &cluster = clusters[c];

                        // Put stuff together
                        std::size_t first_decal = runtime_decals.size();
                        for(auto &cd : cluster_decals) {
                            if(cd.first == c) {
                                auto &decal = scenario.decals[cd.second];
                                auto &d = runtime_decals.emplace_back();
                                d.decal_type = decal.decal_type;
                                d.pitch = decal.pitch;
                                d.yaw = decal.yaw;
                                d.position = decal.position;
                            }
                        }
                        std::size_t decal_end = runtime_decals.size();

                        // Set the decal count
                        if(first_decal != decal_end) {
                            cluster.first_decal_index = static_cast<std::int16_t>(first_decal);
                            cluster.decal_count = static_cast<std::int16_t>(decal_end - first_decal);
                        }
                        else {
                            cluster.first_decal_index = NULL_INDEX;
                            cluster.decal_count = 0;
                        }
                    }

                    // Set the decal count to the number of decals we used and add it to the end of the BSP
                    bsp_tag_data.runtime_decals.count = static_cast<std::uint32_t>(runtime_decals.size());

                    if(runtime_decals.size() != 0) {
                        auto &new_struct_ptr = bsp_tag_struct->pointers.emplace_back();
                        new_struct_ptr.offset = reinterpret_cast<const std::byte *>(&bsp_tag_data.runtime_decals.pointer) - reinterpret_cast<const std::byte *>(&bsp_tag_data);
                        new_struct_ptr.struct_index = workload.structs.size();
                        auto &new_struct = workload.structs.emplace_back();
                        new_struct.bsp = workload.structs[*workload.tags[bsp_id.index].base_struct].bsp;
                        new_struct.data = std::vector<std::byte>(reinterpret_cast<std::byte *>(runtime_decals.data()), reinterpret_cast<std::byte *>(runtime_decals.data() + runtime_decals.size()));
                    }
                }
            }
        }
    }
    
    static void find_conversations(Scenario &scenario, BuildWorkload &workload, std::size_t tag_index, BuildWorkload::BuildWorkloadStruct &scenario_struct, const Scenario::struct_little &scenario_data) {
        std::size_t ai_conversation_count = scenario.ai_conversations.size();
        if(ai_conversation_count) {
            auto &ai_conversation_struct = workload.structs[*scenario_struct.resolve_pointer(&scenario_data.ai_conversations.pointer)];
            auto *ai_conversation_data = reinterpret_cast<Parser::ScenarioAIConversation::struct_little *>(ai_conversation_struct.data.data());
            auto encounter_list_count = scenario.encounters.size();
            for(std::size_t aic = 0; aic < ai_conversation_count; aic++) {
                auto &convo = ai_conversation_data[aic];
                std::size_t participation_count = convo.participants.count.read();
                if(participation_count) {
                    auto &participation_struct = workload.structs[*ai_conversation_struct.resolve_pointer(&convo.participants.pointer)];
                    auto *participation_data = reinterpret_cast<Parser::ScenarioAIConversationParticipant::struct_little *>(participation_struct.data.data());
                    for(std::size_t p = 0; p < participation_count; p++) {
                        auto &participant = participation_data[p];
                        std::optional<std::uint32_t> encounter_index;
                        
                        // Do we have an encounter to look for?
                        if(participant.encounter_name.string[0]) {
                            for(std::size_t e = 0; e < encounter_list_count; e++) {
                                if(scenario.encounters[e].name == participant.encounter_name) {
                                    encounter_index = e;
                                    break;
                                }
                            }
                            if(!encounter_index.has_value()) {
                                REPORT_ERROR_PRINTF(workload, ERROR_TYPE_ERROR, tag_index, "Participant #%zu of conversation #%zu (%s) references a non-existant encounter (%s)", p, aic, convo.name.string, participant.encounter_name.string);
                            }
                        }
                        
                        participant.encounter_index = encounter_index.value_or(0xFFFFFFFF);
                        participant.unknown1 = 0;
                        participant.unknown2 = 0xFFFF;
                        participant.unknown3 = 0xFFFFFFFF;
                        participant.unknown4 = 0xFFFFFFFF;
                    }
                }
            }
        }
    }
}
