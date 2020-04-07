// SPDX-License-Identifier: GPL-3.0-only

#include <invader/script/compiler.hpp>

namespace Invader::Compiler {
    void compile_script_tree(const std::vector<ScriptTree::Object> &, Parser::Scenario &, bool &error, std::string &error_message) {
        error = true;
        error_message = "unimplemented";
    }

    static std::optional<ScriptTree::Object> decompile_node(std::size_t index, const Parser::Scenario &scenario);

    static ScriptTree::Object::Block make_block(const ScriptTree::Object &value) {
        if(value.type == ScriptTree::Object::Type::TYPE_BLOCK) {
            return std::get<ScriptTree::Object::Block>(value.value);
        }
        else {
            return ScriptTree::Object::Block(&value, &value + 1);
        }
    }

    ScriptTree::Object decompile_scenario_script(const Parser::Scenario &scenario, const char *name) {
        HEK::ScenarioScriptValueType return_type = {};
        HEK::ScenarioScriptType script_type = {};
        std::uint32_t expression_index = 0;

        // Find the global
        for(auto &s : scenario.scripts) {
            if(std::strcmp(s.name.string, name) == 0) {
                return_type = s.return_type;
                script_type = s.script_type;
                expression_index = s.root_expression_index;
                break;
            }
        }

        // Decompile it
        auto value = decompile_node(expression_index, scenario);

        // Return what we got
        ScriptTree::Object script_call = {};
        script_call.type = ScriptTree::Object::Type::TYPE_SCRIPT;
        ScriptTree::Object::Script script = {};
        script.block = make_block(value.value());
        script.script_name = name;
        script.script_type = script_type;
        script.script_return_type = return_type;
        script_call.value = script;

        return script_call;
    }

    ScriptTree::Object decompile_scenario_global(const Parser::Scenario &scenario, const char *name) {
        HEK::ScenarioScriptValueType global_type = {};
        std::uint32_t expression_index = 0;

        // Find the global
        for(auto &g : scenario.globals) {
            if(std::strcmp(g.name.string, name) == 0) {
                global_type = g.type;
                expression_index = g.initialization_expression_index;
                break;
            }
        }

        // Decompile it
        auto value = decompile_node(expression_index, scenario);

        // Return what we got
        ScriptTree::Object global_call = {};
        global_call.type = ScriptTree::Object::Type::TYPE_GLOBAL;
        ScriptTree::Object::Global global = {};
        global.block = make_block(value.value());
        global.global_name = name;
        global.global_type = global_type;
        global_call.value = global;

        return global_call;
    }

    static void find_all_required(const ScriptTree::Object::Block &block, std::vector<std::string> &required, const std::vector<std::string> &all_required) {
        auto add_if_not_present = [&required, &all_required](const std::string &what) {
            // Make sure it isn't present already in our required list
            for(auto &r : required) {
                if(r == what) {
                    return;
                }
            }
            
            // Then, if it's present in our globals/scripts list, add it here
            for(auto &r : all_required) {
                if(r == what) {
                    required.emplace_back(what);
                    return;
                }
            }
        };

        for(auto &b : block) {
            switch(b.type) {
                case ScriptTree::Object::Type::TYPE_BLOCK:
                    find_all_required(std::get<ScriptTree::Object::Block>(b.value), required, all_required);
                    break;

                case ScriptTree::Object::Type::TYPE_SCRIPT:
                    find_all_required(std::get<ScriptTree::Object::Script>(b.value).block, required, all_required);
                    break;

                case ScriptTree::Object::Type::TYPE_GLOBAL:
                    find_all_required(std::get<ScriptTree::Object::Global>(b.value).block, required, all_required);
                    break;

                case ScriptTree::Object::Type::TYPE_TOKEN: {
                    const auto &token = std::get<Tokenizer::Token>(b.value);
                    if(token.type == Tokenizer::Token::Type::TYPE_STRING) {
                        add_if_not_present(std::get<std::string>(token.value));
                    }
                    break;
                }

                case ScriptTree::Object::Type::TYPE_FUNCTION_CALL: {
                    const auto &call = std::get<ScriptTree::Object::FunctionCall>(b.value);
                    add_if_not_present(call.function_name);
                    find_all_required(call.block, required, all_required);
                    break;
                }
            }
        }
    }

