// SPDX-License-Identifier: GPL-3.0-only

#include <invader/tag/parser/parser.hpp>
#include <invader/build/build_workload.hpp>
#include <invader/file/file.hpp>

namespace Invader::Parser {
    void Scenario::pre_compile(BuildWorkload &workload, std::size_t tag_index, std::size_t struct_index, std::size_t) {
        if(!workload.cache_file_type.has_value()) {
            workload.cache_file_type = this->type;
        }
        else {
            workload.report_error(BuildWorkload::ErrorType::ERROR_TYPE_FATAL_ERROR, "Multiple scenario tags are used", tag_index);
            throw InvalidTagDataException();
        }

        // Check for unused stuff
        std::size_t name_count = this->object_names.size();
        std::vector<std::uint32_t> name_used(name_count);

        #define CHECK_PALETTE_AND_SPAWNS(object_type_str, scenario_object_type, scenario_palette_type, object_type_int) { \
            std::size_t type_count = this->scenario_palette_type.size(); \
            std::size_t count = this->scenario_object_type.size(); \
            std::vector<std::uint32_t> used(type_count); \
            for(std::size_t i = 0; i < count; i++) { \
                auto &r = this->scenario_object_type[i]; \
                std::size_t name_index = r.name; \
                if(name_index != NULL_INDEX) { \
                    /* Check the name to see if it's valid */ \
                    if(name_index >= name_count) { \
                        REPORT_ERROR_PRINTF(workload, ERROR_TYPE_ERROR, tag_index, object_type_str " spawn #%zu has an invalid name index (%zu >= %zu)", i, name_index, name_count); \
                    } \
                    /* If it is, increment the used counter and assign everything */ \
                    else { \
                        name_used[name_index]++; \
                        auto &name = this->object_names[name_index]; \
                        name.object_index = static_cast<HEK::Index>(i); \
                        name.object_type = HEK::ObjectType::object_type_int; \
                    } \
                } \
                std::size_t type_index = r.type; \
                if(type_index == NULL_INDEX) { \
                    REPORT_ERROR_PRINTF(workload, ERROR_TYPE_WARNING_PEDANTIC, tag_index, object_type_str " spawn #%zu has no object type, so it will be unused", i); \
                } \
                else if(type_index >= type_count) { \
                    REPORT_ERROR_PRINTF(workload, ERROR_TYPE_ERROR, tag_index, object_type_str " spawn #%zu has an invalid type index (%zu >= %zu)", i, type_index, type_count); \
                } \
                else { \
                    used[type_index]++; \
                } \
            } \
            for(std::size_t i = 0; i < type_count; i++) { \
                auto &palette = this->scenario_palette_type[i].name; \
                bool is_null = palette.path.size() == 0; \
                if(!used[i]) { \
                    if(is_null) { \
                        REPORT_ERROR_PRINTF(workload, ERROR_TYPE_WARNING_PEDANTIC, tag_index, object_type_str " palette type #%zu (null) is unused", i); \
                    } \
                    else { \
                        REPORT_ERROR_PRINTF(workload, ERROR_TYPE_WARNING_PEDANTIC, tag_index, object_type_str " palette type #%zu (%s.%s) is unused", i, File::halo_path_to_preferred_path(palette.path).data(), HEK::tag_class_to_extension(palette.tag_class_int)); \
                    } \
                } \
                else if(is_null) { \
                    REPORT_ERROR_PRINTF(workload, ERROR_TYPE_WARNING_PEDANTIC, tag_index, object_type_str " palette type #%zu is null, so %zu reference%s will be unused", i, static_cast<std::size_t>(used[i]), used[i] == 1 ? "" : "s"); \
                } \
            } \
        }

