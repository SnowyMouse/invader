# SPDX-License-Identifier: GPL-3.0-only

if(NOT DEFINED ${INVADER_SCRIPT})
    set(INVADER_SCRIPT true CACHE BOOL "Build invader-script (compiles .hsc scripts)")
endif()

if(${INVADER_SCRIPT})
    add_executable(invader-script
        src/script/script.cpp
    )

    target_link_libraries(invader-script invader ${INVADER_CRT_NOGLOB})

    set(TARGETS_LIST ${TARGETS_LIST} invader-script)

    if(WIN32)
        target_sources(invader-script PRIVATE src/script/script.rc)
    endif()
endif()
