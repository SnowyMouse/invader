# SPDX-License-Identifier: GPL-3.0-only

if(NOT DEFINED ${INVADER_EDIT_QT})
    set(INVADER_EDIT_QT true CACHE BOOL "Build invader-edit-qt (edits tags)")
endif()

if(${INVADER_EDIT_QT})
    set(CMAKE_AUTOMOC ON)
    set(CMAKE_AUTORCC ON)

    add_executable(invader-edit-qt
        src/edit/qt/qt.cpp
        src/edit/qt/editor/subwindow/tag_editor_bitmap_subwindow.cpp
        src/edit/qt/editor/subwindow/tag_editor_font_subwindow.cpp
        src/edit/qt/editor/subwindow/tag_editor_sound_subwindow.cpp
        src/edit/qt/editor/subwindow/tag_editor_string_subwindow.cpp
        src/edit/qt/editor/subwindow/tag_editor_subwindow.cpp
        src/edit/qt/editor/widget/tag_editor_array_widget.cpp
        src/edit/qt/editor/widget/tag_editor_edit_widget_view.cpp
        src/edit/qt/editor/widget/tag_editor_edit_widget.cpp
        src/edit/qt/editor/widget/tag_editor_group_widget.cpp
        src/edit/qt/editor/widget/tag_editor_widget.cpp
        src/edit/qt/editor/tag_editor_window.cpp
        src/edit/qt/tree/tag_tree_dialog.cpp
        src/edit/qt/tree/tag_tree_widget.cpp
        src/edit/qt/tree/tag_tree_window.cpp
        src/edit/qt/qtres.qrc
    )

    if(WIN32)
        target_sources(invader-edit-qt
            PRIVATE src/edit/qt/theme.cpp
            PRIVATE src/edit/qt/qt.rc
        )
        # We don't need these and it makes static linking on MSYS2 less painful
        qt_import_plugins(invader-edit-qt EXCLUDE_BY_TYPE bearer imageformats sqldrivers)

        # Make sure this is set
        if(MINGW)
            target_link_options(invader-edit-qt PRIVATE "-mwindows")
        endif()
    endif()

    target_link_libraries(invader-edit-qt invader Qt6::Widgets ${SDL2_LIBRARIES} ${INVADER_CRT_NOGLOB})
    target_include_directories(invader-edit-qt PUBLIC ${Qt6Widgets_INCLUDE_DIRS} ${SDL2_INCLUDE_DIRS})

    set(TARGETS_LIST ${TARGETS_LIST} invader-edit-qt)
endif()
