# SPDX-License-Identifier: GPL-3.0-only

if(NOT DEFINED ${INVADER_INDEXER})
    set(INVADER_INDEXER true CACHE BOOL "Build invader-indexer (indexes cache files)")
endif()

if(${INVADER_INDEXER})
    add_executable(invader-indexer
        src/indexer/indexer.cpp
    )
    target_link_libraries(invader-indexer invader)
endif()
