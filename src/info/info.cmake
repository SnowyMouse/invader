# SPDX-License-Identifier: GPL-3.0-only

# Map-info executable
add_executable(invader-info
    src/info/info.cpp
)
target_link_libraries(invader-info invader)
