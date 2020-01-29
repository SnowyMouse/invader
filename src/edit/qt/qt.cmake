# SPDX-License-Identifier: GPL-3.0-only

if(NOT DEFINED ${INVADER_EDIT_QT})
    set(INVADER_EDIT_QT true CACHE BOOL "Build invader-edit-qt (edits tags)")
endif()

if(${INVADER_EDIT_QT})
    find_package(Qt5 COMPONENTS Core Widgets REQUIRED)

    add_executable(invader-edit-qt
        src/edit/qt/qt.cpp
        src/edit/qt/tag_tree_widget.cpp
        src/edit/qt/tag_tree_window.cpp
    )
    target_link_libraries(invader-edit-qt invader Qt5::Widgets)
    target_include_directories(invader-edit-qt PUBLIC ${Qt5Widgets_INCLUDE_DIRS})

    set(TARGETS_LIST ${TARGETS_LIST} invader-edit-qt)
endif()
