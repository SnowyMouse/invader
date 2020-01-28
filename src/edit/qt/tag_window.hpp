// SPDX-License-Identifier: GPL-3.0-only

#ifndef INVADER__EDIT__QT__TAG_WINDOW_HPP
#define INVADER__EDIT__QT__TAG_WINDOW_HPP

#include <QMainWindow>
#include <vector>
#include <filesystem>

class QTreeWidget;
class QMenu;
class QLabel;

namespace Invader::EditQt {
    class TagTreeWidget;

    class TagWindow : public QMainWindow {
    public:
        TagWindow();
    private:
        void show_about_window();
        void refresh_view();

        enum : std::size_t {
            SHOW_ALL_MERGED = static_cast<std::size_t>(~0)
        };

        TagTreeWidget *tag_view;
        std::vector<std::filesystem::path> paths;
        std::size_t current_tag_index = SHOW_ALL_MERGED;

        QMenu *pick_tag_array;
        QLabel *tag_count_label;
        QLabel *tag_location_label;
    };
}

#endif
