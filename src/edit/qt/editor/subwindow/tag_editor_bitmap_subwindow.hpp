// SPDX-License-Identifier: GPL-3.0-only

#ifndef INVADER__EDIT__QT__TAG_EDITOR_BITMAP_SUBWINDOW_HPP
#define INVADER__EDIT__QT__TAG_EDITOR_BITMAP_SUBWINDOW_HPP

#include "tag_editor_subwindow.hpp"

namespace Invader::EditQt {
    class TagEditorWindow;

    class TagEditorBitmapSubwindow : public TagEditorSubwindow {
        Q_OBJECT

    friend class TagEditorWindow;

    public:
        /**
         * Update the window
         */
        void update() override;

        /**
         * Instantiate a subwindow
         * @param parent parent window
         */
        TagEditorBitmapSubwindow(TagEditorWindow *parent_window);
    };
}

#endif
