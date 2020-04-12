// SPDX-License-Identifier: GPL-3.0-only

#ifndef INVADER__TAG__PARSER__COMPILE__FONT_HPP
#define INVADER__TAG__PARSER__COMPILE__FONT_HPP

namespace Invader::Parser {
    struct Font;
    
    /**
     * Generate character tables
     * @param font font tag to generate character tables
     */
    void generate_character_tables(Font &font);
}

#endif
