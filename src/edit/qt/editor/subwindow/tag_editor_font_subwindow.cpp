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
#include <invader/tag/parser/compile/font.hpp>
#include "../tag_editor_window.hpp"

namespace Invader::EditQt {
    void TagEditorFontSubwindow::update() {
        Parser::generate_character_tables(dynamic_cast<Parser::Font &>(*this->get_parent_window()->get_parser_data()));
        this->draw_text();
    }
    
    TagEditorFontSubwindow::TagEditorFontSubwindow(TagEditorWindow *parent_window) : TagEditorSubwindow(parent_window) {
        // Set up the layout
        QHBoxLayout *header = new QHBoxLayout();
        QWidget *header_widget = new QWidget();
        header->addWidget((this->text_to_render = new QPlainTextEdit("This cave is not a natural formation.")));
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
        
        // Set up drawing text
        connect(this->text_to_render, &QPlainTextEdit::textChanged, this, &TagEditorFontSubwindow::draw_text);
    }
    
    static std::optional<std::size_t> get_font_character(const Parser::Font &font_data, char16_t c) {
        // Look it up in the character table if we have one present
        auto character_table_count = font_data.character_tables.size();
        if(character_table_count > 0) {
            auto unsigned_version = static_cast<std::uint16_t>(c);
            auto character_table = static_cast<std::uint8_t>(unsigned_version >> 8);
            auto &table = font_data.character_tables[character_table];
            auto ctb_size = table.character_table.size();
            auto character_table_index = static_cast<std::uint8_t>(unsigned_version);
            if(ctb_size > character_table_index) {
                auto index = table.character_table[character_table_index].character_index;
                if(index == NULL_INDEX) {
                    return std::nullopt;
                }
                else {
                    return index;
                }
            }
            return std::nullopt;
        }
        
        // Otherwise, look through each character new_color_maybe
        auto character_count = font_data.characters.size();
        for(std::size_t i = 0; i < character_count; i++) {
            auto &c_other = font_data.characters[i];
            if(c_other.character == c) {
                return i;
            }
        }
        
        return std::nullopt;
    }
    
    static void get_dimensions(std::int32_t &width, std::int32_t &height, const char16_t *text, const Parser::Font &font_data) {
        // Initialize everything
        height = 0;
        width = 0;
        std::int32_t advance = 0;
        std::int32_t line_count = 1;
        bool last_character_was_color_thing = false;
        
        // Go through each character
        for(auto *t = text; *t; t++) {
            if(*t == '^') {
                if(!last_character_was_color_thing) {
                    last_character_was_color_thing = true;
                    continue;
                }
                else {
                    last_character_was_color_thing = false;
                }
            }
            else if(last_character_was_color_thing) {
                last_character_was_color_thing = false;
                continue;
            }
            
            if(*t == '\n') {
                advance = 0;
                line_count++;
                continue;
            }
            
            auto ci = get_font_character(font_data, *t);
            if(ci.has_value()) {
                advance += font_data.characters[*ci].character_width;
                if(advance > width) {
                    width = advance;
                }
            }
        }
        width += font_data.leading_width;
        
        // Set the height
        height = line_count * (font_data.ascending_height + font_data.descending_height);
        
        if(width < 0) {
            width = 0;
        }
        if(height < 0) {
            height = 0;
        }
    }
    
    static void draw_text(std::uint32_t *pixels, std::int32_t width, std::int32_t height, const char16_t *text, const Parser::Font &font_data, std::uint32_t color = 0xFFFFFFFF) {
        std::size_t line = 0;
        std::int32_t line_height = font_data.ascending_height + font_data.descending_height;
        std::int32_t horizontal_advance = 0;
        const auto *font_bitmap_data = reinterpret_cast<const std::uint8_t *>(font_data.pixels.data());
        std::size_t font_bitmap_data_length = font_data.pixels.size();
        auto font_pixel = ColorPlatePixel::convert_from_32_bit(color);
        auto original_color = ColorPlatePixel::convert_from_32_bit(color);
        bool last_character_was_color_thing = false;
        
        // Go through each character
        for(auto *t = text; *t; t++) {
            if(*t == '^') {
                if(!last_character_was_color_thing) {
                    last_character_was_color_thing = true;
                    continue;
                }
                else {
                    last_character_was_color_thing = false;
                }
            }
            else if(last_character_was_color_thing) {
                last_character_was_color_thing = false;
                auto new_color_maybe = ColorPlatePixel::convert_from_color_code(*t);
                if(new_color_maybe.has_value()) {
                    font_pixel = *new_color_maybe;
                }
                else {
                    font_pixel = original_color;
                }
                continue;
            }
            
            if(*t == '\n') {
                line++;
                horizontal_advance = 0;
                continue;
            }
            
            auto ci = get_font_character(font_data, *t);
            if(ci.has_value()) {
                auto &c = font_data.characters[*ci];
                
                // Get the x offset
                std::int32_t bx = horizontal_advance - (c.bitmap_origin_x - font_data.leading_width);
                std::int32_t by = font_data.ascending_height - (c.bitmap_origin_y + font_data.leading_height) + line_height * line;
                
                horizontal_advance += c.character_width;
                auto bitmap_width = c.bitmap_width;
                std::int32_t bxw = bx + bitmap_width;
                std::int32_t byh = by + c.bitmap_height;
                
                std::size_t pixels_required = bitmap_width * c.bitmap_height;
                std::size_t pixels_start = c.pixels_offset;
                
                if(pixels_start + pixels_required > font_bitmap_data_length) {
                    continue;
                }
                
                std::int32_t start_x = bx < 0 ? 0 : bx;
                std::int32_t start_y = by < 0 ? 0 : by;
                
                for(std::int32_t y = start_y; y < byh && y < height; y++) {
                    for(std::int32_t x = start_x; x < bxw && x < width; x++) {
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
        std::int32_t width, height;
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
