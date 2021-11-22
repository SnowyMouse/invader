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

#include "../editor/tag_editor_window.hpp"

class QTreeWidget;
class QMenu;
class QLabel;

namespace Invader::EditQt {
    class TagTreeWidget;

    class TagFetcherThread : public QThread {
        Q_OBJECT
    public:
        TagFetcherThread(QObject *parent, const std::vector<std::filesystem::path> &all_paths);

    signals:
        void tag_count_changed(std::pair<std::mutex, std::size_t> *new_count);
        void fetch_finished(const std::vector<File::TagFile> *tags, int errors);

    private:
        void run() override;
        std::vector<std::filesystem::path> all_paths;
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
         * Open the tags when ready
         * @param tags      tags to open
         * @param full_path use full path mode
         */
        void open_tags_when_ready(const std::vector<std::string> &tags, bool full_path) {
            this->tags_to_open = tags;
            this->tags_to_open_full_path = full_path;
        }

        /**
         * Set whether or not safeguards are enabled
         * @param enabled
         */
        void set_safeguards(bool enabled) noexcept {
            this->safeguards_set = enabled;
        }

        /**
         * Get whether or not safeguards are enabled
         * @return enabled
         */
        bool safeguards() const noexcept {
            return this->safeguards_set;
        }

        /**
         * Set all the tag directories
         * @param directories tag directories
         */
        void set_tag_directories(const std::vector<std::filesystem::path> &directories);

        /**
         * Get all tag directories
         * @return tag directories
         */
        const std::vector<std::filesystem::path> &get_tag_directories() const noexcept;

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
        
        /**
         * Set whether or not to do fast listing
         * @param mode true to enable, false to disable
         */
        void set_fast_listing_mode(bool mode);
        
        /**
         * Get whether or not to do fast listing
         * @return true if fast listing mode is enabled
         */
        bool fast_listing_mode() const noexcept {
            return this->fast_listing;
        }

        virtual void closeEvent(QCloseEvent *event);
        virtual void paintEvent(QPaintEvent *event);

    signals:
        void tags_reloaded(TagTreeWindow *window);

    private:
        /** Reload the tags in the tag array */
        void reload_tags(bool reiterate_directories);

        /** Show the about window */
        void show_about_window();

        /** Free all closed tags */
        void cleanup_windows(TagEditorWindow *also_close = nullptr);

        /** Close all open tags and then cleanup */
        bool close_all_open_tags();

        /** Copy the file path */
        void perform_copy_file_path();

        /** Copy the virtual path */
        void perform_copy_virtual_path();
        
        /** Copy the virtual path without an extension */
        void perform_copy_virtual_path_without_extension();

        /** Copy the virtual directory path */
        void perform_copy_virtual_directory_path();

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
        void tags_reloaded_finished(const std::vector<File::TagFile> *result, int error_count);

        /** We're done */
        void tag_count_changed(std::pair<std::mutex, std::size_t> *count);
        
        /** Set count label */
        void set_count_label(std::size_t count);

        #ifdef SHOW_NIGHTLY_LINK
        /** Nightly build? */
        void show_nightly_build();

        /** Show the sauce! */
        void show_source_code();
        #endif

        enum : std::size_t {
            SHOW_ALL_MERGED = static_cast<std::size_t>(~0)
        };

        std::vector<File::TagFile> all_tags;

        std::vector<std::string> tags_to_open;
        bool tags_to_open_full_path = false;

        TagTreeWidget *tag_view;
        std::vector<std::filesystem::path> paths;
        std::size_t current_tag_index = SHOW_ALL_MERGED;

        QLabel *tag_count_label;
        QLabel *tag_loading_label;
        QLabel *tag_opening_label;

        std::vector<std::unique_ptr<TagEditorWindow>> open_documents;
        void on_double_click(QTreeWidgetItem *item, int column);

        bool initial_load = false;
        bool tags_reloading_queued = false;

        bool safeguards_set = true;

        bool opening_tag = false;

        TagFetcherThread *fetcher_thread;
        QWidget *filter_widget;
        QLineEdit *filter_textbox;
        
        std::size_t listing_errors = 0;
        
        bool fast_listing = false;
        
        void set_filter(const QString &filter);
        void toggle_filter_visible();
    };
}

#endif
