// SPDX-License-Identifier: GPL-3.0-only

#include <optional>
#include <invader/script/script_tree.hpp>

using namespace Invader::Parser;

namespace Invader::ScriptTree {
    using namespace Tokenizer;

    // Get a principal object (script or global)
    static std::optional<Object> get_principal_object(const Token *first_token, const Token *last_token, std::size_t &advance, bool &error, std::size_t &error_line, std::size_t &error_column, std::string &error_token, std::string &error_message);

    std::vector<Object> compile_tokens(const std::vector<Token> &tokens, bool &error, std::size_t &error_line, std::size_t &error_column, std::string &error_token, std::string &error_message) {
        std::vector<Object> objects;
        error = false;

        // Iterate through each token
        std::size_t token_count = tokens.size();
        auto *last_token = tokens.end().base();
        for(std::size_t i = 0; i < token_count; i++) {
            // See if we have something
            auto &token = tokens[i];
            if(token.type == Token::TYPE_PARENTHESIS_OPEN) {
                std::size_t advance;
                auto object = get_principal_object(&token, last_token, advance, error, error_line, error_column, error_token, error_message);
                if(error) {
                    return std::vector<Object>();
                }
                else {
                    i += advance;
                    i--;
                }
                objects.push_back(object.value());
            }

            // Otherwise, bail
            else {
                error = true;
                error_message = "Unexpected token (expected an opening parenthesis)";
                error_line = token.line;
                error_column = token.column;
                error_token = token.raw_value;
                return std::vector<Object>();
            }
        }

        return objects;
    }

    static std::optional<Object> get_global(const Token *first_token, std::size_t advance, bool &error, std::size_t &error_line, std::size_t &error_column, std::string &error_token, std::string &error_message);
    static std::optional<Object> get_script(const Token *first_token, std::size_t advance, bool &error, std::size_t &error_line, std::size_t &error_column, std::string &error_token, std::string &error_message);

    #define RETURN_ERROR_TOKEN(token, error_msg) \
        error = true; \
        error_line = (token).line; \
        error_column = (token).column; \
        error_token = (token).raw_value; \
        error_message = error_msg; \
        return std::nullopt;

    static std::optional<Object> get_principal_object(const Token *first_token, const Token *last_token, std::size_t &advance, bool &error, std::size_t &error_line, std::size_t &error_column, std::string &error_token, std::string &error_message) {
        // Get the remaining amount of tokens
        std::size_t remaining = last_token - first_token;

        // Get how big our block is
        std::size_t depth = 0;
        for(advance = 0; advance < remaining; advance++) {
            auto &token = first_token[advance];
            if(token.type == Token::TYPE_PARENTHESIS_OPEN) {
                depth++;
            }
            else if(token.type == Token::TYPE_PARENTHESIS_CLOSE) {
                depth--;
                if(depth == 0) {
                    advance++;
                    break;
                }
            }
        }

        // If the depth is non-zero, it's unterminated
        if(depth > 0) {
            RETURN_ERROR_TOKEN(*first_token, "Unterminated script block");
        }

        // We need at least enough room for the script/global definition and the parenthesis
        if(advance < 5) {
            RETURN_ERROR_TOKEN(*first_token, "Incomplete script or global definition");
        }

        auto &type_token = first_token[1];
        if(type_token.type != Token::TYPE_STRING) {
            RETURN_ERROR_TOKEN(type_token, "Non-string script or global type");
        }
        auto &type = std::get<std::string>(type_token.value);

        if(type == "script") {
            return get_script(first_token, advance, error, error_line, error_column, error_token, error_message);
        }
        else if(type == "global") {
            return get_global(first_token, advance, error, error_line, error_column, error_token, error_message);
        }
        else {
            RETURN_ERROR_TOKEN(type_token, "Expected \"global\" or \"script\"");
        }
    }

    static std::optional<Object::Block> get_block(const Token *first_token, const Token *last_token, std::size_t &advance, bool &error, std::size_t &error_line, std::size_t &error_column, std::string &error_token, std::string &error_message);

