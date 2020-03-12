// SPDX-License-Identifier: GPL-3.0-only

#include "tag_editor_subwindow.hpp"
#include "../tag_editor_window.hpp"

namespace Invader::EditQt {
    TagEditorSubwindow::TagEditorSubwindow(TagEditorWindow *parent_window) : parent_window(parent_window) {
        this->setWindowTitle(parent_window->get_file().tag_path.c_str());
    }
}
