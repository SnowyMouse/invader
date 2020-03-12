// SPDX-License-Identifier: GPL-3.0-only

#ifndef INVADER__EDIT__QT__TAG_EDITOR_BITMAP_SUBWINDOW_HPP
#define INVADER__EDIT__QT__TAG_EDITOR_BITMAP_SUBWINDOW_HPP

#include "tag_editor_subwindow.hpp"

class QComboBox;
class QScrollArea;

namespace Invader::EditQt {
    class TagEditorWindow;

    class TagEditorBitmapSubwindow : public TagEditorSubwindow {
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
        TagEditorBitmapSubwindow(TagEditorWindow *parent_window);

        ~TagEditorBitmapSubwindow() = default;

    private:
        QComboBox *mipmaps;
        QComboBox *colors;
        QComboBox *bitmaps;
        QComboBox *scale;
        QComboBox *more;
        QScrollArea *images;

        static void set_values(TagEditorBitmapSubwindow *what, QComboBox *bitmaps, QComboBox *colors, QComboBox *mimaps, QComboBox *zoom, QComboBox *more, QScrollArea *images);
        void refresh_data();
        void reload_view();

        friend TagEditorWindow;
    };
}

#endif
