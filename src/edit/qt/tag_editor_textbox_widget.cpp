// SPDX-License-Identifier: GPL-3.0-only

#include <QHBoxLayout>
#include <QLineEdit>
#include <QLabel>
#include "tag_editor_textbox_widget.hpp"

namespace Invader::EditQt {
    TagEditorTextboxWidget::TagEditorTextboxWidget(QWidget *parent, const char *text, TextboxSize size) : TagEditorAbstractWidget(parent, text) {
        QLineEdit *textedit = new QLineEdit();
        switch(size) {
            case SMALL:
                textedit->setMinimumWidth(60);
                break;
            case MEDIUM:
                textedit->setMinimumWidth(120);
                break;
            case LARGE:
                textedit->setMinimumWidth(240);
                break;
        }
        textedit->setMaximumWidth(textedit->minimumWidth());
        textedit->setSizePolicy(QSizePolicy::Policy::Fixed, QSizePolicy::Policy::Fixed);
        this->layout()->addWidget(textedit);
    }
}
