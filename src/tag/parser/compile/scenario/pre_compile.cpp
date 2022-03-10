// SPDX-License-Identifier: GPL-3.0-only

#include <map>
#include <cassert>
#include <invader/tag/parser/parser.hpp>
#include <invader/build/build_workload.hpp>
#include <invader/file/file.hpp>
#include <invader/tag/parser/compile/scenario.hpp>

#include <riat/riat.hpp>

namespace Invader::Parser {
    static void merge_child_scenarios(BuildWorkload &workload, std::size_t tag_index, Scenario &scenario);
    static void check_palettes(BuildWorkload &workload, std::size_t tag_index, Scenario &scenario);
    static void fix_script_data(BuildWorkload &workload, std::size_t tag_index, std::size_t struct_index, Scenario &scenario);
    static void fix_bsp_transitions(BuildWorkload &workload, std::size_t tag_index, Scenario &scenario);
    
    static constexpr const HEK::TagFourCC OBJECT_FOURCCS[] = { TagFourCC::TAG_FOURCC_BIPED, 
                                                               TagFourCC::TAG_FOURCC_VEHICLE,
                                                               TagFourCC::TAG_FOURCC_WEAPON,
                                                               TagFourCC::TAG_FOURCC_EQUIPMENT,
                                                               TagFourCC::TAG_FOURCC_GARBAGE,
                                                               TagFourCC::TAG_FOURCC_SCENERY,
                                                               TagFourCC::TAG_FOURCC_PLACEHOLDER,
                                                               TagFourCC::TAG_FOURCC_SOUND_SCENERY,
                                                               TagFourCC::TAG_FOURCC_DEVICE_CONTROL,
                                                               TagFourCC::TAG_FOURCC_DEVICE_MACHINE,
                                                               TagFourCC::TAG_FOURCC_DEVICE_LIGHT_FIXTURE };
    
    template<typename T> static std::optional<HEK::TagFourCC> script_value_type_to_fourcc(T type) {
        switch(static_cast<RIAT_ValueType>(type)) {
            case RIAT_ValueType::RIAT_VALUE_TYPE_SOUND:
                return HEK::TAG_FOURCC_SOUND;

            case RIAT_ValueType::RIAT_VALUE_TYPE_EFFECT:
                return HEK::TAG_FOURCC_EFFECT;

            case RIAT_ValueType::RIAT_VALUE_TYPE_DAMAGE:
                return HEK::TAG_FOURCC_DAMAGE_EFFECT;

            case RIAT_ValueType::RIAT_VALUE_TYPE_LOOPING_SOUND:
                return HEK::TAG_FOURCC_SOUND_LOOPING;

            case RIAT_ValueType::RIAT_VALUE_TYPE_ANIMATION_GRAPH:
                return HEK::TAG_FOURCC_MODEL_ANIMATIONS;

            case RIAT_ValueType::RIAT_VALUE_TYPE_ACTOR_VARIANT:
                return HEK::TAG_FOURCC_ACTOR_VARIANT;

            case RIAT_ValueType::RIAT_VALUE_TYPE_DAMAGE_EFFECT:
                return HEK::TAG_FOURCC_DAMAGE_EFFECT;

            case RIAT_ValueType::RIAT_VALUE_TYPE_OBJECT_DEFINITION:
                return HEK::TAG_FOURCC_OBJECT;

            default:
                return std::nullopt;
        }
    }
    
