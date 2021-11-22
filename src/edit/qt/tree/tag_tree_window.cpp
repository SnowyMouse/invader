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
#include <QClipboard>
#include <QDesktopWidget>
#include <QDesktopServices>
#include <QInputDialog>
#include <QThread>
#include "tag_tree_window.hpp"
#include "tag_tree_widget.hpp"
#include "tag_tree_dialog.hpp"
#include <invader/version.hpp>
#include <invader/file/file.hpp>
#include <invader/tag/parser/parser.hpp>
#include <QScreen>

#ifdef _WIN32
#include "../theme.hpp"
#endif

namespace Invader::EditQt {
    TagTreeWindow::TagTreeWindow() {
        #ifdef _WIN32
        SixShooter::Theme::set_win32_theme();
        #endif
        
        // Set some window stuff
        this->setWindowTitle("invader-edit-qt");
        this->setMinimumSize(800, 600);

        // Make and set our menu bar
        QMenuBar *bar = new QMenuBar(this);
        this->setMenuBar(bar);

        // File menu
        auto *file_menu = bar->addMenu("File");

        auto *new_document = file_menu->addAction("New...");
        new_document->setIcon(QIcon::fromTheme(QStringLiteral("document-new")));
        new_document->setShortcut(QKeySequence::New);
        connect(new_document, &QAction::triggered, this, &TagTreeWindow::perform_new);

        auto *delete_tag = file_menu->addAction("Delete...");
        delete_tag->setIcon(QIcon::fromTheme(QStringLiteral("edit-delete")));
        delete_tag->setShortcut(QKeySequence::Delete);
        connect(delete_tag, &QAction::triggered, this, &TagTreeWindow::perform_delete);

        file_menu->addSeparator();

        auto *close_all_open = file_menu->addAction("Close all");
        close_all_open->setIcon(QIcon::fromTheme(QStringLiteral("document-close")));
        connect(close_all_open, &QAction::triggered, this, &TagTreeWindow::close_all_open_tags);

        file_menu->addSeparator();

        auto *exit = file_menu->addAction("Quit");
        exit->setIcon(QIcon::fromTheme(QStringLiteral("application-exit")));
        exit->setShortcut(QKeySequence::Quit);
        connect(exit, &QAction::triggered, this, &TagTreeWindow::close);

        // View menu
        auto *view_menu = bar->addMenu("View");
        auto *refresh = view_menu->addAction("Refresh");
        refresh->setShortcut(QKeySequence::Refresh);
        refresh->setIcon(QIcon::fromTheme(QStringLiteral("view-refresh")));
        connect(refresh, &QAction::triggered, this, &TagTreeWindow::refresh_view);
        view_menu->addSeparator();
        auto *filter = view_menu->addAction("Toggle Filter");
        filter->setShortcut(QKeySequence::Find);
        filter->setIcon(QIcon::fromTheme(QStringLiteral("edit-find")));
        connect(filter, &QAction::triggered, this, &TagTreeWindow::toggle_filter_visible);

        // Help menu
        auto *help_menu = bar->addMenu("Help");
        auto *about = help_menu->addAction("About invader-edit-qt...");
        about->setIcon(QIcon::fromTheme(QStringLiteral("help-about")));
        connect(about, &QAction::triggered, this, &TagTreeWindow::show_about_window);

        help_menu->addSeparator();

        // Source and binary buttons are only relevant if we're on a nightly build, as building from source may make these inconsistent with what code was used
        #ifdef SHOW_NIGHTLY_LINK
        auto *source = help_menu->addAction("View source code...");
        source->setIcon(QIcon::fromTheme(QStringLiteral("help-about")));
        connect(source, &QAction::triggered, this, &TagTreeWindow::show_source_code);
        
        auto *nightly = help_menu->addAction("Nightly builds...");
        connect(nightly, &QAction::triggered, this, &TagTreeWindow::show_nightly_build);
        #endif

        // Now, set up the layout
        auto *central_widget = new QWidget(this);
        auto *vbox_layout = new QVBoxLayout(central_widget);
        this->tag_view = new TagTreeWidget(this, this);
        this->tag_view->setContextMenuPolicy(Qt::CustomContextMenu);
        connect(this->tag_view, &TagTreeWidget::customContextMenuRequested, this, &TagTreeWindow::show_context_menu);
        vbox_layout->addWidget(this->tag_view);
        vbox_layout->setContentsMargins(2, 2, 2, 2);
        
        // Add a filter thing
        this->filter_widget = new QWidget(this);
        auto *filter_layout = new QHBoxLayout(filter_widget);
        filter_layout->setContentsMargins(0, 0, 0, 0);
        this->filter_widget->setVisible(false);
        this->filter_widget->setLayout(filter_layout);
        auto *filter_text = new QLabel("Filter:");
        filter_layout->addWidget(filter_text);
        this->filter_textbox = new QLineEdit();
        this->filter_textbox->setPlaceholderText("Example: *.scenario (shows all scenario tags)");
        filter_layout->addWidget(this->filter_textbox);
        vbox_layout->addWidget(this->filter_widget);
        connect(this->filter_textbox, &QLineEdit::textChanged, this, &TagTreeWindow::set_filter);
        
        // Finally set the layout
        central_widget->setLayout(vbox_layout);
        this->setCentralWidget(central_widget);
        connect(this->tag_view, &TagTreeWidget::itemDoubleClicked, this, &TagTreeWindow::on_double_click);

        // Next, set up the status bar
        QStatusBar *status_bar = new QStatusBar();
        this->tag_count_label = new QLabel();
        this->tag_loading_label = new QLabel();
        this->tag_opening_label = new QLabel();
        this->tag_opening_label->setAlignment(Qt::AlignLeft | Qt::AlignTop);
        this->tag_loading_label->setAlignment(Qt::AlignLeft | Qt::AlignTop);
        this->tag_count_label->setAlignment(Qt::AlignRight | Qt::AlignTop);
        status_bar->addWidget(this->tag_loading_label, 0);
        status_bar->addWidget(this->tag_opening_label, 1);
        status_bar->addWidget(this->tag_count_label, 2);
        this->setStatusBar(status_bar);

        // Set more stuff
        this->setWindowFlag(Qt::WindowStaysOnTopHint, 0);

        // Center this
        this->setGeometry(
            QStyle::alignedRect(
                Qt::LeftToRight,
                Qt::AlignCenter,
                this->size(),
                QGuiApplication::primaryScreen()->geometry()
            )
        );
    }

