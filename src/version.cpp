// SPDX-License-Identifier: GPL-3.0-only

#include <invader/version.hpp>
#include <invader/hek/data_type.hpp>
#include <invader/printf.hpp>

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
        oprintf(INVADER_FULL_VERSION_STRING "\n\n");
        oprintf("This program is licensed under the GNU General Public License v3.0.\n");
        oprintf("\n");
        oprintf("Credits:\n");
        oprintf("  Kavawuvi                       - Lead Developer, Project owner\n");
        oprintf("  Vaporeon                       - Testing & QA, AUR maintainer, Development\n");
        oprintf("\n");
        oprintf("Special thanks:\n");
        oprintf("  MosesofEgypt                   - Lots of help with tag data and model geometry\n");
        oprintf("  Tucker933                      - Hosting @ https://invader.opencarnage.net\n");
        oprintf("\n");
        oprintf("Software used in this program:\n");
        oprintf("  FreeType                       - https://www.freetype.org/\n");
        oprintf("  STB library                    - https://github.com/nothings/stb\n");
        oprintf("\n");
        oprintf("Other links:\n");
        oprintf("  Invader source code            - https://github.com/Kavawuvi/Invader\n");
        oprintf("  Mo's Editing Kit               - https://bitbucket.org/Moses_of_Egypt/mek/\n");
    }

    const char *full_version() {
        return INVADER_FULL_VERSION_STRING;
    }
}
