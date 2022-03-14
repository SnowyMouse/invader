# SPDX-License-Identifier: GPL-3.0-only

if(NOT DEFINED ${INVADER_LIGHTMAP})
    set(INVADER_LIGHTMAP true CACHE BOOL "Build invader-lightmap (generate lightmaps)")
endif()

if(${INVADER_LIGHTMAP})
    add_executable(invader-lightmap
        src/lightmap/lightmap.cpp
        src/lightmap/actions.cpp
    )

    if(MINGW)
        target_sources(invader-lightmap PRIVATE ${MINGW_CRT_NOGLOB})
    endif()

    target_link_libraries(invader-lightmap invader)

    set(TARGETS_LIST ${TARGETS_LIST} invader-lightmap)

    do_windows_rc(invader-lightmap invader-lightmap "Invader lightmap compilation tool")
endif()

