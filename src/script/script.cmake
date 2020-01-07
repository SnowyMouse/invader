# SPDX-License-Identifier: GPL-3.0-only

# Script executable
add_executable(invader-script
    src/script/script.cpp
)
target_link_libraries(invader-script invader)