        CHECK_PALETTE_AND_SPAWNS("Biped", bipeds, biped_palette, OBJECT_TYPE_BIPED);
        CHECK_PALETTE_AND_SPAWNS("Vehicle", vehicles, vehicle_palette, OBJECT_TYPE_VEHICLE);
        CHECK_PALETTE_AND_SPAWNS("Weapon", weapons, weapon_palette, OBJECT_TYPE_WEAPON);
        CHECK_PALETTE_AND_SPAWNS("Equipment", equipment, equipment_palette, OBJECT_TYPE_EQUIPMENT);
        CHECK_PALETTE_AND_SPAWNS("Scenery", scenery, scenery_palette, OBJECT_TYPE_SCENERY);
        CHECK_PALETTE_AND_SPAWNS("Machine", machines, machine_palette, OBJECT_TYPE_DEVICE_MACHINE);
        CHECK_PALETTE_AND_SPAWNS("Control", controls, control_palette, OBJECT_TYPE_DEVICE_CONTROL);
        CHECK_PALETTE_AND_SPAWNS("Light fixture", light_fixtures, light_fixture_palette, OBJECT_TYPE_DEVICE_LIGHT_FIXTURE);
        CHECK_PALETTE_AND_SPAWNS("Sound scenery", sound_scenery, sound_scenery_palette, OBJECT_TYPE_SOUND_SCENERY);

        #undef CHECK_PALETTE_AND_SPAWNS

        // Make sure we don't have any fun stuff with object names going on
        for(std::size_t i = 0; i < name_count; i++) {
            std::size_t used = name_used[i];
            auto &name = this->object_names[i];
            const char *name_str = name.name.string;
            if(used == 0) {
                REPORT_ERROR_PRINTF(workload, ERROR_TYPE_WARNING, tag_index, "Object name #%zu (%s) is unused", i, name_str);
            }
            else if(used > 1) {
                REPORT_ERROR_PRINTF(workload, ERROR_TYPE_ERROR, tag_index, "Object name #%zu (%s) is used multiple times (found %zu times)", i, name_str, used);
            }
        }

        // If we don't have any string data, allocate 512 bytes
        if(this->script_string_data.size() == 0) {
            this->script_string_data.resize(512);
        }

        // If we don't have any syntax data, let's make some stuff
        static constexpr char SCRIPT_NODE_LITERAL[] = "script node";
        static constexpr std::size_t SCRIPT_ELEMENT_SIZE = sizeof(ScenarioScriptNode::struct_little);
        if(this->script_syntax_data.size() == 0) {
            ScenarioScriptNodeTable::struct_little t = {};
            static constexpr std::size_t DEFAULT_SCRIPT_NODE_COUNT = 32;
            t.count = 0;
            t.data = 0x64407440;
            t.element_size = SCRIPT_ELEMENT_SIZE;
            t.maximum_count = DEFAULT_SCRIPT_NODE_COUNT;
            t.next_id = 0xE741;
            t.one = 1;
            t.size = 0;
            std::copy(SCRIPT_NODE_LITERAL, SCRIPT_NODE_LITERAL + sizeof(SCRIPT_NODE_LITERAL), t.name.string);

            this->script_syntax_data.resize(sizeof(t) + SCRIPT_ELEMENT_SIZE * DEFAULT_SCRIPT_NODE_COUNT);
            auto *first_node = reinterpret_cast<ScenarioScriptNode::struct_little *>(this->script_syntax_data.data() + sizeof(t));
            for(std::size_t i = 0; i < DEFAULT_SCRIPT_NODE_COUNT; i++) {
                auto &node = first_node[i];
                std::memset(&node, 0xCA, sizeof(node));
                node.salt = 0;
            }

            std::copy(reinterpret_cast<const std::byte *>(&t), reinterpret_cast<const std::byte *>(&t + 1), this->script_syntax_data.data());
        }
        else {
            ScenarioScriptNodeTable::struct_little t;
            if(this->script_syntax_data.size() < sizeof(t)) {
                workload.report_error(BuildWorkload::ErrorType::ERROR_TYPE_FATAL_ERROR, "Script syntax data is invalid", tag_index);
                throw InvalidTagDataException();
            }
            t = *reinterpret_cast<ScenarioScriptNodeTable::struct_big *>(this->script_syntax_data.data());
            *reinterpret_cast<ScenarioScriptNodeTable::struct_little *>(this->script_syntax_data.data()) = t;
            t.first_element_ptr = 0;

            auto *start_big = reinterpret_cast<ScenarioScriptNode::struct_big *>(this->script_syntax_data.data() + sizeof(t));
            auto *start_little = reinterpret_cast<ScenarioScriptNode::struct_little *>(start_big);
            if(t.element_size != SCRIPT_ELEMENT_SIZE) {
                workload.report_error(BuildWorkload::ErrorType::ERROR_TYPE_FATAL_ERROR, "Script node table header is invalid", tag_index);
                throw InvalidTagDataException();
            }

            std::size_t element_count = t.maximum_count;
            std::size_t expected_table_size = element_count * SCRIPT_ELEMENT_SIZE + sizeof(t);
            if(this->script_syntax_data.size() != expected_table_size) {
                REPORT_ERROR_PRINTF(workload, ERROR_TYPE_FATAL_ERROR, tag_index, "Script syntax data is the wrong size (%zu expected, %zu gotten)", expected_table_size, this->script_syntax_data.size());
                throw InvalidTagDataException();
            }

            for(std::size_t i = 0; i < element_count; i++) {
                start_little[i] = start_big[i];
            }
        }

