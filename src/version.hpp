/*
 * Invader (c) 2018 Kavawuvi
 *
 * This program is free software under the GNU General Public License v3.0 or later. See LICENSE for more information.
 */

#ifndef INVADER__VERSION_HPP
#define INVADER__VERSION_HPP

#include "eprintf.hpp"
#include "hek/data_type.hpp"

#define TOSTR2(str) # str
#define TOSTR(str) TOSTR2(str)

#ifndef INVADER_VERSION_MAJOR
#define INVADER_VERSION_MAJOR 0
#define INVADER_VERSION_MINOR 0
#define INVADER_VERSION_PATCH 0
#endif

#ifndef INVADER_VERSION_STRING
#define INVADER_VERSION_STRING TOSTR(INVADER_VERSION_MAJOR) "." TOSTR(INVADER_VERSION_MINOR) "." TOSTR(INVADER_VERSION_PATCH)
#endif

static_assert(sizeof(INVADER_VERSION_STRING) < sizeof(Invader::HEK::TagString), "version string too long");

#define INVADER_SHOW_INFO eprintf("Invader Version " INVADER_VERSION_STRING "\n\n"); \
                          eprintf("This program is licensed under the GNU General Public License v3.0 or later.\n\n"); \
                          eprintf("Credits:\n"); \
                          eprintf("    Kavawuvi                       - Developer\n"); \
                          eprintf("    MosesofEgypt                   - Helped via Discord\n"); \
                          eprintf("    GoofballMichelle               - Helped via Discord\n"); \
                          eprintf("    Vaporeon                       - Testing\n\n"); \
                          eprintf("Other links:\n"); \
                          eprintf("    Invader source code            - https://github.com/Kavawuvi/Invader\n"); \
                          eprintf("    Mo's Editing Kit               - https://bitbucket.org/Moses_of_Egypt/mek/\n");
#endif
