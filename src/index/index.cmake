# SPDX-License-Identifier: GPL-3.0-only

if(NOT DEFINED ${INVADER_INDEX})
    set(INVADER_INDEX true CACHE BOOL "Build invader-index (indexes cache files)")
endif()

if(${INVADER_INDEX})
    add_executable(invader-index
        src/index/index.cpp
    )

    target_link_libraries(invader-index invader ${INVADER_CRT_NOGLOB})

    set(TARGETS_LIST ${TARGETS_LIST} invader-index)

    if(WIN32)
        target_sources(invader-index PRIVATE src/index/index.rc)
    endif()
endif()
