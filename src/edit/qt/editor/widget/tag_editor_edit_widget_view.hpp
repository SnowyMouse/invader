// SPDX-License-Identifier: GPL-3.0-only

#ifndef INVADER__EDIT__QT__TAG_EDITOR_EDIT_WIDGET_VIEW_HPP
#define INVADER__EDIT__QT__TAG_EDITOR_EDIT_WIDGET_VIEW_HPP

#include <invader/tag/parser/parser_struct.hpp>
#include "tag_editor_widget.hpp"
#include <QVBoxLayout>
#include <QFrame>

namespace Invader::EditQt {
    class TagEditorEditWidgetView : public QFrame {
        Q_OBJECT

    public:
        /**
         * Instantiate an edit widget view
         * @param parent        parent widget
         * @param values        struct values
         * @param editor_window editor window
         * @param primary       is the primary widget of the window
         * @param extra_widget  additional widget to add
         */
        TagEditorEditWidgetView(QWidget *parent, const std::vector<Parser::ParserStructValue> &values, TagEditorWindow *editor_window, bool primary, QWidget *extra_widget = nullptr);

    private:
        std::vector<Parser::ParserStructValue> values;
        TagEditorWindow *editor_window;
    };
}

#endif
