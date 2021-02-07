# SPDX-License-Identifier: GPL-3.0-only

if(NOT DEFINED ${INVADER_COMPARE})
    set(INVADER_COMPARE true CACHE BOOL "Build invader-compare (compares tag directories)")
endif()

if(${INVADER_COMPARE})
    add_executable(invader-compare
        src/compare/compare.cpp
    )
    target_link_libraries(invader-compare invader)

    set(TARGETS_LIST ${TARGETS_LIST} invader-compare)
    
    do_windows_rc(invader-compare invader-compare.exe "Invader tag comparison tool")
endif()
