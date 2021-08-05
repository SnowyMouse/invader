// SPDX-License-Identifier: GPL-3.0-only

#ifndef INVADER__SCRIPT__SCRIPT_TREE_HPP
#define INVADER__SCRIPT__SCRIPT_TREE_HPP

#include "../tag/parser/definition/scenario.hpp"
#include "tokenizer.hpp"

namespace Invader::ScriptTree {
    /**
     * Struct to hold an object in a script
     */
    struct Object {
        enum Type {
            TYPE_BLOCK,
            TYPE_TOKEN,
            TYPE_GLOBAL,
            TYPE_SCRIPT,
            TYPE_FUNCTION_CALL
        };

        using Block = std::vector<Object>;

        struct Global {
            std::string global_name;
            Parser::ScenarioScriptValueType global_type;
            Block block;
        };

        struct FunctionCall {
            std::string function_name;
            Block block;
        };

        struct Script {
            std::string script_name;
            Parser::ScenarioScriptType script_type;
            Parser::ScenarioScriptValueType script_return_type;
            Block block;
        };

        using Value = std::variant<
            Block,
            Tokenizer::Token,
            Global,
            Script,
            FunctionCall
        >;

        Type type = {};
        Value value = {};
        std::size_t line = 0;
        std::size_t column = 0;
    };

    /**
     * Compile a syntax tree from tokens
     * @param  tokens        token vector
     * @param  error         if an error occurs, this will be set to true
     * @param  error_line    if an error occurs, this will be set to the line of the token that caused an error
     * @param  error_column  if an error occurs, this will be set to the column of the token that caused an error
     * @param  error_token   if an error occurs, this will be set to the token string value
     * @param  error_message if an error occurs, this will be the error message
     * @return               vector of objects
     */
    std::vector<Object> compile_tokens(const std::vector<Tokenizer::Token> &tokens, bool &error, std::size_t &error_line, std::size_t &error_column, std::string &error_token, std::string &error_message);

    /**
     * Decompile a script tree to tokens
     * @param script_tree script tree to decompile
     */
    std::vector<Tokenizer::Token> decompile_script_tree(const std::vector<Object> &script_tree);
}

#endif
