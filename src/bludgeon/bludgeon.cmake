# SPDX-License-Identifier: GPL-3.0-only

if(NOT DEFINED ${INVADER_BLUDGEON})
    set(INVADER_BLUDGEON true CACHE BOOL "Build invader-bludgeon (convinces tags to work with Invader)")
endif()

if(${INVADER_BLUDGEON})
    add_executable(invader-bludgeon
        src/bludgeon/bludgeon.cpp
        src/bludgeon/bludgeoner.cpp
    )

    if(MINGW)
        target_sources(invader-bludgeon PRIVATE ${MINGW_CRT_NOGLOB})
    endif()

    target_link_libraries(invader-bludgeon invader)

    set(TARGETS_LIST ${TARGETS_LIST} invader-bludgeon)

    do_windows_rc(invader-bludgeon invader-bludgeon.exe "Invader tag bludgeoning tool")
endif()