    void compile_scripts(Scenario &scenario, const HEK::GameEngineInfo &info, RIAT_OptimizationLevel optimization_level, std::vector<std::string> &warnings, const std::vector<std::filesystem::path> &tags_directories, const std::optional<std::vector<std::pair<std::string, std::vector<std::byte>>>> &script_source) {
        // Instantiate it
        RIAT::Instance instance;
        instance.set_compile_target(info.scenario_script_compile_target);
        instance.set_optimization_level(optimization_level);
        instance.set_user_data(&warnings);
        
        // Open the hud message text
        Parser::HUDMessageText hmt;
        if(!scenario.hud_messages.path.empty()) {
            auto file_path = File::tag_path_to_file_path(File::halo_path_to_preferred_path(scenario.hud_messages.path) + ".hud_message_text", tags_directories);
            if(file_path.has_value()) {
                auto hud_message_text_data = File::open_file(*file_path);
                if(!hud_message_text_data.has_value()) {
                    eprintf_error("Failed to open %s\n", file_path->string().c_str());
                    throw std::exception();
                }
                hmt = Parser::HUDMessageText::parse_hek_tag_file(hud_message_text_data->data(), hud_message_text_data->size());
            }
        }
        
        // Eventually get the HUD globals tag
        Parser::HUDGlobals hud_globals;
        auto globals_file_path = File::tag_path_to_file_path(File::halo_path_to_preferred_path("globals\\globals.globals"), tags_directories);
        if(globals_file_path.has_value()) {
            auto globals_data = File::open_file(*globals_file_path);
            if(!globals_data.has_value()) {
                eprintf_error("Failed to open %s\n", globals_file_path->string().c_str());
                throw std::exception();
            }
            
            auto globals = Globals::parse_hek_tag_file(globals_data->data(), globals_data->size());
            if(!globals.interface_bitmaps.empty()) {
                auto &interface_bitmaps = globals.interface_bitmaps[0];
                if(!interface_bitmaps.hud_globals.path.empty()) {
                    auto file_path = File::tag_path_to_file_path(File::halo_path_to_preferred_path(interface_bitmaps.hud_globals.path) + ".hud_globals", tags_directories);
                    if(file_path.has_value()) {
                        auto hud_globals_data = File::open_file(*file_path);
                        if(!hud_globals_data.has_value()) {
                            eprintf_error("Failed to open %s\n", file_path->string().c_str());
                            throw std::exception();
                        }
                        hud_globals = Parser::HUDGlobals::parse_hek_tag_file(hud_globals_data->data(), hud_globals_data->size());
                    }
                }
            }
        }
    
        // Any warnings get eaten up here
        instance.set_warn_callback([](RIAT_Instance *instance, const char *message, const char *file, std::size_t line, std::size_t column) {
            char fmt_message[512];
            std::snprintf(fmt_message, sizeof(fmt_message), "%s.hsc:%zu:%zu: warning: %s", file, line, column, message);
            reinterpret_cast<std::vector<std::string> *>(riat_instance_get_user_data(instance))->emplace_back(fmt_message);
        });
        
        // Load the input from script_source
        decltype(scenario.source_files) source_files;
        if(script_source.has_value()) {
            for(auto &source : *script_source) {
                auto &file = source_files.emplace_back();
                
                // Check if it's too long. If not, copy. Otherwise, error
                if(source.first.size() > sizeof(file.name.string) - 1) {
                    eprintf_error("Script file name '%s' is too long", source.first.c_str());
                    throw std::exception();
                }
                std::strncpy(file.name.string, source.first.c_str(), sizeof(file.name.string) - 1);

                // Set it
                file.source = source.second;
            };
        }
        
        // Use the scenario tag's source data
        else {
            source_files = scenario.source_files;
        }
        
        // Load the scripts
        try {
            for(auto &source : source_files) {
                instance.load_script_source(reinterpret_cast<const char *>(source.source.data()), source.source.size(), std::string(source.name.string).c_str());
            }
            instance.compile_scripts();
        }
        catch(std::exception &e) {
            eprintf_error("Script compilation error: %s", e.what());
            throw InvalidTagDataException();
        }
        
        std::size_t node_limit = info.maximum_scenario_script_nodes;
        
        auto scripts = instance.get_scripts();
        auto globals = instance.get_globals();
        auto nodes = instance.get_nodes();
        
        std::size_t node_count = nodes.size();
        
        if(nodes.size() > node_limit) {
            eprintf_error("Node limit exceeded for the target engine (%zu > %zu)", node_count, node_limit);
            throw InvalidTagDataException();
        }
        
        std::vector<Invader::Parser::ScenarioScriptNode> into_nodes;
        
        auto format_index_to_id = [](std::size_t index) -> std::uint32_t {
            auto index_16_bit = static_cast<std::uint16_t>(index);
            return static_cast<std::uint32_t>(((index_16_bit + 0x6373) | 0x8000) << 16) | index_16_bit;
        };
        
        std::map<std::string, std::size_t> string_index;
        std::vector<std::byte> string_data;
        
        for(std::size_t node_index = 0; node_index < node_count; node_index++) {
            auto &n = nodes[node_index];
            auto &new_node = into_nodes.emplace_back();
            new_node = {};
            
            // Set the salt
            new_node.salt = format_index_to_id(node_index) >> 16;
            
            // Set to 0xFFFFFFFF
            new_node.data.long_int = -1;
            
            // If we have string data, add it
            if(n.string_data != NULL) {
                std::string str = n.string_data;
                if(!string_index.contains(str)) {
                    string_index[str] = string_data.size();
                    const auto *cstr = str.c_str();
                    string_data.insert(string_data.end(), reinterpret_cast<const std::byte *>(cstr), reinterpret_cast<const std::byte *>(cstr) + str.size());
                    string_data.emplace_back(std::byte());
                }
                new_node.string_offset = string_index[str];
            }
            
            // All nodes are marked with this...?
            new_node.flags |= Invader::HEK::ScenarioScriptNodeFlagsFlag::SCENARIO_SCRIPT_NODE_FLAGS_FLAG_IS_GARBAGE_COLLECTABLE;
            
            // Here's the type
            new_node.type = static_cast<Invader::HEK::ScenarioScriptValueType>(n.type);
            new_node.index_union = new_node.type;
            
            // Set this stuff
            if(n.is_primitive) {
                new_node.flags |= Invader::HEK::ScenarioScriptNodeFlagsFlag::SCENARIO_SCRIPT_NODE_FLAGS_FLAG_IS_PRIMITIVE;
                if(n.is_global) {
                    new_node.flags |= Invader::HEK::ScenarioScriptNodeFlagsFlag::SCENARIO_SCRIPT_NODE_FLAGS_FLAG_IS_GLOBAL;
                }
                else {
                    switch(n.type) {
                        case RIAT_ValueType::RIAT_VALUE_TYPE_BOOLEAN:
                            new_node.data.bool_int = n.bool_int;
                            break;
                        case RIAT_ValueType::RIAT_VALUE_TYPE_SCRIPT:
                        case RIAT_ValueType::RIAT_VALUE_TYPE_SHORT:
                        case RIAT_ValueType::RIAT_VALUE_TYPE_TEAM:
                        case RIAT_ValueType::RIAT_VALUE_TYPE_GAME_DIFFICULTY:
                            new_node.data.short_int = n.short_int;
                            break;
                        case RIAT_ValueType::RIAT_VALUE_TYPE_LONG:
                            new_node.data.long_int = n.long_int;
                            break;
                        case RIAT_ValueType::RIAT_VALUE_TYPE_REAL:
                            new_node.data.real = n.real;
                            break;
                        default:
                            break;
                    }
                }
            }
            else {
                new_node.data.tag_id.id = format_index_to_id(n.child_node);
                
                if(n.is_script_call) {
                    new_node.flags |= Invader::HEK::ScenarioScriptNodeFlagsFlag::SCENARIO_SCRIPT_NODE_FLAGS_FLAG_IS_SCRIPT_CALL;
                    new_node.index_union = n.call_index;
                }
            }
            
            // Set the next node?
            if(n.next_node == SIZE_MAX) {
                new_node.next_node = UINT32_MAX;
            }
            else {
                new_node.next_node = format_index_to_id(n.next_node);
            }
            
            // Get the index of the thing
            auto find_thing = [&n, &warnings, &source_files, &new_node](auto &array, const char *name) -> std::size_t {
                if(std::strcmp(name, "none") == 0) {
                    return SIZE_MAX;
                }
                
                auto len = array.size();
                bool exists = false;
                bool multiple_instances = false;
                std::size_t first_instance = 0;
                
                // See if it exists and then find the first multiple instance if it does
                for(std::size_t i = 0; i < len && !multiple_instances; i++) {
                    const char *c = name;
                    const char *d = array[i].name.string;
                    
                    while(*c != 0 && *d != 0 && std::tolower(*c) == std::tolower(*d)) {
                        c++;
                        d++;
                    }
                    
                    if(std::tolower(*c) == std::tolower(*d)) {
                        if(exists) {
                            multiple_instances = true;
                            break;
                        }
                        
                        first_instance = i;
                        exists = true;
                    }
                }
                
                if(!exists) {
                    throw std::exception();
                }
                
                if(multiple_instances) {
                    char warning[512];
                    std::snprintf(warning, sizeof(warning), "%s.hsc:%zu:%zu: warning: multiple instances of %s '%s' found (first instance is %zu)", source_files[n.file].name.string, n.line, n.column, HEK::ScenarioScriptValueType_to_string_pretty(new_node.type), name, first_instance);
                    warnings.emplace_back(warning);
                }
                
                return first_instance;
            };
            
            // Make sure the thing it refers to exists. If so, save the index.
            try {
                if(n.is_primitive && !n.is_global) {
                    switch(new_node.type) {
                        case HEK::ScenarioScriptValueType::SCENARIO_SCRIPT_VALUE_TYPE_CUTSCENE_RECORDING:
                            new_node.data.long_int = find_thing(scenario.recorded_animations, n.string_data);
                            break;
                            
                        case HEK::ScenarioScriptValueType::SCENARIO_SCRIPT_VALUE_TYPE_AI_COMMAND_LIST:
                            new_node.data.long_int = find_thing(scenario.command_lists, n.string_data);
                            break;
                            
                        case HEK::ScenarioScriptValueType::SCENARIO_SCRIPT_VALUE_TYPE_CONVERSATION:
                            new_node.data.long_int = find_thing(scenario.ai_conversations, n.string_data);
                            break;
                            
                        case HEK::ScenarioScriptValueType::SCENARIO_SCRIPT_VALUE_TYPE_DEVICE_GROUP:
                            new_node.data.long_int = find_thing(scenario.device_groups, n.string_data);
                            break;
                            
                        case HEK::ScenarioScriptValueType::SCENARIO_SCRIPT_VALUE_TYPE_TRIGGER_VOLUME:
                            new_node.data.long_int = find_thing(scenario.trigger_volumes, n.string_data);
                            break;
                            
                        case HEK::ScenarioScriptValueType::SCENARIO_SCRIPT_VALUE_TYPE_AI: {
                            const char *sub_encounter = nullptr;
                            
                            // AI can be a encounter/squad or encounter/platoon or just encounter
                            for(const char *c = n.string_data; *c != 0; c++) {
                                if(*c == '/') {
                                    sub_encounter = c + 1;
                                    break;
                                }
                            }
                            
                            /*
                             * XX XX XXXX
                             * ^  ^  ^
                             * |  |  encounter
                             * |  squad (or 00 if no squad)
                             * 80 if encounter/squad, 40 if encounter/platoon. else 00 if just encounter
                             */
                            if(sub_encounter != nullptr) {
                                std::string encounter_name(n.string_data, sub_encounter - 1 - n.string_data);
                                
                                std::size_t encounter_index = find_thing(scenario.encounters, encounter_name.c_str());
                                auto &encounter = scenario.encounters[encounter_index];
                                
                                std::size_t sub_index;
                                std::uint32_t bitfield_base;
                                
                                try {
                                    sub_index = find_thing(encounter.squads, sub_encounter);
                                    bitfield_base = 0x80000000;
                                }
                                catch(std::exception &) {
                                    try {
                                        sub_index = find_thing(encounter.platoons, sub_encounter);
                                        bitfield_base = 0x40000000;
                                    }
                                    catch(std::exception &) {
                                        throw;
                                    }
                                }
                                
                                new_node.data.long_int = static_cast<std::int32_t>(bitfield_base | ((sub_index & 0xFF) << 16) | (encounter_index & 0xFFFF));
                            }
                            else {
                                new_node.data.long_int = find_thing(scenario.encounters, n.string_data);
                            }
                            
                            break;
                        }
                            
                        case HEK::ScenarioScriptValueType::SCENARIO_SCRIPT_VALUE_TYPE_CUTSCENE_TITLE:
                            new_node.data.long_int = find_thing(scenario.cutscene_titles, n.string_data);
                            break;
                        case HEK::ScenarioScriptValueType::SCENARIO_SCRIPT_VALUE_TYPE_CUTSCENE_FLAG:
                            new_node.data.long_int = find_thing(scenario.cutscene_flags, n.string_data);
                            break;
                        case HEK::ScenarioScriptValueType::SCENARIO_SCRIPT_VALUE_TYPE_CUTSCENE_CAMERA_POINT:
                            new_node.data.long_int = find_thing(scenario.cutscene_camera_points, n.string_data);
                            break;
                        case HEK::ScenarioScriptValueType::SCENARIO_SCRIPT_VALUE_TYPE_HUD_MESSAGE:
                            new_node.data.long_int = find_thing(hmt.messages, n.string_data);
                            break;
                        case HEK::ScenarioScriptValueType::SCENARIO_SCRIPT_VALUE_TYPE_NAVPOINT:
                            new_node.data.long_int = find_thing(hud_globals.waypoint_arrows, n.string_data);
                            break;
                            
                        case HEK::ScenarioScriptValueType::SCENARIO_SCRIPT_VALUE_TYPE_OBJECT_NAME:
                        case HEK::ScenarioScriptValueType::SCENARIO_SCRIPT_VALUE_TYPE_OBJECT_LIST:
                        case HEK::ScenarioScriptValueType::SCENARIO_SCRIPT_VALUE_TYPE_OBJECT:
                        case HEK::ScenarioScriptValueType::SCENARIO_SCRIPT_VALUE_TYPE_UNIT:
                        case HEK::ScenarioScriptValueType::SCENARIO_SCRIPT_VALUE_TYPE_VEHICLE:
                        case HEK::ScenarioScriptValueType::SCENARIO_SCRIPT_VALUE_TYPE_WEAPON:
                        case HEK::ScenarioScriptValueType::SCENARIO_SCRIPT_VALUE_TYPE_DEVICE:
                        case HEK::ScenarioScriptValueType::SCENARIO_SCRIPT_VALUE_TYPE_SCENERY:
                        case HEK::ScenarioScriptValueType::SCENARIO_SCRIPT_VALUE_TYPE_UNIT_NAME:
                        case HEK::ScenarioScriptValueType::SCENARIO_SCRIPT_VALUE_TYPE_VEHICLE_NAME:
                        case HEK::ScenarioScriptValueType::SCENARIO_SCRIPT_VALUE_TYPE_WEAPON_NAME:
                        case HEK::ScenarioScriptValueType::SCENARIO_SCRIPT_VALUE_TYPE_DEVICE_NAME:
                        case HEK::ScenarioScriptValueType::SCENARIO_SCRIPT_VALUE_TYPE_SCENERY_NAME: {
                            auto index = find_thing(scenario.object_names, n.string_data);
                            new_node.data.short_int = index;
                            
                            if(index != SIZE_MAX) {
                                bool something_corresponds_to_this = false;
                                
                                auto check_thing = [&something_corresponds_to_this, &index](auto &what) {
                                    if(something_corresponds_to_this) {
                                        return;
                                    }
                                    for(auto &i : what) {
                                        if(i.name == index) {
                                            something_corresponds_to_this = true;
                                        }
                                    }
                                };
                                
                                check_thing(scenario.bipeds);
                                check_thing(scenario.vehicles);
                                check_thing(scenario.scenery);
                                check_thing(scenario.weapons);
                                check_thing(scenario.equipment);
                                check_thing(scenario.machines);
                                check_thing(scenario.controls);
                                check_thing(scenario.light_fixtures);
                                check_thing(scenario.sound_scenery);
                                
                                if(!something_corresponds_to_this) {
                                    for(auto &c : scenario.ai_conversations) {
                                        for(auto &p : c.participants) {
                                            if(p.set_new_name == index) {
                                                something_corresponds_to_this = true;
                                                goto do_object_index_check_now;
                                            }
                                        }
                                    }
                                }
                                
                                do_object_index_check_now:
                                if(!something_corresponds_to_this) {
                                    eprintf_error("%s.hsc:%zu:%zu: error: '%s' is an object name that does not correspond to any object or AI conversation and is thus invalid for use in scripts", source_files[n.file].name.string, n.line, n.column, n.string_data);
                                    throw InvalidTagDataException();
                                }
                            }
                            
                            break;
                        }
                        
                        default:
                            break;
                    }
                }
            }
            catch (InvalidTagDataException &) {
                throw;
            }
            catch(std::exception &) {
                eprintf_error("%s.hsc:%zu:%zu: error: can't find %s '%s'", source_files[n.file].name.string, n.line, n.column, HEK::ScenarioScriptValueType_to_string_pretty(new_node.type), n.string_data);
                throw InvalidTagDataException();
            }
        }
        
        using node_table_header_tag_fmt = Invader::Parser::ScenarioScriptNodeTable::struct_big;
        using node_tag_fmt = std::remove_reference<decltype(*into_nodes.data())>::type::struct_big;
        
        // Initialize the syntax data and write to it
        std::vector<std::byte> syntax_data(sizeof(node_table_header_tag_fmt) + node_limit * sizeof(node_tag_fmt));
        auto &table_output = *reinterpret_cast<node_table_header_tag_fmt *>(syntax_data.data());
        auto *node_output = reinterpret_cast<node_tag_fmt *>(&table_output + 1);
        table_output.count = node_count;
        table_output.size = node_count;
        table_output.maximum_count = node_limit;
        table_output.next_id = format_index_to_id(node_count) >> 16;
        table_output.element_size = sizeof(node_tag_fmt);
        table_output.data = 0x64407440;
        std::strncpy(table_output.name.string, "script node", sizeof(table_output.name.string));
        table_output.one = 1;
        for(std::size_t node_index = 0; node_index < node_count; node_index++) {
            auto output = into_nodes[node_index].generate_hek_tag_data();
            assert(sizeof(node_output[node_index]) == output.size());
            std::memcpy(&node_output[node_index], output.data(), output.size());
        }
        
        std::size_t script_count = scripts.size();
        std::size_t global_count = globals.size();
        
        // Set up scripts
        decltype(scenario.scripts) new_scripts;
        new_scripts.resize(script_count);
        for(std::size_t s = 0; s < script_count; s++) {
            auto &new_script = new_scripts[s];
            const auto &cmp_script = scripts[s];
            
            static_assert(sizeof(new_script.name.string) == sizeof(cmp_script.name));
            memcpy(new_script.name.string, cmp_script.name, sizeof(cmp_script.name));
            
            new_script.return_type = static_cast<decltype(new_script.return_type)>(cmp_script.return_type);
            new_script.script_type = static_cast<decltype(new_script.script_type)>(cmp_script.script_type);
            new_script.root_expression_index = format_index_to_id(cmp_script.first_node);
        }
        
        
        // Set up references
        std::list<std::pair<File::TagFilePath, std::size_t>> new_references_array;
        for(std::size_t n = 0; n < node_count; n++) {
            auto &node = nodes[n];
            
            // Skip non-primitives/globals
            if(!node.is_primitive || node.is_global) {
                continue;
            }
            
            // Check if we know the group
            auto group = script_value_type_to_fourcc(node.type);
            if(!group.has_value()) {
                continue;
            }
            
            // Add it if we don't have it
            auto new_path = File::TagFilePath(File::halo_path_to_preferred_path(node.string_data), *group);
            bool is_found = false;
            for(auto &i : new_references_array) {
                if(i.first == new_path) {
                    is_found = true;
                    break;
                }
            }
            if(!is_found) {
                new_references_array.emplace_back(new_path, n);
            }
        }
        
        // Resolve object references
        decltype(scenario.references) new_references;
        new_references.reserve(new_references_array.size());
        for(auto &r : new_references_array) {
            auto &[path, node_index] = r;
            auto &n = nodes[node_index];
            
            bool resolved = false;
            
            if(!resolved) {
                try {
                    auto resolve_maybe = [&tags_directories, &r]() -> bool {
                        return File::tag_path_to_file_path(r.first, tags_directories).has_value();
                    };
                    auto resolve_with_fourcc_maybe = [&resolve_maybe, &r](HEK::TagFourCC fourcc) -> bool {
                        r.first.fourcc = fourcc;
                        return resolve_maybe();
                    };
                    
                    if(path.fourcc == HEK::TAG_FOURCC_OBJECT) {
                        for(auto &g : OBJECT_FOURCCS) {
                            if((resolved = resolve_with_fourcc_maybe(g))) {
                                break;
                            }
                        }
                        if(!resolved) {
                            path.fourcc = HEK::TAG_FOURCC_OBJECT;
                        }
                    }
                    else {
                        resolved = resolve_maybe();
                    }
                }
                catch(std::exception &) {}
            }
            
            // See if it has the tag group explicitly mentioned
            if(!resolved) {
                auto tfp_maybe = File::split_tag_class_extension(r.first.path);
                if(tfp_maybe.has_value()) {
                    auto &tfp = *tfp_maybe;
                    
                    // Make sure the extension makes sense
                    bool fourcc_matches = false;
                    if(tfp.fourcc != path.fourcc) {
                        if(path.fourcc == HEK::TAG_FOURCC_OBJECT) {
                            for(auto &g : OBJECT_FOURCCS) {
                                if((fourcc_matches = (g == tfp.fourcc))) {
                                    break;
                                }
                            }
                        }
                    }
                    else {
                        fourcc_matches = true;
                    }
                    
                    // Warn if so, but add it
                    if(fourcc_matches) {
                        if((resolved = File::tag_path_to_file_path(tfp, tags_directories).has_value())) {
                            char w[1024];
                            std::snprintf(w, sizeof(w), "%s.hsc:%zu:%zu: warning: using tag paths with explicit groups is a Halo 2 extension and may not work with stock tools or future release of Invader", source_files[n.file].name.string, n.line, n.column);
                            warnings.emplace_back(w);
                            path = tfp;
                        }
                    }
                }
            }
            
            if(!resolved) {
                eprintf_error("%s.hsc:%zu:%zu: error: can't find %s tag \"%s\"", source_files[n.file].name.string, n.line, n.column, HEK::tag_fourcc_to_extension(path.fourcc), path.path.c_str());
                throw InvalidTagDataException();
            }
            
            // Add it
            auto &ref = new_references.emplace_back();
            ref.reference.path = File::preferred_path_to_halo_path(path.path);
            ref.reference.tag_fourcc = path.fourcc;
        }
        
        // Set up globals
        decltype(scenario.globals) new_globals;
        new_globals.resize(global_count);
        for(std::size_t g = 0; g < global_count; g++) {
            auto &new_global = new_globals[g];
            const auto &cmp_global = globals[g];
            
            static_assert(sizeof(new_global.name.string) == sizeof(cmp_global.name));
            memcpy(new_global.name.string, cmp_global.name, sizeof(cmp_global.name));
            
            new_global.type = static_cast<decltype(new_global.type)>(cmp_global.value_type);
            new_global.initialization_expression_index = format_index_to_id(cmp_global.first_node);
        }
        
        string_data.resize(string_data.size() + 1024);
        
        // Clear out the script data
        scenario.scripts = std::move(new_scripts);
        scenario.globals = std::move(new_globals);
        scenario.source_files = std::move(source_files);
        scenario.script_string_data = std::move(string_data);
        scenario.script_syntax_data = std::move(syntax_data);
        scenario.references = std::move(new_references);
    }
    
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
        
