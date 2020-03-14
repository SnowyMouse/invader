// SPDX-License-Identifier: GPL-3.0-only

#ifndef INVADER__EDIT__QT__TAG_EDITOR_WIDGET_HPP
#define INVADER__EDIT__QT__TAG_EDITOR_WIDGET_HPP

#include <QFrame>

namespace Invader::Parser {
    class ParserStructValue;
}

namespace Invader::EditQt {
    class TagEditorWindow;
    class TagEditorArrayWidget;

    class TagEditorWidget : public QFrame {
        Q_OBJECT

    public:
        /**
         * Generate a widget using the appropriate widget type
         * @param  parent        parent widget
         * @param  value         struct value
         * @param  editor_window editor window
         * @param  array_widget  optional array widget to update on change
         * @return               new widget
         */
        static TagEditorWidget *generate_widget(QWidget *parent, Parser::ParserStructValue *value, TagEditorWindow *editor_window, TagEditorArrayWidget *array_widget = nullptr);

        /**
         * Get the struct value
         * @return struct value
         */
        Parser::ParserStructValue *get_struct_value() noexcept {
            return this->struct_value;
        }

        /**
         * Get the editor window
         * @return editor window
         */
        TagEditorWindow *get_editor_window() noexcept {
            return this->editor_window;
        }

        virtual ~TagEditorWidget() = default;

    protected:
        /**
         * Instantiate a widget
         * @param parent        parent widget
         * @param value         value to use
         * @param editor_window editor window
         */
        TagEditorWidget(QWidget *parent, Parser::ParserStructValue *value, TagEditorWindow *editor_window);

        /**
         * The value has been changed. Signal the window that this is the case.
         */
        void value_changed();

    private:
        Parser::ParserStructValue *struct_value;
        TagEditorWindow *editor_window;
    };
}

#endif
