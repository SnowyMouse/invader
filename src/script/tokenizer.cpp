// SPDX-License-Identifier: GPL-3.0-only

#include "tokenizer.hpp"
#include <optional>

namespace Invader {
    std::vector<Tokenizer::Token> Tokenizer::tokenize(const char *string, bool &error, std::size_t &error_line, std::size_t &error_column, const char *&error_token) {
        // Token start
        const char *token_start = nullptr;
        std::size_t token_start_line = 0;
        std::size_t token_start_column = 0;

        // The expected terminator of the token
        char expected_end = 0;

        std::vector<Token> tokens;

        // Go through each character until we hit a null terminator, then stop
        std::size_t line = 1;
        std::size_t column = 1;
        for(const char *c = string; *c; c++) {
            // Is it whitespace?
            bool whitespace = *c == ' ' || *c == '\t' || *c == '\r' || *c == '\n';

            // Is it a ( or ), and are we not in a string?
            bool parenthesis = (*c == '(' || *c == ')') && expected_end != '"';
            bool parenthesis_killed_the_radio_star = parenthesis && token_start;

            // Are we continuing a token?
            if(token_start != nullptr) {
                // Are we breaking a token?
                if((expected_end == ' ' && whitespace) || expected_end == *c || parenthesis) {
                    std::size_t length = c - token_start;
                    auto &token = tokens.emplace_back();
                    token.line = token_start_line;
                    token.column = token_start_column;
                    token.token = std::string(c, length);
                    expected_end = 0;
                    token_start = nullptr;
                }
            }

            // Otherwise, if it isn't a whitespace, we're starting a token
            else if(!whitespace) {
                // If it's a parenthesis, it's only one character long
                if(parenthesis) {
                    auto &token = tokens.emplace_back();
                    token.line = line;
                    token.column = column;
                    token.token = std::string(c, 1);
                }

                // If it's the start of a comment, end the line
                else if(*c == ';') {
                    while(*c != '\n') {
                        c++;
                    }
                }

                // Otherwise it's the start of a token
                else {
                    token_start = c;
                    if(*c == '"') {
                        expected_end = '"';
                    }
                    else {
                        expected_end = ' ';
                    }
                    token_start_column = column;
                    token_start_line = line;
                }
            }

            // If it's parenthesis, let's redo this
            if(parenthesis_killed_the_radio_star) {
                c--;
                continue;
            }

            // Increment the line and column counter
            if(*c == '\n') {
                line++;
                column = 1;
            }
            else {
                column++;
            }
        }

        // if we're ending while still in the middle of a token, then error
        if(token_start) {
            error = true;
            error_line = token_start_line;
            error_column = token_start_column;
            error_token = token_start;
        }
        else {
            error = false;
        }

        return tokens;
    }
}
