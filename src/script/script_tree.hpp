// SPDX-License-Identifier: GPL-3.0-only

#ifndef INVADER__SCRIPT__SCRIPT_TREE_HPP
#define INVADER__SCRIPT__SCRIPT_TREE_HPP

#include "tokenizer.hpp"

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
                TYPE_VALUE,
                TYPE_GLOBAL,
                TYPE_SCRIPT
            };

            using ScriptBlock = std::vector<Object>;

            using Value = std::variant<
                ScriptBlock,
                Tokenizer::Token,
                std::string
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
