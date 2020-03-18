// SPDX-License-Identifier: GPL-3.0-only

#ifndef INVADER__BLUDGEON__BLUDGEONER_HPP

namespace Invader::Parser {
    struct ParserStruct;
}

namespace Invader::Bludgeoner {
    bool bullshit_enums(Parser::ParserStruct *s, bool fix);
    bool refinery_model_markers(Parser::ParserStruct *s, bool fix);
    bool sound_buffer(Parser::ParserStruct *s, bool fix);
    bool fucked_vertices(Parser::ParserStruct *s, bool fix);
}

#endif
