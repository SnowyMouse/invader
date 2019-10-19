// SPDX-License-Identifier: GPL-3.0-only

#include "version.hpp"
#include "hek/data_type.hpp"
#include "printf.hpp"

#ifndef INVADER_VERSION_MAJOR
#define INVADER_VERSION_MAJOR 0
#define INVADER_VERSION_MINOR 0
#define INVADER_VERSION_PATCH 0
#else
#ifndef INVADER_VERSION_STRING
#include "version_str.hpp"
#endif
#endif

#ifndef INVADER_FORK
#define INVADER_FORK "Invader"
#endif

#ifndef INVADER_VERSION_STRING
#define INVADER_VERSION_STRING TOSTR(INVADER_VERSION_MAJOR) "." TOSTR(INVADER_VERSION_MINOR) "." TOSTR(INVADER_VERSION_PATCH) ".unknown"
#endif

#define INVADER_FULL_VERSION_STRING INVADER_FORK " " INVADER_VERSION_STRING

static_assert(sizeof(INVADER_FULL_VERSION_STRING) < sizeof(Invader::HEK::TagString), "version string " INVADER_FULL_VERSION_STRING " too long");

namespace Invader {
    void show_version_info() {
        eprintf(INVADER_FULL_VERSION_STRING "\n\n");
        eprintf("This program is licensed under the GNU General Public License v3.0.\n\n");
        eprintf("Credits:\n");
        eprintf("    Kavawuvi                       - Developer\n");
        eprintf("    MosesofEgypt                   - Helped via Discord\n");
        eprintf("    GoofballMichelle               - Helped via Discord\n");
        eprintf("    Vaporeon                       - Testing\n\n");
        eprintf("Other links:\n");
        eprintf("    Invader source code            - https://github.com/Kavawuvi/Invader\n");
        eprintf("    Mo's Editing Kit               - https://bitbucket.org/Moses_of_Egypt/mek/\n");
    }

    const char *full_version() {
        return INVADER_FULL_VERSION_STRING;
    }
}
