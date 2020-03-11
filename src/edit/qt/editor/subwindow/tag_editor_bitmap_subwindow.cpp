// SPDX-License-Identifier: GPL-3.0-only

#include <QScrollArea>
#include <QHBoxLayout>
#include "tag_editor_bitmap_subwindow.hpp"
#include "tag_editor_subwindow.hpp"

namespace Invader::EditQt {
    void TagEditorBitmapSubwindow::TagEditorBitmapSubwindow::update() {
        auto *scroll_view = new QScrollArea();
        scroll_view->setWidgetResizable(true);
        auto *layout = new QHBoxLayout();
        scroll_view->setLayout(layout);

        this->setCentralWidget(scroll_view);
    }

    TagEditorBitmapSubwindow::TagEditorBitmapSubwindow(TagEditorWindow *parent_window) : TagEditorSubwindow(parent_window) {
        this->update();
    }
}
