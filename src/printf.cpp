// SPDX-License-Identifier: GPL-3.0-only

#include <invader/printf.hpp>

#include <fcntl.h>
#include <io.h>
#include <stdio.h>

namespace Invader {
    void setup_output() {
        // Reopen in binary mode (improves performance on Windows)
        #ifdef _WIN32
        _setmode(_fileno(stdout), _O_BINARY);
        _setmode(_fileno(stderr), _O_BINARY);
        #endif
    }
}
