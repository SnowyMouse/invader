// SPDX-License-Identifier: GPL-3.0-only

#include <invader/tag/parser/parser.hpp>
#include <invader/build/build_workload.hpp>
#include <invader/file/file.hpp>
#include <invader/tag/parser/compile/scenario.hpp>

namespace Invader::Parser {
    static void merge_child_scenarios(BuildWorkload &workload, std::size_t tag_index, Scenario &scenario);
    static void check_palettes(BuildWorkload &workload, std::size_t tag_index, Scenario &scenario);
    static void fix_script_data(BuildWorkload &workload, std::size_t tag_index, std::size_t struct_index, Scenario &scenario);
    static void fix_bsp_transitions(BuildWorkload &workload, std::size_t tag_index, Scenario &scenario);
    
    void Scenario::pre_compile(BuildWorkload &workload, std::size_t tag_index, std::size_t struct_index, std::size_t) {
        merge_child_scenarios(workload, tag_index, *this);

        if(!workload.cache_file_type.has_value()) {
            workload.cache_file_type = this->type;
            workload.demo_ui = this->flags & HEK::ScenarioFlagsFlag::SCENARIO_FLAGS_FLAG_USE_DEMO_UI;
        }

        // Check some things
        check_palettes(workload, tag_index, *this);
        fix_script_data(workload, tag_index, struct_index, *this);
        fix_bsp_transitions(workload, tag_index, *this);
    }
    
    static void fix_bsp_transitions(BuildWorkload &workload, std::size_t tag_index, Scenario &scenario) {
        // BSP transitions
        std::size_t trigger_volume_count = scenario.trigger_volumes.size();
        scenario.bsp_switch_trigger_volumes.clear();
        for(std::size_t tv = 0; tv < trigger_volume_count; tv++) {
            auto &trigger_volume = scenario.trigger_volumes[tv];
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
            if(bsp_from >= scenario.structure_bsps.size() || bsp_to >= scenario.structure_bsps.size()) {
                if(!workload.disable_error_checking) {
                    REPORT_ERROR_PRINTF(workload, ERROR_TYPE_FATAL_ERROR, tag_index, "Trigger volume #%zu (%s) references an invalid BSP index", tv, trigger_volume.name.string);
                    throw InvalidTagDataException();
                }
            }
            else {
                auto &bsp_switch_trigger_volume = scenario.bsp_switch_trigger_volumes.emplace_back();
                bsp_switch_trigger_volume.trigger_volume = static_cast<HEK::Index>(tv);
                bsp_switch_trigger_volume.source = static_cast<HEK::Index>(bsp_from);
                bsp_switch_trigger_volume.destination = static_cast<HEK::Index>(bsp_to);
                bsp_switch_trigger_volume.unknown = 0xFFFF;
            }
        }
    }
    
    static constexpr const std::size_t MAX_SCRIPT_NODE_COUNT = 19001;
    
