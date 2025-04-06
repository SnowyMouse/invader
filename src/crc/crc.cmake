# SPDX-License-Identifier: GPL-3.0-only

if(NOT DEFINED ${INVADER_CRC})
    set(INVADER_CRC true CACHE BOOL "Build invader-crc (Changes the CRC32 of existing cache-files)")
endif()

if(${INVADER_CRC})
    add_executable(invader-crc
        src/crc/crc.cpp
    )

    target_link_libraries(invader-crc invader ${INVADER_CRT_NOGLOB})

    set(TARGETS_LIST ${TARGETS_LIST} invader-crc)

    if(WIN32)
        target_sources(invader-crc PRIVATE src/crc/crc.rc)
    endif()
endif()
