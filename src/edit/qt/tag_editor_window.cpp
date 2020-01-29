// SPDX-License-Identifier: GPL-3.0-only

#include <QCloseEvent>
#include <QMessageBox>
#include <QMessageBox>
#include "tag_file.hpp"
#include "tag_tree_window.hpp"

namespace Invader::EditQt {
    TagEditorWindow::TagEditorWindow(QWidget *parent, TagTreeWindow *parent_window, const TagFile &tag_file) : QMainWindow(parent), parent_window(parent_window), file(tag_file) {
        this->make_dirty(true);
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
        std::fprintf(stderr, "TODO: perform_save()");
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
