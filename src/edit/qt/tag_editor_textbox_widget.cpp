// SPDX-License-Identifier: GPL-3.0-only

#include <QHBoxLayout>
#include <QLineEdit>
#include <QLabel>
#include "tag_editor_textbox_widget.hpp"

namespace Invader::EditQt {
    TagEditorTextboxWidget::TagEditorTextboxWidget(QWidget *parent, const char *text, TextboxSize size, std::size_t count, const std::optional<std::vector<std::string>> &labels) : TagEditorAbstractWidget(parent, text) {
        std::size_t label_count = labels.has_value() ? labels->size() : 0;

        for(std::size_t i = 0; i < count; i++) {
            if(i < label_count) {
                auto *label = new QLabel(this);
                label->setText((*labels)[i].c_str());
                this->layout()->addWidget(label);
            }
            auto *textbox = new QLineEdit();
            switch(size) {
                case SMALL:
                    textbox->setMinimumWidth(60);
                    break;
                case MEDIUM:
                    textbox->setMinimumWidth(120);
                    break;
                case LARGE:
                    textbox->setMinimumWidth(240);
                    break;
            }
            textbox->setMaximumWidth(textbox->minimumWidth());
            textbox->setSizePolicy(QSizePolicy::Policy::Fixed, QSizePolicy::Policy::Fixed);
            this->layout()->addWidget(textbox);
            this->text_boxes.emplace_back(textbox);
        }

        // Add a final label if necessary
        if(count < label_count) {
            auto *label = new QLabel(this);
            label->setText((*labels)[count].c_str());
            this->layout()->addWidget(label);
        }
    }

    QString TagEditorTextboxWidget::get_string(std::size_t textbox) const {
        return this->text_boxes[textbox]->text();
    }

    float TagEditorTextboxWidget::get_float(std::size_t textbox, bool *success) const {
        return this->get_string(textbox).toFloat(success);
    }

    std::int32_t TagEditorTextboxWidget::get_int(std::size_t textbox, bool *success) const {
        return this->get_string(textbox).toInt(success);
    }
}
