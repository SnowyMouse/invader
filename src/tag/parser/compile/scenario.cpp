// SPDX-License-Identifier: GPL-3.0-only

#include <invader/tag/parser/parser.hpp>
#include <invader/build/build_workload.hpp>
#include <invader/file/file.hpp>
#include <invader/tag/hek/class/model_collision_geometry.hpp>
#include <invader/script/compiler.hpp>
#include <invader/tag/parser/compile/scenario.hpp>

namespace Invader::Parser {
    void Invader::Parser::Scenario::postprocess_hek_data() {
        if(this->script_syntax_data.size() >= sizeof(ScenarioScriptNodeTable::struct_little)) {
            auto *table = reinterpret_cast<ScenarioScriptNodeTable::struct_big *>(this->script_syntax_data.data());
            table->first_element_ptr = 0;

            // FFFFFFFF-out all the tag IDs
            std::size_t element_count = std::min(static_cast<std::size_t>(table->size.read()), static_cast<std::size_t>(this->script_syntax_data.size() - sizeof(*table)) / sizeof(ScenarioScriptNode::struct_little));
            auto *nodes = reinterpret_cast<ScenarioScriptNode::struct_big *>(table + 1);

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
                    node.data = HEK::TagID::null_tag_id();
                }
            }
        }
    }

    void ScenarioFiringPosition::pre_compile(BuildWorkload &, std::size_t, std::size_t, std::size_t) {
        this->cluster_index = NULL_INDEX;
        this->surface_index = NULL_INDEX;
    }
    
    static void merge_child_scenario(Scenario &base_scenario, const Scenario &scenario_to_merge) {
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
        MERGE_ARRAY(actor_palette, merge.reference.path != base.reference.path || merge.reference.tag_class_int != base.reference.tag_class_int);
        MERGE_ARRAY(ai_animation_references, merge.animation_name != base.animation_name);
        MERGE_ARRAY(ai_script_references, merge.script_name != base.script_name);
        MERGE_ARRAY(ai_recording_references, merge.recording_name != base.recording_name);
        MERGE_ARRAY(references, merge.reference.path != base.reference.path || merge.reference.tag_class_int != base.reference.tag_class_int);
        MERGE_ARRAY(cutscene_flags, merge.name != base.name);
        MERGE_ARRAY(cutscene_camera_points, merge.name != base.name);
        MERGE_ARRAY(cutscene_titles, merge.name != base.name);
        MERGE_ARRAY(source_files, merge.name != base.name);
        MERGE_ARRAY(decal_palette, merge.reference.path != base.reference.path || merge.reference.tag_class_int != base.reference.tag_class_int);
        
        // Merge palettes
        #define MERGE_PALETTE(what) MERGE_ARRAY(what, merge.name.path != base.name.path || merge.name.tag_class_int != base.name.tag_class_int)
        
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
        #define TRANSLATE_PALETTE(what, match_comparison) [&base_scenario, &scenario_to_merge](HEK::Index old_index) -> HEK::Index { \
            /* If we're null, return null */ \
            if(old_index == NULL_INDEX) { \
                return NULL_INDEX; \
            } \
\
            /* if we're out of bounds, fail */ \
            auto old_count = scenario_to_merge.what.size(); \
            if(old_index >= old_count) { \
                eprintf_error(# what " index is out of bounds (%zu >= %zu)", static_cast<std::size_t>(old_index), old_count); \
                throw OutOfBoundsException(); \
            } \
\
            /* Find it */ \
            auto &merge = scenario_to_merge.what[old_index]; \
            auto new_count = base_scenario.what.size(); \
            for(std::size_t name = 0; name < new_count; name++) { \
                auto &base = base_scenario.what[name]; \
                if((match_comparison)) { \
                    if(name >= NULL_INDEX) { \
                        eprintf_error(# what " exceeded %zu when merging", static_cast<std::size_t>(NULL_INDEX - 1)); \
                        throw InvalidTagDataException(); \
                    } \
                    return name; \
                } \
            } \
            eprintf_error("Failed to find an entry in " # what); \
            throw OutOfBoundsException(); \
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
            auto translate_palette = TRANSLATE_PALETTE(what_palette, (merge.name.path == base.name.path && merge.name.tag_class_int == base.name.tag_class_int)); \
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
        auto translate_decal_palette = TRANSLATE_PALETTE(decal_palette, merge.reference.tag_class_int == base.reference.tag_class_int && merge.reference.path == base.reference.path);
        for(auto &decal : scenario_to_merge.decals) {
            // Add our new decal
            auto &new_decal = base_scenario.decals.emplace_back(decal);
            new_decal.decal_type = translate_decal_palette(new_decal.decal_type);
        }
        
        // AI stuff
        auto translate_actor_palette = TRANSLATE_PALETTE(actor_palette, (merge.reference.tag_class_int == base.reference.tag_class_int && merge.reference.path == base.reference.path));
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

    void Scenario::pre_compile(BuildWorkload &workload, std::size_t tag_index, std::size_t struct_index, std::size_t) {
        // Merge child scenarios
        if(this->child_scenarios.size() != 0 && !workload.disable_recursion) {
            // Let's begin by adding this scenario to the list (in case we reference ourself)
            std::vector<std::string> merged_scenarios;
            merged_scenarios.emplace_back(workload.tags[tag_index].path);
            
            // Take the scenario off the top
            while(this->child_scenarios.size()) {
                // Get the scenario
                auto &first_scenario = this->child_scenarios[0].child_scenario;
                if(!first_scenario.path.empty()) {
                    // If this isn't even a scenario tag... what
                    if(first_scenario.tag_class_int != TagClassInt::TAG_CLASS_SCENARIO) {
                        REPORT_ERROR_PRINTF(workload, ERROR_TYPE_FATAL_ERROR, tag_index, "Non-scenario %s.%s referenced in child scenarios", File::halo_path_to_preferred_path(first_scenario.path).c_str(), HEK::tag_class_to_extension(first_scenario.tag_class_int));
                        throw InvalidTagDataException();
                    }
                    
                    // Make sure we haven't done it already
                    for(auto &m : merged_scenarios) {
                        if(m == first_scenario.path) {
                            workload.report_error(BuildWorkload::ErrorType::ERROR_TYPE_FATAL_ERROR, "Duplicate or cyclical child scenario references are present", tag_index);
                            eprintf_warn("First duplicate scenario: %s.%s", File::halo_path_to_preferred_path(first_scenario.path).c_str(), HEK::tag_class_to_extension(first_scenario.tag_class_int));
                            throw InvalidTagDataException();
                        }
                    }
                    
                    // Add it to the list
                    merged_scenarios.emplace_back(first_scenario.path);
                    
                    // Find it
                    char file_path_cstr[1024];
                    std::snprintf(file_path_cstr, sizeof(file_path_cstr), "%s.%s", File::halo_path_to_preferred_path(first_scenario.path).c_str(), HEK::tag_class_to_extension(first_scenario.tag_class_int));
                    auto file_path = File::tag_path_to_file_path(file_path_cstr, *workload.tags_directories, true);
                    if(!file_path.has_value()) {
                        REPORT_ERROR_PRINTF(workload, ERROR_TYPE_FATAL_ERROR, tag_index, "Child scenario %s not found", file_path_cstr);
                        throw InvalidTagDataException();
                    }
                    
                    // Open it
                    const auto *found_path = file_path->c_str();
                    auto data = File::open_file(found_path);
                    if(!data.has_value()) {
                        REPORT_ERROR_PRINTF(workload, ERROR_TYPE_FATAL_ERROR, tag_index, "Failed to open %s", found_path);
                        throw InvalidTagDataException();
                    }
                    
                    // Parse and merge it
                    try {
                        auto scenario = Scenario::parse_hek_tag_file(data->data(), data->size());
                        data.reset(); // clear it
                        merge_child_scenario(*this, scenario);
                    }
                    catch(std::exception &) {
                        REPORT_ERROR_PRINTF(workload, ERROR_TYPE_FATAL_ERROR, tag_index, "Failed to merge %s%s into %s.%s",
                                            File::halo_path_to_preferred_path(first_scenario.path).c_str(),
                                            HEK::tag_class_to_extension(first_scenario.tag_class_int),
                                            workload.tags[tag_index].path.c_str(),
                                            HEK::tag_class_to_extension(workload.tags[tag_index].tag_class_int)
                                           );
                        throw;
                    }
                }
                
                // Delete the scenario
                this->child_scenarios.erase(this->child_scenarios.begin());
            }
        }

        if(workload.disable_recursion) {
            return; // if recursion is disabled, doing any of this will be a meme
        }

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
                        REPORT_ERROR_PRINTF(workload, ERROR_TYPE_WARNING_PEDANTIC, tag_index, object_type_str " palette type #%zu (%s.%s) is unused", i, File::halo_path_to_preferred_path(palette.path).c_str(), HEK::tag_class_to_extension(palette.tag_class_int)); \
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
                std::memset(reinterpret_cast<std::uint8_t *>(&node), 0xCA, sizeof(node));
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
            if(this->source_files.size() == 0) {
                workload.report_error(BuildWorkload::ErrorType::ERROR_TYPE_WARNING, "Scenario tag has script data but no source file data", tag_index);
                eprintf_warn("This is DEPRECATED and will not be allowed in some future version of Invader when tags with scripts are fully supported.");
                eprintf_warn("To fix this, recompile the scripts");
            }
            else {
                // TODO: Recompile scripts
                workload.report_error(BuildWorkload::ErrorType::ERROR_TYPE_WARNING, "TODO: Tags with scripts are not fully supported at this time", tag_index);
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
        this->bsp_switch_trigger_volumes.clear();
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
                bsp_switch_trigger_volume.unknown = 0xFFFF;
            }
        }
    }

    struct BSPData {
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
        const ScenarioStructureBSPLeaf::struct_little *render_leaves = nullptr;
        std::uint32_t render_leaf_count = 0;
    };

    void Scenario::post_compile(BuildWorkload &workload, std::size_t tag_index, std::size_t struct_index, std::size_t struct_offset) {
        if(workload.disable_recursion) {
            return; // if recursion is disabled, doing any of this will be a meme
        }

        auto &scenario_struct = workload.structs[struct_index];
        const auto &scenario_data = *reinterpret_cast<const struct_little *>(scenario_struct.data.data() + struct_offset);

        // Get the bsp data; saves us from having to get it again
        std::vector<BSPData> bsp_data;
        std::size_t bsp_count = this->structure_bsps.size();
        bsp_data.reserve(bsp_count);
        for(auto &b : this->structure_bsps) {
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

        // Determine which BSP light fixtures and scenery are in
        #define FIND_BSP_INDICES_FOR_OBJECT_ARRAY(array_type, objects) { \
            std::size_t object_count = this->objects.size(); \
            if(object_count) { \
                auto &object_struct = workload.structs[*scenario_struct.resolve_pointer(&scenario_data.objects.pointer)]; \
                auto *object_array = reinterpret_cast<array_type::struct_little *>(object_struct.data.data()); \
                for(std::size_t o = 0; o < object_count; o++) { \
                    std::uint32_t bsp_indices = 0; \
                    auto &object = object_array[o]; \
                    if(object.type != NULL_INDEX) { \
                        for(std::size_t b = 0; b < bsp_count; b++) { \
                            /* Check if we're inside this BSP */ \
                            auto &bsp = bsp_data[b]; \
                            if(!leaf_for_point_of_bsp_tree(object.position, bsp.bsp3d_nodes, bsp.bsp3d_node_count, bsp.planes, bsp.plane_count).is_null()) { \
                                bsp_indices |= 1 << b; \
                            } \
                        } \
                    } \
                    /* Set to the result */ \
                    object.bsp_indices = bsp_indices; \
                } \
            } \
        }

        FIND_BSP_INDICES_FOR_OBJECT_ARRAY(ScenarioScenery, scenery);
        FIND_BSP_INDICES_FOR_OBJECT_ARRAY(ScenarioLightFixture, light_fixtures);

        std::size_t bsp_find_warnings = 0;

        // Determine which BSP the encounters fall in
        std::size_t encounter_list_count = this->encounters.size();
        if(encounter_list_count != 0) {
            auto &encounter_struct = workload.structs[*scenario_struct.resolve_pointer(&scenario_data.encounters.pointer)];
            auto *encounter_array = reinterpret_cast<ScenarioEncounter::struct_little *>(encounter_struct.data.data());
            for(std::size_t i = 0; i < encounter_list_count; i++) {
                auto &encounter = this->encounters[i];
                auto &encounter_data = encounter_array[i];

                // Set this to 1 because memes
                encounter_data.one = 1;

                if(encounter.flags.manual_bsp_index_specified) {
                    encounter_data.precomputed_bsp_index = encounter_data.manual_bsp_index;
                    continue;
                }

                std::size_t best_bsp = NULL_INDEX;
                std::size_t best_bsp_hits = 0;
                std::size_t total_best_bsps = 0;
                std::size_t max_hits = 0;

                std::vector<std::pair<HEK::Index, HEK::Index>> firing_positions_indices;
                std::size_t firing_position_count = encounter.firing_positions.size();
                firing_positions_indices.reserve(firing_position_count);

                bool raycast = encounter.flags._3d_firing_positions == 0;

                auto intersects_directly_below = [&bsp_data](HEK::Point3D<HEK::LittleEndian> &position, std::uint32_t &surface_index, float distance, std::size_t bsp_index) -> bool {
                    auto &bsp = bsp_data[bsp_index];
                    auto position_below = position;
                    position_below.z = position_below.z - distance;

                    std::uint32_t leaf_index;
                    HEK::Point3D<HEK::LittleEndian> intersection_point;
                    return check_for_intersection(
                        position, position_below,
                        bsp.bsp3d_nodes,
                        bsp.bsp3d_node_count,
                        bsp.planes,
                        bsp.plane_count,
                        bsp.leaves,
                        bsp.leaf_count,
                        bsp.bsp2d_nodes,
                        bsp.bsp2d_node_count,
                        bsp.bsp2d_references,
                        bsp.bsp2d_reference_count,
                        bsp.surfaces,
                        bsp.surface_count,
                        bsp.edges,
                        bsp.edge_count,
                        bsp.vertices,
                        bsp.vertex_count,
                        intersection_point,
                        surface_index,
                        leaf_index
                    );
                };

                for(std::size_t b = 0; b < bsp_count; b++) {
                    auto &bsp = bsp_data[b];

                    std::size_t hits = 0;
                    std::size_t total_hits = 0;

                    for(auto &s : encounter.squads) {
                        for(auto &p : s.starting_locations) {
                            total_hits++;
                            auto leaf = leaf_for_point_of_bsp_tree(p.position, bsp.bsp3d_nodes, bsp.bsp3d_node_count, bsp.planes, bsp.plane_count);

                            bool in_bsp = !leaf.is_null();
                            if(in_bsp) {
                                // If raycasting check for a surface that is 0.5 world units below it
                                if(raycast) {
                                    std::uint32_t surface_index;
                                    in_bsp = intersects_directly_below(p.position, surface_index, 2.0F, b);
                                }

                                // Add 1 if still in BSP
                                hits += in_bsp;
                            }
                        }
                    }

                    firing_positions_indices.clear();
                    for(auto &f : encounter.firing_positions) {
                        total_hits++;
                        auto leaf = leaf_for_point_of_bsp_tree(f.position, bsp.bsp3d_nodes, bsp.bsp3d_node_count, bsp.planes, bsp.plane_count);

                        bool in_bsp = !leaf.is_null();
                        if(in_bsp) {
                            // If raycasting check for a surface that is 0.5 world units below it
                            std::uint32_t surface_index = NULL_INDEX;
                            if(raycast) {
                                in_bsp = intersects_directly_below(f.position, surface_index, 0.5F, b);
                            }

                            // Add 1 if still in BSP and set cluster index
                            if(in_bsp) {
                                hits++;
                                firing_positions_indices.emplace_back(bsp.render_leaves[leaf.int_value()].cluster, static_cast<HEK::Index>(surface_index));
                            }
                            else {
                                firing_positions_indices.emplace_back(NULL_INDEX, NULL_INDEX);
                            }
                        }
                    }

                    // If this is the next best BSP, write data
                    if(hits > best_bsp_hits) {
                        best_bsp_hits = hits;
                        best_bsp = b;
                        total_best_bsps = 1;
                        max_hits = total_hits;
                    }
                    else if(hits == best_bsp_hits) {
                        total_best_bsps++;
                    }
                }

                encounter_data.precomputed_bsp_index = static_cast<HEK::Index>(best_bsp);

                if(best_bsp_hits == 0) {
                    REPORT_ERROR_PRINTF(workload, ERROR_TYPE_WARNING, tag_index, "Encounter #%zu (%s) was found in 0 BSPs", i, encounter.name.string);
                    bsp_find_warnings++;
                }
                else if(best_bsp_hits != max_hits) {
                    REPORT_ERROR_PRINTF(workload, ERROR_TYPE_WARNING, tag_index, "Encounter #%zu (%s) is partially outside of BSP#%zu (%zu / %zu hits)", i, encounter.name.string, best_bsp, best_bsp_hits, max_hits);
                    bsp_find_warnings++;
                }
                else if(total_best_bsps > 1) {
                    REPORT_ERROR_PRINTF(workload, ERROR_TYPE_WARNING, tag_index, "Encounter #%zu (%s) was found in %zu BSP%s (will place in BSP #%zu)", i, encounter.name.string, total_best_bsps, total_best_bsps == 1 ? "" : "s", best_bsp);
                    bsp_find_warnings++;
                }

                // If we have a BSP, set all the cluster and surface indices
                if(encounter_data.precomputed_bsp_index != NULL_INDEX && firing_position_count > 0) {
                    auto &firing_positions_struct = workload.structs[*encounter_struct.resolve_pointer(&encounter_data.firing_positions.pointer)];
                    auto *firing_positions_data = reinterpret_cast<ScenarioFiringPosition::struct_little *>(firing_positions_struct.data.data());

                    auto &bsp = bsp_data[best_bsp];
                    for(std::size_t fp = 0; fp < firing_position_count; fp++) {
                        // Get leaf and surface index
                        auto &f = firing_positions_data[fp];
                        auto leaf = leaf_for_point_of_bsp_tree(f.position, bsp.bsp3d_nodes, bsp.bsp3d_node_count, bsp.planes, bsp.plane_count);
                        if(!leaf.is_null()) {
                            f.cluster_index = bsp.render_leaves[leaf.int_value()].cluster;
                            std::uint32_t surface_index = NULL_INDEX;
                            if(raycast) {
                                intersects_directly_below(f.position, surface_index, 0.5F, best_bsp);
                            }
                            f.surface_index = surface_index;
                        }
                    }
                }
            }
        }

        // Determine which BSP the command lists fall in
        std::size_t command_list_count = this->command_lists.size();
        if(command_list_count != 0) {
            auto *command_list_array = reinterpret_cast<ScenarioCommandList::struct_little *>(workload.structs[*scenario_struct.resolve_pointer(&scenario_data.command_lists.pointer)].data.data());
            for(std::size_t i = 0; i < command_list_count; i++) {
                auto &command_list = this->command_lists[i];
                auto &command_list_data = command_list_array[i];

                // If manually specifying a BSP, don't bother checking anything
                if(command_list.flags.manual_bsp_index) {
                    command_list_data.precomputed_bsp_index = command_list.manual_bsp_index;
                    continue;
                }

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
                for(std::size_t b = 0; b < bsp_count; b++) {
                    auto &bsp = bsp_data[b];

                    std::size_t hits = 0;
                    std::size_t total_hits = 0;

                    // Basically, add 1 for every time we find it in here
                    for(auto &p : command_list.points) {
                        total_hits++;
                        hits += !leaf_for_point_of_bsp_tree(p.position, bsp.bsp3d_nodes, bsp.bsp3d_node_count, bsp.planes, bsp.plane_count).is_null();
                    }

                    if(hits > best_bsp_hits) {
                        best_bsp_hits = hits;
                        best_bsp = b;
                        total_best_bsps = 1;
                        max_hits = total_hits;
                    }
                    else if(hits == best_bsp_hits) {
                        total_best_bsps++;
                    }
                }

                command_list_data.precomputed_bsp_index = static_cast<HEK::Index>(best_bsp);
                if(best_bsp_hits == 0) {
                    REPORT_ERROR_PRINTF(workload, ERROR_TYPE_WARNING, tag_index, "Command list #%zu (%s) was found in 0 BSPs", i, command_list.name.string);
                    bsp_find_warnings++;
                }
                else if(best_bsp_hits != max_hits) {
                    REPORT_ERROR_PRINTF(workload, ERROR_TYPE_WARNING, tag_index, "Command list #%zu (%s) is partially outside of BSP#%zu (%zu / %zu hits)", i, command_list.name.string, best_bsp, best_bsp_hits, max_hits);
                    bsp_find_warnings++;
                }
                else if(total_best_bsps > 1) {
                    REPORT_ERROR_PRINTF(workload, ERROR_TYPE_WARNING, tag_index, "Command list #%zu (%s) was found in %zu BSP%s (will place in BSP #%zu)", i, command_list.name.string, total_best_bsps, total_best_bsps == 1 ? "" : "s", best_bsp);
                    bsp_find_warnings++;
                }
            }
        }

        if(bsp_find_warnings == 1) {
            eprintf_warn("Use manual BSP indices to silence this warning");
        }
        else if(bsp_find_warnings > 1) {
            eprintf_warn("Use manual BSP indices to silence these warnings");
        }

        // Decals
        std::size_t decal_count = decals.size();
        if(decal_count > 0) {
            for(std::size_t bsp = 0; bsp < this->structure_bsps.size(); bsp++) {
                auto &b = this->structure_bsps[bsp];
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
                        auto &decal = this->decals[d];
                        auto &bd = bsp_data[bsp];
                        auto leaf = leaf_for_point_of_bsp_tree(decal.position, bd.bsp3d_nodes, bd.bsp3d_node_count, bd.planes, bd.plane_count);
                        if(leaf.is_null()) {
                            continue;
                        }
                        cluster_decals.emplace_back(bd.render_leaves[leaf.int_value()].cluster, d);
                    }

                    // Get clusters
                    std::vector<ScenarioStructureBSPRuntimeDecal::struct_little> runtime_decals;

                    for(std::size_t c = 0; c < bsp_cluster_count; c++) {
                        auto &cluster = clusters[c];

                        // Put stuff together
                        std::size_t first_decal = runtime_decals.size();
                        for(auto &cd : cluster_decals) {
                            if(cd.first == c) {
                                auto &decal = this->decals[cd.second];
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
    
    bool fix_missing_script_source_data(Scenario &scenario, bool fix) {
        if(scenario.source_files.size() == 0) {
            return false;
        }
        
        if(fix) {
            try {
                if(scenario.scripts.size() != 0) {
                    auto script = Tokenizer::detokenize(ScriptTree::decompile_script_tree(Compiler::decompile_scenario(scenario)));
                    const auto *script_data = reinterpret_cast<const std::byte *>(script.c_str());
                    auto &source_file = scenario.source_files.emplace_back();
                    std::snprintf(source_file.name.string, sizeof(source_file.name.string), "extracted");
                    source_file.source = std::vector<std::byte>(script_data, script_data + script.size());
                }
            }
            catch(std::exception &e) {
                eprintf_error("Failed to decompile scripts; scenario will not have any source data: %s", e.what());
                scenario.source_files.clear();
                return false;
            }
        }
        return true;
    }

    void Scenario::post_cache_deformat() {
        auto *script_data = this->script_syntax_data.data();
        auto script_data_size = this->script_syntax_data.size();

        // If we don't have a script node table, give up
        if(script_data_size < sizeof(ScenarioScriptNodeTable::struct_little)) {
            eprintf_error("scenario tag has an invalid scenario script node table");
            throw InvalidTagDataException();
        }

        // Copy the table header
        ScenarioScriptNodeTable::struct_big table = *reinterpret_cast<ScenarioScriptNodeTable::struct_little *>(script_data);
        *reinterpret_cast<ScenarioScriptNodeTable::struct_big *>(script_data) = table;

        // Make sure it's not bullshit
        auto *script_nodes = reinterpret_cast<ScenarioScriptNode::struct_little *>(script_data + sizeof(table));
        auto table_size = table.maximum_count.read();
        std::size_t expected_size = (reinterpret_cast<std::byte *>(script_nodes + table_size) - script_data);
        if(expected_size != script_data_size) {
            eprintf_error("scenario tag has an invalid scenario script node table (%zu vs %zu)", expected_size, script_data_size);
            throw InvalidTagDataException();
        }

        // Copy the rest of the table
        for(std::size_t i = 0; i < table_size; i++) {
            ScenarioScriptNode::struct_big big = script_nodes[i];
            *reinterpret_cast<ScenarioScriptNode::struct_big *>(script_nodes + i) = big;
        }

        // Put scripts in if need be
        fix_missing_script_source_data(*this, true);

        // And lastly, for consistency sake, remove all tag IDs and zero out the pointer
        this->postprocess_hek_data();
    }

    void ScenarioCutsceneTitle::pre_compile(BuildWorkload &, std::size_t, std::size_t, std::size_t) {
        this->fade_in_time *= TICK_RATE;
        this->fade_out_time *= TICK_RATE;
        this->up_time *= TICK_RATE;
    }
}
