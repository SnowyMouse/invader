# SPDX-License-Identifier: GPL-3.0-only

# Check if we can build invader-bitmap
if(${FREETYPE_FOUND})
    # Font executable
    add_executable(invader-font
        src/font/font.cpp
    )

    target_include_directories(invader-font
        PUBLIC ${FREETYPE_INCLUDE_DIRS}
    )

    target_link_libraries(invader-font invader ${FREETYPE_LIBRARIES})
else()
    message("A dependency is missing. invader-font will not compile.")
endif()
