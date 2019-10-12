// SPDX-License-Identifier: GPL-3.0-only

#ifndef INVADER__EPRINTF_HPP
#define INVADER__EPRINTF_HPP

#ifndef NO_OUTPUT
#include <cstdio>
#define eprintf(...) std::fprintf(stderr, __VA_ARGS__)
#define oprintf(...) std::fprintf(stdout, __VA_ARGS__)
#else
#define eprintf(...) {}
#define oprintf(...) {}
#endif
#endif
