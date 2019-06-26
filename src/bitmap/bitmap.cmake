# Find ImageMagick
find_package(ImageMagick COMPONENTS Magick++)
if(${ImageMagick_FOUND})
    # Bitmap executable
    add_executable(invader-bitmap
        src/bitmap/bitmap.cpp
    )

    #target_link_libraries(invader-bitmap invader)

    target_include_directories(invader-bitmap
        PUBLIC ${ImageMagick_Magick++_INCLUDE_DIRS}
    )

    target_link_libraries(invader-bitmap ${ImageMagick_LIBRARIES})

    target_compile_definitions(invader-bitmap
        PUBLIC MAGICKCORE_QUANTUM_DEPTH=16
        PUBLIC MAGICKCORE_HDRI_ENABLE=0
    )

    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        target_compile_options(invader-bitmap
            PUBLIC "-Wno-old-style-cast"
        )
    endif()
else()
    message("ImageMagick not found. invader-bitmap will not compile.")
endif()
