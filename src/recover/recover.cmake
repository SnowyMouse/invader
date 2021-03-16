# SPDX-License-Identifier: GPL-3.0-only

if(NOT DEFINED ${INVADER_MODEL})
    set(INVADER_MODEL true CACHE BOOL "Build invader-recover (recovers source data from tags)")
endif()

if(${INVADER_MODEL})
    add_executable(invader-recover
        src/recover/recover.cpp
        src/recover/recover_method.cpp
    )
    target_link_libraries(invader-recover invader)

    set(TARGETS_LIST ${TARGETS_LIST} invader-recover)

    do_windows_rc(invader-recover invader-recover.exe "Tag source data recovery tool")
endif()

