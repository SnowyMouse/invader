# SPDX-License-Identifier: GPL-3.0-only

add_executable(invader-resource
    src/resource/resource.cpp
)

target_link_libraries(invader-resource invader)
