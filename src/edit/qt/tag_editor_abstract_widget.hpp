// SPDX-License-Identifier: GPL-3.0-only

#ifndef INVADER__EDIT__QT__TAG_EDITOR_ABSTRACT_WIDGET_HPP
#define INVADER__EDIT__QT__TAG_EDITOR_ABSTRACT_WIDGET_HPP

#include <QWidget>

class QLabel;

namespace Invader::EditQt {
    class TagEditorAbstractWidget : public QWidget {
        Q_OBJECT
    protected:
        /**
         * Instantiate a TagEditorAbstractWidget
         * @param parent         parent widget
         * @param text           element text
         */
        TagEditorAbstractWidget(QWidget *parent, const char *text);

        /**
         * Get the title label
         * @return title label
         */
        QLabel *get_title_label() noexcept;
    private:
        /** Title label */
        QLabel *title_label;
    };
}

#endif
