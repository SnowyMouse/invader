# SPDX-License-Identifier: GPL-3.0-only

if(NOT DEFINED ${INVADER_SCAN})
    set(INVADER_SCAN false CACHE BOOL "Build invader-scan (scans for unknown values in cache files)")
endif()

if(${INVADER_SCAN})
    add_executable(invader-scan
        src/scan/scan.cpp
    )

    if(MINGW)
        target_sources(invader-scan PRIVATE ${MINGW_CRT_NOGLOB})
    endif()

    target_link_libraries(invader-scan invader)

    set(TARGETS_LIST ${TARGETS_LIST} invader-scan)

    do_windows_rc(invader-scan invader-scan.exe "Scan cache files for unknown tag fields")
endif()
