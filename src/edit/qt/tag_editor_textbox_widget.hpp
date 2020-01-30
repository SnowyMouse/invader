// SPDX-License-Identifier: GPL-3.0-only

#ifndef INVADER__EDIT__QT__TAG_EDITOR_TEXTBOX_WIDGET_HPP
#define INVADER__EDIT__QT__TAG_EDITOR_TEXTBOX_WIDGET_HPP

#include "tag_editor_abstract_widget.hpp"

namespace Invader::EditQt {
    class TagEditorTextboxWidget : public TagEditorAbstractWidget {
        Q_OBJECT
    public:
        enum TextboxSize {
            SMALL,
            MEDIUM,
            LARGE
        };

        /**
         * Instantiate a TagEditorTextboxWidget
         * @param parent         parent widget
         * @param text           element text
         * @param size           size of the textbox
         */
        TagEditorTextboxWidget(QWidget *parent, const char *text, TextboxSize size);
    private:
    };
}

#endif
