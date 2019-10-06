// SPDX-License-Identifier: GPL-3.0-only

#ifndef INVADER__SCRIPT__TOKENIZER_HPP
#define INVADER__SCRIPT__TOKENIZER_HPP

#include <vector>
#include <string>

namespace Invader {
    /**
     * Tokenizer library class
     */
    class Tokenizer {
    public:
        /**
         * Struct to hold a token
         */
        struct RawToken {
            std::string token;
            std::size_t line;
            std::size_t column;
        };

        /**
         * Split up a string into HSC tokens and return them
         * @param string       string to interrogate
         * @param error        will be set to true if an error occured due to a token not being finished or false if an error didn't occur
         * @param error_line   if an error occured, this is the line of the token
         * @param error_column if an error occured, this is the column of the token
         * @param error_token  if an error occured, this is a copy of the incomplete token
         */
        static std::vector<RawToken> tokenize(const char *string, bool &error, std::size_t &error_line, std::size_t &error_column, const char *&error_token);

    private:
        Tokenizer() = default;
    };
}

#endif
