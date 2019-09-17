# Check if we can build invader-bitmap
if(${TIFF_FOUND} AND ${ZLIB_FOUND})
    # Bitmap executable
    add_executable(invader-bitmap
        src/bitmap/bitmap.cpp
        src/bitmap/color_plate_scanner.cpp
        src/bitmap/stb/stb_impl.c
        src/bitmap/image_loader.cpp
        "${CMAKE_CURRENT_BINARY_DIR}/p8_palette.cpp"
    )

    # Include version script
    add_custom_command(
        OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/p8_palette.cpp"
        COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/src/bitmap/p8/palette.py" "${CMAKE_CURRENT_SOURCE_DIR}/src/bitmap/p8/p8_palette" "${CMAKE_CURRENT_BINARY_DIR}/p8_palette.cpp"
        DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/bitmap/p8/palette.py"
        DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/bitmap/p8/p8_palette"
    )

    set_source_files_properties(src/bitmap/stb/stb_impl.c PROPERTIES COMPILE_FLAGS -Wno-unused-function)

    target_include_directories(invader-bitmap
        PUBLIC ${ZLIB_INCLUDE_DIRS} ${TIFF_INCLUDE_DIRS}
    )

    target_link_libraries(invader-bitmap invader ${ZLIB_LIBRARIES} ${TIFF_LIBRARIES})
else()
    message("A dependency is missing. invader-bitmap will not compile.")
endif()
