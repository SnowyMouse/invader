#
# Invader (c) 2018 Kavawuvi
#
# This program is free software under the GNU General Public License v3.0 or later. See LICENSE for more information.
#

# Require at least version 3.10
cmake_minimum_required(VERSION 3.10)

# Project name
project(Invader
    VERSION 0.1.0
    DESCRIPTION "Modding toolkit for Halo Custom Edition"
    LANGUAGES C CXX
)

# Use C++17
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
find_package(Python3)

# Define some version stuff
add_definitions(-DINVADER_VERSION_MAJOR=${Invader_VERSION_MAJOR} -DINVADER_VERSION_MINOR=${Invader_VERSION_MINOR} -DINVADER_VERSION_PATCH=${Invader_VERSION_PATCH})

include(src/invader.cmake)
include(src/build/build.cmake)
include(src/crc/crc.cmake)
include(src/indexer/indexer.cmake)
# include(src/scenario/scenario.cmake)
include(src/bitmap/bitmap.cmake)
include(src/resource/resource.cmake)
