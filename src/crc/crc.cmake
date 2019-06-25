# CRC executable
add_executable(invader-crc
    src/crc/crc.cpp
    src/crc/crc32.c
    src/crc/crc_spoof.c
)
target_link_libraries(invader-crc invader)