    static std::optional<Object> get_script(const Token *first_token, std::size_t advance, bool &error, std::size_t &error_line, std::size_t &error_column, std::string &error_token, std::string &error_message) {
        // Begin!
        Object::Script script;

        // Get the script type
        auto &script_type = first_token[2];
        if(script_type.type != Token::TYPE_STRING) {
            RETURN_ERROR_TOKEN(first_token[2], "Expected a string");
        }
        auto &script_type_value = std::get<std::string>(script_type.value);

        std::size_t name_index = 3;

        if(script_type_value == "static" || script_type_value == "stub") {
            name_index++;
            // Get the script return type
            auto &script_return_type = first_token[3];
            if(script_type.type != Token::TYPE_STRING) {
                RETURN_ERROR_TOKEN(first_token[3], "Expected a string");
            }

            // Make sure it's valid
            try {
                std::string return_type_value = std::get<std::string>(script_return_type.value);
                for(char &c : return_type_value) {
                    if(c == ' ') {
                        c = '-';
                    }
                }
                script.script_return_type = ScenarioScriptValueType_from_string(return_type_value.c_str()).value();
            }
            catch(std::exception &) {
                RETURN_ERROR_TOKEN(first_token[3], "Invalid script return type");
            }
            script.script_type = ScenarioScriptType::SCENARIO_SCRIPT_TYPE_STATIC;
        }
        else {
            script.script_return_type = ScenarioScriptValueType::SCENARIO_SCRIPT_VALUE_TYPE_VOID;
            if(script_type_value == "stub") {
                script.script_type = ScenarioScriptType::SCENARIO_SCRIPT_TYPE_STUB;
            }
            else if(script_type_value == "startup") {
                script.script_type = ScenarioScriptType::SCENARIO_SCRIPT_TYPE_STARTUP;
            }
            else if(script_type_value == "continuous") {
                script.script_type = ScenarioScriptType::SCENARIO_SCRIPT_TYPE_CONTINUOUS;
            }
            else if(script_type_value == "dormant") {
                script.script_type = ScenarioScriptType::SCENARIO_SCRIPT_TYPE_DORMANT;
            }
            else {
                RETURN_ERROR_TOKEN(first_token[2], "Invalid script type");
            }
        }

        // Get the script name
        auto &script_name = first_token[name_index];
        if(script_name.type != Token::TYPE_STRING) {
            RETURN_ERROR_TOKEN(first_token[name_index], "Expected a string");
        }
        script.script_name = std::get<std::string>(script_name.value);

        // Get the block
        std::size_t advance_block;
        auto block = get_block(first_token + name_index + 1, first_token + advance, advance_block, error, error_line, error_column, error_token, error_message);
        if(error) {
            return std::nullopt;
        }
        script.block = std::move(block.value());

        // Initialize and get everything going
        std::optional<Object> r = Object();
        auto &script_object = r.value();
        script_object.type = Object::Type::TYPE_SCRIPT;
        script_object.value = script;
        r->column = first_token->column;
        r->line = first_token->line;

        return r;
    }

    static std::optional<Object> get_global(const Token *first_token, std::size_t advance, bool &error, std::size_t &error_line, std::size_t &error_column, std::string &error_token, std::string &error_message) {
        // Make sure we have a complete global definition: "(" "global" <type> <name> <value> ")"
        if(advance < 6) {
            RETURN_ERROR_TOKEN(*first_token, "Incomplete global definition");
        }

        // Begin!
        Object::Global global;

        #define SET_GLOBAL_STRING_OR_BAIL(token, str) \
            if(token.type != Token::TYPE_STRING) { \
                RETURN_ERROR_TOKEN(token, "Expected a string"); \
            } \
            global.str = std::get<std::string>(token.value);

        // Get the global type
        auto &global_type = first_token[2];
        if(global_type.type != Token::TYPE_STRING) {
            RETURN_ERROR_TOKEN(first_token[2], "Expected a string");
        }
        try {
            std::string global_type_value = std::get<std::string>(global_type.value);
            for(char &c : global_type_value) {
                if(c == ' ') {
                    c = '-';
                }
            }
            global.global_type = ScenarioScriptValueType_from_string(global_type_value.c_str()).value();
        }
        catch(std::exception &) {
            RETURN_ERROR_TOKEN(first_token[2], "Invalid global type");
        }

        // Get the global name
        auto &global_name = first_token[3];
        if(global_name.type != Token::TYPE_STRING) {
            RETURN_ERROR_TOKEN(first_token[3], "Expected a string");
        }
        global.global_name = std::get<std::string>(global_name.value);

        // Get the block
        std::size_t advance_block;
        auto block = get_block(first_token + 4, first_token + advance, advance_block, error, error_line, error_column, error_token, error_message);
        if(error) {
            return std::nullopt;
        }
        global.block = std::move(block.value());

        // Make sure the block has exactly one element in it
        if(global.block.size() != 1) {
            char error_msg[256] = {};
            std::snprintf(error_msg, sizeof(error_msg), "Expected exactly 1 value for global; found %zu", global.block.size());
            RETURN_ERROR_TOKEN(first_token[4], error_msg);
        }

        #undef SET_GLOBAL_STRING_OR_BAIL

        // Initialize and get everything going
        std::optional<Object> r = Object();
        auto &global_object = r.value();
        global_object.type = Object::Type::TYPE_GLOBAL;
        global_object.value = global;
        r->column = first_token->column;
        r->line = first_token->line;

        return r;
    }

