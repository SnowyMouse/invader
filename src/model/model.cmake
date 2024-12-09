# SPDX-License-Identifier: GPL-3.0-only

if(NOT DEFINED ${INVADER_MODEL})
    set(INVADER_MODEL true CACHE BOOL "Build invader-model (creates model tags)")
endif()

if(${INVADER_MODEL})
    add_executable(invader-model
        src/model/model.cpp
    )

    target_link_libraries(invader-model invader ${INVADER_CRT_NOGLOB})

    set(TARGETS_LIST ${TARGETS_LIST} invader-model)

    if(WIN32)
        target_sources(invader-model PRIVATE src/model/model.rc)
    endif()
endif()

