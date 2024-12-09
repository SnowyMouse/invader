# SPDX-License-Identifier: GPL-3.0-only

if(NOT DEFINED ${INVADER_STRIP})
    set(INVADER_STRIP true CACHE BOOL "Build invader-strip (strips hidden values from tags)")
endif()

if(${INVADER_STRIP})
    add_executable(invader-strip
        src/strip/strip.cpp
    )

    target_link_libraries(invader-strip invader ${INVADER_CRT_NOGLOB})

    set(TARGETS_LIST ${TARGETS_LIST} invader-strip)

    if(WIN32)
        target_sources(invader-strip PRIVATE src/strip/strip.rc)
    endif()
endif()
