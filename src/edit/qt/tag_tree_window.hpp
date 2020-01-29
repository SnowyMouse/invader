// SPDX-License-Identifier: GPL-3.0-only

#ifndef INVADER__EDIT__QT__TagTreeWindow_HPP
#define INVADER__EDIT__QT__TagTreeWindow_HPP

#include <QMainWindow>
#include <QTreeWidgetItem>
#include <vector>
#include <filesystem>
#include <QObject>

#include "tag_file.hpp"
#include "tag_editor_window.hpp"

class QTreeWidget;
class QMenu;
class QLabel;

namespace Invader::EditQt {
    class TagTreeWidget;

    class TagTreeWindow : public QMainWindow {
        friend class TagEditorWindow;
        Q_OBJECT
    public:
        TagTreeWindow();

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

        /**
         * Refresh the view, reloading the tags
         */
        void refresh_view();

        /**
         * Close event
         * @param event event pointer
         */
        virtual void closeEvent(QCloseEvent *event);

    signals:
        void tags_reloaded(TagTreeWindow *window);

    private:
        /** Reload the tags in the tag array */
        void reload_tags();

        /** Show the about window */
        void show_about_window();

        /** Free all closed tags */
        void cleanup_windows();

        /** Close all open tags and then cleanup */
        bool close_all_open_tags();

        /** Make a new document */
        bool perform_new();

        /** Delete a tag */
        bool perform_delete();

        enum : std::size_t {
            SHOW_ALL_MERGED = static_cast<std::size_t>(~0)
        };

        std::vector<TagFile> all_tags;

        TagTreeWidget *tag_view;
        std::vector<std::filesystem::path> paths;
        std::size_t current_tag_index = SHOW_ALL_MERGED;

        QLabel *tag_count_label;
        QLabel *tag_location_label;

        std::vector<std::unique_ptr<TagEditorWindow>> open_documents;
        void on_double_click(QTreeWidgetItem *item, int column);
    };
}

#endif
