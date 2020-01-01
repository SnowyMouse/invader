# SPDX-License-Identifier: GPL-3.0-only

if(NOT DEFINED ${INVADER_COMPRESS})
    set(INVADER_COMPRESS true CACHE BOOL "Build invader-compress (compresses cache files)")
endif()

if(${INVADER_COMPRESS})
    add_executable(invader-compress
        src/compress/compress.cpp
    )
    target_link_libraries(invader-compress invader)

    set(TARGETS_LIST ${TARGETS_LIST} invader-compress)
endif()
