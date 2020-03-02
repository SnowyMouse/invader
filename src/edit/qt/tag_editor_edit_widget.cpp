// SPDX-License-Identifier: GPL-3.0-only

#include <invader/tag/parser/parser.hpp>
#include <QLineEdit>
#include "tag_editor_edit_widget.hpp"

namespace Invader::EditQt {
    TagEditorEditWidget::TagEditorEditWidget(QWidget *parent, Parser::ParserStructValue *value, TagEditorWindow *editor_window) :
        TagEditorWidget(parent, value, editor_window),
        title_label(value->get_name()),
        hbox_layout(this) {
        this->setSizePolicy(QSizePolicy::Policy::Fixed, QSizePolicy::Policy::Fixed);
        int label_width = 300;
        this->title_label.setSizePolicy(QSizePolicy::Policy::Fixed, QSizePolicy::Policy::Fixed);
        this->title_label.setAlignment(Qt::AlignLeft | Qt::AlignTop);
        this->hbox_layout.addWidget(&this->title_label);
        this->hbox_layout.setMargin(0);
        this->setLayout(&hbox_layout);

        int value_index = 0;
        auto &widgets_array = this->widgets;
        auto &layout = this->hbox_layout;
        auto values = value->get_values();

        auto &textbox_widgets = this->textbox_widgets;

        auto add_widget = [&value_index, &value, &widgets_array, &layout, &values, &label_width, &textbox_widgets]() {
            auto add_single_textbox = [&value, &value_index, &widgets_array, &layout, &values, &label_width, &textbox_widgets](int size, const char *prefix = nullptr) {
                // If we've got a prefix, set it
                if(prefix) {
                    auto *label = reinterpret_cast<QLabel *>(widgets_array.emplace_back(std::make_unique<QLabel>()).get());
                    label->setText(prefix);
                    layout.addWidget(label);

                    // Align it
                    if(value_index == 0) {
                        QFontMetrics fm = label->fontMetrics();
                        label_width -= fm.boundingRect(prefix).width() + layout.spacing() + 1;
                    }
                }

                // Make our textbox
                int width = 50 * size;
                auto *textbox = reinterpret_cast<QLineEdit *>(widgets_array.emplace_back(std::make_unique<QLineEdit>()).get());
                textbox_widgets.emplace_back(textbox);
                textbox->setMinimumWidth(width);
                textbox->setMaximumWidth(width);
                layout.addWidget(textbox);

                // Radians get converted to degrees
                if(value->get_type() == Parser::ParserStructValue::VALUE_TYPE_ANGLE) {
                    textbox->setText(QString::number(RADIANS_TO_DEGREES(std::get<double>(values[value_index]))));
                }
                else if(value->get_number_format() == Parser::ParserStructValue::NumberFormat::NUMBER_FORMAT_FLOAT) {
                    textbox->setText(QString::number(std::get<double>(values[value_index])));
                }
                else if(value->get_number_format() == Parser::ParserStructValue::NumberFormat::NUMBER_FORMAT_INT) {
                    textbox->setText(QString::number(std::get<std::int64_t>(values[value_index])));
                }
                else if(value->get_type() == Parser::ParserStructValue::VALUE_TYPE_TAGSTRING) {
                    textbox->setText(value->get_string());
                }

                value_index++;
            };

            switch(value->get_type()) {
                case Parser::ParserStructValue::VALUE_TYPE_INT8:
                    return add_single_textbox(1);
                case Parser::ParserStructValue::VALUE_TYPE_UINT8:
                    return add_single_textbox(1);
                case Parser::ParserStructValue::VALUE_TYPE_INT16:
                    return add_single_textbox(2);
                case Parser::ParserStructValue::VALUE_TYPE_UINT16:
                    return add_single_textbox(2);
                case Parser::ParserStructValue::VALUE_TYPE_INDEX:
                    return add_single_textbox(2);
                case Parser::ParserStructValue::VALUE_TYPE_INT32:
                    return add_single_textbox(3);
                case Parser::ParserStructValue::VALUE_TYPE_UINT32:
                    return add_single_textbox(3);
                case Parser::ParserStructValue::VALUE_TYPE_FLOAT:
                    return add_single_textbox(3);
                case Parser::ParserStructValue::VALUE_TYPE_FRACTION:
                    return add_single_textbox(3);
                case Parser::ParserStructValue::VALUE_TYPE_ANGLE:
                    return add_single_textbox(3);

                case Parser::ParserStructValue::VALUE_TYPE_COLORARGBINT:
                    add_single_textbox(2, "a");
                    add_single_textbox(2, "r");
                    add_single_textbox(2, "g");
                    add_single_textbox(2, "b");
                    break;
                case Parser::ParserStructValue::VALUE_TYPE_POINT2DINT:
                    add_single_textbox(2, "x");
                    add_single_textbox(2, "y");
                    break;
                case Parser::ParserStructValue::VALUE_TYPE_RECTANGLE2D:
                    add_single_textbox(2, "top");
                    add_single_textbox(2, "right");
                    add_single_textbox(2, "bottom");
                    add_single_textbox(2, "left");
                    break;
                case Parser::ParserStructValue::VALUE_TYPE_COLORARGB:
                    add_single_textbox(3, "a");
                    add_single_textbox(3, "r");
                    add_single_textbox(3, "g");
                    add_single_textbox(3, "b");
                    break;
                case Parser::ParserStructValue::VALUE_TYPE_COLORRGB:
                    add_single_textbox(3, "r");
                    add_single_textbox(3, "g");
                    add_single_textbox(3, "b");
                    break;
                case Parser::ParserStructValue::VALUE_TYPE_VECTOR2D:
                    add_single_textbox(3, "i");
                    add_single_textbox(3, "j");
                    break;
                case Parser::ParserStructValue::VALUE_TYPE_VECTOR3D:
                    add_single_textbox(3, "i");
                    add_single_textbox(3, "j");
                    add_single_textbox(3, "k");
                    break;
                case Parser::ParserStructValue::VALUE_TYPE_EULER2D:
                    add_single_textbox(3, "yaw");
                    add_single_textbox(3, "pitch");
                    break;
                case Parser::ParserStructValue::VALUE_TYPE_EULER3D:
                    add_single_textbox(3, "yaw");
                    add_single_textbox(3, "pitch");
                    add_single_textbox(3, "roll");
                    break;
                case Parser::ParserStructValue::VALUE_TYPE_PLANE2D:
                    add_single_textbox(3, "x");
                    add_single_textbox(3, "y");
                    add_single_textbox(3, "w");
                    break;
                case Parser::ParserStructValue::VALUE_TYPE_POINT2D:
                    add_single_textbox(3, "x");
                    add_single_textbox(3, "y");
                    break;
                case Parser::ParserStructValue::VALUE_TYPE_POINT3D:
                    add_single_textbox(3, "x");
                    add_single_textbox(3, "y");
                    add_single_textbox(3, "z");
                    break;
                case Parser::ParserStructValue::VALUE_TYPE_PLANE3D:
                case Parser::ParserStructValue::VALUE_TYPE_QUATERNION:
                    add_single_textbox(3, "x");
                    add_single_textbox(3, "y");
                    add_single_textbox(3, "z");
                    add_single_textbox(3, "w");
                    break;
                case Parser::ParserStructValue::VALUE_TYPE_MATRIX:
                    add_single_textbox(3, "x0");
                    add_single_textbox(3, "y0");
                    add_single_textbox(3, "z0");
                    add_single_textbox(3, "x1");
                    add_single_textbox(3, "y1");
                    add_single_textbox(3, "z1");
                    add_single_textbox(3, "x2");
                    add_single_textbox(3, "y2");
                    add_single_textbox(3, "z2");
                    break;
                case Parser::ParserStructValue::VALUE_TYPE_TAGSTRING:
                    return add_single_textbox(6);
                case Parser::ParserStructValue::VALUE_TYPE_DATA:
                case Parser::ParserStructValue::VALUE_TYPE_TAGDATAOFFSET:
                case Parser::ParserStructValue::VALUE_TYPE_ENUM:
                case Parser::ParserStructValue::VALUE_TYPE_BITMASK:
                case Parser::ParserStructValue::VALUE_TYPE_DEPENDENCY:
                    break;

                case Parser::ParserStructValue::VALUE_TYPE_REFLEXIVE:
                    std::terminate();
            }
        };

        add_widget();
        if(value->is_bounds()) {
            hbox_layout.addWidget(widgets_array.emplace_back(std::make_unique<QLabel>(" - ")).get());
            add_widget();
        }

        if(value->get_type() == Parser::ParserStructValue::VALUE_TYPE_ANGLE) {
            layout.addWidget(widgets_array.emplace_back(std::make_unique<QLabel>("degrees")).get());
        }

        this->title_label.setMinimumWidth(label_width);
        this->title_label.setMaximumWidth(this->title_label.minimumWidth());

        for(auto *textbox_widget : this->textbox_widgets) {
            connect(textbox_widget, &QLineEdit::textEdited, this, &TagEditorEditWidget::on_change);
        }
    }

