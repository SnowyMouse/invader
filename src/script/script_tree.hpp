// SPDX-License-Identifier: GPL-3.0-only

#ifndef INVADER__SCRIPT__SCRIPT_TREE_HPP
#define INVADER__SCRIPT__SCRIPT_TREE_HPP

#include "tokenizer.hpp"
#include "../tag/hek/class/scenario.hpp"

namespace Invader {
    /**
     * Script tree library class
     */
    class ScriptTree {
    public:
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
                HEK::ScenarioScriptValueType global_type;
                Block block;
            };

            struct FunctionCall {
                std::string function_name;
                Block block;
            };

            struct Script {
                std::string script_name;
                std::string script_type;
                HEK::ScenarioScriptValueType script_return_type;
                Block block;
            };

            using Value = std::variant<
                Block,
                Tokenizer::Token,
                Global,
                Script,
                FunctionCall
            >;

            Type type;
            Value value;
        };

        static std::vector<Object> compile_script_tree(const std::vector<Tokenizer::Token> &tokens, bool &error, std::size_t &error_line, std::size_t &error_column, std::string &error_token, std::string &error_message);

    private:
        ScriptTree() = default;
    };
}

#endif
