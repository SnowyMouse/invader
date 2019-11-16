# SPDX-License-Identifier: GPL-3.0-only

if(NOT DEFINED ${INVADER_RESOURCE})
    set(INVADER_RESOURCE true CACHE BOOL "Build invader-resource (builds resource map files)")
endif()

if(${INVADER_RESOURCE})
    add_executable(invader-resource
        src/resource/resource.cpp
    )
    target_link_libraries(invader-resource invader)
endif()
