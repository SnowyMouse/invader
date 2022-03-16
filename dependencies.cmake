# SPDX-License-Identifier: GPL-3.0-only

# Run a dependency script if needed
if(DEFINED INVADER_BUILD_DEPENDENCY_SCRIPT)
    set(INVADER_BUILD_DEPENDENCY_SCRIPT_PRE_RUN YES)
    include(${INVADER_BUILD_DEPENDENCY_SCRIPT})
endif()

# Find some packages
find_package(Python3 REQUIRED)
find_package(Threads REQUIRED)
find_package(ZLIB REQUIRED)
find_package(LibArchive)
find_package(TIFF)
find_package(Freetype)
find_package(Git)
find_package(SDL2)

# Qt6
find_package(Qt6 COMPONENTS Core Widgets REQUIRED)
find_package(SDL2 REQUIRED)

# Load RIAT
add_subdirectory(ext/riat)

# Audio things
set(DEP_AUDIO_LIBRARIES FLAC vorbisenc vorbisfile vorbis ogg samplerate)

# Set dependencies
set(DEP_SQUISH_LIBRARIES squish)

# Again, run a dependency script if needed
if(DEFINED INVADER_BUILD_DEPENDENCY_SCRIPT)
    set(INVADER_BUILD_DEPENDENCY_SCRIPT_PRE_RUN NO)
    include(${INVADER_BUILD_DEPENDENCY_SCRIPT})
endif()
