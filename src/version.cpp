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
               "This software is licensed under the GNU General Public License v3.0 as published\n"
               "by the Free Software Foundation. See https://www.gnu.org/licenses/gpl-3.0.txt\n"
               "\n"
               "Credits:\n"
               "  Snowy Mouse                     - Lead developer, project owner\n"
               "  Vaporeon                        - Testing & QA, AUR maintainer, development\n"
               "  ST34MF0X                        - Icon design\n"
               "\n"
               "Special thanks:\n"
               "  Tucker933                       - Hosting @ https://invader.opencarnage.net\n"
               "  MosesofEgypt                    - Tag data & model data research and code,\n"
               "                                    mouth data generation, ADPCM-XQ Xbox ADPCM\n"
               "                                    conversion\n"
               "\n"
               "Software used in this program:\n"
               "  FreeType (font rasterization)   - https://www.freetype.org/\n"
               "  libarchive (archiving)          - https://www.libarchive.org/\n"
               "  libFLAC (FLAC decoding)         - https://github.com/xiph/flac\n"
               "  LibTIFF (TIFF handling)         - http://www.libtiff.org/\n"
               "  libsquish (DXT compression)     - https://sourceforge.net/projects/libsquish/\n"
               "  libvorbis (Ogg Vorbis encoding) - https://xiph.org/vorbis/\n"
               "  Qt (GUI toolkit)                - https://www.qt.io/\n"
               "  Rat In a Tube (HSC compiling)   - https://github.com/SnowyMouse/riat\n"
               "  Secret Rabbit Code (resampling) - http://www.mega-nerd.com/SRC/\n"
               "  STB library (image decoding)    - https://github.com/nothings/stb\n"
               "  zlib (DEFLATE compression)      - https://zlib.net/\n"
               "\n"
               "Links:\n"
               "  Invader source code             - https://github.com/SnowyMouse/invader\n"
               #ifdef SHOW_NIGHTLY_LINK
               "  Invader nightly builds          - https://invader.opencarnage.net/builds/nightly/\n"
               #endif
               ;
    }
}
