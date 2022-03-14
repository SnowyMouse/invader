# SPDX-License-Identifier: GPL-3.0-only

if(NOT DEFINED ${INVADER_SCRIPT})
    set(INVADER_SCRIPT true CACHE BOOL "Build invader-script (compiles .hsc scripts)")
endif()

if(${INVADER_SCRIPT})
    add_executable(invader-script
        src/script/script.cpp
    )

    if(MINGW)
        target_sources(invader-script PRIVATE ${MINGW_CRT_NOGLOB})
    endif()

    target_link_libraries(invader-script invader)

    set(TARGETS_LIST ${TARGETS_LIST} invader-script)

    do_windows_rc(invader-script invader-script.exe "Script compiler")
endif()
