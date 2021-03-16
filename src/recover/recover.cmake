# SPDX-License-Identifier: GPL-3.0-only

if(NOT ${TIFF_FOUND})
    set(INVADER_RECOVER false)
    message(WARNING "Unable to automatically find libtiff; invader-recover will be disabled")
endif()

if(NOT DEFINED ${INVADER_RECOVER})
    set(INVADER_RECOVER true CACHE BOOL "Build invader-recover (recovers source data from tags)")
endif()

if(${INVADER_RECOVER})
    add_executable(invader-recover
        src/recover/recover.cpp
        src/recover/recover_method.cpp
    )

    target_include_directories(invader-recover
        PUBLIC ${TIFF_INCLUDE_DIRS}
    )
    
    target_link_libraries(invader-recover ${TIFF_LIBRARIES} invader)

    set(TARGETS_LIST ${TARGETS_LIST} invader-recover)

    do_windows_rc(invader-recover invader-recover.exe "Tag source data recovery tool")
endif()

