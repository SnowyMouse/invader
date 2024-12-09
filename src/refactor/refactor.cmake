# SPDX-License-Identifier: GPL-3.0-only

if(NOT DEFINED ${INVADER_REFACTOR})
    set(INVADER_REFACTOR true CACHE BOOL "Build invader-refactor (refactors tag paths)")
endif()

if(${INVADER_REFACTOR})
    add_executable(invader-refactor
        src/refactor/refactor.cpp
    )

    target_link_libraries(invader-refactor invader ${INVADER_CRT_NOGLOB})

    set(TARGETS_LIST ${TARGETS_LIST} invader-refactor)

    if(WIN32)
        target_sources(invader-refactor PRIVATE src/refactor/refactor.rc)
    endif()
endif()
