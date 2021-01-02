# SPDX-License-Identifier: GPL-3.0-only

set(INVADER_CONVERT ${INVADER_CONVERT} CACHE BOOL "Build invader-convert")

if(${INVADER_CONVERT})
    add_executable(invader-convert
        src/convert/convert.cpp
    )

    target_link_libraries(invader-convert invader ${FREETYPE_LIBRARIES})

    set(TARGETS_LIST ${TARGETS_LIST} invader-convert)

    do_windows_rc(invader-convert invader-convert.exe "Invader tag conversion tool")
endif()
