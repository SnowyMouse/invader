// SPDX-License-Identifier: GPL-3.0-only

#ifndef INVADER__EDIT__QT__TAG_EDITOR_EDIT_WIDGET_HPP
#define INVADER__EDIT__QT__TAG_EDITOR_EDIT_WIDGET_HPP

#include "tag_editor_widget.hpp"
#include <QLabel>
#include <QHBoxLayout>

class QLineEdit;
class QComboBox;
class QCheckBox;

namespace Invader::EditQt {
    class TagEditorArrayWidget;

    class TagEditorEditWidget : public TagEditorWidget {
        Q_OBJECT

    public:
        /**
         * Instantiate an edit widget
         * @param parent        parent widget
         * @param value         struct value
         * @param editor_window editor window
         * @param array_widget  optional array widget to update if changed
         */
        TagEditorEditWidget(QWidget *parent, Parser::ParserStructValue *value, TagEditorWindow *editor_window, TagEditorArrayWidget *array_widget = nullptr);
        void activate_auxiliary_widget();

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
        TagEditorArrayWidget *array_widget;
        QWidget *auxiliary_widget;
        QCheckBox *auxiliary_checkbox;
        void update_auxiliary_widget();
    };
}

#endif
