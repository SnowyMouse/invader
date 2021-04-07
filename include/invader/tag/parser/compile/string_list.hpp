// SPDX-License-Identifier: GPL-3.0-only

#ifndef INVADER__TAG__PARSER__COMPILE__STRING_LIST_HPP
#define INVADER__TAG__PARSER__COMPILE__STRING_LIST_HPP

namespace Invader::Parser {
    struct StringList;
    struct UnicodeStringList;

    /**
     * Detect and/or repair invalid string lists
     * @param  list list to attempt to fix
     * @return      true if the list had invalid strings
     */
    bool fix_broken_strings(StringList &list, bool fix);

    /**
     * Detect and/or repair invalid string lists
     * @param  list list to attempt to fix
     * @return      true if the list had invalid strings
     */
    bool fix_broken_strings(UnicodeStringList &list, bool fix);
    
    
    /**
     * Detect if the string list has broken strings
     * @param list list to check
     * @return     true if the list has invalid strings
     */
    bool check_for_broken_strings(const StringList &list);
    
    /**
     * Detect if the string list has broken strings
     * @param list list to check
     * @return     true if the list has invalid strings
     */
    bool check_for_broken_strings(const UnicodeStringList &list);
}

#endif
