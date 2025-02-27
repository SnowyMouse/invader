# SPDX-License-Identifier: GPL-3.0-only

if(NOT DEFINED ${INVADER_INFO})
    set(INVADER_INFO true CACHE BOOL "Build invader-info (displays metadata for cache files)")
endif()

if(${INVADER_INFO})
    add_executable(invader-info
        src/info/info.cpp
        src/info/info_def.cpp
    )

    target_link_libraries(invader-info invader ${INVADER_CRT_NOGLOB})

    set(TARGETS_LIST ${TARGETS_LIST} invader-info)

    if(WIN32)
        target_sources(invader-info PRIVATE src/info/info.rc)
    endif()
endif()
