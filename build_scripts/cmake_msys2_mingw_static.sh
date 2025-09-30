#!/bin/bash

# Use this shell script if you want to compile Invader with MSYS2 MinGW-w64 toolchains.
#
# All arguments passed into the shell script will be passed directly into CMake.

# Check we are on some MSYS2 subsystem
if [ -z "${MSYSTEM_PREFIX}" ]; then
    echo "MSYSTEM_PREFIX must be set to run this script"
    echo "This script is intended to be used on MSYS2 within either the UCRT64 or MINGW64 subsystems"
    exit 1
fi

# Check if CMake exists
if command -v cmake &> /dev/null; then
    CMAKE=cmake
else
    echo "CMake was not found"
    echo "CMake is required for building invader"
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
BUILD_DEP_PATH=$(realpath "${SOURCE_DIR}/build_scripts/cmake_msys2_mingw_static.cmake")
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

run_arguments_and_say_what_i_ran "${CMAKE}" "${SOURCE_DIR}" "${@}" \
                                 "-DINVADER_BUILD_DEPENDENCY_SCRIPT=${BUILD_DEP_PATH}" \
                                 "-DCMAKE_PREFIX_PATH=${MSYSTEM_PREFIX}/qt6-static" \
                                 "-DINVADER_MINGW_PREFIX=${MSYSTEM_PREFIX}"