    const std::vector<std::filesystem::path> &TagTreeWindow::get_tag_directories() const noexcept {
        return this->paths;
    }
    
    void TagTreeWindow::set_count_label(std::size_t count) {
        char tag_count_str[256];
        std::snprintf(tag_count_str, sizeof(tag_count_str), "%zu tag%s", count, count == 1 ? "" : "s");
        this->tag_count_label->setText(tag_count_str);
    }

    void TagTreeWindow::tag_count_changed(std::pair<std::mutex, std::size_t> *tag_count) {
        tag_count->first.lock();
        this->set_count_label(tag_count->second);
        tag_count->first.unlock();
    }

    void TagTreeWindow::refresh_view() {
        this->reload_tags(true);
    }

    void TagTreeWindow::show_about_window() {
        // Instantiate it
        QDialog dialog;
        dialog.setWindowTitle("About");
        dialog.setWindowFlags(Qt::Dialog | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);

        // Make a layout
        auto *vbox_layout = new QVBoxLayout(&dialog);
        vbox_layout->setSizeConstraint(QLayout::SetFixedSize);
        
        // Add an icon thing
        QLabel *icon_label = new QLabel();
        QWidget *icon_label_widget = new QWidget();
        QHBoxLayout *icon_hbox_layout = new QHBoxLayout();
        icon_hbox_layout->addSpacing(1);
        icon_hbox_layout->addWidget(icon_label);
        icon_hbox_layout->addSpacing(1);
        icon_label_widget->setLayout(icon_hbox_layout);
        icon_label->setSizePolicy(QSizePolicy::Policy::Fixed, QSizePolicy::Policy::Fixed);
        icon_label->setPixmap(QPixmap(":icon/invader-edit-qt.png"));

        // Show the version
        QLabel *label = new QLabel(full_version_and_credits());
        int id = QFontDatabase::addApplicationFont(":icon/roboto-mono.ttf");
        QString family = QFontDatabase::applicationFontFamilies(id).at(0);
        QFont monospace(family);
        label->setFont(monospace);
        label->setTextFormat(Qt::RichText);
        label->setAlignment(Qt::AlignLeft | Qt::AlignTop);
        label->setTextInteractionFlags(Qt::TextBrowserInteraction);
        label->setOpenExternalLinks(true);
        
        // Replace line breaks with <br>s
        label->setText(label->text().replace("  ", "&nbsp;&nbsp;"));
        label->setText(label->text().replace("\n", "<br />"));
        
        // Replace the hyperlinks
        #define REPLACE_HL(what) label->setText(label->text().replace(label->text().indexOf(what), QString(what).size(), "<a href=\"" what "\">" what "</a>"));
        REPLACE_HL("https://invader.opencarnage.net/builds/nightly/")
        REPLACE_HL("https://invader.opencarnage.net")
        REPLACE_HL("https://github.com/nothings/stb");
        REPLACE_HL("https://www.freetype.org/");
        REPLACE_HL("https://github.com/SnowyMouse/invader");
        #undef REPLACE_HL
        
        vbox_layout->addWidget(icon_label_widget);
        vbox_layout->addWidget(label);

        // Set our layout and disable resizing
        dialog.setLayout(vbox_layout);

        // Done. Show it!
        dialog.exec();
    }

