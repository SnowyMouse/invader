# Check if we can build invader-bitmap
if(${TIFF_FOUND} AND ${ZLIB_FOUND})
    # Bitmap executable
    add_executable(invader-bitmap
        src/bitmap/bitmap.cpp
        src/bitmap/composite_bitmap.cpp
    )

    target_include_directories(invader-bitmap
        PUBLIC ${ZLIB_INCLUDE_DIRS} ${TIFF_INCLUDE_DIRS}
    )

    target_link_libraries(invader-bitmap invader ${ZLIB_LIBRARIES} ${TIFF_LIBRARIES})

    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        target_compile_options(invader-bitmap
            PUBLIC "-Wno-old-style-cast"
        )
    endif()
else()
    message("A dependency is missing. invader-bitmap will not compile.")
endif()
