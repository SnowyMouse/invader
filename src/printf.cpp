// SPDX-License-Identifier: GPL-3.0-only

#include <invader/printf.hpp>

namespace Invader {
    void setup_output() {
        // Reopen in binary mode (improves performance on Windows)
        #ifdef _WIN32
        setmode(fileno(stdout), O_BINARY);
        setmode(fileno(stderr), O_BINARY);
        #endif
    }
}