    void TagTreeWindow::set_tag_directories(const std::vector<std::filesystem::path> &directories) {
        this->paths = directories;
        this->current_tag_index = SHOW_ALL_MERGED;

        if(this->initial_load) {
            this->refresh_view();
        }
        
        // Set the title
        std::string title;
        for(auto &i : directories) {
            auto str = i.string();
            if(!title.empty()) {
                title += ", ";
                
                if(title.size() + str.size() > 512) {
                    title += "...";
                    break;
                }
            }
            title += str;
        }
        title += std::string(" — ") + qApp->applicationDisplayName().toStdString();
        this->setWindowTitle(title.c_str());
    }

    void TagTreeWindow::paintEvent(QPaintEvent *event) {
        if(!initial_load) {
            event->accept();
            this->initial_load = true;
            this->refresh_view();
        }
    }

    void TagFetcherThread::run() {
        // Function for loading it
        std::size_t error_count;
        auto load_it = [&error_count](std::vector<File::TagFile> *to, std::vector<std::filesystem::path> *all_paths, std::pair<std::mutex, std::size_t> *statuser) {
            *to = Invader::File::load_virtual_tag_folder(*all_paths, false, statuser, &error_count);
        };

        // Run this in parallel
        QThread *t = QThread::create(load_it, &this->all_tags, &this->all_paths, &this->statuser);
        t->start();
        std::size_t last_tag_count = 0;
        while(!t->isFinished()) {
            this->statuser.first.lock();
            std::size_t new_count = this->statuser.second;
            this->statuser.first.unlock();
            if(new_count > last_tag_count + 100) {
                last_tag_count = new_count;
                emit tag_count_changed(&this->statuser);
            }
        }
        delete t;

        // Emit one last signal
        emit fetch_finished(&this->all_tags, static_cast<int>(error_count));
    }

    TagFetcherThread::TagFetcherThread(QObject *parent, const std::vector<std::filesystem::path> &all_paths) : QThread(parent), all_paths(all_paths) {}

