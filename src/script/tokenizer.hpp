// SPDX-License-Identifier: GPL-3.0-only

#ifndef INVADER__SCRIPT__TOKENIZER_HPP
#define INVADER__SCRIPT__TOKENIZER_HPP

#include <vector>
#include <string>
#include <variant>

namespace Invader::Tokenizer {
    /**
     * Struct to hold a raw token from tokenize()
     */
    struct Token {
        enum Type {
            TYPE_STRING,
            TYPE_INTEGER,
            TYPE_DECIMAL,
            TYPE_PARENTHESIS_OPEN,
            TYPE_PARENTHESIS_CLOSE
        };

        using Value = std::variant<
            std::string,
            std::int32_t,
            float
        >;

        Value value;
        std::size_t line;
        std::size_t column;
        Type type;
        std::string raw_value;
    };

    /**
     * Split up a string into HSC tokens and return them
     * @param string        string to interrogate
     * @param error         will be set to true if an error occured or false if an error didn't occur
     * @param error_line    if an error occurred, this is the line of the token
     * @param error_column  if an error occurred, this is the column of the token
     * @param error_token   if an error occurred, this is a copy of the invalid
     * @param error_message if an error occurred, this is the message
     */
    std::vector<Token> tokenize(const char *string, bool &error, std::size_t &error_line, std::size_t &error_column, std::string &error_token, std::string &error_message);
}

#endif
