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
#include <QStandardItemModel>
#include <QStandardItem>

namespace Invader::EditQt {
    TagEditorArrayWidget::TagEditorArrayWidget(QWidget *parent, Parser::ParserStructValue *value, TagEditorWindow *editor_window) : TagEditorWidget(parent, value, editor_window) {
        this->vbox_layout = new QVBoxLayout();
        this->reflexive_index = new QComboBox();
        this->item_model = new QStandardItemModel(this->reflexive_index);
        this->reflexive_index->setModel(this->item_model);

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
        title_label->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        title_label->setMinimumWidth(300);
        title_label->setMaximumWidth(300);
        this->reflexive_index->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);
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

        // Buttons
        this->add_button = new QPushButton("Add");
        connect(this->add_button, &QPushButton::clicked, this, &TagEditorArrayWidget::perform_add);
        header_layout->addWidget(this->add_button);
        this->add_button->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

        this->delete_button = new QPushButton("Delete");
        connect(this->delete_button, &QPushButton::clicked, this, &TagEditorArrayWidget::perform_delete);
        header_layout->addWidget(this->delete_button);
        this->delete_button->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

        this->duplicate_button = new QPushButton("Duplicate");
        connect(this->duplicate_button, &QPushButton::clicked, this, &TagEditorArrayWidget::perform_duplicate);
        header_layout->addWidget(this->duplicate_button);
        this->duplicate_button->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

        this->shift_up_button = new QPushButton("Shift Up");
        connect(this->shift_up_button, &QPushButton::clicked, this, &TagEditorArrayWidget::perform_shift_up);
        header_layout->addWidget(this->shift_up_button);
        this->shift_up_button->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

        this->shift_down_button = new QPushButton("Shift Down");
        connect(this->shift_down_button, &QPushButton::clicked, this, &TagEditorArrayWidget::perform_shift_down);
        header_layout->addWidget(this->shift_down_button);
        this->shift_down_button->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

        this->clear_button = new QPushButton("Clear");
        connect(this->clear_button, &QPushButton::clicked, this, &TagEditorArrayWidget::perform_clear);
        header_layout->addWidget(this->clear_button);
        this->clear_button->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

        // Set this stuff up
        this->regenerate_enum();
        this->regenerate_widget();
        connect(this->reflexive_index, &QComboBox::currentTextChanged, this, &TagEditorArrayWidget::regenerate_widget);

        header_layout->addStretch(1);
    }

    int TagEditorArrayWidget::current_index() const noexcept {
        return this->reflexive_index->currentIndex();
    }

    void TagEditorArrayWidget::perform_add() {
        int index = this->current_index() + 1;
        this->get_struct_value()->insert_objects_in_array(index, 1);
        this->regenerate_enum();
        this->value_changed();

        this->reflexive_index->blockSignals(true);
        this->reflexive_index->setCurrentIndex(index);
        this->reflexive_index->blockSignals(false);
        this->regenerate_widget();
    }

    void TagEditorArrayWidget::perform_delete() {
        // Delete the object at the current index
        int index = this->current_index();
        this->get_struct_value()->delete_objects_in_array(static_cast<std::size_t>(index), 1);

        // Decrement if possible
        if(--index < 0 && this->get_struct_value()->get_array_size() > 0) {
            index = 0;
        }

        this->regenerate_enum();
        this->value_changed();

        this->reflexive_index->blockSignals(true);
        this->reflexive_index->setCurrentIndex(index);
        this->reflexive_index->blockSignals(false);
        this->regenerate_widget();
    }

    void TagEditorArrayWidget::perform_duplicate() {
        auto index = static_cast<std::size_t>(this->current_index());
        this->get_struct_value()->duplicate_objects_in_array(index, index, 1);

        this->regenerate_enum();
        this->value_changed();

        this->reflexive_index->blockSignals(true);
        this->reflexive_index->setCurrentIndex(index + 1);
        this->reflexive_index->blockSignals(false);
        this->regenerate_widget();
    }

    void TagEditorArrayWidget::perform_clear() {
        this->get_struct_value()->delete_objects_in_array(0, this->get_struct_value()->get_array_size());
        this->regenerate_enum();
        this->value_changed();

        this->reflexive_index->blockSignals(true);
        this->reflexive_index->setCurrentIndex(-1);
        this->reflexive_index->blockSignals(false);
        this->regenerate_widget();
    }

    void TagEditorArrayWidget::perform_shift_up() {
        auto index = static_cast<std::size_t>(this->current_index());
        this->get_struct_value()->duplicate_objects_in_array(index, index + 2, 1);
        this->get_struct_value()->delete_objects_in_array(index, 1);

        this->regenerate_enum();
        this->value_changed();

        this->reflexive_index->blockSignals(true);
        this->reflexive_index->setCurrentIndex(index + 1);
        this->reflexive_index->blockSignals(false);
        this->regenerate_widget();
    }

    void TagEditorArrayWidget::perform_shift_down() {
        auto index = static_cast<std::size_t>(this->current_index());
        this->get_struct_value()->duplicate_objects_in_array(index, index - 1, 1);
        this->get_struct_value()->delete_objects_in_array(index + 1, 1);

        this->regenerate_enum();
        this->value_changed();

        this->reflexive_index->blockSignals(true);
        this->reflexive_index->setCurrentIndex(index - 1);
        this->reflexive_index->blockSignals(false);
        this->regenerate_widget();
    }

    void TagEditorArrayWidget::regenerate_widget() {
        delete this->tag_view_widget;

        this->set_buttons_enabled();

        // Make sure we got it!
        int index = this->current_index();
        std::size_t count = this->get_struct_value()->get_array_size();
        if(index < 0 || static_cast<std::size_t>(index) > count) {
            this->tag_view_widget = nullptr;
            return;
        }

        // Get it!
        auto index_unsigned = static_cast<std::size_t>(index);
        auto &s = this->get_struct_value()->get_object_in_array(index_unsigned);
        this->tag_view_widget = new TagEditorEditWidgetView(nullptr, s.get_values(), this->get_editor_window(), false);
        this->vbox_layout->addWidget(this->tag_view_widget);
    }

    void TagEditorArrayWidget::regenerate_enum() {
        this->reflexive_index->blockSignals(true);
        this->reflexive_index->setUpdatesEnabled(false);

        // Use a QStandardItemModel - it's a bit faster than adding directly, especially on Windows for whatever reason
        this->item_model->clear();
        std::size_t count = this->get_struct_value()->get_array_size();
        for(std::size_t i = 0; i < count; i++) {
            this->item_model->appendRow(new QStandardItem(QString::number(i)));
        }

        this->reflexive_index->setUpdatesEnabled(true);
        this->reflexive_index->blockSignals(false);
    }

    void TagEditorArrayWidget::set_buttons_enabled() {
        std::size_t count = this->get_struct_value()->get_array_size();
        int index = this->current_index();

        this->delete_button->setEnabled(index >= 0);
        this->clear_button->setEnabled(count > 0);
        this->shift_down_button->setEnabled(index > 0);
        this->shift_up_button->setEnabled(index >= 0 ? static_cast<std::size_t>(index) + 1 < count : false);
        this->duplicate_button->setEnabled(index >= 0);
    }
}
