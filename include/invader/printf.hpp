// SPDX-License-Identifier: GPL-3.0-only

#ifndef INVADER__PRINTF_HPP
#define INVADER__PRINTF_HPP

#ifndef NO_OUTPUT

// If NO_OUTPUT is not enabled, then we have eprintf and oprintf as a macro for, basically, printf
#include <cstdio>
#define eprintf(...) std::fprintf(stderr, __VA_ARGS__)
#define oprintf(...) std::fprintf(stdout, __VA_ARGS__)
#define oflush() std::fflush(stdout)
#else

// Otherwise, we have eprintf and oprintf as an inline, variadic function that does nothing. This is so we don't get any unused variable warnings.
static inline void eprintf(...) {}
static inline void oprintf(...) {}
static inline void oflush() {}
#endif

#endif