    void TagEditorEditWidget::on_change(QString string_value) {
        auto *value = this->get_struct_value();
        if(value->get_type() == Parser::ParserStructValue::ValueType::VALUE_TYPE_TAGSTRING) {
            value->set_string(string_value.toLatin1().data());
        }
        else {
            auto widget_count = this->textbox_widgets.size();
            std::vector<Parser::ParserStructValue::Number> numbers(widget_count);
            for(std::size_t w = 0; w < widget_count; w++) {
                auto *widget = this->textbox_widgets[w];
                switch(value->get_number_format()) {
                    case Parser::ParserStructValue::NumberFormat::NUMBER_FORMAT_FLOAT: {
                        double double_value = widget->text().toDouble();
                        if(value->get_type() == Parser::ParserStructValue::ValueType::VALUE_TYPE_ANGLE) {
                            double_value = DEGREES_TO_RADIANS(double_value);
                        }
                        numbers[w] = double_value;
                        break;
                    }
                    case Parser::ParserStructValue::NumberFormat::NUMBER_FORMAT_INT:
                        numbers[w] = static_cast<std::int64_t>(widget->text().toLongLong());
                        break;
                    default:
                        std::terminate();
                }
            }
            value->set_values(numbers);
        }
        this->value_changed();
    }
}
