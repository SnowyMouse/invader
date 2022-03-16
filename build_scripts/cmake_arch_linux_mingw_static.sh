#!/bin/bash

# Use this shell script if you want to compile Invader on Arch Linux via the
# ownstuff MinGW packages

# Check for arguments
if [[ "$#" -lt "1" ]]; then
    echo "Usage: $0 <invader-src> [...]"
    exit 1
fi

# Check if MinGW-w64 cmake exists
CMAKE=/usr/bin/x86_64-w64-mingw32-cmake
if [[ ! -f "$CMAKE" ]]; then
    echo "No cmake found at $CMAKE"
    echo "This script is intended to be used on Arch Linux with MinGW-w64 installed"
    exit 2
fi

# Make sure it points to an actual CMake project
BUILD_DEP_PATH=$(realpath "$1/build_scripts/cmake_arch_linux_mingw_static.cmake")
if [[ ! -f "$BUILD_DEP_PATH" ]]; then
    echo "No build script file found at $BUILD_DEP_PATH"
    $0
    exit 3
fi

$CMAKE "$@" "-DINVADER_BUILD_DEPENDENCY_SCRIPT=$BUILD_DEP_PATH" "-DINVADER_MINGW_PREFIX=/usr/x86_64-w64-mingw32"
