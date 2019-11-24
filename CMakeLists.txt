# SPDX-License-Identifier: GPL-3.0-only

# Require at least version 3.12
cmake_minimum_required(VERSION 3.12)

# Project name - You should change this if you're making a fork
project(Invader
    VERSION 0.19.2
    DESCRIPTION "Modding toolkit for Halo Combat Evolved on the PC"
    LANGUAGES C CXX
)

# Use C++17
set(CMAKE_CXX_STANDARD 17)

# Use C99
set(CMAKE_C_STANDARD 99)

# Add our dependencies
include(dependencies.cmake)

# Start here
set(TARGETS_LIST invader)

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
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic -Wold-style-cast -Wno-missing-braces")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -march=native")
endif()

# If it's MinGW, this may be required to prevent linking errors
if(MINGW)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fstack-protector")
endif()

# Turn on console messages
option(INVADER_NO_OUTPUT "don't show most console messages including most errors" OFF)
if(INVADER_NO_OUTPUT)
    add_definitions(-DNO_OUTPUT)
endif()

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include(src/invader.cmake)
include(src/build/build.cmake)
include(src/indexer/indexer.cmake)
include(src/bitmap/bitmap.cmake)
include(src/resource/resource.cmake)
include(src/archive/archive.cmake)
include(src/dependency/dependency.cmake)
include(src/font/font.cmake)
include(src/string/string.cmake)
include(src/compress/compress.cmake)
include(src/info/info.cmake)
include(src/extract/extract.cmake)

# Install stuff
install(
    DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/invader
    DESTINATION include
    FILES_MATCHING PATTERN "*.h" PATTERN "*.hpp"
)

# Install programs
install(
    TARGETS ${TARGETS_LIST}
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
)
