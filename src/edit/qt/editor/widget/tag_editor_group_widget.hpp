// SPDX-License-Identifier: GPL-3.0-only

#ifndef INVADER__EDIT__QT__TAG_EDITOR_GROUP_WIDGET_HPP
#define INVADER__EDIT__QT__TAG_EDITOR_GROUP_WIDGET_HPP

#include <QFrame>

#include "tag_editor_widget.hpp"

namespace Invader::EditQt {
    class TagEditorGroupWidget : public TagEditorWidget {
        Q_OBJECT

    public:
        /**
         * Instantiate an edit widget
         * @param parent        parent widget
         * @param value         struct value
         * @param editor_window editor window
         */
        TagEditorGroupWidget(QWidget *parent, Parser::ParserStructValue *value, TagEditorWindow *editor_window);
        ~TagEditorGroupWidget() = default;
    };
}

#endif
