# SPDX-License-Identifier: GPL-3.0-only

if(NOT DEFINED ${INVADER_SOUND})
    set(INVADER_SOUND true CACHE BOOL "Build invader-sound (builds sound tags)")
endif()

if(${INVADER_RESOURCE})
    add_executable(invader-sound
        src/sound/sound.cpp
    )
    target_link_libraries(invader-sound invader ogg vorbis vorbisenc samplerate FLAC)

    set(TARGETS_LIST ${TARGETS_LIST} invader-sound)
endif()
