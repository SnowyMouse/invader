#
# Invader (c) 2018 Kavawuvi
#
# This program is free software under the GNU General Public License v3.0 or later. See LICENSE for more information.
#

# Require at least version 3.10
cmake_minimum_required(VERSION 3.10)

# Use C++
enable_language(CXX)
set(CMAKE_CXX_STANDARD 17)

# Do stuff depending on the compiler
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic -Wold-style-cast")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -march=native")
elseif(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_SCL_SECURE_NO_WARNINGS -D_CRT_SECURE_NO_WARNINGS")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O2")
endif()

# Turn on console messages
option(NO_OUTPUT "don't show any console messages" OFF)
if(NO_OUTPUT)
    add_definitions(-DNO_OUTPUT)
endif()

# Find some packages
find_package(TIFF)
find_package(ZLIB)

include(src/invader.cmake)
include(src/build/build.cmake)
include(src/crc/crc.cmake)
include(src/indexer/indexer.cmake)
include(src/scenario/scenario.cmake)
include(src/bitmap/bitmap.cmake)
