// SPDX-License-Identifier: GPL-3.0-only

#ifndef INVADER__EDIT__QT__TAG_EDITOR_SUBWINDOW_HPP
#define INVADER__EDIT__QT__TAG_EDITOR_SUBWINDOW_HPP

#include <QWindow>

namespace Invader::EditQt {
    class TagEditorWindow;

    class TagEditorSubwindow : public QWindow {
        Q_OBJECT

    public:
        /**
         * Update the window
         */
        virtual void update() = 0;

    protected:
        /**
         * Instantiate a subwindow
         * @param parent parent window
         */
        TagEditorSubwindow(QWindow *parent, TagEditorWindow *parent_window);
    };
}

#endif