        // If we have scripts, do stuff
        if(this->scripts.size() > 0) {
            if(this->scripts.size() > 0) {
                workload.report_error(BuildWorkload::ErrorType::ERROR_TYPE_WARNING, "Scenario tag has script data but no source file data", tag_index);
                eprintf_warn("This is DEPRECATED. See https://github.com/Kavawuvi/invader/issues/80");
                eprintf_warn("To fix this, recompile the scripts");
            }
            else {
                // TODO: Recompile scripts
                workload.report_error(BuildWorkload::ErrorType::ERROR_TYPE_WARNING, "TODO: Implement script re-compiling", tag_index);
            }
        }

        // Let's start on the script data
        BuildWorkload::BuildWorkloadStruct script_data_struct = {};
        script_data_struct.data = std::move(this->script_syntax_data);
        const char *string_data = reinterpret_cast<const char *>(this->script_string_data.data());
        std::size_t string_data_length = this->script_string_data.size();

        // Ensure we're null terminated
        while(string_data_length > 0) {
            if(string_data[string_data_length - 1] != 0) {
                string_data_length--;
            }
            else {
                break;
            }
        }

        const char *string_data_end = string_data + string_data_length;
        auto *syntax_data = script_data_struct.data.data();
        auto &table_header = *reinterpret_cast<ScenarioScriptNodeTable::struct_little *>(syntax_data);
        std::uint16_t element_count = table_header.size.read();
        auto *nodes = reinterpret_cast<ScenarioScriptNode::struct_little *>(&table_header + 1);
        std::size_t errors = 0;

