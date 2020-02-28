# SPDX-License-Identifier: GPL-3.0-only

# Check if we can build invader-bitmap
if(NOT DEFINED ${INVADER_BITMAP})
    set(INVADER_BITMAP true CACHE BOOL "Build invader-bitmap (creates bitmap tags)")
endif()

if(${INVADER_BITMAP})
    add_executable(invader-bitmap
        src/bitmap/bitmap.cpp
    )

    target_include_directories(invader-bitmap
        PUBLIC ${ZLIB_INCLUDE_DIRS}
    )

    target_link_libraries(invader-bitmap invader ${ZLIB_LIBRARIES})

    set(TARGETS_LIST ${TARGETS_LIST} invader-bitmap)
endif()
