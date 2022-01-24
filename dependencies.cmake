# SPDX-License-Identifier: GPL-3.0-only

# Find some packages
find_package(Python3 REQUIRED)
find_package(Threads REQUIRED)
find_package(ZLIB)
find_package(LibArchive)
find_package(TIFF)
find_package(Freetype)
find_package(Git)

# Load RIAT
add_subdirectory(ext/riat)

if(WIN32)
    option(INVADER_WIN32_EXE_STATIC_LINK "set whether or not to make a completely static build of all of the Invader programs" ON)
endif()

set(DEP_AUDIO_LIBRARIES FLAC vorbisenc vorbisfile vorbis ogg samplerate)

if(${INVADER_WIN32_EXE_STATIC_LINK})
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-allow-multiple-definition -static -static-libgcc -static-libstdc++")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -static -lwinpthread -static-libgcc -static-libstdc++")
    
    if(${INVADER_STATIC_BUILD})
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -lwinpthread")
    endif()
    
    set(ZLIB_LIBRARIES z)
    set(TIFF_LIBRARIES tiff jpeg lzma)
    set(FREETYPE_LIBRARIES freetype png brotlidec-static brotlicommon-static bz2 harfbuzz graphite2 freetype)
    set(LibArchive_LIBRARIES archive bcrypt bz2 lzma iconv zstd)
    set(SQUISH_LIBRARIES squish gomp)
else()
    set(SQUISH_LIBRARIES squish)
endif()


