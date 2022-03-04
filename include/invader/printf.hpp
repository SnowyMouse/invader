// SPDX-License-Identifier: GPL-3.0-only

#ifndef INVADER__PRINTF_HPP
#define INVADER__PRINTF_HPP

#define ON_COLOR_TERM(fd) (is_on_color_term())

void set_up_color_term() noexcept;
bool is_on_color_term() noexcept;

#include <unistd.h>
#include <cstdlib>
#include <cstring>
#include <cstdio>

#define eprintf(...) std::fprintf(stderr, __VA_ARGS__)
#define oprintf(...) std::fprintf(stdout, __VA_ARGS__)
#define oflush(...) std::fflush(stdout)

#define eprintf_error(...) if(ON_COLOR_TERM(stderr)) {\
    eprintf("\x1B[1;38;5;1m"); \
    eprintf(__VA_ARGS__); \
    eprintf("\x1B[m\n"); \
} \
else {\
    eprintf(__VA_ARGS__); \
    eprintf("\n"); \
}

#define eprintf_warn(...) if(ON_COLOR_TERM(stderr)) {\
    eprintf("\x1B[1;38;5;3m"); \
    eprintf(__VA_ARGS__); \
    eprintf("\x1B[m\n"); \
} \
else {\
    eprintf(__VA_ARGS__); \
    eprintf("\n"); \
}

#define eprintf_warn_lesser(...) if(ON_COLOR_TERM(stderr)) {\
    eprintf("\x1B[1;38;5;5m"); \
    eprintf(__VA_ARGS__); \
    eprintf("\x1B[m\n"); \
} \
else {\
    eprintf(__VA_ARGS__); \
    eprintf("\n"); \
}

#define oprintf_success(...) if(ON_COLOR_TERM(stdout)) {\
    oprintf("\x1B[38;5;2m"); \
    oprintf(__VA_ARGS__); \
    oprintf("\x1B[m\n"); \
} \
else {\
    oprintf(__VA_ARGS__); \
    oprintf("\n"); \
}

#define oprintf_success_warn(...) if(ON_COLOR_TERM(stdout)) {\
    oprintf("\x1B[1;38;5;3m"); \
    oprintf(__VA_ARGS__); \
    oprintf("\x1B[m\n"); \
} \
else {\
    oprintf(__VA_ARGS__); \
    oprintf("\n"); \
}
#define oprintf_success_lesser_warn(...) if(ON_COLOR_TERM(stdout)) {\
    oprintf("\x1B[1;38;5;5m"); \
    oprintf(__VA_ARGS__); \
    oprintf("\x1B[m\n"); \
} \
else {\
    oprintf(__VA_ARGS__); \
    oprintf("\n"); \
}
#define oprintf_fail(...) if(ON_COLOR_TERM(stdout)) {\
    oprintf("\x1B[1;38;5;1m"); \
    oprintf(__VA_ARGS__); \
    oprintf("\x1B[m\n"); \
} \
else {\
    oprintf(__VA_ARGS__); \
    oprintf("\n"); \
}

#endif