    static std::optional<Object::Block> get_block(const Token *first_token, const Token *last_token, std::size_t &advance, bool &error, std::size_t &error_line, std::size_t &error_column, std::string &error_token, std::string &error_message) {
        advance = 0;
        Object::Block block;
        std::size_t max_advance = last_token - first_token;
        for(advance = 0; advance < max_advance; advance++) {
            auto &token = first_token[advance];

            // If we're closing a block, break
            if(token.type == Token::Type::TYPE_PARENTHESIS_CLOSE) {
                advance++;
                break;
            }

            // Function call
            if(token.type == Token::Type::TYPE_PARENTHESIS_OPEN) {
                Object::FunctionCall call;

                // Get the function name?
                auto *function_call = &token + 1;
                auto *function_block = &token + 2;

                // If it ends too soon, bail
                if(function_call >= last_token) {
                    RETURN_ERROR_TOKEN(token, "Expected a function");
                }

                // Make sure the function is a valid function name
                if(function_call->type != Token::Type::TYPE_STRING) {
                    RETURN_ERROR_TOKEN(*function_call, "Expected a string");
                }
                call.function_name = std::get<std::string>(function_call->value);

                // Make sure we've got a block
                std::size_t function_block_advance;
                auto returned_block = get_block(function_block, last_token, function_block_advance, error, error_line, error_column, error_token, error_message);

                // If it failed, darn
                if(error) {
                    return std::nullopt;
                }

                // Get the block
                call.block = returned_block.value();

                // Advance it
                advance += function_block_advance + 1;

                auto &value = block.emplace_back();
                value.value = call;
                value.type = Object::Type::TYPE_FUNCTION_CALL;
                value.column = token.column;
                value.line = token.line;
            }

            else {
                auto &value = block.emplace_back();
                value.value = token;
                value.type = Object::Type::TYPE_TOKEN;
                value.column = token.column;
                value.line = token.line;
            }
        }

        return block;
    }

    #undef RETURN_ERROR_TOKEN

