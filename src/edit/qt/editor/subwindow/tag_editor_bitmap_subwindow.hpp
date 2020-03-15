// SPDX-License-Identifier: GPL-3.0-only

#ifndef INVADER__EDIT__QT__TAG_EDITOR_BITMAP_SUBWINDOW_HPP
#define INVADER__EDIT__QT__TAG_EDITOR_BITMAP_SUBWINDOW_HPP

#include "tag_editor_subwindow.hpp"

class QComboBox;
class QScrollArea;
class QGraphicsView;

namespace Invader::Parser {
    struct BitmapGroupSequence;
    struct BitmapData;
}

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
        QScrollArea *images;

        QComboBox *sequence;
        QComboBox *sprite;

        enum Colors {
            COLOR_ARGB,
            COLOR_RGB,
            COLOR_ALPHA,
            COLOR_RED,
            COLOR_GREEN,
            COLOR_BLUE
        };

        std::vector<Parser::BitmapGroupSequence> *all_sequences;

        static void set_values(TagEditorBitmapSubwindow *what, QComboBox *bitmaps, QComboBox *mipmaps, QComboBox *colors, QComboBox *scale, QComboBox *sequence, QComboBox *sprite, QScrollArea *images, std::vector<Parser::BitmapGroupSequence> *all_sequences);
        void refresh_data();
        void reload_view();

        QGraphicsView *draw_bitmap_to_widget(Parser::BitmapData *bitmap_data, std::size_t mipmap, std::size_t index, Colors mode, int scale, const std::vector<std::byte> *pixel_data);
        void highlight_sprite(std::uint32_t *data, std::size_t real_width, std::size_t real_height);
        void show_channel(std::uint32_t *data, std::size_t real_width, std::size_t real_height, Colors mode);
        std::vector<std::uint32_t> decode_bitmap(std::size_t real_width, std::size_t real_height, std::size_t width, std::size_t height, std::size_t pixels_required, std::size_t pixel_count, const std::byte *bytes, const Parser::BitmapData *bitmap_data);
        void scale_bitmap(int scale, std::size_t &real_width, std::size_t &real_height, std::size_t &pixel_count, std::vector<std::uint32_t> &data);

        void refresh_sprite_list();
        void select_sprite();

        friend TagEditorWindow;
    };
}

#endif
