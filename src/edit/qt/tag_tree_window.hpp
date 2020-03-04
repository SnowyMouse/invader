// SPDX-License-Identifier: GPL-3.0-only

#ifndef INVADER__EDIT__QT__TagTreeWindow_HPP
#define INVADER__EDIT__QT__TagTreeWindow_HPP

#include <QMainWindow>
#include <QTreeWidgetItem>
#include <vector>
#include <filesystem>
#include <QObject>
#include <QThread>
#include <invader/file/file.hpp>

#include "tag_editor_window.hpp"

class QTreeWidget;
class QMenu;
class QLabel;

namespace Invader::EditQt {
    class TagTreeWidget;

    class TagFetcherThread : public QThread {
        Q_OBJECT
    public:
        TagFetcherThread(QObject *parent, const std::vector<std::string> &all_paths);

    signals:
        void tag_count_changed(std::pair<std::mutex, std::size_t> *new_count);
        void fetch_finished(const std::vector<File::TagFile> *tags);

    private:
        void run() override;
        std::vector<std::string> all_paths;
        std::vector<File::TagFile> all_tags;
        std::pair<std::mutex, std::size_t> statuser;
        std::size_t last_tag_count;
    };

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
        const std::vector<File::TagFile> &get_all_tags() const noexcept;

        /**
         * Refresh the view, reloading the tags
         */
        void refresh_view();

        /**
         * Attempt to open the tag with the given path
         * @param path      path to open
         * @param full_path this is the full path
         */
        void open_tag(const char *path, bool full_path);

        virtual void closeEvent(QCloseEvent *event);
        virtual void paintEvent(QPaintEvent *event);

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

        /** Open an existing document */
        void perform_open();

        /** Make a new document */
        bool perform_new();

        /** Delete a tag */
        bool perform_delete();

        /** Refactor a tag */
        bool perform_refactor();

        /** Info about a tag */
        bool show_properties();

        /** Show the context menu */
        void show_context_menu(const QPoint &point);

        /** We're done */
        void tags_reloaded_finished(const std::vector<File::TagFile> *result);

        /** We're done */
        void tag_count_changed(std::pair<std::mutex, std::size_t> *count);

        /** Show the sauce! */
        void show_source_code();

        #ifdef SHOW_NIGHTLY_LINK
        /** Nightly build? */
        void show_nightly_build();
        #endif

        enum : std::size_t {
            SHOW_ALL_MERGED = static_cast<std::size_t>(~0)
        };

        std::vector<File::TagFile> all_tags;

        TagTreeWidget *tag_view;
        std::vector<std::filesystem::path> paths;
        std::size_t current_tag_index = SHOW_ALL_MERGED;

        QLabel *tag_count_label;
        QLabel *tag_loading_label;

        std::vector<std::unique_ptr<TagEditorWindow>> open_documents;
        void on_double_click(QTreeWidgetItem *item, int column);

        bool initial_load = false;
        bool tags_reloading_queued = false;

        TagFetcherThread *fetcher_thread;
    };
}

#endif