    void TagTreeWindow::reload_tags(bool reiterate_directories) {
        // Ensure we only reload once
        if(this->tags_reloading_queued) {
            return;
        }
        
        // If we have fast listing mode, we don't need to do much
        if(this->fast_listing) {
            this->tags_reloaded_finished(nullptr, 0);
            return;
        }
        
        this->tags_reloading_queued = true;
        this->tag_loading_label->setText("Listing tags...");
        this->tag_loading_label->setStyleSheet("");
        this->tag_loading_label->show();

        // Clear all tags
        if(reiterate_directories) {
            // Now... let's do this
            this->fetcher_thread = new TagFetcherThread(this, this->paths);
            connect(this->fetcher_thread, &TagFetcherThread::tag_count_changed, this, &TagTreeWindow::tag_count_changed);
            connect(this->fetcher_thread, &TagFetcherThread::fetch_finished, this, &TagTreeWindow::tags_reloaded_finished);
            connect(this->fetcher_thread, &TagFetcherThread::finished, this->fetcher_thread, &TagFetcherThread::deleteLater);
            this->fetcher_thread->start();
        }
        
        // Just a simple refresh
        else {
            auto all_tags_copy = this->all_tags;
            this->tags_reloaded_finished(&all_tags_copy, 0);
        }
    }

    void TagTreeWindow::tags_reloaded_finished(const std::vector<File::TagFile> *result, int error_count) {
        this->tags_reloading_queued = false;
        
        if(result) {
            this->set_count_label(result->size());
            all_tags = *result;
        }
        else {
            this->set_count_label(0);
            all_tags.clear();
        }
        
        if(error_count) {
            char error_message[256];
            std::snprintf(error_message,sizeof(error_message),"Failed to list %i subdirector%s. Check stderr for more information.", error_count, error_count == 1 ? "y" : "ies");
            this->tag_loading_label->show();
            this->tag_loading_label->setStyleSheet("color: red;");
            this->tag_loading_label->setText(error_message);
        }
        else {
            this->tag_loading_label->hide();
        }
        
        // If we have to open some tags, do it
        if(this->tags_to_open.size()) {
            for(auto &t : this->tags_to_open) {
                this->open_tag(t.c_str(), this->tags_to_open_full_path);
            }
            this->tags_to_open.clear();
        }
        
        // Done
        emit tags_reloaded(this);
    }

    const std::vector<File::TagFile> &TagTreeWindow::get_all_tags() const noexcept {
        return this->all_tags;
    }

    void TagTreeWindow::closeEvent(QCloseEvent *event) {
        if(this->tags_reloading_queued) {
            this->fetcher_thread->wait();
        }

        this->close_all_open_tags();
        event->setAccepted(this->open_documents.size() == 0);
    }

    void TagTreeWindow::on_double_click(QTreeWidgetItem *, int) {
        this->perform_open();
    }

    void TagTreeWindow::cleanup_windows(TagEditorWindow *also_close) {
        bool window_closed;
        do {
            window_closed = false;
            for(auto &w : this->open_documents) {
                if(w->isHidden() || w.get() == also_close) {
                    w.release()->deleteLater();
                    this->open_documents.erase(this->open_documents.begin() + (&w - this->open_documents.data()));
                    window_closed = true;
                    break;
                }
            }
        }
        while(window_closed);
    }

    bool TagTreeWindow::close_all_open_tags() {
        while(this->open_documents.size() > 0) {
            if(!this->open_documents[0]->isHidden() && !this->open_documents[0]->close()) {
                return false; // failed to close due to user input
            }
            this->cleanup_windows();
        }
        return true;
    }

    void TagTreeWindow::perform_open() {
        const auto tag = this->tag_view->get_selected_tag();
        if(tag.has_value()) {
            this->open_tag(tag->full_path.string().c_str(), true);
        }
    }

    void TagTreeWindow::perform_copy_virtual_path() {
        const auto tag = this->tag_view->get_selected_tag();
        if(tag.has_value()) {
            QGuiApplication::clipboard()->setText(Invader::File::halo_path_to_preferred_path(tag->tag_path).c_str());
        }
    }