    static void fix_script_data(BuildWorkload &workload, std::size_t tag_index, std::size_t struct_index, Scenario &scenario) {
        // If we don't have any string data, allocate 512 bytes
        if(scenario.script_string_data.size() == 0) {
            scenario.script_string_data.resize(512);
        }

        // If we don't have any syntax data, let's make some stuff
        static constexpr char SCRIPT_NODE_LITERAL[] = "script node";
        static constexpr std::size_t SCRIPT_ELEMENT_SIZE = sizeof(ScenarioScriptNode::struct_little);
        if(scenario.script_syntax_data.size() == 0) {
            ScenarioScriptNodeTable::struct_little t = {};
            static constexpr std::size_t DEFAULT_SCRIPT_NODE_COUNT = 128;
            t.count = 0;
            t.data = 0x64407440;
            t.element_size = SCRIPT_ELEMENT_SIZE;
            t.maximum_count = DEFAULT_SCRIPT_NODE_COUNT;
            t.next_id = 0xE741;
            t.one = 1;
            t.size = 0;
            std::copy(SCRIPT_NODE_LITERAL, SCRIPT_NODE_LITERAL + sizeof(SCRIPT_NODE_LITERAL), t.name.string);

            scenario.script_syntax_data.resize(sizeof(t) + SCRIPT_ELEMENT_SIZE * DEFAULT_SCRIPT_NODE_COUNT);
            auto *first_node = reinterpret_cast<ScenarioScriptNode::struct_little *>(scenario.script_syntax_data.data() + sizeof(t));
            for(std::size_t i = 0; i < DEFAULT_SCRIPT_NODE_COUNT; i++) {
                auto &node = first_node[i];
                std::memset(reinterpret_cast<std::uint8_t *>(&node), 0xCA, sizeof(node));
                node.salt = 0;
            }

            std::copy(reinterpret_cast<const std::byte *>(&t), reinterpret_cast<const std::byte *>(&t + 1), scenario.script_syntax_data.data());
        }
        else {
            ScenarioScriptNodeTable::struct_little t;
            if(scenario.script_syntax_data.size() < sizeof(t)) {
                if(!workload.disable_error_checking) {
                    workload.report_error(BuildWorkload::ErrorType::ERROR_TYPE_FATAL_ERROR, "Script syntax data is invalid", tag_index);
                    throw InvalidTagDataException();
                }
            }
            else {
                t = *reinterpret_cast<ScenarioScriptNodeTable::struct_big *>(scenario.script_syntax_data.data());
                *reinterpret_cast<ScenarioScriptNodeTable::struct_little *>(scenario.script_syntax_data.data()) = t;
                t.first_element_ptr = 0;
            
                // Maximum node count exceeded?
                if(t.maximum_count > MAX_SCRIPT_NODE_COUNT) {
                    if(!workload.disable_error_checking) {
                        REPORT_ERROR_PRINTF(workload, ERROR_TYPE_FATAL_ERROR, tag_index, "Script node table contains too many script nodes for the target engine (%zu > %zu)", static_cast<std::size_t>(t.maximum_count), MAX_SCRIPT_NODE_COUNT);
                        throw InvalidTagDataException();
                    }
                }
                else {
                    auto *start_big = reinterpret_cast<ScenarioScriptNode::struct_big *>(scenario.script_syntax_data.data() + sizeof(t));
                    auto *start_little = reinterpret_cast<ScenarioScriptNode::struct_little *>(start_big);
                    if(t.element_size != SCRIPT_ELEMENT_SIZE) {
                        if(!workload.disable_error_checking) {
                            workload.report_error(BuildWorkload::ErrorType::ERROR_TYPE_FATAL_ERROR, "Script node table header is invalid", tag_index);
                            throw InvalidTagDataException();
                        }
                    }
                    else {
                        std::size_t element_count = t.maximum_count;
                        std::size_t expected_table_size = element_count * SCRIPT_ELEMENT_SIZE + sizeof(t);
                        if(scenario.script_syntax_data.size() != expected_table_size) {
                            if(!workload.disable_error_checking) {
                                REPORT_ERROR_PRINTF(workload, ERROR_TYPE_FATAL_ERROR, tag_index, "Script syntax data is the wrong size (%zu expected, %zu gotten)", expected_table_size, scenario.script_syntax_data.size());
                                throw InvalidTagDataException();
                            }
                        }
                        else {
                            for(std::size_t i = 0; i < element_count; i++) {
                                start_little[i] = start_big[i];
                            }
                        }
                    }
                }
            }
        }
        
        // If we have scripts, do stuff
        if(scenario.scripts.size() > 0 || scenario.globals.size() > 0) {
            if(scenario.source_files.size() == 0) {
                if(!workload.disable_error_checking) {
                    workload.report_error(BuildWorkload::ErrorType::ERROR_TYPE_FATAL_ERROR, "Scenario tag has script data but no source file data", tag_index);
                    eprintf_warn("To fix this, recompile the scripts");
                    throw InvalidTagDataException();
                }
            }
            else {
                // TODO: Recompile scripts
                workload.report_error(BuildWorkload::ErrorType::ERROR_TYPE_WARNING, "TODO: Tags with scripts are not fully supported at this time", tag_index);
            }
        }

        // Let's start on the script data
        BuildWorkload::BuildWorkloadStruct script_data_struct = {};
        script_data_struct.data = std::move(scenario.script_syntax_data);
        const char *string_data = reinterpret_cast<const char *>(scenario.script_string_data.data());
        std::size_t string_data_length = scenario.script_string_data.size();

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
                if(
                    (flags & HEK::ScenarioScriptNodeFlagsFlag::SCENARIO_SCRIPT_NODE_FLAGS_FLAG_IS_GLOBAL) ||
                    (flags & HEK::ScenarioScriptNodeFlagsFlag::SCENARIO_SCRIPT_NODE_FLAGS_FLAG_IS_SCRIPT_CALL)
                ) {
                    continue;
                }

                // Get the string
                const char *string = string_data + node.string_offset.read();
                if(string >= string_data_end) {
                    if(!workload.disable_error_checking) {
                        REPORT_ERROR_PRINTF(workload, ERROR_TYPE_FATAL_ERROR, tag_index, "Script node #%zu has an invalid string offset. The scripts need recompiled.", static_cast<std::size_t>(i));
                    }
                    break;
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
                for(auto &r : scenario.references) {
                    if(r.reference.tag_fourcc == new_tag.tag_fourcc && r.reference.path == new_tag.path) {
                        exists = true;
                        break;
                    }
                }
                if(!exists) {
                    auto &reference = scenario.references.emplace_back().reference;
                    reference.tag_fourcc = new_tag.tag_fourcc;
                    reference.path = new_tag.path;
                    reference.tag_id = HEK::TagID { static_cast<std::uint32_t>(new_id) };
                }
            }
        }

