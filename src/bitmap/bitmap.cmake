# Check if we can build invader-bitmap
if(${TIFF_FOUND} AND ${ZLIB_FOUND})
    # Bitmap executable
    add_executable(invader-bitmap
        src/bitmap/bitmap.cpp
        src/bitmap/color_plate_scanner.cpp
        src/bitmap/stb/stb_impl.c
    )

    set_source_files_properties(src/bitmap/stb/stb_impl.c PROPERTIES COMPILE_FLAGS -Wno-unused-function)

    target_include_directories(invader-bitmap
        PUBLIC ${ZLIB_INCLUDE_DIRS} ${TIFF_INCLUDE_DIRS}
    )

    target_link_libraries(invader-bitmap invader ${ZLIB_LIBRARIES} ${TIFF_LIBRARIES})
else()
    message("A dependency is missing. invader-bitmap will not compile.")
endif()
