// SPDX-License-Identifier: GPL-3.0-only

#include "tag_editor_array_widget.hpp"
#include "tag_editor_edit_widget_view.hpp"
#include <invader/tag/parser/parser_struct.hpp>

#include <QSpacerItem>
#include <QFontDatabase>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>

namespace Invader::EditQt {
    TagEditorArrayWidget::TagEditorArrayWidget(QWidget *parent, Parser::ParserStructValue *value, TagEditorWindow *editor_window) : TagEditorWidget(parent, value, editor_window) {
        this->vbox_layout = new QVBoxLayout();
        this->reflexive_index = new QComboBox();

        // Set our header stuff
        QFrame *header = new QFrame();
        QHBoxLayout *header_layout = new QHBoxLayout();
        QLabel *title_label = new QLabel();
        title_label->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
        this->setLayout(this->vbox_layout);
        this->vbox_layout->addWidget(header);
        header->setLayout(header_layout);
        header_layout->addWidget(title_label);
        header_layout->addWidget(this->reflexive_index);
        header->setFrameStyle(QFrame::Panel | QFrame::Raised);
        header->setLineWidth(2);
        header_layout->setMargin(8);
        header_layout->setMargin(8);
        this->vbox_layout->setSpacing(0);

        // Set size stuff
        this->setSizePolicy(QSizePolicy::Policy::Fixed, QSizePolicy::Policy::Fixed);
        title_label->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        title_label->setMinimumWidth(300);
        title_label->setMaximumWidth(300);
        this->reflexive_index->setMinimumWidth(150);
        this->reflexive_index->setMaximumWidth(150);
        this->reflexive_index->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        this->vbox_layout->setMargin(0);
        title_label->setText(value->get_name());

        // Font stuff
        QFont font = QFontDatabase::systemFont(QFontDatabase::TitleFont);
        font.setBold(true);
        font.setCapitalization(QFont::AllUppercase);
        title_label->setFont(font);

        // Set this stuff up
        this->regenerate_enum();
        this->regenerate_widget();
        connect(this->reflexive_index, &QComboBox::currentTextChanged, this, &TagEditorArrayWidget::regenerate_widget);

        // Buttons
        auto *add_button = new QPushButton("Add");
        connect(add_button, &QPushButton::clicked, this, &TagEditorArrayWidget::perform_add);
        header_layout->addWidget(add_button);
        add_button->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

        auto *delete_button = new QPushButton("Delete");
        connect(delete_button, &QPushButton::clicked, this, &TagEditorArrayWidget::perform_delete);
        header_layout->addWidget(delete_button);
        delete_button->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

        auto *duplicate_button = new QPushButton("Duplicate");
        connect(duplicate_button, &QPushButton::clicked, this, &TagEditorArrayWidget::perform_duplicate);
        header_layout->addWidget(duplicate_button);
        duplicate_button->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

        auto *shift_up_button = new QPushButton("Shift Up");
        connect(shift_up_button, &QPushButton::clicked, this, &TagEditorArrayWidget::perform_shift_up);
        header_layout->addWidget(shift_up_button);
        shift_up_button->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

        auto *shift_down_button = new QPushButton("Shift Down");
        connect(shift_down_button, &QPushButton::clicked, this, &TagEditorArrayWidget::perform_shift_down);
        header_layout->addWidget(shift_down_button);
        shift_down_button->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

        auto *clear_button = new QPushButton("Clear");
        connect(clear_button, &QPushButton::clicked, this, &TagEditorArrayWidget::perform_clear);
        header_layout->addWidget(clear_button);
        clear_button->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

        header_layout->addStretch(1);
    }

    void TagEditorArrayWidget::perform_add() {
        std::printf("TODO: perform_add()\n");
    }

    void TagEditorArrayWidget::perform_delete() {
        std::printf("TODO: perform_delete()\n");
    }

    void TagEditorArrayWidget::perform_duplicate() {
        std::printf("TODO: perform_duplicate()\n");
    }

    void TagEditorArrayWidget::perform_clear() {
        std::printf("TODO: perform_clear()\n");
    }

    void TagEditorArrayWidget::perform_shift_up() {
        std::printf("TODO: perform_shift_up()\n");
    }

    void TagEditorArrayWidget::perform_shift_down() {
        std::printf("TODO: perform_shift_down()\n");
    }

    void TagEditorArrayWidget::regenerate_widget() {
        delete this->tag_view_widget;

        // Make sure we got it!
        int index = this->reflexive_index->currentIndex();
        std::size_t count = this->get_struct_value()->get_array_size();
        if(index < 0 || static_cast<std::size_t>(index) > count) {
            return;
        }

        // Get it!
        auto index_unsigned = static_cast<std::size_t>(index);
        auto &s = this->get_struct_value()->get_object_in_array(index_unsigned);
        this->tag_view_widget = new TagEditorEditWidgetView(nullptr, s.get_values(), this->get_editor_window(), false);
        this->vbox_layout->addWidget(this->tag_view_widget);
    }

    void TagEditorArrayWidget::regenerate_enum() {
        this->reflexive_index->clear();
        std::size_t count = this->get_struct_value()->get_array_size();
        for(std::size_t i = 0; i < count; i++) {
            this->reflexive_index->addItem(QString::number(i));
        }
    }
}