    void TagTreeWindow::perform_copy_virtual_path_without_extension() {
        const auto tag = this->tag_view->get_selected_tag();
        if(tag.has_value()) {
            QGuiApplication::clipboard()->setText(Invader::File::split_tag_class_extension(File::halo_path_to_preferred_path(tag->tag_path).c_str()).value().path.c_str());
        }
    }
    
    void TagTreeWindow::perform_copy_virtual_directory_path() {
        auto directory = this->tag_view->get_selected_directory();
        if(directory.has_value()) {
            QGuiApplication::clipboard()->setText(directory->c_str());
        }
    }

    void TagTreeWindow::perform_copy_file_path() {
        const auto tag = this->tag_view->get_selected_tag();
        if(tag.has_value()) {
            QGuiApplication::clipboard()->setText(std::filesystem::absolute(tag->full_path).string().c_str());
        }
    }

    void TagTreeWindow::open_tag(const char *path, bool full_path) {
        // See if we can figure out this path
        File::TagFile tag;
        bool found = false;
        
        // If fast listing mode is disabled (which is default), we don't need to query the filesystem since we already did that
        if(!this->fast_listing) {
            if(full_path) {
                for(auto &t : this->get_all_tags()) {
                    if(t.full_path == path) {
                        tag = t;
                        found = true;
                        break;
                    }
                }
            }
            else {
                auto preferred_path = File::halo_path_to_preferred_path(path);
                for(auto &t : this->get_all_tags()) {
                    if(File::halo_path_to_preferred_path(t.tag_path) == path) {
                        tag = t;
                        found = true;
                        break;
                    }
                }
            }
        }
        
        // But if fast listing mode is enabled, we have to do that
        else {
            if(full_path) {
                for(auto &i : paths) {
                    // Find a relative path
                    std::error_code ec;
                    auto relative = std::filesystem::relative(path, i, ec);
                    if(ec) {
                        continue;
                    }
                    
                    auto split = File::split_tag_class_extension(relative.string());
                    if(std::filesystem::exists(path)) {
                        tag.full_path = path;
                        tag.tag_directory = &i - paths.data();
                        tag.tag_fourcc = split->fourcc;
                        tag.tag_path = split->path;
                        found = true;
                        break;
                    }
                }
            }
            else {
                auto preferred_path = File::halo_path_to_preferred_path(path);
                auto split = File::split_tag_class_extension(preferred_path);
                
                for(auto &i : paths) {
                    auto test_full_path = i / preferred_path;
                    if(std::filesystem::exists(test_full_path)) {
                        tag.full_path = test_full_path;
                        tag.tag_directory = &i - paths.data();
                        tag.tag_fourcc = split->fourcc;
                        tag.tag_path = split->path;
                        found = true;
                        break;
                    }
                }
            }
        }

        if(!found) {
            char tagpath[512];
            std::snprintf(tagpath, sizeof(tagpath), "Failed to find %s.\n\nMake sure the path is correct.", path);
            QMessageBox box(QMessageBox::Critical, "Failed to find a tag", tagpath, QMessageBox::Cancel);
            box.exec();
            return;
        }

        // Clean some things up
        this->cleanup_windows();

        // Do we have it open already?
        for(auto &doc : this->open_documents) {
            if(doc->isVisible() && doc->get_file().full_path == tag.full_path) {
                doc->raise();
                QApplication::setActiveWindow(doc.get());
                return;
            }
        }

        // Don't open something if we're busy
        if(this->opening_tag) {
            return;
        }

        // Open; benchmark
        auto start = std::chrono::steady_clock::now();
        char open_label[256];
        std::snprintf(open_label, sizeof(open_label), "Opening %s...", tag.full_path.string().c_str());
        this->tag_opening_label->setText(open_label);
        this->opening_tag = true;
        QCoreApplication::processEvents();
        auto document = std::make_unique<TagEditorWindow>(nullptr, this, tag);
        this->opening_tag = false;
        this->tag_opening_label->setText("");
        auto *window = document.get();
        if(!window->is_successfully_opened()) {
            document.release()->deleteLater();
            return;
        }
        window->show();
        this->open_documents.emplace_back(std::move(document));
        auto end = std::chrono::steady_clock::now();
        std::printf("Opened %s in %zu ms\n", tag.full_path.string().c_str(), std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count());
    }

