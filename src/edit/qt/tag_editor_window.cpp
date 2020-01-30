// SPDX-License-Identifier: GPL-3.0-only

#include <QCloseEvent>
#include <QMessageBox>
#include <QMenuBar>
#include <QScrollArea>
#include <QLabel>
#include <QApplication>
#include <QVBoxLayout>
#include <QSpacerItem>
#include <filesystem>
#include "tag_file.hpp"
#include "tag_tree_window.hpp"
#include "tag_editor_textbox_widget.hpp"

namespace Invader::EditQt {
    TagEditorWindow::TagEditorWindow(QWidget *parent, TagTreeWindow *parent_window, const TagFile &tag_file) : QMainWindow(parent), parent_window(parent_window), file(tag_file) {
        this->make_dirty(false);

        // Make and set our menu bar
        QMenuBar *bar = new QMenuBar(this);
        this->setMenuBar(bar);

        // File menu
        auto *file_menu = bar->addMenu("File");

        auto *save = file_menu->addAction("Save");
        save->setIcon(QIcon::fromTheme(QStringLiteral("document-save")));
        save->setShortcut(QKeySequence::Save);
        connect(save, &QAction::triggered, this, &TagEditorWindow::perform_save);

        auto *save_as = file_menu->addAction("Save as...");
        save_as->setIcon(QIcon::fromTheme(QStringLiteral("document-save-as")));
        save_as->setShortcut(QKeySequence::SaveAs);
        connect(save_as, &QAction::triggered, this, &TagEditorWindow::perform_save_as);

        file_menu->addSeparator();

        auto *refactor = file_menu->addAction("Refactor...");
        refactor->setIcon(QIcon::fromTheme(QStringLiteral("edit-rename")));
        connect(refactor, &QAction::triggered, this, &TagEditorWindow::perform_refactor);

        file_menu->addSeparator();

        auto *close = file_menu->addAction("Close");
        close->setShortcut(QKeySequence::Close);
        close->setIcon(QIcon::fromTheme(QStringLiteral("document-close")));
        connect(close, &QAction::triggered, this, &TagEditorWindow::close);

        // Set up the scroll area
        auto *scroll_view = new QScrollArea(this);
        this->setCentralWidget(scroll_view);
        auto *vbox_layout = new QVBoxLayout(scroll_view);
        auto *full_widget = new QWidget(this);

        // TEST: Add widgets
        std::vector<std::string> xyz;
        xyz.emplace_back("x");
        xyz.emplace_back("y");
        xyz.emplace_back("z");
        for(std::size_t i = 0; i < 16; i++) {
            char widget_name[256];
            std::snprintf(widget_name, sizeof(widget_name), "Test #%zu", i);
            TagEditorTextboxWidget *textbox = new TagEditorTextboxWidget(full_widget, widget_name, static_cast<TagEditorTextboxWidget::TextboxSize>(i % (TagEditorTextboxWidget::TextboxSize::LARGE + 1)), 3, xyz);
            vbox_layout->addWidget(textbox);
        }

        // Add a spacer so it doesn't try to evenly space everything if we're too big
        auto *spacer = new QSpacerItem(0 ,0);
        vbox_layout->addSpacerItem(spacer);
        vbox_layout->setSpacing(2);
        full_widget->setLayout(vbox_layout);
        scroll_view->setWidget(full_widget);

        // Lock the scroll view and window to a set width
        int max_width = full_widget->width() + qApp->style()->pixelMetric(QStyle::PM_ScrollBarExtent);
        scroll_view->setSizePolicy(QSizePolicy::Policy::Fixed, QSizePolicy::Policy::Preferred);
        scroll_view->setMinimumWidth(max_width);
        scroll_view->setMaximumWidth(max_width);
        scroll_view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        this->setMaximumWidth(scroll_view->width());
    }

    void TagEditorWindow::closeEvent(QCloseEvent *event) {
        bool accept;
        if(dirty) {
            char message_entire_text[512];
            std::snprintf(message_entire_text, sizeof(message_entire_text), "This file \"%s\" has been modified.\nDo you want to save your changes?", this->file.full_path.string().c_str());
            QMessageBox are_you_sure(QMessageBox::Icon::Question, "Unsaved changes", message_entire_text, QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel, this);
            switch(are_you_sure.exec()) {
                case QMessageBox::Accepted:
                    accept = this->perform_save();
                    break;
                case QMessageBox::Cancel:
                    accept = false;
                    break;
                case QMessageBox::Discard:
                    accept = true;
                    break;
                default:
                    std::terminate();
            }
        }
        else {
            accept = true;
        }

        event->setAccepted(accept);
    }

    bool TagEditorWindow::perform_save() {
        std::fprintf(stderr, "TODO: perform_save()\n");
        return false;
    }

    bool TagEditorWindow::perform_save_as() {
        std::fprintf(stderr, "TODO: perform_save_as()\n");
        return false;
    }

    bool TagEditorWindow::perform_refactor() {
        std::fprintf(stderr, "TODO: perform_refactor()\n");
        return false;
    }

    void TagEditorWindow::make_dirty(bool dirty) {
        this->dirty = dirty;
        char title_bar[512];
        const char *asterisk = dirty ? " *" : "";
        std::snprintf(title_bar, sizeof(title_bar), "%s%s \u2014 invader-edit-qt", this->file.tag_path.c_str(), asterisk);
        this->setWindowTitle(title_bar);
    }

    const TagFile &TagEditorWindow::get_file() const noexcept {
        return this->file;
    }
}
