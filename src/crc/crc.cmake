# SPDX-License-Identifier: GPL-3.0-only

# CRC executable
add_executable(invader-crc
    src/crc/crc.cpp
)
target_link_libraries(invader-crc invader)
