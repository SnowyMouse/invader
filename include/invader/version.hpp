// SPDX-License-Identifier: GPL-3.0-only

#ifndef INVADER__VERSION_HPP
#define INVADER__VERSION_HPP

#define TOSTR2(str) # str
#define TOSTR(str) TOSTR2(str)

namespace Invader {
    /**
     * Print version and credits information to standard output
     */
    void show_version_info();

    /**
     * Get the full formatted version of invader
     * @return full formatted version of invader
     */
    const char *full_version();

    /**
     * Get the full version and credits of Invader
     * @return full version and credits
     */
    const char *full_version_and_credits();
}

#ifdef INVADER_EXTRACT_HIDDEN_VALUES
#define EXIT_IF_INVADER_EXTRACT_HIDDEN_VALUES eprintf_error("Error: Invader was compiled with INVADER_EXTRACT_HIDDEN_VALUES."); eprintf_error("This program cannot be used."); std::exit(EXIT_FAILURE);
#else
#define EXIT_IF_INVADER_EXTRACT_HIDDEN_VALUES
#endif

#endif
