# SPDX-License-Identifier: GPL-3.0-only

if(NOT DEFINED ${INVADER_SOUND})
    set(INVADER_SOUND true CACHE BOOL "Build invader-sound (builds sound tags)")
endif()

if(${INVADER_SOUND})
    if(NOT ${INVADER_USE_AUDIO})
        message(FATAL_ERROR "invader-sound requires audio")
    endif()

    add_executable(invader-sound
        src/sound/sound.cpp
    )
    target_link_libraries(invader-sound invader)

    set(TARGETS_LIST ${TARGETS_LIST} invader-sound)
    do_windows_rc(invader-sound invader-sound.exe "Invader sound tag generation tool")
endif()
