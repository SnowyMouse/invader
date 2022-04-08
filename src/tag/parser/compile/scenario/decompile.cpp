// SPDX-License-Identifier: GPL-3.0-only

#include <invader/tag/parser/parser.hpp>
#include <invader/build/build_workload.hpp>
#include <invader/file/file.hpp>
#include <invader/tag/hek/class/model_collision_geometry.hpp>
#include <invader/tag/parser/compile/scenario.hpp>
#include "../../../../version.h"

namespace Invader::Parser {
    bool fix_missing_script_source_data(Scenario &scenario, bool fix) {
        if(!scenario.source_files.empty() || (scenario.scripts.empty() && scenario.globals.empty())) {
            return false;
        }
        
        if(fix) {
            try {
                auto *string_data = reinterpret_cast<const char *>(scenario.script_string_data.data());
                auto string_data_length = scenario.script_string_data.size();
                
                // Make sure the string data is null terminated
                while(string_data_length > 1) {
                    if(string_data[string_data_length] != 0) {
                        string_data_length--;
                    }
                    else {
                        break;
                    }
                }
                
                // If there's no string data or it's only one character and that character is non-null, bail
                if(string_data_length == 0 || (string_data_length == 1 && string_data[string_data_length] != 0)) {
                    return false;
                }
                
                // This is the maximum string data offset we can use
                auto max_string_data_offset = string_data_length - 1;
                
                // Next, let's get the nodes
                auto syntax_data_len = scenario.script_syntax_data.size();
                auto *script_node_table = reinterpret_cast<const HEK::ScenarioScriptNodeTable<HEK::BigEndian> *>(scenario.script_syntax_data.data());
                if(syntax_data_len < sizeof(*script_node_table)) {
                    return false;
                }
                
                // Make sure we have enough nodes
                auto node_count = script_node_table->size.read();
                auto *script_node_nodes = reinterpret_cast<const HEK::ScenarioScriptNode<HEK::BigEndian> *>(script_node_table + 1);
                if(syntax_data_len < sizeof(*script_node_table) + sizeof(*script_node_nodes) * node_count) {
                    return false;
                }
                
                auto decompile_node = [&node_count, &script_node_nodes, &string_data, &max_string_data_offset, &scenario](std::uint32_t node, std::vector<std::string> &tokens, auto decompile_node) -> bool {
                    while(node != 0xFFFFFFFF) {
                        auto node_index = node & 0xFFFF;
                        if(node_index >= node_count) {
                            return false; // out of bounds
                        }
                        
                        auto &node_to_decomp = script_node_nodes[node_index];
                        
                        auto flags = node_to_decomp.flags.read();
                        auto data = node_to_decomp.data.read();
                        
                        if(flags & HEK::ScenarioScriptNodeFlagsFlag::SCENARIO_SCRIPT_NODE_FLAGS_FLAG_IS_PRIMITIVE) {
                            #define APPEND_STRING_DATA auto string_offset = node_to_decomp.string_offset.read(); \
                                                       if(string_offset < max_string_data_offset) { \
                                                           tokens.emplace_back(string_data + string_offset); \
                                                       } \
                                                       else {\
                                                           eprintf_error("Failed to decompile script: String offset %u out of bounds", string_offset);\
                                                           return false;\
                                                       }\
                            
                            if(flags & HEK::ScenarioScriptNodeFlagsFlag::SCENARIO_SCRIPT_NODE_FLAGS_FLAG_IS_GLOBAL) {
                                APPEND_STRING_DATA
                            }
                            else {
                                switch(node_to_decomp.type.read()) {
                                    // No node? tool.exe generates these sometimes when expanding (cond) into (if) blocks
                                    case HEK::ScenarioScriptValueType::SCENARIO_SCRIPT_VALUE_TYPE_VOID:
                                        break;
                                        
                                    // Numbers
                                    case HEK::ScenarioScriptValueType::SCENARIO_SCRIPT_VALUE_TYPE_SHORT:
                                        tokens.emplace_back(std::to_string(data.short_int));
                                        break;
                                    case HEK::ScenarioScriptValueType::SCENARIO_SCRIPT_VALUE_TYPE_LONG:
                                        tokens.emplace_back(std::to_string(data.long_int));
                                        break;
                                    case HEK::ScenarioScriptValueType::SCENARIO_SCRIPT_VALUE_TYPE_REAL:
                                        tokens.emplace_back(std::to_string(data.real));
                                        break;
                                        
                                    // Boolean
                                    case HEK::ScenarioScriptValueType::SCENARIO_SCRIPT_VALUE_TYPE_BOOLEAN:
                                        tokens.emplace_back(data.bool_int == 0 ? "0" : "1");
                                        break;
                                    
                                    // Script name
                                    case HEK::ScenarioScriptValueType::SCENARIO_SCRIPT_VALUE_TYPE_SCRIPT: {
                                        auto index = static_cast<std::size_t>(data.short_int);
                                        if(index < scenario.scripts.size()) {
                                            tokens.emplace_back(scenario.scripts[index].name.string);
                                            break;
                                        }
                                        else {
                                            eprintf_error("Failed to decompile script: Unknown script index %i", data.short_int);
                                            return false;
                                        }
                                    }
                                    
                                    // Game difficulty
                                    case HEK::ScenarioScriptValueType::SCENARIO_SCRIPT_VALUE_TYPE_GAME_DIFFICULTY:
                                        switch(data.short_int) {
                                            case 0:
                                                tokens.emplace_back("easy");
                                                break;
                                            case 1:
                                                tokens.emplace_back("normal");
                                                break;
                                            case 2:
                                                tokens.emplace_back("hard");
                                                break;
                                            case 3:
                                                tokens.emplace_back("impossible");
                                                break;
                                            default:
                                                eprintf_error("Failed to decompile script: Unknown difficulty index %i", data.short_int);
                                                return false; // unknown
                                        }
                                        break;
                                    
                                    // Team index
                                    case HEK::ScenarioScriptValueType::SCENARIO_SCRIPT_VALUE_TYPE_TEAM:
                                        try {
                                            tokens.emplace_back(HEK::UnitDefaultTeam_to_string(static_cast<HEK::UnitDefaultTeam>(data.short_int)));
                                            break;
                                        }
                                        catch(std::exception &) {
                                            eprintf_error("Failed to decompile script: Unknown team index %i", data.short_int);
                                            return false; // unknown
                                        }
                                    
                                    // Default to the string data
                                    default: {
                                        APPEND_STRING_DATA
                                    }
                                }
                            }
                        }
                        
                        // Recursion
                        else {
                            tokens.emplace_back("(");
                            if(!decompile_node(static_cast<std::uint32_t>(node_to_decomp.data.read().long_int), tokens, decompile_node)) {
                                return false;
                            }
                            tokens.emplace_back(")");
                        }
                        
                        node = node_to_decomp.next_node;
                    }
                    
                    return true; // done
                };
                
                // Get the type
                auto type_to_type_str = [](HEK::ScenarioScriptValueType t) -> std::string {
                    auto type_str = std::string(HEK::ScenarioScriptValueType_to_string(t));
                    for(auto &c : type_str) {
                        if(c == ' ') {
                            c = '_';
                        }
                    }
                    return type_str;
                };
                
                // Convert to string
                auto tokens_to_str = [](std::vector<std::string> &tokens) -> std::string {
                    std::string str;
                    
                    auto append_token = [&str](const std::string &token) {
                        bool quotes = token.empty();
                        for(char c : token) {
                            if(c == ' ') {
                                quotes = true;
                                break;
                            }
                        }
                        
                        if(quotes) {
                            str += "\"";
                        }
                        
                        str += token;
                        
                        if(quotes) {
                            str += "\"";
                        }
                    };
                    
                    auto append_range = [&tokens, &str, &append_token](std::size_t from, std::size_t to) {
                        for(std::size_t i = from; i < to; i++) {
                            // Add whitespace if it's not the first one, the previous isn't a '(' and the next isn't a ')'
                            if(i != from && tokens[i - 1] != "(" && tokens[i] != ")") {
                                str += " ";
                            }
                            
                            // Add the thing
                            append_token(tokens[i]);
                        }
                        
                        str += "\r\n";
                    };
                    
                    auto token_count = tokens.size();
                    int depth = 0;
                    
                    for(std::size_t i = 0; i < token_count;) {
                        // Are we ending a block? Reduce the depth by 1
                        if(tokens[i] == ")") {
                            depth--;
                        }
                        
                        for(int q = 0; q < depth; q++) {
                            str += "    ";
                        }
                        
                        // Are we starting a block?
                        if(tokens[i] == "(") {
                            // Do we have to do line breaks? Check to see if this is a short block
                            int relative_depth = 1;
                            std::size_t j;
                            
                            static constexpr const std::size_t max_tokens_per_line = 10;
                            
                            for(j = i + 1; relative_depth > 0 && j < i + max_tokens_per_line; j++) {
                                if(tokens[j] == ")") {
                                    relative_depth--;
                                }
                                if(tokens[j] == "(") {
                                    relative_depth++;
                                }
                            }
                            
                            // It's short, so we can include it
                            if(relative_depth == 0) {
                                append_range(i, j);
                                i = j;
                                continue;
                            }
                            
                            // Increment the depth otherwise
                            depth++;
                        
                            // Is it the top level block? We can include the first few things
                            if(i == 0) {
                                if(tokens[i + 1] == "global" || (tokens[i + 2] != "static" && tokens[i + 2] != "stub")) { // static/stub has 1 more token than other script types
                                    append_range(0, 4);
                                    i = 4;
                                }
                                else {
                                    append_range(0, 5);
                                    i = 5;
                                }
                                continue;
                            }
                            
                            // Otherwise, just include the next two
                            append_range(i, i + 2);
                            i += 2;
                            
                            continue;
                        }
                        
                        append_token(tokens[i++]);
                        str += "\r\n";
                    }
                    
                    return str + "\r\n";
                };
                
                std::vector<Parser::ScenarioSourceFile> new_sources;
                auto add_empty_source = [&new_sources]() -> Parser::ScenarioSourceFile & {
                    auto &new_source = new_sources.emplace_back();
                    std::snprintf(new_source.name.string, sizeof(new_source.name.string), "extracted_%04zu", new_sources.size() - 1);
                    return new_source;
                };
                add_empty_source();
                
                auto append_str_to_hsc = [&new_sources, &add_empty_source](const std::string &what) {
                    std::size_t source_count = new_sources.size();
                    
                    auto len = what.size();
                    auto *ws = reinterpret_cast<const std::byte *>(what.c_str());
                    auto *we = ws + len;
                    
                    auto &last_script = new_sources[source_count - 1];
                    
                    // If it's empty, just set it
                    if(last_script.source.empty()) {
                        last_script.source = std::vector<std::byte>(ws, we);
                    }
                    
                    // Split on 256 KiB, accounting for a null byte, a CRLF, and a header
                    else if(len + 64 + last_script.source.size() >= (256 * 1024)) {
                        add_empty_source().source = std::vector<std::byte>(ws, we);
                    }
                    
                    // Append if we can fit it otherwise
                    else {
                        last_script.source.emplace_back(static_cast<std::byte>('\r'));
                        last_script.source.emplace_back(static_cast<std::byte>('\n'));
                        last_script.source.insert(last_script.source.end(), ws, we);
                    }
                };
                
                // Globals!
                for(auto &g : scenario.globals) {
                    std::vector<std::string> tokens = { "(", "global", type_to_type_str(g.type), g.name.string };
                    if(!decompile_node(g.initialization_expression_index, tokens, decompile_node)) {
                        return false;
                    }
                    tokens.emplace_back(")");
                    
                    append_str_to_hsc(tokens_to_str(tokens));
                }
                
                // Scripts!
                for(auto &s : scenario.scripts) {
                    std::vector<std::string> tokens = { "(", "script" };
                    tokens.emplace_back(HEK::ScenarioScriptType_to_string(s.script_type));
                    switch(s.script_type) {
                        case HEK::SCENARIO_SCRIPT_TYPE_STUB:
                        case HEK::SCENARIO_SCRIPT_TYPE_STATIC:
                            tokens.emplace_back(type_to_type_str(s.return_type));
                            break;
                        default: break;
                    }
                    tokens.emplace_back(s.name.string);
                    
                    auto first_function_call = tokens.size();
                    if(!decompile_node(s.root_expression_index, tokens, decompile_node)) {
                        return false;
                    }
                    tokens.emplace_back(")");
                    
                    // Remove unnecessary begin blocks
                    for(std::size_t t = first_function_call; t < tokens.size(); t++) {
                        while(true) {
                            std::size_t name_offset = t + 1;
                            
                            if(tokens[t] == "(" && tokens[name_offset] == "begin") {
                                std::size_t end;
                                
                                // Is it NOT a topmost block? If so, do NOT remove it if things_in_depth_1 is not 1
                                if(t != first_function_call) {
                                    std::size_t depth = 1;
                                    auto token_count = tokens.size();
                                    
                                    // Find the end of the topmost begin block
                                    std::size_t things_in_depth_1 = 0;
                                    std::size_t i;
                                    for(i = name_offset + 1; i < token_count && depth > 0; i++) {
                                        if(tokens[i] == ")") {
                                            depth--;
                                        }
                                        else {
                                            if(depth == 1) {
                                                things_in_depth_1++;
                                            }
                                            if(tokens[i] == "(") {
                                                depth++;
                                            }
                                        }
                                    }
                                    
                                    // Is it NOT a topmost block? If so, do NOT remove it if things_in_depth_1 is not 1
                                    if(things_in_depth_1 != 1) {
                                        break;
                                    }
                                    
                                    end = i - 1;
                                }
                                else {
                                    end = tokens.size() - 1;
                                }
                                
                                // No? Remove it then
                                tokens.erase(tokens.begin() + end);
                                tokens.erase(tokens.begin() + t);
                                tokens.erase(tokens.begin() + t);
                            }
                            else {
                                break;
                            }
                        }
                    }
                    
                    append_str_to_hsc(tokens_to_str(tokens));
                }
                
                // Add a header comment for metadata
                auto source_count = new_sources.size();
                if(source_count == 1) {
                    std::strncpy(new_sources[0].name.string, "extracted", sizeof(new_sources[0].name.string));
                }
                
                for(std::size_t i = 0; i < source_count; i++) {
                    char generated_by[80];
                    std::snprintf(generated_by, sizeof(generated_by), "generated by %s", INVADER_FULL_VERSION_STRING);
                    char file_number[80];
                    std::snprintf(file_number, sizeof(file_number), "%s.hsc, file %zu of %zu", new_sources[i].name.string, i + 1, source_count);
                    
                    char header[512];
                    std::snprintf(header, sizeof(header), ";*\r\n"
                                                          "================================================================================\r\n"
                                                          "\r\n"
                                                          "    %s\r\n"
                                                          "\r\n"
                                                          "    %s\r\n"
                                                          "\r\n"
                                                          "================================================================================\r\n"
                                                          "*;\r\n\r\n", file_number, generated_by);
                    auto length = std::strlen(header);
                    
                    // Append the header
                    auto &src = new_sources[i].source;
                    src.insert(src.begin(), reinterpret_cast<const std::byte *>(header), reinterpret_cast<const std::byte *>(header + length));
                    src.emplace_back(); // add a null terminator
                }
                
                scenario.source_files = new_sources;
                
                return true;
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
}
