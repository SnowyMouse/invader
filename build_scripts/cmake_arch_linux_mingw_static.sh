#!/bin/bash

# Use this shell script if you want to compile Invader on Arch Linux via the
# ownstuff MinGW packages.
#
# All arguments passed into the shell script will be passed directly into CMake.

# Check if MinGW-w64 cmake exists
CMAKE=/usr/bin/x86_64-w64-mingw32-cmake
if [[ ! -f "${CMAKE}" ]]; then
    echo "No cmake found at ${CMAKE}"
    echo "This script is intended to be used on Arch Linux with MinGW-w64 installed"
    exit 2
fi

# Get our directories
SCRIPT_LOCATION="$(realpath "${BASH_SOURCE[0]}")"
SOURCE_DIR="$(realpath "$(dirname "${SCRIPT_LOCATION}")/..")"

# Make sure there's a proper CMakeLists.txt
if [[ ! -f "${SOURCE_DIR}/CMakeLists.txt" ]]; then
    echo "No CMakeLists.txt found at ${SOURCE_DIR}"
    echo "This script needs to be in the build_scripts of an Invader directory"
    exit 3
fi

# Make sure it points to an actual CMake project
BUILD_DEP_PATH=$(realpath "${SOURCE_DIR}/build_scripts/cmake_arch_linux_mingw_static.cmake")
if [[ ! -f "${BUILD_DEP_PATH}" ]]; then
    echo "No build script file found at ${BUILD_DEP_PATH}"
    exit 3
fi

# Print what we're running
function run_arguments_and_say_what_i_ran() {
    echo "=> ${@}"
    "${@}"
    exit $?
}

run_arguments_and_say_what_i_ran "${CMAKE}" "${SOURCE_DIR}" "${@}" "-DINVADER_BUILD_DEPENDENCY_SCRIPT=${BUILD_DEP_PATH}" "-DINVADER_MINGW_PREFIX=/usr/x86_64-w64-mingw32" "-DCMAKE_DISABLE_FIND_PACKAGE_harfbuzz=TRUE"
