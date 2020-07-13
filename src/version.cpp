// SPDX-License-Identifier: GPL-3.0-only

#include <invader/hek/data_type.hpp>
#include <invader/printf.hpp>

#include <invader/version.hpp>
#include "version.h"

namespace Invader {
    void show_version_info() {
        oprintf("%s", full_version_and_credits());
    }

    const char *full_version() {
        return INVADER_FULL_VERSION_STRING;
    }

    const char *full_version_and_credits() {
        return INVADER_FULL_VERSION_STRING "." INVADER_VERSION_COMMIT_HASH "\n\n"
               "This program is licensed under the GNU General Public License v3.0.\n"
               "\n"
               "Credits:\n"
               "  Kavawuvi                     - Lead Developer, Project owner\n"
               "  Vaporeon                     - Testing & QA, AUR maintainer, Development\n"
               "\n"
               "Special thanks:\n"
               "  Tucker933                    - Hosting @ https://invader.opencarnage.net\n"
               "  MosesofEgypt                 - Tag data & model data research and code,\n"
               "                                 mouth data generation, ADPCM-XQ Xbox ADPCM\n"
               "                                 conversion\n"
               "\n"
               "Software used in this program:\n"
               "  FreeType                     - https://www.freetype.org/\n"
               "  STB library                  - https://github.com/nothings/stb\n"
               "\n"
               "Links:\n"
               "  Invader source code          - https://github.com/Kavawuvi/invader\n"
               #ifdef SHOW_NIGHTLY_LINK
               "  Invader nightly builds       - https://invader.opencarnage.net/builds/nightly/\n"
               #endif
               ;
    }
}
