# SPDX-License-Identifier: GPL-3.0-only

# Compress executable
add_executable(invader-compress
    src/compress/compress.cpp
)
target_link_libraries(invader-compress invader)
