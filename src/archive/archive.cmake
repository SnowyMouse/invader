# SPDX-License-Identifier: GPL-3.0-only

if(NOT DEFINED ${INVADER_ARCHIVE})
    if(${LibArchive_FOUND})
        set(INVADER_ARCHIVE true)
    else()
        set(INVADER_ARCHIVE false)
        message(WARNING "Unable to automatically find libarchive; invader-archive will be disabled")
    endif()
    set(INVADER_ARCHIVE ${INVADER_ARCHIVE} CACHE BOOL "Build invader-archive (archives tags; requires libarchive)")
endif()

if(${INVADER_ARCHIVE})
    add_executable(invader-archive
        src/archive/archive.cpp
    )
    target_include_directories(invader-archive PUBLIC ${LibArchive_INCLUDE_DIRS})
    target_link_libraries(invader-archive invader ${LibArchive_LIBRARIES})

    set(TARGETS_LIST ${TARGETS_LIST} invader-archive)
endif()
