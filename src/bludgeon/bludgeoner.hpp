// SPDX-License-Identifier: GPL-3.0-only

#ifndef INVADER__BLUDGEON__BLUDGEONER_HPP
#define INVADER__BLUDGEON__BLUDGEONER_HPP

namespace Invader::Parser {
    struct ParserStruct;
}

namespace Invader::Bludgeoner {
    bool bullshit_enums(Parser::ParserStruct *s, bool fix);
    bool refinery_model_markers(Parser::ParserStruct *s, bool fix);
    bool sound_buffer(Parser::ParserStruct *s, bool fix);
    bool fucked_vertices(Parser::ParserStruct *s, bool fix);
    bool bullshit_references(Parser::ParserStruct *s, bool fix);
    bool power_of_two_fix(Parser::ParserStruct *s, bool fix);
    bool bullshit_range_fix(Parser::ParserStruct *s, bool fix);
    bool where_the_fuck_are_the_scripts(Parser::ParserStruct *s, bool fix);
}

#endif
