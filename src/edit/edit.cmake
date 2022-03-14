# SPDX-License-Identifier: GPL-3.0-only

if(NOT DEFINED ${INVADER_EDIT})
    set(INVADER_EDIT true CACHE BOOL "Build invader-edit (edits tags)")
endif()

if(${INVADER_EDIT})
    add_executable(invader-edit
        src/edit/edit.cpp
    )

    if(MINGW)
        target_sources(invader-edit PRIVATE ${MINGW_CRT_NOGLOB})
    endif()

    target_link_libraries(invader-edit invader)

    set(TARGETS_LIST ${TARGETS_LIST} invader-edit)

    do_windows_rc(invader-edit invader-edit.exe "Invader command-line tag editor")
endif()
