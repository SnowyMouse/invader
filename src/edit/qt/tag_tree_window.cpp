// SPDX-License-Identifier: GPL-3.0-only

#include <QMenuBar>
#include <QDialog>
#include <QLayout>
#include <QLabel>
#include <QTreeWidget>
#include <QFontDatabase>
#include <QMessageBox>
#include <QApplication>
#include <QStatusBar>
#include <QCloseEvent>
#include "tag_tree_window.hpp"
#include "tag_tree_widget.hpp"
#include "tag_tree_dialog.hpp"
#include <invader/version.hpp>
#include <invader/file/file.hpp>

namespace Invader::EditQt {
    TagTreeWindow::TagTreeWindow() {
        // Set some window stuff
        this->setWindowTitle("invader-edit-qt");
        this->setMinimumSize(800, 600);

        // Make and set our menu bar
        QMenuBar *bar = new QMenuBar(this);
        this->setMenuBar(bar);

        // File menu
        auto *file_menu = bar->addMenu("File");
        auto *exit = file_menu->addAction("Exit");
        exit->setShortcut(QKeySequence::Quit);
        connect(exit, &QAction::triggered, this, &TagTreeWindow::close);
        // View menu
        auto *view_menu = bar->addMenu("View");
        auto *refresh = view_menu->addAction("Refresh");
        refresh->setShortcut(QKeySequence::Refresh);
        connect(refresh, &QAction::triggered, this, &TagTreeWindow::refresh_view);

        // Help menu
        auto *help_menu = bar->addMenu("Help");
        auto *about = help_menu->addAction("About");
        connect(about, &QAction::triggered, this, &TagTreeWindow::show_about_window);

        // Now, set up the layout
        auto *central_widget = new QWidget(this);
        auto *vbox_layout = new QVBoxLayout(central_widget);
        this->tag_view = new TagTreeWidget(this, this);
        vbox_layout->addWidget(this->tag_view);
        vbox_layout->setMargin(0);
        central_widget->setLayout(vbox_layout);
        this->setCentralWidget(central_widget);
        connect(this->tag_view, &TagTreeWidget::itemDoubleClicked, this, &TagTreeWindow::on_double_click);

        // Next, set up the status bar
        QStatusBar *status_bar = new QStatusBar();
        this->tag_count_label = new QLabel();
        this->tag_location_label = new QLabel();
        this->tag_count_label->setAlignment(Qt::AlignRight | Qt::AlignTop);
        this->tag_location_label->setAlignment(Qt::AlignLeft | Qt::AlignTop);
        status_bar->addWidget(this->tag_location_label, 1);
        status_bar->addWidget(this->tag_count_label, 0);
        this->setStatusBar(status_bar);
    }

    void TagTreeWindow::refresh_view() {
        this->reload_tags();
        char tag_count_str[256];
        auto tag_count = this->tag_view->get_total_tags();
        std::snprintf(tag_count_str, sizeof(tag_count_str), "%zu tag%s", tag_count, tag_count == 1 ? "" : "s");
        this->tag_count_label->setText(tag_count_str);
    }

    void TagTreeWindow::show_about_window() {
        // Instantiate it
        QDialog dialog;
        dialog.setWindowTitle("About");
        dialog.setWindowFlags(Qt::Dialog | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);

        // Make a layout
        auto *vbox_layout = new QVBoxLayout(&dialog);
        vbox_layout->setSizeConstraint(QLayout::SetFixedSize);

        // Show the version
        QLabel *label = new QLabel(full_version_and_credits());
        label->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
        label->setAlignment(Qt::AlignLeft | Qt::AlignTop);
        vbox_layout->addWidget(label);

        // Set our layout and disable resizing
        dialog.setLayout(vbox_layout);

        // Done. Show it!
        dialog.exec();
    }

    void TagTreeWindow::set_tag_directories(const std::vector<std::filesystem::path> &directories) {
        this->paths = directories;
        this->current_tag_index = SHOW_ALL_MERGED;
        this->refresh_view();
    }

