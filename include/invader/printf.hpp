// SPDX-License-Identifier: GPL-3.0-only

#ifndef INVADER__PRINTF_HPP
#define INVADER__PRINTF_HPP

#ifndef NO_OUTPUT

#include <cstdlib>

// If NO_OUTPUT is not enabled, then we have eprintf and oprintf as a macro for, basically, printf
#include <cstdio>
#define ON_COLOR_TERM (std::strcmp(std::getenv("TERM"), "xterm-256color") == 0 || std::strcmp(std::getenv("TERM"), "xterm-color") == 0 || std::strcmp(std::getenv("TERM"), "xterm-16color") == 0)
#define eprintf(...) std::fprintf(stderr, __VA_ARGS__)
#define eprintf_error(...) if(ON_COLOR_TERM) {\
    std::fprintf(stderr, "\x1B[1;38;5;1m"); \
    std::fprintf(stderr, __VA_ARGS__); \
    std::fprintf(stderr, "\x1B[m\n"); \
} \
else {\
    std::fprintf(stderr, __VA_ARGS__); \
    std::fprintf(stderr, "\n"); \
}
#define eprintf_warn(...) if(ON_COLOR_TERM) {\
    std::fprintf(stderr, "\x1B[1;38;5;3m"); \
    std::fprintf(stderr, __VA_ARGS__); \
    std::fprintf(stderr, "\x1B[m\n"); \
} \
else {\
    std::fprintf(stderr, __VA_ARGS__); \
    std::fprintf(stderr, "\n"); \
}
#define oprintf(...) std::fprintf(stdout, __VA_ARGS__)
#define oprintf_success(...) if(ON_COLOR_TERM) {\
    std::fprintf(stdout, "\x1B[38;5;2m"); \
    std::fprintf(stdout, __VA_ARGS__); \
    std::fprintf(stdout, "\x1B[m\n"); \
} \
else {\
    std::fprintf(stdout, __VA_ARGS__); \
    std::fprintf(stdout, "\n"); \
}
#define oflush() std::fflush(stdout)

#else

// Otherwise, we have eprintf and oprintf as an inline, variadic function that does nothing. This is so we don't get any unused variable warnings.
static inline void eprintf(...) {}
static inline void eprintf_error(...) {}
static inline void eprintf_warn(...) {}
static inline void oprintf(...) {}
static inline void oflush() {}
#endif

#endif
