// SPDX-License-Identifier: GPL-3.0-only

#ifndef INVADER__SCRIPT__TOKENIZER_HPP
#define INVADER__SCRIPT__TOKENIZER_HPP

#include <vector>
#include <string>
#include <variant>

namespace Invader {
    /**
     * Tokenizer library class
     */
    class Tokenizer {
    public:
        /**
         * Struct to hold a raw token from tokenize()
         */
        struct Token {
            enum Type {
                TOKEN_TYPE_STRING,
                TOKEN_TYPE_INTEGER,
                TOKEN_TYPE_DECIMAL,
                TOKEN_TYPE_PARENTHESIS_OPEN,
                TOKEN_TYPE_PARENTHESIS_CLOSE
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
        };

        /**
         * Split up a string into HSC tokens and return them
         * @param string       string to interrogate
         * @param error        will be set to true if an error occured or false if an error didn't occur
         * @param error_line   if an error occured, this is the line of the token
         * @param error_column if an error occured, this is the column of the token
         * @param error_token  if an error occured, this is a copy of the invalid
         */
        static std::vector<Token> tokenize(const char *string, bool &error, std::size_t &error_line, std::size_t &error_column, std::string &error_token);

    private:
        Tokenizer() = default;
    };
}

#endif