    bool TagTreeWindow::perform_new() {
        // Start creating a new tag
        QInputDialog dialog;
        dialog.setWindowFlags(Qt::Dialog | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);
        QStringList items;
        for(auto i : Parser::ParserStruct::all_tag_classes(false)) {
            items.append(HEK::tag_fourcc_to_extension(i));
        }
        dialog.setComboBoxItems(items);
        dialog.setLabelText("Choose a class for the new tag");
        dialog.setWindowTitle("New tag");

        // If we got it, onwards!
        if(dialog.exec() == QDialog::Accepted) {
            File::TagFile tag = {};
            tag.tag_fourcc = HEK::tag_extension_to_fourcc(dialog.textValue().toLatin1().data());

            // Create; benchmark
            auto start = std::chrono::steady_clock::now();
            auto document = std::make_unique<TagEditorWindow>(this, this, tag);
            auto *window = document.get();
            if(!window->is_successfully_opened()) {
                document.release()->deleteLater();
                return false;
            }
            window->show();
            this->open_documents.emplace_back(std::move(document));
            auto end = std::chrono::steady_clock::now();
            std::printf("Created %s in %zu ms\n", dialog.textValue().toLatin1().data(), std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count());

            return true;
        }

        return false;
    }

    bool TagTreeWindow::perform_delete() {
        const auto tag = this->tag_view->get_selected_tag();
        if(!tag.has_value()) {
            return false;
        }

        // TODO: Show all tags that depend on this tag
        char message_entire_text[512];
        std::snprintf(message_entire_text, sizeof(message_entire_text), "Are you sure you want to delete \"%s\"?\n\nIf a tag depends on this tag, then that tag may no longer function.", tag->full_path.string().c_str());
        QMessageBox are_you_sure(QMessageBox::Icon::Warning, "Delete tag", message_entire_text, QMessageBox::Yes | QMessageBox::Cancel);
        switch(are_you_sure.exec()) {
            case QMessageBox::Yes:
                // Remove from file system
                std::filesystem::remove(tag->full_path);
                
                // Remove from array
                for(auto &i : this->all_tags) {
                    if(i.full_path == tag->full_path) {
                        this->all_tags.erase(this->all_tags.begin() + (&i - this->all_tags.data()));
                        break;
                    }
                }
                
                // Quickly refresh
                this->reload_tags(false);
                return true;
            case QMessageBox::Cancel:
                return false;
            default:
                std::terminate();
        }

    }

    bool TagTreeWindow::perform_refactor() {
        std::fprintf(stderr, "TODO: perform_refactor()\n");
        return false;
    }

    bool TagTreeWindow::show_properties() {
        std::fprintf(stderr, "TODO: show_properties()\n");
        return false;
    }