    std::vector<ScriptTree::Object> decompile_scenario(const Parser::Scenario &scenario) {
        std::vector<ScriptTree::Object> return_value;
        std::vector<std::string> resolved_tokens;
        
        // Find all names and scripts so we know what to look for and decompile
        std::vector<std::string> all_required;
        std::vector<ScriptTree::Object> objects_to_sort;
        for(auto &g : scenario.globals) {
            all_required.emplace_back(g.name.string);
            objects_to_sort.emplace_back(decompile_scenario_global(scenario, g.name.string));
        }
        for(auto &s : scenario.scripts) {
            all_required.emplace_back(s.name.string);
            objects_to_sort.emplace_back(decompile_scenario_script(scenario, s.name.string));
        }

        std::vector<std::string> stuff_added;
        auto add_object = [&stuff_added, &return_value, &all_required, &objects_to_sort](ScriptTree::Object &object, auto &add_object) {
            auto add_it_all = [&return_value, &all_required, &objects_to_sort, &object](auto *what, auto &add_object) {
                std::vector<std::string> required;
                find_all_required(what->block, required, all_required);
                
                // Go through all dependencies first
                for(auto &r : required) {
                    for(std::size_t ar = 0; ar < all_required.size(); ar++) {
                        if(all_required[ar] == r) {
                            add_object(objects_to_sort[ar], add_object);
                            break;
                        }
                    }
                }
                
                // Add it
                return_value.emplace_back(object);
            };
            
            auto *script = std::get_if<ScriptTree::Object::Script>(&object.value);
            if(script) {
                for(auto &r : stuff_added) {
                    if(r == script->script_name) {
                        return;
                    }
                }
                stuff_added.emplace_back(script->script_name);
                add_it_all(script, add_object);
                return;
            }
            auto *global = std::get_if<ScriptTree::Object::Global>(&object.value);
            if(global) {
                for(auto &r : stuff_added) {
                    if(r == global->global_name) {
                        return;
                    }
                }
                stuff_added.emplace_back(global->global_name);
                add_it_all(global, add_object);
                return;
            }
        };
        
        for(auto &o : objects_to_sort) {
            add_object(o, add_object);
        }

        return return_value;
    }

    static std::optional<std::size_t> node_for_index(std::uint32_t index) {
        if(index == 0xFFFFFFFF) {
            return std::nullopt;
        }
        else {
            return index & 0xFFFF;
        }
    }

