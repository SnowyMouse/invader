# SPDX-License-Identifier: GPL-3.0-only

# Dependency executable
add_executable(invader-dependency
    src/dependency/dependency.cpp
)
target_link_libraries(invader-dependency invader)
