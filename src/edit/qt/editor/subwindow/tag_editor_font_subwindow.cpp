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
#include <invader/bitmap/pixel.hpp>
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
