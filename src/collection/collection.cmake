# SPDX-License-Identifier: GPL-3.0-only

if(NOT DEFINED ${INVADER_COLLECTION})
    set(INVADER_COLLECTION true CACHE BOOL "Build invader-collection (generates tag collection tags)")
endif()

if(${INVADER_COLLECTION})
    add_executable(invader-collection
        src/collection/collection.cpp
    )
    target_link_libraries(invader-collection invader)

    set(TARGETS_LIST ${TARGETS_LIST} invader-collection)
endif()
