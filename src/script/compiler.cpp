// SPDX-License-Identifier: GPL-3.0-only

#include "compiler.hpp"

namespace Invader::Compiler {
    void compile_script_tree(const std::vector<ScriptTree::Object> &, Parser::Scenario &, bool &error, std::string &error_message) {
        error = true;
        error_message = "unimplemented";
    }

    static ScriptTree::Object decompile_node(std::size_t index, const Parser::ScenarioScriptNode::struct_big *nodes, std::size_t node_count, const Parser::Scenario &scenario);

    std::vector<ScriptTree::Object> decompile_scenario(const Parser::Scenario &scenario, bool &error, std::string &error_message) {
        std::vector<ScriptTree::Object> return_value;

        const auto *script_syntax_data_data = scenario.script_syntax_data.data();
        std::size_t script_syntax_data_size = scenario.script_syntax_data.size();

        const auto *table = reinterpret_cast<const Parser::ScenarioScriptNodeTable::struct_big *>(script_syntax_data_data);
        const auto *nodes = reinterpret_cast<const Parser::ScenarioScriptNode::struct_big *>(table + 1);

        if(script_syntax_data_size < sizeof(*table)) {
            error = true;
            error_message = "Script syntax data is too small to fit a node table";
            return {};
        }

        std::size_t max_index = table->size;
        if(script_syntax_data_size < max_index * sizeof(*nodes) + sizeof(*table)) {
            error = true;
            error_message = "Script syntax data is invalid";
            return {};
        }

        /*

        auto decompile_node = [&max_index, &nodes, &string_data](std::uint32_t id, auto &decompile_node) -> std::optional<ScriptTree::Object> {
            if(id == 0xFFFFFFFF) {
                return std::nullopt;
            }
            std::size_t index = id & 0xFFFF;
            if(index >= max_index) {
                throw std::exception();
            }

            auto &n = nodes[index];
            auto type = n.type.read();

            switch(n.type.read()) {
                case HEK::ScenarioScriptValueType::SCENARIO_SCRIPT_VALUE_TYPE_VOID: {
                    ScriptTree::Object o = {};
                    o.type = ScriptTree::Object::Type::TYPE_BLOCK;

                    // Go through each node and add it
                    ScriptTree::Object::Block b;
                    std::vector<ScriptTree::Object::Block> block_stack;
                    std::uint32_t next_node = static_cast<std::uint32_t>(n.data.read().long_int);
                    auto *node = nodes + (next_node & 0xFFFF);
                    auto value = decompile_node(next_node, decompile_node);
                    if(!value.has_value()) {
                        return std::nullopt;
                    }

                    // If it's a function call, let's add that
                    auto &value_value = *value;
                    if(value_value.type == ScriptTree::Object::Type::TYPE_FUNCTION_CALL) {
                        auto &call = std::get<ScriptTree::Object::FunctionCall>(value_value.value);
                        auto &block_to_add_to = call.block;
                        while(true) {
                            next_node = static_cast<std::uint32_t>(node->next_node.read());
                            auto value = decompile_node(next_node, decompile_node);
                            if(!value.has_value()) {
                                break;
                            }
                            block_to_add_to.push_back(*value);
                            node = nodes + (next_node & 0xFFFF);
                        }

                        // If it's a block with only one block, reduce it
                        while(block_to_add_to.size() == 1 && block_to_add_to[0].type == ScriptTree::Object::Type::TYPE_BLOCK) {
                            block_to_add_to = std::get<ScriptTree::Object::Block>(block_to_add_to[0].value);
                        }

                        // Next, if there's only one item in the block and that item is a function call for "begin" AND we're a begin block, reduce it
                        while(call.function_name == "begin" && block_to_add_to.size() == 1 && block_to_add_to[0].type == ScriptTree::Object::Type::TYPE_FUNCTION_CALL && std::get<ScriptTree::Object::FunctionCall>(block_to_add_to[0].value).function_name == "begin") {
                            call = std::get<ScriptTree::Object::FunctionCall>(block_to_add_to[0].value);
                        }
                    }
                    else {
                        eprintf("Expected function call\n");
                        throw std::exception();
                    }
                    b.emplace_back(*value);

                    o.value = b;
                    return o;
                }
                case HEK::ScenarioScriptValueType::SCENARIO_SCRIPT_VALUE_TYPE_FUNCTION_NAME: {
                    ScriptTree::Object o = {};
                    o.type = ScriptTree::Object::Type::TYPE_FUNCTION_CALL;
                    ScriptTree::Object::FunctionCall call;
                    call.function_name = string_data + n.string_offset.read();
                    o.value = call;
                    return o;
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
                case HEK::ScenarioScriptValueType::SCENARIO_SCRIPT_VALUE_TYPE_UNPARSED: {
                    return decompile_node(n.next_node.read(), decompile_node);
                }
                // fallthrough
                default:
                    ScriptTree::Object o = {};
                    o.type = ScriptTree::Object::Type::TYPE_TOKEN;
                    Tokenizer::Token t = {};
                    t.type = Tokenizer::Token::Type::TYPE_STRING;
                    t.value = std::string(string_data + n.string_offset.read());
                    o.value = t;
                    return o;
            }
        };*/

        // Go through each global
        for(auto &g : scenario.globals) {
            oprintf("Decompiling global %s...\n", g.name.string);
            auto value = decompile_node(g.initialization_expression_index, nodes, max_index, scenario);

            ScriptTree::Object script_call = {};
            script_call.type = ScriptTree::Object::Type::TYPE_GLOBAL;
            ScriptTree::Object::Global global = {};

            if(value.type == ScriptTree::Object::Type::TYPE_BLOCK) {
                global.block = std::get<ScriptTree::Object::Block>(value.value);
            }
            else {
                ScriptTree::Object::Block b;
                b.emplace_back(value);
                global.block = b;
            }

            global.global_name = g.name.string;
            global.global_type = g.type;
            script_call.value = global;

            return_value.emplace_back(script_call);
        }

        // Go through each script
        for(auto &s : scenario.scripts) {
            oprintf("Decompiling script %s...\n", s.name.string);
            auto value = decompile_node(s.root_expression_index, nodes, max_index, scenario);

            ScriptTree::Object script_call = {};
            script_call.type = ScriptTree::Object::Type::TYPE_SCRIPT;
            ScriptTree::Object::Script script = {};

            if(value.type == ScriptTree::Object::Type::TYPE_BLOCK) {
                script.block = std::get<ScriptTree::Object::Block>(value.value);
            }
            else {
                ScriptTree::Object::Block b;
                b.emplace_back(value);
                script.block = b;
            }

            script.script_name = s.name.string;
            script.script_return_type = s.return_type;
            script.script_type = s.script_type;
            script_call.value = script;

            return_value.emplace_back(script_call);
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

    static ScriptTree::Object decompile_node(std::size_t index, const Parser::ScenarioScriptNode::struct_big *nodes, std::size_t node_count, const Parser::Scenario &scenario) {
        auto node_index_to_check = node_for_index(index);
        if(!node_index_to_check.has_value() || *node_index_to_check >= node_count) {
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

        if(flags.is_global) {
            ScriptTree::Object o = {};
            Tokenizer::Token t = {};
            t.type = Tokenizer::Token::Type::TYPE_STRING;
            o.type = ScriptTree::Object::Type::TYPE_TOKEN;
            std::size_t global_index = static_cast<std::size_t>(n.data.read().long_int);

            // Check it
            if(global_index >= scenario.globals.size()) {
                eprintf_error("Global index is out of bounds (%zu >= %zu)", global_index, scenario.globals.size());
                throw InvalidTagDataException();
            }

            t.value = scenario.globals[global_index].name.string;
            o.value = t;

            return o;
        }
        else if(flags.is_script_call || !flags.is_primitive) {
            return decompile_node(static_cast<std::size_t>(n.data.read().long_int), nodes, node_count, scenario);
        }

        switch(type) {
            case HEK::ScenarioScriptValueType::SCENARIO_SCRIPT_VALUE_TYPE_VOID:
                return decompile_node(n.data.read().long_int, nodes, node_count, scenario);
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
                auto next_node_index = node_for_index(n.next_node);
                while(next_node_index.has_value()) {
                    auto &nn = nodes[*next_node_index];
                    block.emplace_back(decompile_node(*next_node_index, nodes, node_count, scenario));
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
