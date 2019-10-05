# SPDX-License-Identifier: GPL-3.0-only

# String executable
add_executable(invader-string
    src/string/string.cpp
)
target_link_libraries(invader-string invader)
