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
    if((NOT ${INVADER_USE_ZLIB}) OR (NOT ${ZLIB_FOUND}))
        message(FATAL_ERROR "invader-bitmap requires zlib")
    endif()

    add_executable(invader-bitmap
        src/bitmap/bitmap.cpp
        src/bitmap/stb/stb_impl.c
        src/bitmap/color_plate_scanner.cpp
        src/bitmap/image_loader.cpp
        src/bitmap/bitmap_data_writer.cpp
    )

    target_include_directories(invader-bitmap
        PUBLIC ${TIFF_INCLUDE_DIRS}
    )

    target_link_libraries(invader-bitmap ${TIFF_LIBRARIES} invader)

    set(TARGETS_LIST ${TARGETS_LIST} invader-bitmap)
    
    do_windows_rc(invader-bitmap invader-bitmap.exe "Invader bitmap tag generation tool")
endif()
