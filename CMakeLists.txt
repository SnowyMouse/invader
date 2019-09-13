#
# Invader (c) 2018 Kavawuvi
#
# This program is free software under the GNU General Public License v3.0 or later. See LICENSE for more information.
#

# Require at least version 3.10
cmake_minimum_required(VERSION 3.10)

# Project name - You should change this if you're making a fork
project(Invader
    VERSION 0.9.0
    DESCRIPTION "Modding toolkit for Halo Custom Edition"
    LANGUAGES C CXX
)

# Use C++17
set(CMAKE_CXX_STANDARD 17)

# Use C99
set(CMAKE_C_STANDARD 99)

# Find some packages
find_package(TIFF)
find_package(ZLIB)
find_package(Python3)
find_package(Git)
find_package(LibArchive)
find_package(Freetype)

# If we don't have Python, bail
if(${Python3_FOUND} STREQUAL FALSE)
    message(FATAL_ERROR "Python 3 is required to build Invader.")
endif()

# If we have Git, try to see if we are in a git repo
if(${GIT_FOUND})
    execute_process(
        COMMAND ${GIT_EXECUTABLE} --git-dir '${CMAKE_CURRENT_SOURCE_DIR}/.git' rev-parse --short HEAD
        OUTPUT_VARIABLE GIT_COMMIT_F
        ERROR_QUIET
    )
    if("${GIT_COMMIT_F}" STREQUAL "")
        set(IN_GIT_REPO TRUE)
    else()
        set(IN_GIT_REPO FALSE)
    endif()
else()
    set(IN_GIT_REPO TRUE)
endif()

# Do stuff depending on the compiler
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic -Wold-style-cast")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -march=native")
endif()

# Here's to set up constants for version.hpp
add_definitions(-DINVADER_VERSION_MAJOR=${PROJECT_VERSION_MAJOR} -DINVADER_VERSION_MINOR=${PROJECT_VERSION_MINOR} -DINVADER_VERSION_PATCH=${PROJECT_VERSION_PATCH} "-DINVADER_FORK=\"${PROJECT_NAME}\"")

# Turn on console messages
option(NO_OUTPUT "don't show any console messages" OFF)
if(NO_OUTPUT)
    add_definitions(-DNO_OUTPUT)
endif()

include(src/invader.cmake)
include(src/build/build.cmake)
include(src/crc/crc.cmake)
include(src/indexer/indexer.cmake)
include(src/bitmap/bitmap.cmake)
include(src/resource/resource.cmake)
include(src/archive/archive.cmake)
include(src/dependency/dependency.cmake)
include(src/font/font.cmake)
include(src/string/string.cmake)
