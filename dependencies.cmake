# SPDX-License-Identifier: GPL-3.0-only

# We need Python
if(NOT DEFINED Python3_EXECUTABLE)
    set(Python3_EXECUTABLE "Python3-NotFound" CACHE PATH "Path to the python3 executable (required for building Invader)")
    if(${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.12.4")
        find_package(Python3)
    endif()
endif()
if(NOT DEFINED Python3_EXECUTABLE)
    message(WARNING "Unable to automatically find Python3; You will need to manually define Python3_EXECUTABLE to build Invader")
endif()

# Find some packages
find_package(TIFF)
find_package(ZLIB)
find_package(LibArchive)
find_package(Freetype)
find_package(Git)
