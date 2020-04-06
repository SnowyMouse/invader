// SPDX-License-Identifier: GPL-3.0-only

#ifndef INVADER__EDIT__QT__TAG_EDITOR_FONT_SUBWINDOW_HPP
#define INVADER__EDIT__QT__TAG_EDITOR_FONT_SUBWINDOW_HPP

#include "tag_editor_subwindow.hpp"

class QLineEdit;
class QScrollArea;
class QGraphicsView;

namespace Invader::EditQt {
    class TagEditorWindow;

    class TagEditorFontSubwindow : public TagEditorSubwindow {
        Q_OBJECT

    public:
        /**
         * Update the window
         */
        void update() override;

        /**
         * Instantiate a subwindow
         * @param parent parent window
         */
        TagEditorFontSubwindow(TagEditorWindow *parent_window);

        ~TagEditorFontSubwindow() = default;

    private:
        friend TagEditorWindow;
        
        QLineEdit *text_to_render;
        QScrollArea *scroll_area;
        
        void draw_text();
    };
}

#endif