    void TagTreeWindow::reload_tags() {
        // Clear all tags
        auto &all_tags = this->all_tags;
        all_tags.clear();

        // Go through the directory and all directories it references
        auto iterate_directories = [&all_tags](const std::vector<std::string> &the_story_thus_far, const std::filesystem::path &dir, auto &iterate_directories, int depth, std::size_t priority, const std::vector<std::string> &main_dir) -> void {
            if(++depth == 256) {
                return;
            }

            for(auto &d : std::filesystem::directory_iterator(dir)) {
                std::vector<std::string> add_dir = the_story_thus_far;
                auto file_path = d.path();
                add_dir.emplace_back(file_path.filename().string());
                if(d.is_directory()) {
                    iterate_directories(add_dir, d, iterate_directories, depth, priority, main_dir);
                }
                else if(file_path.has_extension()) {
                    auto extension = file_path.extension().string();
                    auto tag_class_int = HEK::extension_to_tag_class(extension.c_str() + 1);

                    // First, make sure it's valid
                    if(tag_class_int == HEK::TagClassInt::TAG_CLASS_NULL || tag_class_int == HEK::TagClassInt::TAG_CLASS_NONE) {
                        continue;
                    }

                    // Next, add it
                    TagFile file;
                    file.full_path = file_path;
                    file.tag_class_int = tag_class_int;
                    file.tag_path_separated = std::move(add_dir);
                    file.tag_directory = priority;
                    file.tag_path = Invader::File::file_path_to_tag_path(file_path.string(), main_dir, false).value();
                    all_tags.emplace_back(std::move(file));
                }
            }
        };

        // Go through each directory
        std::size_t dir_count = this->paths.size();
        for(std::size_t i = 0; i < dir_count; i++) {
            auto &d = this->paths[i];
            auto dir_str = d.string();
            try {
                iterate_directories(std::vector<std::string>(), d, iterate_directories, 0, i, std::vector<std::string>(&dir_str, &dir_str + 1));
            }
            catch (std::filesystem::filesystem_error &e) {
                char formatted_error[512];
                std::snprintf(formatted_error, sizeof(formatted_error), "Failed to list tags due to an exception error:\n\n%s\n\nMake sure your tag directories are correct and that you have permission.", e.what());
                QMessageBox(QMessageBox::Icon::Critical, "Error", formatted_error, QMessageBox::Ok, this).exec();
                return;
            }
        }

        emit tags_reloaded(this);
    }

    const std::vector<TagFile> &TagTreeWindow::get_all_tags() const noexcept {
        return this->all_tags;
    }

    void TagTreeWindow::closeEvent(QCloseEvent *event) {
        for(auto &w : this->open_documents) {
            if(!w->isHidden()) {
                w->close();
            }
        }
        this->cleanup_windows();
        event->setAccepted(this->open_documents.size() == 0);
    }

    void TagTreeWindow::on_double_click(QTreeWidgetItem *, int) {
        this->cleanup_windows();

        const auto *tag = this->tag_view->get_selected_tag();
        if(tag) {
            for(auto &doc : this->open_documents) {
                if(doc->get_file().full_path == tag->full_path) {
                    doc->raise();
                    QApplication::setActiveWindow(doc.get());
                    return;
                }
            }
            this->open_documents.emplace_back(std::make_unique<TagEditorWindow>(this, this, *tag))->show();
        }
    }

    void TagTreeWindow::cleanup_windows() {
        bool window_closed;
        do {
            window_closed = false;
            for(auto &w : this->open_documents) {
                if(w->isHidden()) {
                    this->open_documents.erase(this->open_documents.begin() + (&w - this->open_documents.data()));
                    window_closed = true;
                    break;
                }
            }
        }
        while(window_closed);
    }

    bool TagTreeWindow::close_all_open_tags() {
        for(auto &w : this->open_documents) {
            if(!w->isHidden() && !w->close()) {
                this->cleanup_windows();
                return false;
            }
        }
        this->cleanup_windows();
        return true;
    }
}