    static std::optional<ScriptTree::Object> decompile_node(std::size_t index, const Parser::Scenario &scenario) {
        // Get these values
        const auto *script_syntax_data_data = scenario.script_syntax_data.data();
        const auto *table = reinterpret_cast<const Parser::ScenarioScriptNodeTable::struct_big *>(script_syntax_data_data);
        const auto *nodes = reinterpret_cast<const Parser::ScenarioScriptNode::struct_big *>(table + 1);
        std::size_t node_count = table->size;

        // Make sure we're in bounds
        auto node_index_to_check = node_for_index(index);
        if(!node_index_to_check.has_value() || *node_index_to_check >= node_count || (reinterpret_cast<std::uintptr_t>(nodes + *node_index_to_check) - reinterpret_cast<std::uintptr_t>(table)) > scenario.script_syntax_data.size()) {
            eprintf_error("Node index is out of bounds");
            throw InvalidTagDataException();
        }
        auto &n = nodes[*node_index_to_check];

        // Get the string, returning nullptr if it's not null terminated or is out of bounds
        auto sanitary_get_string = [&scenario](std::size_t offset) -> const char * {
            std::size_t string_data_length = scenario.script_string_data.size();
            const char *string_data = reinterpret_cast<const char *>(scenario.script_string_data.data());
            for(std::size_t i = offset; i < string_data_length; i++) {
                if(string_data[i] == 0) {
                    return string_data + offset;
                }
            }
            return nullptr;
        };

        // Get the type and flags
        auto type = n.type.read();
        auto flags = n.flags.read();

        // Globals are just strings
        if(flags & HEK::ScenarioScriptNodeFlagsFlag::SCENARIO_SCRIPT_NODE_FLAGS_FLAG_IS_GLOBAL) {
            ScriptTree::Object o = {};
            Tokenizer::Token t = {};
            t.type = Tokenizer::Token::Type::TYPE_STRING;
            o.type = ScriptTree::Object::Type::TYPE_TOKEN;
            t.value = sanitary_get_string(n.string_offset);
            o.value = t;
            return o;
        }
        
        // Handle script calls as so
        else if(
            (flags & HEK::ScenarioScriptNodeFlagsFlag::SCENARIO_SCRIPT_NODE_FLAGS_FLAG_IS_SCRIPT_CALL) || 
            !(flags & HEK::ScenarioScriptNodeFlagsFlag::SCENARIO_SCRIPT_NODE_FLAGS_FLAG_IS_PRIMITIVE)) {
            return decompile_node(static_cast<std::size_t>(n.data.read().long_int), scenario);
        }

        switch(type) {
            case HEK::ScenarioScriptValueType::SCENARIO_SCRIPT_VALUE_TYPE_VOID:
                return decompile_node(n.data.read().long_int, scenario);
            case HEK::ScenarioScriptValueType::SCENARIO_SCRIPT_VALUE_TYPE_FUNCTION_NAME: {
                ScriptTree::Object object = {};
                ScriptTree::Object::FunctionCall function_call = {};
                ScriptTree::Object::Block block;
                object.type = ScriptTree::Object::Type::TYPE_FUNCTION_CALL;

                // Function call stuff
                const char *function_name = sanitary_get_string(n.string_offset.read());
                if(!function_name) {
                    eprintf_error("Function name is out of bounds");
                    throw InvalidTagDataException();
                }
                function_call.function_name = function_name;

                // Go through each thing in the function
                auto next_node_index = node_for_index(static_cast<std::uint32_t>(n.next_node.read()));
                while(next_node_index.has_value()) {
                    auto &nn = nodes[*next_node_index];
                    auto decompiled_node = decompile_node(*next_node_index, scenario);
                    if(decompiled_node.has_value()) {
                        block.emplace_back(decompiled_node.value());
                    }
                    next_node_index = node_for_index(nn.next_node);
                }

                // Remove redundant begins
                if(function_call.function_name == "begin") {
                    while(block.size() == 1 && block[0].type == ScriptTree::Object::Type::TYPE_FUNCTION_CALL && std::get<ScriptTree::Object::FunctionCall>(block[0].value).function_name == "begin") {
                        block = std::get<ScriptTree::Object::FunctionCall>(block[0].value).block;
                    }
                }

                // Set the function call stuff
                function_call.block = block;
                object.value = function_call;
                return object;
            }

            case HEK::ScenarioScriptValueType::SCENARIO_SCRIPT_VALUE_TYPE_REAL:
            case HEK::ScenarioScriptValueType::SCENARIO_SCRIPT_VALUE_TYPE_SHORT:
            case HEK::ScenarioScriptValueType::SCENARIO_SCRIPT_VALUE_TYPE_LONG:
            case HEK::ScenarioScriptValueType::SCENARIO_SCRIPT_VALUE_TYPE_BOOLEAN: {
                ScriptTree::Object o = {};
                o.type = ScriptTree::Object::Type::TYPE_TOKEN;
                Tokenizer::Token t = {};
                switch(type) {
                    case HEK::ScenarioScriptValueType::SCENARIO_SCRIPT_VALUE_TYPE_REAL:
                        t.type = Tokenizer::Token::Type::TYPE_DECIMAL;
                        t.value = n.data.read().real;
                        break;
                    case HEK::ScenarioScriptValueType::SCENARIO_SCRIPT_VALUE_TYPE_SHORT:
                        t.type = Tokenizer::Token::Type::TYPE_INTEGER;
                        t.value = n.data.read().short_int;
                        break;
                    case HEK::ScenarioScriptValueType::SCENARIO_SCRIPT_VALUE_TYPE_LONG:
                        t.type = Tokenizer::Token::Type::TYPE_INTEGER;
                        t.value = n.data.read().long_int;
                        break;
                    case HEK::ScenarioScriptValueType::SCENARIO_SCRIPT_VALUE_TYPE_BOOLEAN:
                        t.type = Tokenizer::Token::Type::TYPE_INTEGER;
                        t.value = n.data.read().bool_int;
                        break;
                    default:
                        std::terminate();
                }
                o.value = t;
                return o;
            }
            
            case HEK::ScenarioScriptValueType::SCENARIO_SCRIPT_VALUE_TYPE_UNPARSED:
                return std::nullopt;

            default: {
                ScriptTree::Object o = {};
                o.type = ScriptTree::Object::Type::TYPE_TOKEN;
                Tokenizer::Token t = {};
                t.type = Tokenizer::Token::Type::TYPE_STRING;
                auto *str = sanitary_get_string(n.string_offset.read());
                if(!str) {
                    eprintf("String out of bounds (%zu >= %zu, %s)\n", static_cast<std::size_t>(n.string_offset.read()), scenario.script_string_data.size(), reinterpret_cast<const char *>(scenario.script_string_data.data()) + static_cast<std::size_t>(n.string_offset.read()));
                    throw InvalidTagDataException();
                }
                t.value = std::string(str);
                o.value = t;
                return o;
            }
        }
    }
}
