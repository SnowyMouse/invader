// SPDX-License-Identifier: GPL-3.0-only

#include "tag_editor_group_widget.hpp"
#include <invader/tag/parser/parser_struct.hpp>
#include <QVBoxLayout>
#include <QLabel>
#include <QApplication>
#include <QFontDatabase>

namespace Invader::EditQt {
    TagEditorGroupWidget::TagEditorGroupWidget(QWidget *parent, Parser::ParserStructValue *value, TagEditorWindow *editor_window) : TagEditorWidget(parent, value, editor_window) {
        auto *layout = new QVBoxLayout();
        
        QFrame *header = new QFrame();
        header->setFrameStyle(QFrame::Panel);
        header->setLineWidth(1);
        
        auto *header_layout = new QVBoxLayout();
        
        auto *title_label = new QLabel(value->get_name());
        
        QFont font = QFontDatabase::systemFont(QFontDatabase::TitleFont);
        font.setBold(true);
        font.setCapitalization(QFont::AllUppercase);
        title_label->setFont(font);
        header_layout->addWidget(title_label);
        
        auto *description_label = new QLabel(value->get_comment());
        description_label->setWordWrap(true);
        header_layout->addWidget(description_label);
        
        QPalette palette;
        header->setAutoFillBackground(true);
        palette.setColor(QPalette::Window, QApplication::palette().color(QPalette::Light));
        header->setPalette(palette);
        
        header->setLayout(header_layout);
        
        layout->addWidget(header);
        this->setLayout(layout);
    }
}

