# SPDX-License-Identifier: GPL-3.0-only

if(NOT DEFINED ${INVADER_SCAN})
    set(INVADER_SCAN true CACHE BOOL "Build invader-scan (scans for unknown values in cache files)")
endif()

if(${INVADER_SCAN})
    add_executable(invader-scan
        src/scan/scan.cpp
    )
    target_link_libraries(invader-scan invader)

    set(TARGETS_LIST ${TARGETS_LIST} invader-scan)
endif()
