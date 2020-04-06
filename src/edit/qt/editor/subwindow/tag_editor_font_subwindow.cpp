// SPDX-License-Identifier: GPL-3.0-only

#include <invader/tag/parser/parser.hpp>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPlainTextEdit>
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
        header->addWidget((this->text_to_render = new QPlainTextEdit("A quick brown fox jumped over the lazy dog.")));
        header_widget->setLayout(header);
        
        // Move cursor to end
        this->text_to_render->selectAll();
        
        QVBoxLayout *layout = new QVBoxLayout();
        layout->addWidget(header_widget);
        layout->addWidget((this->scroll_area = new QScrollArea()));
        this->scroll_area->setStyleSheet("background-color: #000");
        QWidget *center_widget = new QWidget();
        center_widget->setLayout(layout);
        this->setCentralWidget(center_widget);
        
        this->update();
        this->center_window();
        this->draw_text();
        
        // Set up drawing text
        connect(this->text_to_render, &QPlainTextEdit::textChanged, this, &TagEditorFontSubwindow::draw_text);
    }
    
    static void get_dimensions(std::size_t &width, std::size_t &height, const char16_t *text, const Parser::Font &font_data) {
        // Initialize everything
        height = 0;
        width = 0;
        std::size_t advance = 0;
        std::size_t line_count = 1;
        
        // Go through each character
        for(auto *t = text; *t; t++) {
            if(*t == '\n') {
                advance = 0;
                line_count++;
                continue;
            }
            for(auto &i : font_data.characters) {
                if(i.character == *t) {
                    std::size_t actual_right = advance - (i.bitmap_origin_x - font_data.leading_width) + i.bitmap_width;
                    if(actual_right > width) {
                        width = actual_right;
                    }
                    advance += i.character_width;
                    break;
                }
            }
        }
        
        // Set the height
        height = line_count * (font_data.ascending_height + font_data.descending_height);
    }
    
    static void draw_text(std::uint32_t *pixels, std::size_t width, std::size_t height, const char16_t *text, const Parser::Font &font_data, std::uint32_t color = 0xFFFFFFFF) {
        std::size_t line = 0;
        std::size_t line_height = font_data.ascending_height + font_data.descending_height;
        std::size_t horizontal_advance = 0;
        const auto *font_bitmap_data = reinterpret_cast<const std::uint8_t *>(font_data.pixels.data());
        std::size_t font_bitmap_data_length = font_data.pixels.size();
        auto font_pixel = ColorPlatePixel::convert_from_32_bit(color);
        
        // Go through each character
        for(auto *t = text; *t; t++) {
            if(*t == '\n') {
                line++;
                horizontal_advance = 0;
                continue;
            }
            
            for(auto &c : font_data.characters) {
                if(c.character != *t) {
                    continue;
                }
                
                std::size_t bx = horizontal_advance - (c.bitmap_origin_x - font_data.leading_width);
                std::size_t by = font_data.ascending_height - (c.bitmap_origin_y + font_data.leading_height) + line_height * line;
                
                horizontal_advance += c.character_width;
                auto bitmap_width = c.bitmap_width;
                std::size_t bxw = bx + bitmap_width;
                std::size_t byh = by + c.bitmap_height;
                
                std::size_t pixels_required = bitmap_width * c.bitmap_height;
                std::size_t pixels_start = c.pixels_offset;
                
                if(pixels_start + pixels_required > font_bitmap_data_length) {
                    continue;
                }
                
                for(std::size_t y = by; y < byh && y < height; y++) {
                    for(std::size_t x = bx; x < bxw && x < width; x++) {
                        auto &resulting_pixel = pixels[x + y * width];
                        font_pixel.alpha = font_bitmap_data[x - bx + (y - by) * bitmap_width + pixels_start];
                        resulting_pixel = ColorPlatePixel::convert_from_32_bit(resulting_pixel).alpha_blend(font_pixel).convert_to_32_bit();
                    }
                }
            }
        }
    }
    
    static QGraphicsView *draw_text_to_widget(const Parser::Font &font_data, const QString &text) {
        // Draw it
        std::size_t width, height;
        auto text_data = text.toStdU16String();
        get_dimensions(width, height, text_data.c_str(), font_data);
        std::vector<std::uint32_t> pixels(width * height);
        draw_text(pixels.data(), width, height, text_data.c_str(), font_data);
        
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
            this->scroll_area->setWidget(draw_text_to_widget(*font_data, this->text_to_render->toPlainText()));
        }
    }
}
