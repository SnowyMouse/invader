// SPDX-License-Identifier: GPL-3.0-only

#include <QGuiApplication>
#include <QScreen>

#include "tag_editor_subwindow.hpp"
#include "../tag_editor_window.hpp"

namespace Invader::EditQt {
    TagEditorSubwindow::TagEditorSubwindow(TagEditorWindow *parent_window) : parent_window(parent_window) {
        this->setWindowTitle(parent_window->get_file().tag_path.c_str());
    }

    void TagEditorSubwindow::center_window() {
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
}