        // Ensure we have the necessary things for singleplayer/multiplayer
        switch(this->type) {
            case HEK::ScenarioType::SCENARIO_TYPE_SINGLEPLAYER: {
                bool spawn_present = false;
                for(auto &s : this->player_starting_locations) {
                    if(s.type_0 == HEK::ScenarioSpawnType::SCENARIO_SPAWN_TYPE_NONE && s.type_1 == HEK::ScenarioSpawnType::SCENARIO_SPAWN_TYPE_NONE && s.type_2 == HEK::ScenarioSpawnType::SCENARIO_SPAWN_TYPE_NONE && s.type_3 == HEK::ScenarioSpawnType::SCENARIO_SPAWN_TYPE_NONE) {
                        spawn_present = true;
                        break;
                    }
                }
                if(!spawn_present) {
                    REPORT_ERROR_PRINTF(workload, ERROR_TYPE_WARNING, tag_index, "No viable player starting locations are present (type 0, etc. must be set to none). No players will spawn.");
                }
                break;
            }
            case HEK::ScenarioType::SCENARIO_TYPE_MULTIPLAYER: {
                // TODO: Check for 16 spawns for each CTF team, 16 spawns for all other gametypes
                break;
            }
            default: break;
        }
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
    
    static void fix_script_data(BuildWorkload &workload, std::size_t tag_index, std::size_t struct_index, Scenario &scenario) {
        // If we have scripts, do stuff
        if((scenario.scripts.size() > 0 || scenario.globals.size() > 0) && scenario.source_files.size() == 0) {
            if(!workload.disable_error_checking) {
                workload.report_error(BuildWorkload::ErrorType::ERROR_TYPE_FATAL_ERROR, "Scenario tag has script data but no source file data", tag_index);
                eprintf_warn("To fix this, recompile the scripts");
                throw InvalidTagDataException();
            }
        }
        
        // Recompile scripts
        try {
            std::vector<std::string> warnings;
            compile_scripts(scenario, HEK::GameEngineInfo::get_game_engine_info(workload.get_build_parameters()->details.build_game_engine), workload.get_build_parameters()->script_optimization_level, warnings, workload.get_build_parameters()->tags_directories);
            for(auto &w : warnings) {
                REPORT_ERROR_PRINTF(workload, ERROR_TYPE_WARNING, tag_index, "Script compilation warning: %s", w.c_str());
            }
        }
        catch(std::exception &e) {
            REPORT_ERROR_PRINTF(workload, ERROR_TYPE_FATAL_ERROR, tag_index, "Failed to compile scripts: %s", e.what());
            throw;
        }
        
        // Check for stubs and warn
        for(auto &script : scenario.scripts) {
            if(script.script_type == HEK::ScenarioScriptType::SCENARIO_SCRIPT_TYPE_STUB) {
                REPORT_ERROR_PRINTF(workload, ERROR_TYPE_WARNING, tag_index, "Script '%s' is a stub script but has not been replaced by a static script. It will function as a static script, instead.", script.name.string);
            }
        }
        
        // Flip the endianness
        auto t = *reinterpret_cast<ScenarioScriptNodeTable::struct_big *>(scenario.script_syntax_data.data());
        *reinterpret_cast<ScenarioScriptNodeTable::struct_little *>(scenario.script_syntax_data.data()) = t;
        t.first_element_ptr = 0;
        
        auto *start_big = reinterpret_cast<ScenarioScriptNode::struct_big *>(scenario.script_syntax_data.data() + sizeof(t));
        auto *start_little = reinterpret_cast<ScenarioScriptNode::struct_little *>(start_big);
        
        // And now flip the endianness of the nodes
        std::size_t max_element_count = t.maximum_count;
        for(std::size_t i = 0; i < max_element_count; i++) {
            start_little[i] = start_big[i];
        }

        // Get these things
        BuildWorkload::BuildWorkloadStruct script_data_struct = {};
        script_data_struct.data = std::move(scenario.script_syntax_data);
        scenario.script_syntax_data.clear();
        const char *string_data = reinterpret_cast<const char *>(scenario.script_string_data.data());
        
        auto *syntax_data = script_data_struct.data.data();
        auto &table_header = *reinterpret_cast<ScenarioScriptNodeTable::struct_little *>(syntax_data);
        std::uint16_t element_count = table_header.size.read();
        auto *nodes = reinterpret_cast<ScenarioScriptNode::struct_little *>(&table_header + 1);
        
        // Get all references
        for(auto &r : scenario.references) {
            try {
                r.reference.tag_id = { static_cast<std::uint32_t>(workload.compile_tag_recursively(r.reference.path.c_str(), r.reference.tag_fourcc)) };
            }
            catch(std::exception &) {
                eprintf_error("I'm pretty sure this was found when compiling scripts, though! This is a bug. Please report it!");
                std::terminate();
            }
        }

        for(std::uint16_t i = 0; i < element_count; i++) {
            // Check if we know the tag group
            auto &node = nodes[i];
            auto node_flags = node.flags.read();
            if(!(node_flags & HEK::ScenarioScriptNodeFlagsFlag::SCENARIO_SCRIPT_NODE_FLAGS_FLAG_IS_PRIMITIVE) || (node_flags & HEK::ScenarioScriptNodeFlagsFlag::SCENARIO_SCRIPT_NODE_FLAGS_FLAG_IS_GLOBAL)) {
                continue;
            }
            auto group = script_value_type_to_fourcc(node.type.read());
            if(!group.has_value()) {
                continue;
            }
            
            auto *path = string_data + node.string_offset.read();
            
            std::optional<std::size_t> node_object_index;
            
            // Look for it
            for(auto &r : scenario.references) {
                if(r.reference.path == path) {
                    if(*group == HEK::TagFourCC::TAG_FOURCC_OBJECT) {
                        bool should_continue = true;
                        
                        for(auto &n : OBJECT_FOURCCS) {
                            if(!(should_continue = (r.reference.tag_fourcc == n))) {
                                break;
                            }
                        }
                        
                        if(should_continue) {
                            continue;
                        }
                    }
                    else if(r.reference.tag_fourcc != *group) {
                        continue;
                    }
                    node_object_index = r.reference.tag_id.index;
                    break;
                }
            }
            
            // Explicit?
            if(!node_object_index.has_value()) {
                auto path_split = File::split_tag_class_extension(path);
                if(path_split.has_value()) {
                    for(auto &r : scenario.references) {
                        if(r.reference.tag_fourcc == path_split->fourcc && r.reference.path == path_split->path) {
                            node_object_index = r.reference.tag_id.index;
                            break; 
                        }
                    }
                }
            }
            
            // Error!
            if(!node_object_index.has_value()) {
                eprintf_error("Couldn't find \"%s.%s\" in the references array. This is a bug. Please report it!\n", path, HEK::tag_fourcc_to_extension(*group));
                std::terminate();
            }
            
            // If we found it, set it
            node.data = HEK::TagID { static_cast<std::uint32_t>(*node_object_index) };
            auto &new_dep = script_data_struct.dependencies.emplace_back();
            new_dep.offset = reinterpret_cast<std::byte *>(&node.data) - syntax_data;
            new_dep.tag_id_only = true;
            new_dep.tag_index = *node_object_index;
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
                        REPORT_ERROR_PRINTF(workload, ERROR_TYPE_WARNING_PEDANTIC, tag_index, object_type_str " palette type #%zu (%s.%s) is unused", i, File::halo_path_to_preferred_path(palette.path).c_str(), HEK::tag_fourcc_to_extension(palette.tag_fourcc)); \
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
                    scenario.object_names[new_name].object_index = 0;
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
                REPORT_ERROR_PRINTF(workload, ERROR_TYPE_WARNING_PEDANTIC, tag_index, "Object name #%zu (%s) has no corresponding object or AI conversation", i, name_str); // crashes the game if it's used in a script (though oddly not on the tag test)
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
                base_scenario.what.emplace_back(merge); \
            } \
        }
        
