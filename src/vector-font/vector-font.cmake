# SPDX-License-Identifier: GPL-3.0-only

if(NOT DEFINED ${INVADER_VECTOR_FONT})
    set(INVADER_VECTOR_FONT ${INVADER_VECTOR_FONT} true CACHE BOOL "Build invader-vector-font (makes vector_font_data tags)")
endif()

if(${INVADER_VECTOR_FONT})
    add_executable(invader-vector-font
        src/vector-font/vector-font.cpp
    )

    target_link_libraries(invader-vector-font invader ${INVADER_CRT_NOGLOB})

    set(TARGETS_LIST ${TARGETS_LIST} invader-vector-font)

    if(WIN32)
        target_sources(invader-vector-font PRIVATE src/vector-font/vector-font.rc)
    endif()
endif()
