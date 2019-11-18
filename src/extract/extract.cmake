# SPDX-License-Identifier: GPL-3.0-only

if(NOT DEFINED ${INVADER_EXTRACT})
    set(INVADER_EXTRACT true CACHE BOOL "Build invader-extract (extracts data from cache files)")
endif()

if(${INVADER_EXTRACT})
    add_executable(invader-extract
        src/extract/extract.cpp
    )
    target_link_libraries(invader-extract invader)

    set(TARGETS_LIST ${TARGETS_LIST} invader-extract)
endif()