        MERGE_ARRAY(child_scenarios, true);
        MERGE_ARRAY(functions, true);
        MERGE_ARRAY(comments, true);
        MERGE_ARRAY(object_names, merge.name != base.name);
        MERGE_ARRAY(device_groups, merge.name != base.name);
        MERGE_ARRAY(player_starting_profile, true);
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
        if(!scenario.child_scenarios.empty() && !workload.disable_recursion) {
            // Let's begin by adding this scenario to the list (in case we reference ourself)
            std::vector<std::string> merged_scenarios;
            merged_scenarios.emplace_back(workload.tags[tag_index].path);
            
            // Take the scenario off the top
            while(scenario.child_scenarios.size()) {
                // Get the scenario
                auto first_scenario = scenario.child_scenarios[0].child_scenario;
                
                if(!first_scenario.path.empty()) {
                    // If this isn't even a scenario tag... what
                    if(first_scenario.tag_fourcc != TagFourCC::TAG_FOURCC_SCENARIO) {
                        // This should fail even if we aren't checking for errors because this is invalid
                        REPORT_ERROR_PRINTF(workload, ERROR_TYPE_FATAL_ERROR, tag_index, "Non-scenario %s.%s referenced in child scenarios", File::halo_path_to_preferred_path(first_scenario.path).c_str(), HEK::tag_fourcc_to_extension(first_scenario.tag_fourcc));
                        throw InvalidTagDataException();
                    }
                    
                    // Make sure we haven't done it already
                    for(auto &m : merged_scenarios) {
                        // This should fail even if we aren't checking for errors because this is invalid
                        if(m == first_scenario.path) {
                            workload.report_error(BuildWorkload::ErrorType::ERROR_TYPE_FATAL_ERROR, "Duplicate or cyclical child scenario references are present", tag_index);
                            eprintf_warn("First duplicate scenario: %s.%s", File::halo_path_to_preferred_path(first_scenario.path).c_str(), HEK::tag_fourcc_to_extension(first_scenario.tag_fourcc));
                            throw InvalidTagDataException();
                        }
                    }
                    
                    // Add it to the list
                    merged_scenarios.emplace_back(first_scenario.path);
                    
                    // Find it
                    char file_path_cstr[1024];
                    std::snprintf(file_path_cstr, sizeof(file_path_cstr), "%s.%s", File::halo_path_to_preferred_path(first_scenario.path).c_str(), HEK::tag_fourcc_to_extension(first_scenario.tag_fourcc));
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
                        merge_child_scenario(scenario, child, workload, tag_index, (File::halo_path_to_preferred_path(first_scenario.path) + "." + HEK::tag_fourcc_to_extension(first_scenario.tag_fourcc)).c_str());
                    }
                    catch(std::exception &) {
                        REPORT_ERROR_PRINTF(workload, ERROR_TYPE_FATAL_ERROR, tag_index, "Failed to merge %s.%s into %s.%s",
                                            File::halo_path_to_preferred_path(first_scenario.path).c_str(),
                                            HEK::tag_fourcc_to_extension(first_scenario.tag_fourcc),
                                            File::halo_path_to_preferred_path(workload.tags[tag_index].path).c_str(),
                                            HEK::tag_fourcc_to_extension(workload.tags[tag_index].tag_fourcc)
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
        this->up_time += this->fade_in_time;
    }
    
    void ScenarioFiringPosition::pre_compile(BuildWorkload &, std::size_t, std::size_t, std::size_t) {
        this->cluster_index = NULL_INDEX;
        this->surface_index = NULL_INDEX;
    }
}
