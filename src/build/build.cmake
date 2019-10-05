# SPDX-License-Identifier: GPL-3.0-only

# Build executable
add_executable(invader-build
    src/build/build.cpp
)
target_link_libraries(invader-build invader)
