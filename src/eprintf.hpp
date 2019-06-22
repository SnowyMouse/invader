/*
 * Invader (c) 2018 Kavawuvi
 *
 * This program is free software under the GNU General Public License v3.0 or later. See LICENSE for more information.
 */

#ifndef INVADER__EPRINTF_HPP
#define INVADER__EPRINTF_HPP

#ifndef NO_OUTPUT
#include <cstdio>
#define eprintf(...) std::fprintf(stderr, __VA_ARGS__)
#else
#define eprintf(...) {}
#endif
#endif
