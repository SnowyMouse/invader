# SPDX-License-Identifier: GPL-3.0-only

if(NOT DEFINED ${INVADER_BUILD})
    set(INVADER_BUILD true CACHE BOOL "Build invader-build (builds cache files)")
endif()

if(${INVADER_BUILD})
    add_executable(invader-build
        src/build/build.cpp
    )

    if(MINGW)
        target_sources(invader-build PRIVATE ${MINGW_CRT_NOGLOB})
    endif()

    target_link_libraries(invader-build invader)

    set(TARGETS_LIST ${TARGETS_LIST} invader-build)

    do_windows_rc(invader-build invader-build.exe "Invader map compilation tool")
endif()
