# SPDX-License-Identifier: GPL-3.0-only

# Check if we can build invader-font
if(NOT ${FREETYPE_FOUND})
    set(INVADER_FONT false)
    message(WARNING "Unable to automatically find freetype; invader-font will be disabled")
endif()

set(INVADER_FONT ${INVADER_FONT} CACHE BOOL "Build invader-font (requires freetype)")

if(${INVADER_FONT})
    add_executable(invader-font
        src/font/font.cpp
    )

    target_include_directories(invader-font
        PUBLIC ${FREETYPE_INCLUDE_DIRS}
    )

    target_link_libraries(invader-font invader ${FREETYPE_LIBRARIES})

    set(TARGETS_LIST ${TARGETS_LIST} invader-font)


    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        set_source_files_properties(src/font/font.cpp
            PROPERTIES COMPILE_FLAGS -Wno-old-style-cast
        )
    endif()
    
    do_windows_rc(invader-font invader-font.exe "Invader font tag generation tool")

endif()
