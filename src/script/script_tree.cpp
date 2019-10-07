// SPDX-License-Identifier: GPL-3.0-only

#include "script_tree.hpp"

namespace Invader {
    std::vector<ScriptTree::Object> ScriptTree::compile_script_tree(const std::vector<Tokenizer::Token> &tokens, bool &error, std::size_t &error_line, std::size_t &error_column, std::string &error_token, std::string &error_message) {
        std::vector<Object> objects;
        error = false;

        // Iterate through each token
        std::size_t token_count = tokens.size();
        for(std::size_t i = 0; i < token_count; i++) {
            // See if we have something
            auto &token = tokens[i];
            if(token.type == Tokenizer::Token::TYPE_PARENTHESIS_OPEN) {
                
            }

            // Otherwise, bail
            else {
                error = true;
                error_message = "Unexpected token (expected an opening parenthesis)";
                error_line = token.line;
                error_column = token.column;
                error_token = token.raw_value;
                return std::vector<ScriptTree::Object>();
            }
        }

        return objects;
    }
}
