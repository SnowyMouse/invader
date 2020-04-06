// SPDX-License-Identifier: GPL-3.0-only

#include <invader/tag/parser/parser.hpp>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QImage>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QScrollArea>
#include <QLabel>
#include "tag_editor_font_subwindow.hpp"
#include "../../../../bitmap/color_plate_scanner.hpp"
#include "../tag_editor_window.hpp"

namespace Invader::EditQt {
    void TagEditorFontSubwindow::update() {
        
    }
    
    TagEditorFontSubwindow::TagEditorFontSubwindow(TagEditorWindow *parent_window) : TagEditorSubwindow(parent_window) {
        // Set up the layout
        QHBoxLayout *header = new QHBoxLayout();
        QWidget *header_widget = new QWidget();
        header->addWidget((this->text_to_render = new QLineEdit("A quick brown fox jumped over the lazy dog.")));
        header_widget->setLayout(header);
        
        QVBoxLayout *layout = new QVBoxLayout();
        layout->addWidget(header_widget);
        layout->addWidget((this->scroll_area = new QScrollArea()));
        QWidget *center_widget = new QWidget();
        center_widget->setLayout(layout);
        this->setCentralWidget(center_widget);
        
        this->update();
        this->center_window();
        this->draw_text();
        
        // Set up drawing text
        connect(this->text_to_render, &QLineEdit::textChanged, this, &TagEditorFontSubwindow::draw_text);
    }
    
    static QGraphicsView *draw_text_to_widget(const Parser::Font &font_data, const QString &text) {
        std::vector<const Parser::FontCharacter *> characters;
        std::size_t advance = 0;
        std::size_t width = 0;
        std::size_t height = (font_data.ascending_height + font_data.descending_height);
        auto text_data = text.toStdU16String();
        
        // Grab our characters. Figure out the width, too
        for(auto c : text_data) {
            bool added = false;
            for(auto &i : font_data.characters) {
                if(i.character == c) {
                    characters.emplace_back(&i);
                    std::size_t actual_right = advance - (i.bitmap_origin_x - font_data.leading_width) + i.bitmap_width;
                    if(actual_right > width) {
                        width = actual_right;
                    }
                    advance += i.character_width;
                    added = true;
                    break;
                }
            }
            if(!added) {
                characters.emplace_back(nullptr);
            }
        }
        
        // Generate our bitmap
        std::vector<std::uint32_t> pixels(width * height, 0xFF000000);
        
        // Go through each character
        std::size_t horizontal_advance = 0;
        const auto *font_bitmap_data = reinterpret_cast<const std::uint8_t *>(font_data.pixels.data());
        std::size_t font_bitmap_data_length = font_data.pixels.size();
        for(auto *c : characters) {
            if(!c) {
                continue;
            }
            
            // ColorPlatePixel pixel;
            
            std::size_t bx = horizontal_advance - (c->bitmap_origin_x - font_data.leading_width);
            std::size_t by = font_data.ascending_height - (c->bitmap_origin_y + font_data.leading_height);
            
            horizontal_advance += c->character_width;
            auto bitmap_width = c->bitmap_width;
            std::size_t bxw = bx + bitmap_width;
            std::size_t byh = by + c->bitmap_height;
            
            std::size_t pixels_required = bitmap_width * c->bitmap_height;
            std::size_t pixels_start = c->pixels_offset;
            
            if(pixels_start + pixels_required > font_bitmap_data_length) {
                eprintf_warn("Character %zu has an invalid offset", static_cast<std::size_t>(c->character));
                continue;
            }
            
            for(std::size_t y = by; y < byh && y < height; y++) {
                for(std::size_t x = bx; x < bxw && x < width; x++) {
                    auto pixel_to_read = font_bitmap_data[x - bx + (y - by) * bitmap_width + pixels_start];
                    pixels[x + y * width] |= (0x010101 * pixel_to_read);
                }
            }
        }
        
        // Finish up
        QGraphicsView *view = new QGraphicsView();
        QGraphicsScene *scene = new QGraphicsScene(view);
        QPixmap map;
        map.convertFromImage(QImage(reinterpret_cast<const uchar *>(pixels.data()), static_cast<int>(width), static_cast<int>(height), QImage::Format_ARGB32));
        scene->addPixmap(map);
        view->setScene(scene);

        view->setFrameStyle(0);
        view->setMinimumSize(width,height);
        view->setMaximumSize(width,height);
        view->setHorizontalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOff);
        view->setVerticalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOff);
        view->setSizePolicy(QSizePolicy::Policy::Fixed, QSizePolicy::Policy::Fixed);

        return view;
    }
    
    void TagEditorFontSubwindow::draw_text() {
        auto *data = this->get_parent_window()->get_parser_data();
        auto *font_data = dynamic_cast<const Parser::Font *>(data);
        if(font_data) {
            this->scroll_area->setWidget(draw_text_to_widget(*font_data, this->text_to_render->text()));
        }
    }
}