        // Add the new structs
        auto &new_ptr = workload.structs[struct_index].pointers.emplace_back();
        auto &scenario_struct = *reinterpret_cast<Scenario::struct_little *>(workload.structs[struct_index].data.data());
        scenario_struct.script_syntax_data.size = static_cast<std::uint32_t>(script_data_struct.data.size());
        new_ptr.offset = reinterpret_cast<std::byte *>(&scenario_struct.script_syntax_data.pointer) - reinterpret_cast<std::byte *>(&scenario_struct);
        new_ptr.struct_index = workload.structs.size();
        workload.structs.emplace_back(std::move(script_data_struct));
    }
    
    static void check_palettes(BuildWorkload &workload, std::size_t tag_index, Scenario &scenario) {
        // Check for unused stuff
        std::size_t name_count = scenario.object_names.size();
        std::vector<std::vector<std::pair<const char *, std::size_t>>> name_used(name_count);

        // We want to make sure things are valid
        #define CHECK_PALETTE_AND_SPAWNS(object_type_str, scenario_object_type, scenario_palette_type, object_type_int) { \
            std::size_t type_count = scenario.scenario_palette_type.size(); \
            std::size_t count = scenario.scenario_object_type.size(); \
            std::vector<std::uint32_t> used(type_count); \
            for(std::size_t i = 0; i < count; i++) { \
                auto &r = scenario.scenario_object_type[i]; \
                std::size_t name_index = r.name; \
                if(name_index != NULL_INDEX) { \
                    /* Check the name to see if it's valid */ \
                    if(name_index >= name_count) { \
                        if(!workload.disable_error_checking) { \
                            REPORT_ERROR_PRINTF(workload, ERROR_TYPE_FATAL_ERROR, tag_index, object_type_str " spawn #%zu has an invalid name index (%zu >= %zu)", i, name_index, name_count); \
                            throw InvalidTagDataException(); \
                        } \
                    } \
                    /* If it is, increment the used counter and assign everything */ \
                    else { \
                        name_used[name_index].emplace_back(object_type_str, i); \
                        auto &name = scenario.object_names[name_index]; \
                        name.object_index = static_cast<HEK::Index>(i); \
                        name.object_type = HEK::ObjectType::object_type_int; \
                    } \
                } \
                std::size_t type_index = r.type; \
                if(type_index == NULL_INDEX) { \
                    REPORT_ERROR_PRINTF(workload, ERROR_TYPE_WARNING_PEDANTIC, tag_index, object_type_str " spawn #%zu has no object type, so it will be unused", i); \
                } \
                else if(type_index >= type_count) { \
                    if(!workload.disable_error_checking) { \
                        REPORT_ERROR_PRINTF(workload, ERROR_TYPE_FATAL_ERROR, tag_index, object_type_str " spawn #%zu has an invalid type index (%zu >= %zu)", i, type_index, type_count); \
                        throw InvalidTagDataException(); \
                    } \
                } \
                else { \
                    used[type_index]++; \
                } \
            } \
            for(std::size_t i = 0; i < type_count; i++) { \
                auto &palette = scenario.scenario_palette_type[i].name; \
                bool is_null = palette.path.size() == 0; \
                if(!used[i]) { \
                    if(is_null) { \
                        REPORT_ERROR_PRINTF(workload, ERROR_TYPE_WARNING_PEDANTIC, tag_index, object_type_str " palette type #%zu (null) is unused", i); \
                    } \
                    else { \
                        REPORT_ERROR_PRINTF(workload, ERROR_TYPE_WARNING_PEDANTIC, tag_index, object_type_str " palette type #%zu (%s.%s) is unused", i, File::halo_path_to_preferred_path(palette.path).c_str(), HEK::tag_class_to_extension(palette.tag_fourcc)); \
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
        
        // Next, let's make sure "set new name" is used
        for(auto &c : scenario.ai_conversations) {
            for(auto &p : c.participants) {
                auto new_name = p.set_new_name;
                if(new_name > name_count || new_name == NULL_INDEX) {
                    continue;
                }
                else if(name_used[new_name].size() == 0) {
                    name_used[new_name].emplace_back(); 
                }
            }
        }

        // Make sure we don't have any fun stuff with object names going on
        for(std::size_t i = 0; i < name_count; i++) {
            auto &used_arr = name_used[i];
            auto used = used_arr.size();
            auto &name = scenario.object_names[i];
            const char *name_str = name.name.string;
            if(used == 0) {
                REPORT_ERROR_PRINTF(workload, ERROR_TYPE_WARNING, tag_index, "Object name #%zu (%s) is unused", i, name_str);
            }
            else if(used > 1 && !workload.disable_error_checking) {
                REPORT_ERROR_PRINTF(workload, ERROR_TYPE_FATAL_ERROR, tag_index, "Object name #%zu (%s) is used multiple times (found %zu times)", i, name_str, used);
                
                // Put together a list to help the user track everything down
                char found[1024] = {};
                std::size_t p = 0;
                
                std::size_t f = 0;
                for(auto &u : used_arr) {
                    // Don't show more than 3 elements
                    if(f++ == 3) {
                        std::snprintf(found + p, sizeof(found) - p, ", ...");
                        break;
                    }
                    else {
                        p += std::snprintf(found + p, sizeof(found) - p, "%s%s #%zu", f == 1 ? "" : ", ", u.first, u.second);
                        if(p > sizeof(found)) {
                            break;
                        }
                    }
                }
                
                // List everything off
                eprintf_warn_lesser("    - objects with this name: [%s]", found);
                throw InvalidTagDataException();
            }
        }
    }
    
    static void merge_child_scenario(Scenario &base_scenario, const Scenario &scenario_to_merge, BuildWorkload &workload, std::size_t tag_index, const char *child_scenario_path) {
        #define MERGE_ARRAY(what, condition) for(auto &merge : scenario_to_merge.what) { \
            bool can_merge = true; \
            for([[maybe_unused]] auto &base : base_scenario.what) { \
                if(!(condition)) { \
                    can_merge = false; \
                    break; \
                } \
            } \
            if(can_merge) { \
                base_scenario.what.push_back(merge); \
            } \
        }
        
        MERGE_ARRAY(child_scenarios, true);
        MERGE_ARRAY(functions, true);
        MERGE_ARRAY(comments, true);
        MERGE_ARRAY(object_names, merge.name != base.name);
        MERGE_ARRAY(device_groups, merge.name != base.name);
        MERGE_ARRAY(player_starting_profile, true);
        MERGE_ARRAY(player_starting_locations, true);
        MERGE_ARRAY(trigger_volumes, merge.name != base.name);
        MERGE_ARRAY(recorded_animations, merge.name != base.name);
        MERGE_ARRAY(netgame_flags, true);
        MERGE_ARRAY(netgame_equipment, true);
        MERGE_ARRAY(starting_equipment, true);
        MERGE_ARRAY(actor_palette, merge.reference.path != base.reference.path || merge.reference.tag_fourcc != base.reference.tag_fourcc);
        MERGE_ARRAY(ai_animation_references, merge.animation_name != base.animation_name);
        MERGE_ARRAY(ai_script_references, merge.script_name != base.script_name);
        MERGE_ARRAY(ai_recording_references, merge.recording_name != base.recording_name);
        MERGE_ARRAY(references, merge.reference.path != base.reference.path || merge.reference.tag_fourcc != base.reference.tag_fourcc);
        MERGE_ARRAY(cutscene_flags, merge.name != base.name);
        MERGE_ARRAY(cutscene_camera_points, merge.name != base.name);
        MERGE_ARRAY(cutscene_titles, merge.name != base.name);
        MERGE_ARRAY(source_files, merge.name != base.name);
        MERGE_ARRAY(decal_palette, merge.reference.path != base.reference.path || merge.reference.tag_fourcc != base.reference.tag_fourcc);
        
        // Merge palettes
        #define MERGE_PALETTE(what) MERGE_ARRAY(what, merge.name.path != base.name.path || merge.name.tag_fourcc != base.name.tag_fourcc)
        
        MERGE_PALETTE(scenery_palette);
        MERGE_PALETTE(biped_palette);
        MERGE_PALETTE(vehicle_palette);
        MERGE_PALETTE(equipment_palette);
        MERGE_PALETTE(weapon_palette);
        MERGE_PALETTE(machine_palette);
        MERGE_PALETTE(control_palette);
        MERGE_PALETTE(light_fixture_palette);
        MERGE_PALETTE(sound_scenery_palette);
        
        // Make some lambdas for finding stuff quickly
        #define TRANSLATE_PALETTE(what, match_comparison) [&base_scenario, &scenario_to_merge, &workload, &tag_index, &child_scenario_path](HEK::Index old_index) -> HEK::Index { \
            /* If we're null, return null */ \
            if(old_index == NULL_INDEX) { \
                return NULL_INDEX; \
            } \
\
            /* if we're out of bounds, fail */ \
            auto old_count = scenario_to_merge.what.size(); \
            if(old_index >= old_count) { \
                if(!workload.disable_error_checking) { \
                    REPORT_ERROR_PRINTF(workload, ERROR_TYPE_FATAL_ERROR, tag_index, # what " index in child scenario %s is out of bounds (%zu >= %zu)", child_scenario_path, static_cast<std::size_t>(old_index), old_count); \
                    throw OutOfBoundsException(); \
                } \
                return NULL_INDEX; \
            } \
\
            /* Find it */ \
            auto &merge = scenario_to_merge.what[old_index]; \
            auto new_count = base_scenario.what.size(); \
            for(std::size_t name = 0; name < new_count; name++) { \
                auto &base = base_scenario.what[name]; \
                if((match_comparison)) { \
                    if(name >= NULL_INDEX) { \
                        if(!workload.disable_error_checking) { \
                            REPORT_ERROR_PRINTF(workload, ERROR_TYPE_FATAL_ERROR, tag_index, # what " in child scenario %s exceeded %zu when merging", child_scenario_path, static_cast<std::size_t>(NULL_INDEX - 1)); \
                            throw InvalidTagDataException(); \
                        } \
                        return NULL_INDEX; \
                    } \
                    return name; \
                } \
            } \
            if(!workload.disable_error_checking) { \
                REPORT_ERROR_PRINTF(workload, ERROR_TYPE_FATAL_ERROR, tag_index, "Failed to find an entry in " # what " for child scenario %s", child_scenario_path); \
                throw OutOfBoundsException(); \
            } \
            return NULL_INDEX; \
        }
        auto translate_object_name = TRANSLATE_PALETTE(object_names, (merge.name == base.name));
        auto translate_device_group = TRANSLATE_PALETTE(device_groups, (merge.name == base.name));
        
        // Merge AI conversations
        for(auto &aic : scenario_to_merge.ai_conversations) {
            auto &new_aic = base_scenario.ai_conversations.emplace_back(aic);
            for(auto &p : new_aic.participants) {
                p.set_new_name = translate_object_name(p.set_new_name);
                p.use_this_object = translate_object_name(p.use_this_object);
            }
        }

        #undef MERGE_PALETTE
        #undef MERGE_ARRAY
        
        #define MERGE_OBJECTS_ALL(what, what_palette, ...) { \
            auto object_count = scenario_to_merge.what.size(); \
            auto translate_palette = TRANSLATE_PALETTE(what_palette, (merge.name.path == base.name.path && merge.name.tag_fourcc == base.name.tag_fourcc)); \
            for(std::size_t o = 0; o < object_count; o++) { \
                auto &new_element = base_scenario.what.emplace_back(scenario_to_merge.what[o]); \
                new_element.name = translate_object_name(new_element.name); \
                new_element.type = translate_palette(new_element.type); \
                __VA_ARGS__ \
            } \
        }
        
        #define MERGE_OBJECTS(what, what_palette) MERGE_OBJECTS_ALL(what, what_palette, {})
        #define MERGE_DEVICES(what, what_palette) MERGE_OBJECTS_ALL(what, what_palette, { \
            new_element.power_group = translate_device_group(new_element.power_group); \
            new_element.position_group = translate_device_group(new_element.position_group); \
        })
        
        MERGE_OBJECTS(scenery,scenery_palette);
        MERGE_OBJECTS(bipeds,biped_palette);
        MERGE_OBJECTS(vehicles,vehicle_palette);
        MERGE_OBJECTS(equipment,equipment_palette);
        MERGE_OBJECTS(weapons,weapon_palette);
        MERGE_DEVICES(machines,machine_palette);
        MERGE_DEVICES(controls,control_palette);
        MERGE_DEVICES(light_fixtures,light_fixture_palette);
        MERGE_OBJECTS(sound_scenery,sound_scenery_palette);
        
        #undef MERGE_OBJECTS
        #undef MERGE_OBJECTS_ALL
        
        // Decals
        auto translate_decal_palette = TRANSLATE_PALETTE(decal_palette, merge.reference.tag_fourcc == base.reference.tag_fourcc && merge.reference.path == base.reference.path);
        for(auto &decal : scenario_to_merge.decals) {
            // Add our new decal
            auto &new_decal = base_scenario.decals.emplace_back(decal);
            new_decal.decal_type = translate_decal_palette(new_decal.decal_type);
        }
        
        // AI stuff
        auto translate_actor_palette = TRANSLATE_PALETTE(actor_palette, (merge.reference.tag_fourcc == base.reference.tag_fourcc && merge.reference.path == base.reference.path));
        auto translate_animation_palette = TRANSLATE_PALETTE(ai_animation_references, merge.animation_name == base.animation_name);
        auto translate_command_list = TRANSLATE_PALETTE(command_lists, merge.name == base.name);
        auto translate_recording = TRANSLATE_PALETTE(ai_recording_references, merge.recording_name == base.recording_name);
        auto translate_script_reference = TRANSLATE_PALETTE(ai_script_references, merge.script_name == base.script_name);
        
        // Merge command lists
        for(auto &command_list : scenario_to_merge.command_lists) {
            // First, make sure we don't have this in here already
            bool exists = false;
            for(auto &existing_command_list : base_scenario.command_lists) {
                if(existing_command_list.name == command_list.name) {
                    exists = true;
                    break;
                }
            }
            // Darn
            if(exists) {
                continue;
            }
            
            // Add our new list
            auto &new_command_list = base_scenario.command_lists.emplace_back(command_list);
            for(auto &command : new_command_list.commands) {
                command.animation = translate_animation_palette(command.animation);
                command.recording = translate_recording(command.recording);
                command.object_name = translate_object_name(command.object_name);
                command.animation = translate_animation_palette(command.animation);
                command.script = translate_script_reference(command.script);
            }
        }
        
        // Merge encounters
        for(auto &encounter : scenario_to_merge.encounters) {
            // First, make sure we don't have this in here already
            bool exists = false;
            for(auto &existing_encounters : base_scenario.encounters) {
                if(existing_encounters.name == encounter.name) {
                    exists = true;
                    break;
                }
            }
            // Darn
            if(exists) {
                continue;
            }
            
            // Add our new encounter
            auto &new_encounter = base_scenario.encounters.emplace_back(encounter);
            for(auto &squad : new_encounter.squads) {
                squad.actor_type = translate_actor_palette(squad.actor_type);
                for(auto &mp : squad.move_positions) {
                    mp.animation = translate_animation_palette(mp.animation);
                }
                for(auto &sl : squad.starting_locations) {
                    sl.actor_type = translate_actor_palette(sl.actor_type);
                    sl.command_list = translate_command_list(sl.command_list);
                }
            }
        }
        
        #undef TRANSLATE_PALETTE
    }

    static void merge_child_scenarios(BuildWorkload &workload, std::size_t tag_index, Scenario &scenario) {
        // Merge child scenarios
        if(scenario.child_scenarios.size() != 0 && !workload.disable_recursion) {
            // Let's begin by adding this scenario to the list (in case we reference ourself)
            std::vector<std::string> merged_scenarios;
            merged_scenarios.emplace_back(workload.tags[tag_index].path);
            
            // Take the scenario off the top
            while(scenario.child_scenarios.size()) {
                // Get the scenario
                auto &first_scenario = scenario.child_scenarios[0].child_scenario;
                if(!first_scenario.path.empty()) {
                    // If this isn't even a scenario tag... what
                    if(first_scenario.tag_fourcc != TagClassInt::TAG_CLASS_SCENARIO) {
                        // This should fail even if we aren't checking for errors because this is invalid
                        REPORT_ERROR_PRINTF(workload, ERROR_TYPE_FATAL_ERROR, tag_index, "Non-scenario %s.%s referenced in child scenarios", File::halo_path_to_preferred_path(first_scenario.path).c_str(), HEK::tag_class_to_extension(first_scenario.tag_fourcc));
                        throw InvalidTagDataException();
                    }
                    
                    // Make sure we haven't done it already
                    for(auto &m : merged_scenarios) {
                        // This should fail even if we aren't checking for errors because this is invalid
                        if(m == first_scenario.path) {
                            workload.report_error(BuildWorkload::ErrorType::ERROR_TYPE_FATAL_ERROR, "Duplicate or cyclical child scenario references are present", tag_index);
                            eprintf_warn("First duplicate scenario: %s.%s", File::halo_path_to_preferred_path(first_scenario.path).c_str(), HEK::tag_class_to_extension(first_scenario.tag_fourcc));
                            throw InvalidTagDataException();
                        }
                    }
                    
                    // Add it to the list
                    merged_scenarios.emplace_back(first_scenario.path);
                    
                    // Find it
                    char file_path_cstr[1024];
                    std::snprintf(file_path_cstr, sizeof(file_path_cstr), "%s.%s", File::halo_path_to_preferred_path(first_scenario.path).c_str(), HEK::tag_class_to_extension(first_scenario.tag_fourcc));
                    auto file_path = File::tag_path_to_file_path(file_path_cstr, workload.get_build_parameters()->tags_directories);
                    if(!file_path.has_value() || !std::filesystem::exists(*file_path)) {
                        REPORT_ERROR_PRINTF(workload, ERROR_TYPE_FATAL_ERROR, tag_index, "Child scenario %s not found", file_path_cstr);
                        throw InvalidTagDataException();
                    }
                    
                    // Open it
                    auto data = File::open_file(*file_path);
                    if(!data.has_value()) {
                        REPORT_ERROR_PRINTF(workload, ERROR_TYPE_FATAL_ERROR, tag_index, "Failed to open %s", file_path->string().c_str());
                        throw InvalidTagDataException();
                    }
                    
                    // Parse and merge it
                    try {
                        auto child = Scenario::parse_hek_tag_file(data->data(), data->size());
                        data.reset(); // clear it
                        merge_child_scenario(scenario, child, workload, tag_index, (File::halo_path_to_preferred_path(first_scenario.path) + "." + HEK::tag_class_to_extension(first_scenario.tag_fourcc)).c_str());
                    }
                    catch(std::exception &) {
                        REPORT_ERROR_PRINTF(workload, ERROR_TYPE_FATAL_ERROR, tag_index, "Failed to merge %s%s into %s.%s",
                                            File::halo_path_to_preferred_path(first_scenario.path).c_str(),
                                            HEK::tag_class_to_extension(first_scenario.tag_fourcc),
                                            workload.tags[tag_index].path.c_str(),
                                            HEK::tag_class_to_extension(workload.tags[tag_index].tag_fourcc)
                                           );
                        throw;
                    }
                }
                
                // Delete the scenario
                scenario.child_scenarios.erase(scenario.child_scenarios.begin());
            }
        }
    }

    void ScenarioCutsceneTitle::pre_compile(BuildWorkload &, std::size_t, std::size_t, std::size_t) {
        this->fade_in_time *= TICK_RATE;
        this->fade_out_time *= TICK_RATE;
        this->up_time *= TICK_RATE;
    }
    
    void ScenarioFiringPosition::pre_compile(BuildWorkload &, std::size_t, std::size_t, std::size_t) {
        this->cluster_index = NULL_INDEX;
        this->surface_index = NULL_INDEX;
    }
    
    bool fix_excessive_script_nodes(Scenario &scenario, bool fix) {
        if(scenario.script_syntax_data.size() == 0) {
            return false;
        }
        else {
            ScenarioScriptNodeTable::struct_big *t;
            if(scenario.script_syntax_data.size() < sizeof(*t)) {
                return false; // invalid
            }
            t = reinterpret_cast<ScenarioScriptNodeTable::struct_big *>(scenario.script_syntax_data.data());
            
            // Maximum node count exceeded? And can we safely resize it?
            if(t->maximum_count > MAX_SCRIPT_NODE_COUNT && static_cast<std::size_t>(t->size) + 128 < MAX_SCRIPT_NODE_COUNT) {
                if(fix) {
                    t->maximum_count = MAX_SCRIPT_NODE_COUNT;
                    scenario.script_syntax_data.resize(sizeof(*t) + sizeof(ScenarioScriptNode::struct_big) * t->maximum_count);
                }
                return true;
            }
            
            return false;
        }
    }
}
