// SPDX-License-Identifier: GPL-3.0-only

#ifndef INVADER__TAG__PARSER__COMPILE__OBJECT_HPP
#define INVADER__TAG__PARSER__COMPILE__OBJECT_HPP

namespace Invader::Parser {
    class ParserStruct;
    
    /**
     * Convert an object from one type to another
     * @param  from struct to convert from
     * @param  fix  struct to write changes to
     */
    void convert_object(const ParserStruct &from, ParserStruct &to);
}

#endif
