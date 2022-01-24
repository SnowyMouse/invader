# SPDX-License-Identifier: GPL-3.0-only

if(NOT DEFINED ${INVADER_EDIT_QT})
    set(INVADER_EDIT_QT true CACHE BOOL "Build invader-edit-qt (edits tags)")
endif()

if(${INVADER_EDIT_QT})
    if(${INVADER_WIN32_EXE_STATIC_LINK})
        set(USE_STATIC_QT_BY_DEFAULT ON)
    endif()
    
    find_package(Qt5 COMPONENTS Core Widgets Multimedia REQUIRED)

    set(CMAKE_AUTOMOC ON)
    set(CMAKE_AUTORCC ON)

    if(WIN32)
        set(INVADER_EDIT_QT_RC "src/edit/qt/qt.rc")
    endif()
    
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
        ${INVADER_EDIT_QT_RC}
    )

    if(WIN32)
        target_sources(invader-edit-qt PRIVATE src/edit/qt/theme.cpp)
    endif()
    
    if(${INVADER_WIN32_EXE_STATIC_LINK})
        target_link_libraries(invader-edit-qt -static qwindows qwindowsvistastyle Qt5::QWindowsAudioPlugin Qt5EventDispatcherSupport Qt5WindowsUIAutomationSupport Qt5VulkanSupport Qt5AccessibilitySupport Qt5FontDatabaseSupport Qt5ThemeSupport wtsapi32 qwindowsvistastyle)
        
        target_compile_definitions(invader-edit-qt PRIVATE INVADER_WIN32_EXE_STATIC_LINK)
    endif()

    target_link_libraries(invader-edit-qt invader Qt5::Widgets Qt5::Multimedia)
    target_include_directories(invader-edit-qt PUBLIC ${Qt5Widgets_INCLUDE_DIRS})

    set(TARGETS_LIST ${TARGETS_LIST} invader-edit-qt)
endif()
