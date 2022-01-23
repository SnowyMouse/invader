# SPDX-License-Identifier: GPL-3.0-only

# Script executable
add_executable(invader-script
    src/script/script.cpp
)
target_link_libraries(invader-script invader)

set(TARGETS_LIST ${TARGETS_LIST} invader-script)
do_windows_rc(invader-script invader-script.exe "Script compiler")
