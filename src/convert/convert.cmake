# SPDX-License-Identifier: GPL-3.0-only

if(NOT DEFINED ${INVADER_CONVERT})
    set(INVADER_CONVERT true CACHE BOOL "Build invader-convert (converts tags)")
endif()

if(${INVADER_CONVERT})
    add_executable(invader-convert
        src/convert/convert.cpp
    )

    target_link_libraries(invader-convert invader)

    set(TARGETS_LIST ${TARGETS_LIST} invader-convert)

    do_windows_rc(invader-convert invader-convert.exe "Invader tag conversion tool")
endif()
