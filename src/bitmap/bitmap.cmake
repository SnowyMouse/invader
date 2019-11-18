# SPDX-License-Identifier: GPL-3.0-only

# Check if we can build invader-bitmap
if(NOT DEFINED ${INVADER_BITMAP})
    if(${TIFF_FOUND} AND ${ZLIB_FOUND})
        set(INVADER_BITMAP true)
    else()
        set(INVADER_BITMAP false)
        message(WARNING "Unable to automatically find libTIFF and zlib; invader-bitmap will be disabled")
    endif()
    set(INVADER_BITMAP ${INVADER_BITMAP} CACHE BOOL "Build invader-bitmap (creates bitmap tags; requires libTIFF and zlib)")
endif()

if(${INVADER_BITMAP})
    add_executable(invader-bitmap
        src/bitmap/bitmap.cpp
        src/bitmap/stb/stb_impl.c
    )

    set_source_files_properties(src/bitmap/stb/stb_impl.c PROPERTIES COMPILE_FLAGS -Wno-unused-function)

    target_include_directories(invader-bitmap
        PUBLIC ${ZLIB_INCLUDE_DIRS} ${TIFF_INCLUDE_DIRS}
    )

    target_link_libraries(invader-bitmap invader ${ZLIB_LIBRARIES} ${TIFF_LIBRARIES})

    set(TARGETS_LIST ${TARGETS_LIST} invader-bitmap)
endif()
