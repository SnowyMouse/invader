# SPDX-License-Identifier: GPL-3.0-only

if(NOT DEFINED ${INVADER_LIGHTMAP})
    set(INVADER_LIGHTMAP true CACHE BOOL "Build invader-lightmap (generate lightmaps)")
endif()

if(${INVADER_LIGHTMAP})
    add_executable(invader-lightmap
        src/lightmap/lightmap.cpp
        src/lightmap/actions.cpp
    )

    target_link_libraries(invader-lightmap invader ${INVADER_CRT_NOGLOB})

    set(TARGETS_LIST ${TARGETS_LIST} invader-lightmap)

    if(WIN32)
        target_sources(invader-lightmap PRIVATE src/lightmap/lightmap.rc)
    endif()
endif()

