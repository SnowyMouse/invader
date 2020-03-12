// SPDX-License-Identifier: GPL-3.0-only

#ifndef INVADER__EDIT__QT__TAG_EDITOR_EDIT_WIDGET_HPP
#define INVADER__EDIT__QT__TAG_EDITOR_EDIT_WIDGET_HPP

#include "tag_editor_widget.hpp"
#include <QLabel>
#include <QHBoxLayout>

class QLineEdit;
class QComboBox;

namespace Invader::EditQt {
    class TagEditorEditWidget : public TagEditorWidget {
        Q_OBJECT

    public:
        /**
         * Instantiate an edit widget
         * @param parent        parent widget
         * @param value         struct value
         * @param editor_window editor window
         */
        TagEditorEditWidget(QWidget *parent, Parser::ParserStructValue *value, TagEditorWindow *editor_window);

        ~TagEditorEditWidget() = default;

    private slots:
        void on_change();

    private:
        void place_textbox(int size, QLabel *prefix);
        std::vector<QWidget *> widgets;
        std::vector<QLineEdit *> textbox_widgets;
        void verify_dependency_path();
        void find_dependency();
        void open_dependency();
        bool read_only;
    };
}

#endif