        for(std::uint16_t i = 0; i < element_count; i++) {
            // Check if we know the class
            std::optional<TagClassInt> tag_class;
            auto &node = nodes[i];

            // Check the class type
            switch(node.type.read()) {
                case HEK::SCENARIO_SCRIPT_VALUE_TYPE_SOUND:
                    tag_class = HEK::TAG_CLASS_SOUND;
                    break;

                case HEK::SCENARIO_SCRIPT_VALUE_TYPE_EFFECT:
                    tag_class = HEK::TAG_CLASS_EFFECT;
                    break;

                case HEK::SCENARIO_SCRIPT_VALUE_TYPE_DAMAGE:
                    tag_class = HEK::TAG_CLASS_DAMAGE_EFFECT;
                    break;

                case HEK::SCENARIO_SCRIPT_VALUE_TYPE_LOOPING_SOUND:
                    tag_class = HEK::TAG_CLASS_SOUND_LOOPING;
                    break;

                case HEK::SCENARIO_SCRIPT_VALUE_TYPE_ANIMATION_GRAPH:
                    tag_class = HEK::TAG_CLASS_MODEL_ANIMATIONS;
                    break;

                case HEK::SCENARIO_SCRIPT_VALUE_TYPE_ACTOR_VARIANT:
                    tag_class = HEK::TAG_CLASS_ACTOR_VARIANT;
                    break;

                case HEK::SCENARIO_SCRIPT_VALUE_TYPE_DAMAGE_EFFECT:
                    tag_class = HEK::TAG_CLASS_DAMAGE_EFFECT;
                    break;

                case HEK::SCENARIO_SCRIPT_VALUE_TYPE_OBJECT_DEFINITION:
                    tag_class = HEK::TAG_CLASS_OBJECT;
                    break;

                default:
                    continue;
            }

            if(tag_class.has_value()) {
                // Check if we should leave it alone
                auto flags = node.flags.read();
                if(flags.is_global || flags.is_script_call) {
                    continue;
                }

                // Get the string
                const char *string = string_data + node.string_offset.read();
                if(string >= string_data_end) {
                    if(++errors == 5) {
                        eprintf_error("... and more errors. Suffice it to say, the script node table needs recompiled");
                        break;
                    }
                    else {
                        REPORT_ERROR_PRINTF(workload, ERROR_TYPE_ERROR, tag_index, "Script node #%zu has an invalid string offset", static_cast<std::size_t>(i));
                    }
                    continue;
                }

                // Add it to the list
                std::size_t dependency_offset = reinterpret_cast<const std::byte *>(&node.data) - syntax_data;
                std::size_t new_id = workload.compile_tag_recursively(string, *tag_class);
                node.data = HEK::TagID { static_cast<std::uint32_t>(new_id) };
                auto &dependency = script_data_struct.dependencies.emplace_back();
                dependency.offset = dependency_offset;
                dependency.tag_id_only = true;
                dependency.tag_index = new_id;

                // Let's also add up a reference too. This is 110% pointless and only wastes tag data space, but it's what tool.exe does, and a Vap really wanted it.
                bool exists = false;
                auto &new_tag = workload.tags[new_id];
                for(auto &r : this->references) {
                    if(r.reference.tag_class_int == new_tag.tag_class_int && r.reference.path == new_tag.path) {
                        exists = true;
                        break;
                    }
                }
                if(!exists) {
                    auto &reference = this->references.emplace_back().reference;
                    reference.tag_class_int = new_tag.tag_class_int;
                    reference.path = new_tag.path;
                    reference.tag_id = HEK::TagID { static_cast<std::uint32_t>(new_id) };
                }
            }
        }

        if(errors > 0 && errors < 5) {
            eprintf_error("The scripts need recompiled");
        }

        // Add the new structs
        auto &new_ptr = workload.structs[struct_index].pointers.emplace_back();
        auto &scenario_struct = *reinterpret_cast<struct_little *>(workload.structs[struct_index].data.data());
        scenario_struct.script_syntax_data.size = static_cast<std::uint32_t>(script_data_struct.data.size());
        new_ptr.offset = reinterpret_cast<std::byte *>(&scenario_struct.script_syntax_data.pointer) - reinterpret_cast<std::byte *>(&scenario_struct);
        new_ptr.struct_index = workload.structs.size();
        workload.structs.emplace_back(std::move(script_data_struct));

