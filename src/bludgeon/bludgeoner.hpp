// SPDX-License-Identifier: GPL-3.0-only

#ifndef INVADER__BLUDGEON__BLUDGEONER_HPP
#define INVADER__BLUDGEON__BLUDGEONER_HPP

namespace Invader::Parser {
    struct ParserStruct;
}

namespace Invader::Bludgeoner {
    bool broken_enums(Parser::ParserStruct *s, bool fix);
    bool invalid_model_markers(Parser::ParserStruct *s, bool fix);
    bool sound_buffer(Parser::ParserStruct *s, bool fix);
    bool broken_vertices(Parser::ParserStruct *s, bool fix);
    bool broken_references(Parser::ParserStruct *s, bool fix);
    bool uppercase_references(Parser::ParserStruct *s, bool fix);
    bool broken_range_fix(Parser::ParserStruct *s, bool fix);
    bool missing_scripts(Parser::ParserStruct *s, bool fix);
    bool broken_indices_fix(Parser::ParserStruct *s, bool fix);
    bool broken_normals(Parser::ParserStruct *s, bool fix);
    bool broken_strings(Parser::ParserStruct *s, bool fix);
    bool excessive_script_nodes(Parser::ParserStruct *s, bool fix);
}

#endif
