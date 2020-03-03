# SPDX-License-Identifier: GPL-3.0-only

if(NOT DEFINED ${INVADER_REFACTOR})
    set(INVADER_STRIP true CACHE BOOL "Build invader-refactor (refactors tag paths)")
endif()

if(${INVADER_REFACTOR})
    add_executable(invader-refactor
        src/refactor/refactor.cpp
    )
    target_link_libraries(invader-refactor invader)

    set(TARGETS_LIST ${TARGETS_LIST} invader-refactor)
endif()
