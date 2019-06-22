/*
 * Invader (c) 2018 Kavawuvi
 *
 * This program is free software under the GNU General Public License v3.0 or later. See LICENSE for more information.
 */

#ifndef INVADER__VERSION_HPP
#define INVADER__VERSION_HPP

#define TOSTR2(str) # str
#define TOSTR(str) TOSTR2(str)
#define INVADER_VERSION_MAJOR 0
#define INVADER_VERSION_MINOR 1
#define INVADER_VERSION_BUILD 0
#define INVADER_VERSION_STRING TOSTR(INVADER_VERSION_MAJOR) "." TOSTR(INVADER_VERSION_MINOR) "." TOSTR(INVADER_VERSION_BUILD)

#define INVADER_SHOW_INFO std::cout << "Invader Version " INVADER_VERSION_STRING                                          << std::endl; \
                          std::cout                                                                                       << std::endl; \
                          std::cout << "This program is licensed under the GNU General Public License v3.0 or later."     << std::endl; \
                          std::cout                                                                                       << std::endl; \
                          std::cout << "Credits:"                                                                         << std::endl; \
                          std::cout << "    Kavawuvi                       - Developer"                                   << std::endl; \
                          std::cout << "    MosesofEgypt                   - Helped via Discord"                          << std::endl; \
                          std::cout << "    GoofballMichelle               - Helped via Discord"                          << std::endl; \
                          std::cout << "    Vaporeon                       - Testing"                                     << std::endl; \
                          std::cout                                                                                       << std::endl; \
                          std::cout << "Other links:"                                                                     << std::endl; \
                          std::cout << "    Invader source code            - https://github.com/Kavawuvi/Invader      "   << std::endl; \
                          std::cout << "    Mo's Editing Kit               - https://bitbucket.org/Moses_of_Egypt/mek/"   << std::endl;
#endif