    std::vector<Tokenizer::Token> decompile_script_tree(const std::vector<Object> &script_tree) {
        std::vector<Tokenizer::Token> r;

        auto decompile_object = [&r](const Object &object, auto &decompile_object) -> void {
            auto decompile_block = [&decompile_object] (const Object::Block &block) -> void {
                if(block.size() == 0) {
                    return;
                }
                for(auto &o : block) {
                    decompile_object(o, decompile_object);
                }
            };

            switch(object.type) {
                case Object::Type::TYPE_BLOCK:
                    decompile_block(std::get<Object::Block>(object.value));
                    break;
                case Object::Type::TYPE_TOKEN:
                    r.emplace_back(std::get<Tokenizer::Token>(object.value));
                    break;
                case Object::Type::TYPE_GLOBAL: {
                    auto &lp = r.emplace_back();
                    lp.column = 0;
                    lp.line = 0;
                    lp.type = Tokenizer::Token::Type::TYPE_PARENTHESIS_OPEN;

                    auto &global_literal = r.emplace_back();
                    global_literal.column = 0;
                    global_literal.line = 0;
                    global_literal.raw_value = "global";
                    global_literal.type = Tokenizer::Token::Type::TYPE_STRING;
                    global_literal.value = global_literal.raw_value;

                    auto &global = std::get<Object::Global>(object.value);

                    auto &global_type = r.emplace_back();
                    global_type.column = 0;
                    global_type.line = 0;
                    global_type.type = Tokenizer::Token::Type::TYPE_STRING;
                    global_type.raw_value = ScenarioScriptValueType_to_string(global.global_type);
                    for(char &c : global_type.raw_value) {
                        if(c == '-') {
                            c = '_';
                        }
                    }
                    global_type.value = global_type.raw_value;

                    auto &global_name = r.emplace_back();
                    global_name.column = 0;
                    global_name.line = 0;
                    global_name.raw_value = global.global_name;
                    global_name.type = Tokenizer::Token::Type::TYPE_STRING;
                    global_name.value = global_name.raw_value;

                    decompile_block(global.block);

                    auto &rp = r.emplace_back();
                    rp.column = 0;
                    rp.line = 0;
                    rp.type = Tokenizer::Token::Type::TYPE_PARENTHESIS_CLOSE;
                    break;
                }
                case Object::Type::TYPE_SCRIPT: {
                    auto &script = std::get<Object::Script>(object.value);
                    auto &lp = r.emplace_back();
                    lp.column = 0;
                    lp.line = 0;
                    lp.type = Tokenizer::Token::Type::TYPE_PARENTHESIS_OPEN;

                    auto &script_literal = r.emplace_back();
                    script_literal.column = 0;
                    script_literal.line = 0;
                    script_literal.raw_value = "script";
                    script_literal.type = Tokenizer::Token::Type::TYPE_STRING;
                    script_literal.value = script_literal.raw_value;

                    auto &script_type = r.emplace_back();
                    script_type.column = 0;
                    script_type.line = 0;
                    script_type.raw_value = ScenarioScriptType_to_string(script.script_type);
                    script_type.type = Tokenizer::Token::Type::TYPE_STRING;
                    script_type.value = script_type.raw_value;

                    if(script.script_type == ScenarioScriptType::SCENARIO_SCRIPT_TYPE_STATIC || script.script_type == ScenarioScriptType::SCENARIO_SCRIPT_TYPE_STUB) {
                        auto &script_return_type = r.emplace_back();
                        script_return_type.column = 0;
                        script_return_type.line = 0;
                        script_return_type.type = Tokenizer::Token::Type::TYPE_STRING;
                        script_return_type.raw_value = ScenarioScriptValueType_to_string(script.script_return_type);
                        for(char &c : script_return_type.raw_value) {
                            if(c == '-') {
                                c = '_';
                            }
                        }
                        script_return_type.value = script_return_type.raw_value;
                    }

                    auto &script_name = r.emplace_back();
                    script_name.column = 0;
                    script_name.line = 0;
                    script_name.raw_value = script.script_name;
                    script_name.type = Tokenizer::Token::Type::TYPE_STRING;
                    script_name.value = script_name.raw_value;

                    decompile_block(script.block);

                    auto &rp = r.emplace_back();
                    rp.column = 0;
                    rp.line = 0;
                    rp.type = Tokenizer::Token::Type::TYPE_PARENTHESIS_CLOSE;
                    break;
                }
                case Object::Type::TYPE_FUNCTION_CALL: {
                    auto &lp = r.emplace_back();
                    lp.column = 0;
                    lp.line = 0;
                    lp.type = Tokenizer::Token::Type::TYPE_PARENTHESIS_OPEN;

                    auto &function_call = std::get<Object::FunctionCall>(object.value);
                    auto &function_name = r.emplace_back();
                    function_name.column = 0;
                    function_name.line = 0;
                    function_name.raw_value = function_call.function_name;
                    function_name.type = Tokenizer::Token::Type::TYPE_STRING;
                    function_name.value = function_name.raw_value;

                    decompile_block(function_call.block);

                    auto &rp = r.emplace_back();
                    rp.column = 0;
                    rp.line = 0;
                    rp.type = Tokenizer::Token::Type::TYPE_PARENTHESIS_CLOSE;
                    break;
                }
            }
        };

        for(auto &t : script_tree) {
            decompile_object(t, decompile_object);
        }

        return r;
    }
}
