// SPDX-License-Identifier: GPL-3.0-only

#ifndef INVADER__EDIT__QT__TAG_WINDOW_HPP
#define INVADER__EDIT__QT__TAG_WINDOW_HPP

#include <QMainWindow>
#include <QTreeWidgetItem>
#include <vector>
#include <filesystem>

#include "tag_file.hpp"

class QTreeWidget;
class QMenu;
class QLabel;

namespace Invader::EditQt {
    class TagTreeWidget;

    class TagWindow : public QMainWindow {
    public:
        TagWindow();

        /**
         * Set all the tag directories
         * @param directories tag directories
         */
        void set_tag_directories(const std::vector<std::filesystem::path> &directories);

        /**
         * Get all of the tags available
         * @return all tags available
         */
        const std::vector<TagFile> &get_all_tags() const noexcept;

    private:
        void show_about_window();
        void refresh_view();
        void reload_tags();

        enum : std::size_t {
            SHOW_ALL_MERGED = static_cast<std::size_t>(~0)
        };

        std::vector<TagFile> all_tags;

        TagTreeWidget *tag_view;
        std::vector<std::filesystem::path> paths;
        std::size_t current_tag_index = SHOW_ALL_MERGED;

        QLabel *tag_count_label;
        QLabel *tag_location_label;
    };
}

#endif
