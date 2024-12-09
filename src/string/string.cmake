# SPDX-License-Identifier: GPL-3.0-only

if(NOT DEFINED ${INVADER_STRING})
    set(INVADER_STRING true CACHE BOOL "Build invader-string (generates string tags)")
endif()

if(${INVADER_STRING})
    add_executable(invader-string
        src/string/string.cpp
    )

    target_link_libraries(invader-string invader ${INVADER_CRT_NOGLOB})

    set(TARGETS_LIST ${TARGETS_LIST} invader-string)

    if(WIN32)
        target_sources(invader-string PRIVATE src/string/string.rc)
    endif()
endif()
