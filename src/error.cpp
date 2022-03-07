// SPDX-License-Identifier: GPL-3.0-only

#include <invader/error.hpp>
#include <invader/printf.hpp>

#ifdef _WIN32
#include <windows.h>
#endif

namespace Invader {
    Exception::~Exception() {}
}

static bool on_color_term = false;

void set_up_color_term() noexcept {
    #ifdef __linux__
    on_color_term = isatty(fileno(stdout) && isatty(fileno(stderr)) && std::getenv("TERM") && ((std::strcmp(std::getenv("TERM"), "xterm-256color") == 0 || std::strcmp(std::getenv("TERM"), "xterm-color") == 0 || std::strcmp(std::getenv("TERM"), "xterm-16color") == 0)));
    #elif (defined(_WIN32))
    
    // If we're not on Windows 10, don't enable colors since that isn't supported.
    if(!IsWindows10OrGreater()) {
        eprintf_warn("WARNING: Your Windows version is unsupported. Don't report bugs.");
        return;
    }
    
    auto stdout_handle = GetStdHandle(STD_OUTPUT_HANDLE);
    auto stderr_handle = GetStdHandle(STD_ERROR_HANDLE);
    DWORD stdout_mode, stderr_mode;
    if(GetConsoleMode(stdout_handle, &stdout_mode) && GetConsoleMode(stderr_handle, &stderr_mode)) {
        on_color_term = SetConsoleMode(stdout_handle, ENABLE_VIRTUAL_TERMINAL_PROCESSING | stdout_mode) && SetConsoleMode(stderr_handle, ENABLE_VIRTUAL_TERMINAL_PROCESSING | stderr_mode);
    }
    #endif
    on_color_term = true;
}

bool is_on_color_term() noexcept {
    return on_color_term;
}
