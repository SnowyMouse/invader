# SPDX-License-Identifier: GPL-3.0-only

if(NOT DEFINED ${INVADER_BLUDGEON})
    set(INVADER_BLUDGEON true CACHE BOOL "Build invader-bludgeon (convinces tags to work with Invader)")
endif()

if(${INVADER_BLUDGEON})
    add_executable(invader-bludgeon
        src/bludgeon/bludgeon.cpp
        src/bludgeon/bludgeoner.cpp
    )

    target_link_libraries(invader-bludgeon invader ${INVADER_CRT_NOGLOB})

    set(TARGETS_LIST ${TARGETS_LIST} invader-bludgeon)

    if(WIN32)
        target_sources(invader-bludgeon PRIVATE src/bludgeon/bludgeon.rc)
    endif()
endif()