        // BSP transitions
        std::size_t trigger_volume_count = this->trigger_volumes.size();
        for(std::size_t tv = 0; tv < trigger_volume_count; tv++) {
            auto &trigger_volume = this->trigger_volumes[tv];
            if(std::strncmp(trigger_volume.name.string, "bsp", 3) != 0) {
                continue;
            }

            // Parse it
            unsigned int bsp_from = ~0;
            unsigned int bsp_to = ~0;
            if(std::sscanf(trigger_volume.name.string, "bsp%u,%u", &bsp_from, &bsp_to) != 2) {
                continue;
            }

            // Save it
            if(bsp_from >= this->structure_bsps.size() || bsp_to >= this->structure_bsps.size()) {
                REPORT_ERROR_PRINTF(workload, ERROR_TYPE_ERROR, tag_index, "Trigger volume #%zu (%s) references an invalid BSP index", tv, trigger_volume.name.string);
            }
            else {
                auto &bsp_switch_trigger_volume = this->bsp_switch_trigger_volumes.emplace_back();
                bsp_switch_trigger_volume.trigger_volume = static_cast<HEK::Index>(tv);
                bsp_switch_trigger_volume.source = static_cast<HEK::Index>(bsp_from);
                bsp_switch_trigger_volume.destination = static_cast<HEK::Index>(bsp_to);
            }
        }
    }

    struct BSPData {
        bool valid = false;
        const ModelCollisionGeometryBSP3DNode::struct_little *bsp3d_nodes = nullptr;
        std::uint32_t bsp3d_node_count = 0;
        const ModelCollisionGeometryBSPPlane::struct_little *planes = nullptr;
        std::uint32_t plane_count = 0;
        const ModelCollisionGeometryBSPLeaf::struct_little *leaves = nullptr;
        std::uint32_t leaf_count = 0;
        const ModelCollisionGeometryBSP2DNode::struct_little *bsp2d_nodes = nullptr;
        std::uint32_t bsp2d_node_count = 0;
        const ModelCollisionGeometryBSP2DReference::struct_little *bsp2d_references = nullptr;
        std::uint32_t bsp2d_reference_count = 0;
        const ModelCollisionGeometryBSPSurface::struct_little *surfaces = nullptr;
        std::uint32_t surface_count = 0;
        const ModelCollisionGeometryBSPEdge::struct_little *edges = nullptr;
        std::uint32_t edge_count = 0;
        const ModelCollisionGeometryBSPVertex::struct_little *vertices = nullptr;
        std::uint32_t vertex_count = 0;
    };

    void Scenario::post_compile(BuildWorkload &workload, std::size_t tag_index, std::size_t, std::size_t) {
        // Get the bsp data; saves us from having to get it again
        std::vector<BSPData> bsp_data;
        bsp_data.reserve(this->structure_bsps.size());
        for(auto &b : this->structure_bsps) {
            auto &bsp_data_s = bsp_data.emplace_back();
            if(b.structure_bsp.tag_id.is_null()) {
                continue;
            }

            auto &bsp_tag_struct = workload.structs[*workload.tags[b.structure_bsp.tag_id.index].base_struct];
            auto &bsp_tag_data = *reinterpret_cast<const ScenarioStructureBSP::struct_little *>(bsp_tag_struct.data.data());
            if(bsp_tag_data.collision_bsp.count == 0) {
                continue;
            }

            auto &collision_bsp_struct = workload.structs[*bsp_tag_struct.resolve_pointer(&bsp_tag_data.collision_bsp.pointer)];
            auto &collision_bsp_data = *reinterpret_cast<const ModelCollisionGeometryBSP::struct_little *>(collision_bsp_struct.data.data());

            bsp_data_s.bsp3d_node_count = collision_bsp_data.bsp3d_nodes.count;
            bsp_data_s.plane_count = collision_bsp_data.planes.count;
            bsp_data_s.leaf_count = collision_bsp_data.leaves.count;
            bsp_data_s.bsp2d_node_count = collision_bsp_data.bsp2d_nodes.count;
            bsp_data_s.bsp2d_reference_count = collision_bsp_data.bsp2d_references.count;
            bsp_data_s.surface_count = collision_bsp_data.surfaces.count;
            bsp_data_s.edge_count = collision_bsp_data.edges.count;
            bsp_data_s.vertex_count = collision_bsp_data.vertices.count;

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
        }

        // TODO: Position encounters and command lists
        for(auto &encounter : this->encounters) {
            if(encounter.flags.manual_bsp_index_specified) {
                continue;
            }
            workload.report_error(BuildWorkload::ErrorType::ERROR_TYPE_WARNING, "TODO: Implement encounter BSP location", tag_index);
            break;
        }

        // Determine where the command list is
        for(auto &command_list : this->command_lists) {
            if(command_list.flags.manual_bsp_index) {
                continue;
            }
            workload.report_error(BuildWorkload::ErrorType::ERROR_TYPE_WARNING, "TODO: Implement command list BSP location", tag_index);
            break;
        }

        // Decals
        std::size_t decal_count = decals.size();
        if(decal_count > 0) {
            for(auto &b : this->structure_bsps) {
                auto &bsp_id = b.structure_bsp.tag_id;
                if(!bsp_id.is_null()) {
                    auto &bsp_tag_struct = workload.structs[*workload.structs[*(workload.tags[bsp_id.index].base_struct)].resolve_pointer(static_cast<std::size_t>(0))];
                    auto &bsp_tag_data = *reinterpret_cast<ScenarioStructureBSP::struct_little *>(bsp_tag_struct.data.data());
                    std::vector<ScenarioStructureBSPRuntimeDecal::struct_little> runtime_decals;
                    std::size_t bsp_cluster_count = bsp_tag_data.clusters.count.read();

                    if(bsp_cluster_count == 0) {
                        continue;
                    }

                    // Make sure we know which decals we used for this BSP so we don't reuse them
                    std::vector<bool> used(decal_count, false);

                    // Now let's go do stuff
                    auto &bsp_cluster_struct = workload.structs[*bsp_tag_struct.resolve_pointer(&bsp_tag_data.clusters.pointer)];
                    auto *clusters = reinterpret_cast<ScenarioStructureBSPCluster::struct_little *>(bsp_cluster_struct.data.data());

                    // Get clusters
                    for(std::size_t c = 0; c < bsp_cluster_count; c++) {
                        auto &cluster = clusters[c];
                        std::vector possible(decal_count, false);
                        std::size_t subcluster_count = cluster.subclusters.count.read();
                        if(subcluster_count == 0) {
                            continue;
                        }

                        auto &subcluster_struct = workload.structs[*bsp_cluster_struct.resolve_pointer(&cluster.subclusters.pointer)];
                        auto *subclusters = reinterpret_cast<ScenarioStructureBSPSubcluster::struct_little *>(subcluster_struct.data.data());

                        // Go through subclusters to see which one can hold a decal
                        for(std::size_t s = 0; s < subcluster_count; s++) {
                            auto &subcluster = subclusters[s];
                            for(std::size_t d = 0; d < decal_count; d++) {
                                if(used[d]) {
                                    continue;
                                }
                                auto &decal = decals[d];
                                float x = decal.position.x;
                                float y = decal.position.y;
                                float z = decal.position.z;
                                if(x >= subcluster.world_bounds_x.from && x <= subcluster.world_bounds_x.to &&
                                   y >= subcluster.world_bounds_y.from && y <= subcluster.world_bounds_y.to &&
                                   z >= subcluster.world_bounds_z.from && z <= subcluster.world_bounds_z.to) {
                                    possible[d] = decal.decal_type != NULL_INDEX;
                                }
                            }
                        }

                        // Now add all of the decals that we found
                        std::size_t runtime_decal_count_first = runtime_decals.size();
                        std::size_t cluster_decal_count = 0;

                        for(std::size_t p = 0; p < decal_count; p++) {
                            if(possible[p]) {
                                auto &decal_to_copy = this->decals[p];
                                auto &decal = runtime_decals.emplace_back();
                                decal.decal_type = decal_to_copy.decal_type;
                                decal.pitch = decal_to_copy.pitch;
                                decal.yaw = decal_to_copy.yaw;
                                decal.position = decal_to_copy.position;
                                cluster_decal_count++;
                                used[p] = true;
                            }
                        }

                        // Set the decal count
                        if(cluster_decal_count != 0) {
                            cluster.first_decal_index = static_cast<std::int16_t>(runtime_decal_count_first);
                            cluster.decal_count = static_cast<std::int16_t>(cluster_decal_count);
                        }
                        else {
                            cluster.first_decal_index = -1;
                            cluster.decal_count = 0;
                        }
                    }

                    // Set the decal count to the number of decals we used and add it to the end of the BSP
                    bsp_tag_data.runtime_decals.count = static_cast<std::uint32_t>(runtime_decals.size());

                    if(runtime_decals.size() != 0) {
                        auto &new_struct_ptr = bsp_tag_struct.pointers.emplace_back();
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

    void ScenarioCutsceneTitle::pre_compile(BuildWorkload &, std::size_t, std::size_t, std::size_t) {
        this->fade_in_time *= TICK_RATE;
        this->fade_out_time *= TICK_RATE;
        this->up_time *= TICK_RATE;
    }
}
