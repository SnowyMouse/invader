// SPDX-License-Identifier: GPL-3.0-only

#include "tokenizer.hpp"
#include <optional>
#include <sstream>

namespace Invader::Tokenizer {
    std::vector<Token> tokenize(const char *string, bool &error, std::size_t &error_line, std::size_t &error_column, std::string &error_token, std::string &error_message) {
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
                    // Get the token string
                    std::size_t length = c - token_start;
                    std::string raw_string = std::string(token_start, length);

                    // Set these so we can determine if a string is numeric or not
                    bool numeric = true;
                    bool decimal = false;

                    // Start adding the token
                    auto &token = tokens.emplace_back();
                    token.line = token_start_line;
                    token.column = token_start_column;
                    expected_end = 0;
                    token_start = nullptr;

                    // Get everything in the string
                    std::string s;
                    bool quoted = raw_string[0] == '"';
                    for(char &c : raw_string) {
                        // If it's not a quoted string and we have a quote, that's bad
                        if(!quoted && c == '"') {
                            error = true;
                            error_line = token.line;
                            error_column = token.column;
                            error_token = raw_string;
                            error_message = "Unexpected quotation mark";
                            return std::vector<Token>();
                        }

                        // Otherwise business as usual
                        else {
                            // Check to see if we are making something not numeric
                            if((c < '0' || c > '9') && (c != '-' || s.size() > 0)) {
                                // If it's a decimal, we aren't a decimal already, and the next character isn't a null terminator, set decimal to true
                                if(c == '.' && *(&c + 1) != 0 && decimal == false) {
                                    decimal = true;
                                }
                                else {
                                    numeric = false;
                                }
                            }
                            s += c;
                        }
                    }

                    token.raw_value = raw_string;

                    // Remove the parenthesis if needed
                    if(quoted) {
                        s = s.substr(1, s.size() - 1);
                    }

                    // If it's non-numeric, set the value as is
                    if(!numeric) {
                        token.value = s;
                        token.type = Token::Type::TYPE_STRING;
                    }

                    // If it's a decimal, let's see it
                    else if(decimal) {
                        token.value = static_cast<float>(std::strtof(s.data(), nullptr));
                        token.type = Token::Type::TYPE_DECIMAL;
                    }

                    // Otherwise, if it's an integer, let's see it
                    else {
                        token.value = static_cast<std::int32_t>(std::strtol(s.data(), nullptr, 10));
                        token.type = Token::Type::TYPE_INTEGER;
                    }
                }
            }

            // Otherwise, if it isn't a whitespace, we're starting a token
            else if(!whitespace) {
                // If it's a parenthesis, it's only one character long
                if(parenthesis) {
                    auto &token = tokens.emplace_back();
                    token.line = line;
                    token.column = column;
                    token.type = *c == '(' ? Token::Type::TYPE_PARENTHESIS_OPEN : Token::Type::TYPE_PARENTHESIS_CLOSE;
                    token.raw_value = "(";
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
            error_message = "Unterminated token";
            return std::vector<Token>();
        }
        else {
            error = false;
        }

        return tokens;
    }

    std::string detokenize(const std::vector<Token> &tokens) {
        std::string string_data;
        bool add_spacing = false;
        std::size_t token_count = tokens.size();
        std::vector<std::string> indent_stack;
        int indent_total = 0;
        std::size_t override_spacing = 0;

        // Go through each token
        for(std::size_t i = 0; i < token_count; i++) {
            std::size_t indent_stack_size = indent_stack.size();
            std::string indentation = (indent_stack_size > 0) ? indent_stack[indent_stack_size - 1] : " ";
            if(override_spacing) {
                override_spacing--;
                indentation = " ";
            }
            auto &t = tokens[i];
            switch(t.type) {
                case Token::Type::TYPE_STRING: {
                    if(add_spacing) {
                        string_data += indentation;
                    }

                    bool contains_space = false;
                    for(const char &c : std::get<std::string>(t.value)) {
                        if(c == ' ') {
                            contains_space = true;
                            break;
                        }
                    }

                    // If the string contains spaces, include those, escaping any and all quotes
                    if(contains_space) {
                        string_data += "\"";
                        for(const char &c : std::get<std::string>(t.value)) {
                            char b[2] = {};
                            std::snprintf(b, sizeof(b), "%c", c);
                            string_data += b;
                        }
                        string_data += "\"";
                    }
                    else {
                        string_data += std::get<std::string>(t.value).c_str();
                    }
                    add_spacing = true;
                    break;
                }

                case Token::Type::TYPE_INTEGER:
                    if(add_spacing) {
                        string_data += indentation;
                    }

                    // Print an int
                    char buffer[256];
                    std::snprintf(buffer, sizeof(buffer), "%li", static_cast<long>(std::get<std::int32_t>(t.value)));
                    string_data += buffer;
                    add_spacing = true;
                    break;

                case Token::Type::TYPE_DECIMAL: {
                    if(add_spacing) {
                        string_data += indentation;
                    }

                    // Print a decimal
                    std::ostringstream ss;
                    ss << std::get<float>(t.value);
                    std::string s(ss.str());
                    add_spacing = true;
                    break;
                }

                case Token::Type::TYPE_PARENTHESIS_OPEN: {
                    if(add_spacing) {
                        string_data += indentation;
                    }
                    string_data += "(";

                    // Determine if we need a newline or not. This is for readability
                    bool found = false;
                    std::size_t open = 1;
                    for(std::size_t q = i + 1; q < token_count; q++) {
                        if(tokens[q].type == Token::Type::TYPE_PARENTHESIS_CLOSE) {
                            open--;
                            if(open == 0) {
                                bool should_indent = (q - i) > 8;
                                found = true;

                                if(should_indent) {
                                    indent_total++;
                                    indentation = "\n";
                                    for(int i = 0; i < indent_total; i++) {
                                        indentation += "    ";
                                    }
                                    indent_stack.push_back(indentation);

                                    // If it's a script or global, we should make sure the "script" or "global" metadata is on the same line as the (
                                    if(token_count - (i + 1) > 4 && indent_total == 1) {
                                        auto &first = tokens[i + 1];
                                        auto &second = tokens[i + 2];
                                        auto &third = tokens[i + 3];
                                        auto &fourth = tokens[i + 4];

                                        bool is_script = first.type == Token::Type::TYPE_STRING && std::get<std::string>(first.value) == "script";
                                        bool is_static = is_script && second.type == Token::Type::TYPE_STRING && std::get<std::string>(second.value) == "static";

                                        if(is_script && second.type == Token::Type::TYPE_STRING && third.type == Token::Type::TYPE_STRING && (!is_static || fourth.type == Token::Type::TYPE_STRING)) {
                                            override_spacing = is_static ? 4 : 3;
                                        }

                                        bool is_global = first.type == Token::Type::TYPE_STRING && std::get<std::string>(first.value) == "global";
                                        if(is_global && second.type == Token::Type::TYPE_STRING && third.type == Token::Type::TYPE_STRING) {
                                            override_spacing = 3;
                                        }
                                    }
                                }
                                else {
                                    indentation = " ";
                                    indent_stack.push_back(indentation);
                                }

                                add_spacing = false;

                                break;
                            }
                        }
                        else if(tokens[q].type == Token::Type::TYPE_PARENTHESIS_OPEN) {
                            open++;
                        }
                    }

                    if(!found) {
                        indent_stack.push_back(" ");
                        add_spacing = false;
                    }

                    break;
                }

                case Token::Type::TYPE_PARENTHESIS_CLOSE: {
                    if(indent_stack_size) {
                        indent_stack.erase(indent_stack.begin() + (indent_stack_size - 1));
                    }
                    if(indentation != " ") {
                        indent_total--;
                        string_data += "\n";
                        for(int a = 0; a < indent_total; a++) {
                            string_data += "    ";
                        }
                    }
                    string_data += ")";
                    if(indent_total == 0) {
                        string_data += "\n";
                        if(indentation != " ") {
                            string_data += "\n";
                        }
                        add_spacing = false;
                    }
                    break;
                }
            }
        }

        // Remove excess newlines
        while(string_data.size() > 4 && string_data[string_data.size() - 1] == '\n' && string_data[string_data.size() - 2] == '\n') {
            string_data.erase(string_data.size() - 1, 1);
        }

        return string_data;
    }
}
