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

# Qt6
find_package(Qt6 COMPONENTS Core Widgets REQUIRED)

# SDL2
find_package(SDL2 REQUIRED)

# Load Rat In a Tube
option(INVADER_USE_SYSTEM_CORROSION "Use system installed corrosion package" ON)
if(INVADER_USE_SYSTEM_CORROSION)
    find_package(Corrosion REQUIRED)
else()
    include(FetchContent)
    FetchContent_Declare(
        Corrosion
        GIT_REPOSITORY https://github.com/corrosion-rs/corrosion.git
        GIT_TAG v0.4.4
    )
    FetchContent_MakeAvailable(Corrosion)
endif()

corrosion_import_crate(MANIFEST_PATH ext/riat/riatc/Cargo.toml)

# Audio things
set(DEP_AUDIO_LIBRARIES FLAC vorbisenc vorbisfile vorbis ogg samplerate)

# Set dependencies
set(DEP_SQUISH_LIBRARIES squish)

# Again, run a dependency script if needed
if(DEFINED INVADER_BUILD_DEPENDENCY_SCRIPT)
    set(INVADER_BUILD_DEPENDENCY_SCRIPT_PRE_RUN NO)
    include(${INVADER_BUILD_DEPENDENCY_SCRIPT})
endif()