    void TagTreeWindow::show_context_menu(const QPoint &point) {
        if(this->tag_view->get_selected_tag().has_value()) {
            QMenu right_click_menu;

            auto *copy_virtual_path = right_click_menu.addAction("Copy virtual path");
            copy_virtual_path->setIcon(QIcon::fromTheme(QStringLiteral("edit-copy")));
            connect(copy_virtual_path, &QAction::triggered, this, &TagTreeWindow::perform_copy_virtual_path);

            auto *copy_virtual_path_without_extension = right_click_menu.addAction("Copy virtual path (without extension)");
            copy_virtual_path_without_extension->setIcon(QIcon::fromTheme(QStringLiteral("edit-copy")));
            connect(copy_virtual_path_without_extension, &QAction::triggered, this, &TagTreeWindow::perform_copy_virtual_path_without_extension);

            auto *copy_file_path = right_click_menu.addAction("Copy file path");
            copy_file_path->setIcon(QIcon::fromTheme(QStringLiteral("edit-copy")));
            connect(copy_file_path, &QAction::triggered, this, &TagTreeWindow::perform_copy_file_path);

            right_click_menu.addSeparator();

            auto *open = right_click_menu.addAction("Open...");
            open->setIcon(QIcon::fromTheme(QStringLiteral("document-open")));
            connect(open, &QAction::triggered, this, &TagTreeWindow::perform_open);

            right_click_menu.addSeparator();

            auto *delete_tag = right_click_menu.addAction("Delete...");
            delete_tag->setIcon(QIcon::fromTheme(QStringLiteral("edit-delete")));
            delete_tag->setShortcut(QKeySequence::Delete);
            connect(delete_tag, &QAction::triggered, this, &TagTreeWindow::perform_delete);

            // Future functionality
            /*
            right_click_menu.addSeparator();

            auto *refactor = right_click_menu.addAction("Refactor...");
            refactor->setIcon(QIcon::fromTheme(QStringLiteral("edit-rename")));
            connect(refactor, &QAction::triggered, this, &TagTreeWindow::perform_refactor);

            right_click_menu.addSeparator();

            auto *properties = right_click_menu.addAction("Properties...");
            properties->setIcon(QIcon::fromTheme(QStringLiteral("document-properties")));
            connect(properties, &QAction::triggered, this, &TagTreeWindow::show_properties);
            */

            right_click_menu.exec(this->tag_view->mapToGlobal(point));
        }
        else if(this->tag_view->get_selected_directory().has_value()) {
            QMenu right_click_menu;
            
            auto *copy_virtual_path = right_click_menu.addAction("Copy virtual path");
            copy_virtual_path->setIcon(QIcon::fromTheme(QStringLiteral("edit-copy")));
            connect(copy_virtual_path, &QAction::triggered, this, &TagTreeWindow::perform_copy_virtual_directory_path);

            right_click_menu.exec(this->tag_view->mapToGlobal(point));
        }
    }

    #ifdef SHOW_NIGHTLY_LINK
    void TagTreeWindow::show_source_code() {
        QDesktopServices::openUrl(QUrl::fromUserInput("https://github.com/SnowyMouse/invader"));
    }

    void TagTreeWindow::show_nightly_build() {
        QDesktopServices::openUrl(QUrl::fromUserInput("https://invader.opencarnage.net/builds/nightly/download-latest.html"));
    }
    #endif
    
    void TagTreeWindow::set_filter(const QString &filter) {
        if(filter.length() == 0) {
            this->tag_view->set_filter();
        }
        else {
            std::vector<std::string> filters;
            filters.emplace_back(filter.toLatin1().data());
            this->tag_view->set_filter(std::nullopt, std::nullopt, filters);
        }
    }
    
    void TagTreeWindow::toggle_filter_visible() {
        if(this->fast_listing) {
            QMessageBox mb;
            mb.setWindowTitle("Recursive listing mode is currently disabled");
            mb.setText("Recursive listing mode needs to be enabled in order to use filtering. Enable recursive listing mode?");
            mb.setIcon(QMessageBox::Icon::Question);
            mb.addButton(QMessageBox::Button::Yes);
            mb.addButton(QMessageBox::Button::Cancel);
            if(mb.exec() == QMessageBox::Button::Cancel) {
                return;
            }
            this->set_fast_listing_mode(false);
        }
        
        this->filter_widget->setVisible(!this->filter_widget->isVisible());
        this->filter_textbox->setText("");
        if(this->filter_widget->isVisible()) {
            this->filter_textbox->setFocus();
        }
    }
    
    void TagTreeWindow::set_fast_listing_mode(bool mode) {
        this->fast_listing = mode;
        this->tag_count_label->setVisible(!mode);
        this->reload_tags(true);
    }
}
