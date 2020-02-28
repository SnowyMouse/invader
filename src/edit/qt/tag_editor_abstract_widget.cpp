// SPDX-License-Identifier: GPL-3.0-only

#include <QHBoxLayout>
#include <QLabel>
#include "tag_editor_abstract_widget.hpp"

namespace Invader::EditQt {
    TagEditorAbstractWidget::TagEditorAbstractWidget(QWidget *parent, const char *text) : QWidget(parent) {
        auto *hbox_layout = new QHBoxLayout(this);
        this->setSizePolicy(QSizePolicy::Policy::Fixed, QSizePolicy::Policy::Fixed);
        this->title_label = new QLabel(this);
        this->title_label->setMinimumWidth(300);
        this->title_label->setMaximumWidth(this->title_label->minimumWidth());
        this->title_label->setText(text);
        this->title_label->setSizePolicy(QSizePolicy::Policy::Fixed, QSizePolicy::Policy::Fixed);
        this->title_label->setAlignment(Qt::AlignLeft | Qt::AlignTop);
        hbox_layout->addWidget(this->title_label);
        hbox_layout->setMargin(0);
        this->setLayout(hbox_layout);
    }

    QLabel *TagEditorAbstractWidget::get_title_label() noexcept {
        return this->title_label;
    }
}
