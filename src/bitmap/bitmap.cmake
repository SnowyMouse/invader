# SPDX-License-Identifier: GPL-3.0-only

# Check if we can build invader-bitmap
if(NOT ${TIFF_FOUND})
    set(INVADER_BITMAP false)
    message(WARNING "Unable to automatically find libtiff; invader-bitmap will be disabled")
endif()

if(NOT DEFINED ${INVADER_BITMAP})
    set(INVADER_BITMAP true CACHE BOOL "Build invader-bitmap (creates bitmap tags)")
endif()

if(${INVADER_BITMAP})
    add_executable(invader-bitmap
        src/bitmap/bitmap.cpp
        src/bitmap/image_loader.cpp
        src/bitmap/stb/stb_impl.c
        src/bitmap/bitmap_data_writer.cpp
    )

    target_include_directories(invader-bitmap
        PUBLIC ${TIFF_INCLUDE_DIRS}
    )

    target_link_libraries(invader-bitmap ${TIFF_LIBRARIES} invader ${INVADER_CRT_NOGLOB})

    set(TARGETS_LIST ${TARGETS_LIST} invader-bitmap)

    if(WIN32)
        target_sources(invader-bitmap PRIVATE src/bitmap/bitmap.rc)
    endif()
endif()
