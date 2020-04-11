// SPDX-License-Identifier: GPL-3.0-only

#ifndef INVADER__EDIT__QT__TAG_EDITOR_STRING_SUBWINDOW_HPP
#define INVADER__EDIT__QT__TAG_EDITOR_STRING_SUBWINDOW_HPP

#include "tag_editor_subwindow.hpp"

class QComboBox;
class QPlainTextEdit;

namespace Invader::EditQt {
    class TagEditorStringSubwindow : public TagEditorSubwindow {
        Q_OBJECT

    public:
        void update() override;
    
        TagEditorStringSubwindow(TagEditorWindow *parent);
        ~TagEditorStringSubwindow() = default;
        
    private:
        QPlainTextEdit *text_view;
        QComboBox *combo_box;
        
        std::vector<const std::vector<std::byte> *> lists;
        bool utf16;
        void selection_changed();
        
        QString decode_string(const std::vector<std::byte> *data, bool first_line_only);
        void refresh_string();
    };
}

#endif
