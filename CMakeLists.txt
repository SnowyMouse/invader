# SPDX-License-Identifier: GPL-3.0-only

# Require at least version 3.14
cmake_minimum_required(VERSION 3.14)

# Project name - You should change this if you're making a fork
project(Invader
    VERSION 0.51.2
    DESCRIPTION "Modding toolkit for Halo Combat Evolved"
    LANGUAGES C CXX
)

# Use C++20
set(CMAKE_CXX_STANDARD 20)

# Use C11
set(CMAKE_C_STANDARD 11)

# Build shared libs?
option(BUILD_SHARED_LIBS "Build with shared libs" ON)

# Add our dependencies
include(dependencies.cmake)

# Start here
set(TARGETS_LIST invader)

# If we have Git, try to see if we are in a git repo
if(${GIT_FOUND})
    execute_process(
        COMMAND ${GIT_EXECUTABLE} --git-dir "${CMAKE_CURRENT_SOURCE_DIR}/.git" rev-parse --short HEAD
        OUTPUT_VARIABLE GIT_COMMIT_F
        ERROR_QUIET
    )
    if("${GIT_COMMIT_F}" STREQUAL "")
        set(IN_GIT_REPO FALSE)
    else()
        set(IN_GIT_REPO TRUE)
    endif()
else()
    set(IN_GIT_REPO FALSE)
endif()

# Do stuff depending on the compiler
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic -Wold-style-cast -Wno-missing-braces")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -march=native")
endif()

if(MINGW)
    # If it's MinGW, this may be required to prevent linking errors
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fstack-protector")

    # Windows 10 stuff too
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--major-os-version 10")
    
    # MinGW does this weird globbing thing by default to account for poor shells
    # when we do *not* want it since "*" is used for pattern matching.
    #
    # You can use bash on Windows now anyway.
    add_library(invader-crt-noglob OBJECT
        src/mingw_crt_noglob.cpp
    )
    set(INVADER_CRT_NOGLOB "invader-crt-noglob")
endif()

# Turn on console messages
option(INVADER_NO_OUTPUT "don't show most console messages including most errors" OFF)
if(INVADER_NO_OUTPUT)
    add_definitions(-DNO_OUTPUT)
endif()

# Define this if on Windows so %zu works
if(WIN32)
    add_definitions(-D__USE_MINGW_ANSI_STDIO=1)
endif()

function(do_windows_rc target file_name description)
    if(WIN32)
        target_sources(${target} PRIVATE src/windows.rc)
        target_compile_definitions(${target} PRIVATE "-DINVADER_BINARY_NAME=\\\"${target}\\\"")
        target_compile_definitions(${target} PRIVATE "-DINVADER_BINARY_FILE_NAME=\\\"${file_name}\\\"")
        target_compile_definitions(${target} PRIVATE "-DINVADER_BINARY_DESCRIPTION=\\\"${description}\\\"")
    endif()
endfunction()

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include(src/invader.cmake)
include(src/build/build.cmake)
include(src/index/index.cmake)
include(src/bitmap/bitmap.cmake)
include(src/resource/resource.cmake)
include(src/archive/archive.cmake)
include(src/dependency/dependency.cmake)
include(src/font/font.cmake)
include(src/string/string.cmake)
include(src/script/script.cmake)
include(src/info/info.cmake)
include(src/extract/extract.cmake)
include(src/sound/sound.cmake)
include(src/strip/strip.cmake)
include(src/refactor/refactor.cmake)
include(src/collection/collection.cmake)
include(src/bludgeon/bludgeon.cmake)
include(src/compare/compare.cmake)
include(src/scan/scan.cmake)
include(src/convert/convert.cmake)
include(src/edit/edit.cmake)
include(src/model/model.cmake)
include(src/recover/recover.cmake)
include(src/lightmap/lightmap.cmake)

# Qt stuff
include(src/edit/qt/qt.cmake)

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
