# SPDX-License-Identifier: GPL-3.0-only

if(NOT DEFINED ${INVADER_CONVERT})
    set(INVADER_CONVERT true CACHE BOOL "Build invader-convert (converts tags)")
endif()

if(${INVADER_CONVERT})
    add_executable(invader-convert
        src/convert/convert.cpp
    )

    target_link_libraries(invader-convert invader ${INVADER_CRT_NOGLOB})

    set(TARGETS_LIST ${TARGETS_LIST} invader-convert)

    if(WIN32)
        target_sources(invader-convert PRIVATE src/convert/convert.rc)
    endif()
endif()
