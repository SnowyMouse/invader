// SPDX-License-Identifier: GPL-3.0-only

#include <invader/tag/parser/parser.hpp>
#include <QLineEdit>
#include <QComboBox>
#include <QScrollBar>
#include <QPushButton>
#include <QListWidget>
#include <QListWidgetItem>
#include "tag_editor_edit_widget.hpp"
#include "tag_tree_window.hpp"
#include "tag_tree_dialog.hpp"

#define INTERNAL_VALUE "internal-value"

namespace Invader::EditQt {
    TagEditorEditWidget::TagEditorEditWidget(QWidget *parent, Parser::ParserStructValue *value, TagEditorWindow *editor_window) :
        TagEditorWidget(parent, value, editor_window),
        title_label(value->get_name()),
        hbox_layout(this) {
        this->setSizePolicy(QSizePolicy::Policy::Fixed, QSizePolicy::Policy::Fixed);
        int label_width = 300;
        int standard_width = this->title_label.fontMetrics().boundingRect("MMM").width();
        int prefix_label_width = standard_width * 3 / 5;
        this->title_label.setSizePolicy(QSizePolicy::Policy::Fixed, QSizePolicy::Policy::Fixed);
        this->title_label.setAlignment(Qt::AlignLeft | Qt::AlignTop);
        this->hbox_layout.addWidget(&this->title_label);
        this->hbox_layout.setMargin(0);
        this->setLayout(&hbox_layout);

        std::size_t value_index = 0;
        auto &widgets_array = this->widgets;
        auto &layout = this->hbox_layout;
        auto values = value->get_values();

        auto &textbox_widgets = this->textbox_widgets;

        auto add_widget = [&value_index, &value, &widgets_array, &layout, &values, &label_width, &textbox_widgets, &standard_width, &prefix_label_width]() {
            auto add_single_textbox = [&value, &value_index, &widgets_array, &layout, &values, &label_width, &textbox_widgets, &standard_width, &prefix_label_width](int size, const char *prefix = nullptr) -> QLineEdit * {
                // Make our textbox
                auto *textbox = reinterpret_cast<QLineEdit *>(widgets_array.emplace_back(std::make_unique<QLineEdit>()).get());

                // If we've got a prefix, set it
                if(prefix) {
                    auto *label = reinterpret_cast<QLabel *>(widgets_array.emplace_back(std::make_unique<QLabel>()).get());
                    label->setText(prefix);
                    label->setMaximumWidth(prefix_label_width);
                    label->setMinimumWidth(prefix_label_width);
                    label->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
                    layout.addWidget(label);

                    // Align it
                    if(value_index == 0) {
                        label_width -= prefix_label_width + layout.spacing();
                    }
                }

                // Parameters for textbox
                int width = standard_width * size;
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
                return textbox;
            };

            switch(value->get_type()) {
                // Simple value types; just textboxes
                case Parser::ParserStructValue::VALUE_TYPE_INT8:
                case Parser::ParserStructValue::VALUE_TYPE_UINT8:
                    add_single_textbox(1);
                    break;

                case Parser::ParserStructValue::VALUE_TYPE_INT16:
                case Parser::ParserStructValue::VALUE_TYPE_UINT16:
                    add_single_textbox(2);
                    break;

                case Parser::ParserStructValue::VALUE_TYPE_INDEX:
                    add_single_textbox(2);
                    break;

                case Parser::ParserStructValue::VALUE_TYPE_INT32:
                case Parser::ParserStructValue::VALUE_TYPE_UINT32:
                case Parser::ParserStructValue::VALUE_TYPE_FLOAT:
                case Parser::ParserStructValue::VALUE_TYPE_FRACTION:
                case Parser::ParserStructValue::VALUE_TYPE_ANGLE:
                    add_single_textbox(3);
                    break;

                case Parser::ParserStructValue::VALUE_TYPE_TAGSTRING:
                    add_single_textbox(12);
                    break;

                // Some more complex stuff with multiple boxes
                case Parser::ParserStructValue::VALUE_TYPE_COLORARGBINT:
                    add_single_textbox(2, "a:");
                    add_single_textbox(2, "r:");
                    add_single_textbox(2, "g:");
                    add_single_textbox(2, "b:");
                    break;
                case Parser::ParserStructValue::VALUE_TYPE_POINT2DINT:
                    add_single_textbox(2, "x:");
                    add_single_textbox(2, "y:");
                    break;
                case Parser::ParserStructValue::VALUE_TYPE_RECTANGLE2D:
                    add_single_textbox(2, "t:");
                    add_single_textbox(2, "r:");
                    add_single_textbox(2, "b:");
                    add_single_textbox(2, "l:");
                    break;
                case Parser::ParserStructValue::VALUE_TYPE_COLORARGB:
                    add_single_textbox(3, "a:");
                    add_single_textbox(3, "r:");
                    add_single_textbox(3, "g:");
                    add_single_textbox(3, "b:");
                    break;
                case Parser::ParserStructValue::VALUE_TYPE_COLORRGB:
                    add_single_textbox(3, "r:");
                    add_single_textbox(3, "g:");
                    add_single_textbox(3, "b:");
                    break;
                case Parser::ParserStructValue::VALUE_TYPE_VECTOR2D:
                    add_single_textbox(3, "i:");
                    add_single_textbox(3, "j:");
                    break;
                case Parser::ParserStructValue::VALUE_TYPE_VECTOR3D:
                    add_single_textbox(3, "i:");
                    add_single_textbox(3, "j:");
                    add_single_textbox(3, "k:");
                    break;
                case Parser::ParserStructValue::VALUE_TYPE_EULER2D:
                    add_single_textbox(3, "y:");
                    add_single_textbox(3, "p:");
                    break;
                case Parser::ParserStructValue::VALUE_TYPE_EULER3D:
                    add_single_textbox(3, "y:");
                    add_single_textbox(3, "p:");
                    add_single_textbox(3, "r:");
                    break;
                case Parser::ParserStructValue::VALUE_TYPE_PLANE2D:
                    add_single_textbox(3, "x:");
                    add_single_textbox(3, "y:");
                    add_single_textbox(3, "w:");
                    break;
                case Parser::ParserStructValue::VALUE_TYPE_POINT2D:
                    add_single_textbox(3, "x:");
                    add_single_textbox(3, "y:");
                    break;
                case Parser::ParserStructValue::VALUE_TYPE_POINT3D:
                    add_single_textbox(3, "x:");
                    add_single_textbox(3, "y:");
                    add_single_textbox(3, "z:");
                    break;
                case Parser::ParserStructValue::VALUE_TYPE_PLANE3D:
                    add_single_textbox(3, "x:");
                    add_single_textbox(3, "y:");
                    add_single_textbox(3, "z:");
                    add_single_textbox(3, "w:");
                    break;

                case Parser::ParserStructValue::VALUE_TYPE_QUATERNION:
                    add_single_textbox(3, "i:");
                    add_single_textbox(3, "j:");
                    add_single_textbox(3, "k:");
                    add_single_textbox(3, "w:");
                    break;
                case Parser::ParserStructValue::VALUE_TYPE_MATRIX:
                    add_single_textbox(3, "x0:");
                    add_single_textbox(3, "y0:");
                    add_single_textbox(3, "z0:");
                    add_single_textbox(3, "x1:");
                    add_single_textbox(3, "y1:");
                    add_single_textbox(3, "z1:");
                    add_single_textbox(3, "x2:");
                    add_single_textbox(3, "y2:");
                    add_single_textbox(3, "z2:");
                    break;

                // Dependencies!
                case Parser::ParserStructValue::VALUE_TYPE_DEPENDENCY: {
                    // Here's the combobox
                    auto *combobox = reinterpret_cast<QComboBox *>(widgets_array.emplace_back(new QComboBox()).get());
                    auto &allowed_classes = value->get_allowed_classes();
                    for(auto &c : allowed_classes) {
                        combobox->addItem(HEK::tag_class_to_extension(c));
                    }
                    layout.addWidget(combobox);

                    // Next, the textbox
                    auto *textbox = add_single_textbox(12);

                    // Next, get our thing
                    auto &dependency = value->get_dependency();

                    // Lastly, set the dependency
                    textbox->setText(Invader::File::halo_path_to_preferred_path(dependency.path).c_str());
                    for(auto &c : allowed_classes) {
                        if(c == dependency.tag_class_int) {
                            combobox->setCurrentIndex(&c - allowed_classes.data());
                            break;
                        }
                    }

                    break;
                }

                case Parser::ParserStructValue::VALUE_TYPE_TAGDATAOFFSET: {
                    auto *textbox = reinterpret_cast<QLineEdit *>(widgets_array.emplace_back(new QLineEdit()).get());
                    textbox->setText(QString::number(value->get_data_size()));
                    textbox->setReadOnly(true);
                    textbox->setMinimumWidth(3 * standard_width);
                    textbox->setMaximumWidth(textbox->minimumWidth());
                    layout.addWidget(textbox);
                    layout.addWidget(widgets_array.emplace_back(new QLabel("bytes")).get());
                    break;
                }

                case Parser::ParserStructValue::VALUE_TYPE_ENUM: {
                    auto *combobox = reinterpret_cast<QComboBox *>(widgets_array.emplace_back(new QComboBox()).get());

                    // Internal items
                    auto possible_values = value->list_enum();
                    QStringList internal_items = QList<QString>(possible_values.data(), possible_values.data() + possible_values.size());
                    combobox->setProperty(INTERNAL_VALUE, internal_items);

                    // "Pretty" items
                    auto pretty_values = value->list_enum_pretty();
                    QStringList pretty_items = QList<QString>(pretty_values.data(), pretty_values.data() + pretty_values.size());
                    combobox->addItems(pretty_items);
                    combobox->setFocusPolicy(Qt::StrongFocus);

                    auto current_value = value->read_enum();

                    // Set values
                    for(auto &v : possible_values) {
                        if(std::strcmp(current_value, v) == 0) {
                            combobox->setCurrentIndex(&v - possible_values.data());
                            break;
                        }
                    }
                    layout.addWidget(combobox);

                    break;
                }
                case Parser::ParserStructValue::VALUE_TYPE_BITMASK: {
                    auto *list = reinterpret_cast<QListWidget *>(widgets_array.emplace_back(new QListWidget()).get());
                    layout.addWidget(list);

                    // Internal items
                    auto possible_values = value->list_enum();
                    auto value_count = possible_values.size();
                    QStringList internal_items = QList<QString>(possible_values.data(), possible_values.data() + possible_values.size());
                    list->setProperty(INTERNAL_VALUE, internal_items);

                    // "Pretty" items
                    auto pretty_values = value->list_enum_pretty();
                    QStringList pretty_items = QList<QString>(pretty_values.data(), pretty_values.data() + pretty_values.size());
                    bool height_set = false;

                    for(std::size_t i = 0; i < value_count; i++) {
                        QListWidgetItem *item = new QListWidgetItem(tr(pretty_values[i]), list);
                        item->setCheckState(value->read_bitfield(possible_values[i]) ? Qt::Checked : Qt::Unchecked);
                        if(!height_set) {
                            int height = list->visualItemRect(item).height() * value_count + list->frameWidth() * 2;

                            list->setMinimumHeight(height);
                            list->setMaximumHeight(height);
                        }
                    }

                    break;
                }

                case Parser::ParserStructValue::VALUE_TYPE_REFLEXIVE:
                    std::terminate();
            }
        };

        add_widget();
        if(value->is_bounds()) {
            auto *hyphen = reinterpret_cast<QLabel *>(widgets_array.emplace_back(new QLabel(" - ")).get());
            hyphen->setMinimumWidth(prefix_label_width);
            hyphen->setMaximumWidth(prefix_label_width);
            hyphen->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
            hbox_layout.addWidget(hyphen);
            add_widget();
        }

        if(value->get_type() == Parser::ParserStructValue::VALUE_TYPE_ANGLE) {
            layout.addWidget(widgets_array.emplace_back(std::make_unique<QLabel>("degrees")).get());
        }
        else if(value->get_type() == Parser::ParserStructValue::VALUE_TYPE_DEPENDENCY) {
            // Set up dependency stuff
            this->verify_dependency_path();
            auto *combobox = reinterpret_cast<QComboBox *>(this->widgets[0].get());
            int comboWidth = combobox->fontMetrics().boundingRect("shader_transparent_chicago_extended").width() * 5 / 4;
            combobox->setMaximumWidth(comboWidth);
            combobox->setMinimumWidth(comboWidth);

            auto *button = reinterpret_cast<QPushButton *>(widgets_array.emplace_back(new QPushButton("Find...")).get());
            layout.addWidget(button);

            connect(button, &QPushButton::clicked, this, &TagEditorEditWidget::find_dependency);
            connect(combobox, &QComboBox::currentTextChanged, this, &TagEditorEditWidget::on_change);
        }
        else if(value->get_type() == Parser::ParserStructValue::VALUE_TYPE_ENUM) {
            // Set up the enum
            auto *combobox = reinterpret_cast<QComboBox *>(this->widgets[0].get());
            int comboWidth = standard_width * 12;
            combobox->setMaximumWidth(comboWidth);
            combobox->setMinimumWidth(comboWidth);
            connect(combobox, &QComboBox::currentTextChanged, this, &TagEditorEditWidget::on_change);
        }
        else if(value->get_type() == Parser::ParserStructValue::VALUE_TYPE_BITMASK) {
            // Set the list up
            auto *list = reinterpret_cast<QListWidget *>(this->widgets[0].get());
            int listWidth = standard_width * 12;
            list->setMaximumWidth(listWidth);
            list->setMinimumWidth(listWidth);
            connect(list, &QListWidget::itemChanged, this, &TagEditorEditWidget::on_change);
        }

        this->title_label.setMinimumWidth(label_width);
        this->title_label.setMaximumWidth(this->title_label.minimumWidth());

        for(auto *textbox_widget : this->textbox_widgets) {
            connect(textbox_widget, &QLineEdit::textEdited, this, &TagEditorEditWidget::on_change);
        }
    }

    void TagEditorEditWidget::on_change() {
        auto *value = this->get_struct_value();

        switch(value->get_type()) {
            case Parser::ParserStructValue::ValueType::VALUE_TYPE_TAGSTRING:
                value->set_string(this->textbox_widgets[0]->text().toLatin1().data());
                break;

            case Parser::ParserStructValue::ValueType::VALUE_TYPE_DEPENDENCY: {
                // Get the dependency stuff
                auto class_int = HEK::extension_to_tag_class(reinterpret_cast<QComboBox *>(this->widgets[0].get())->currentText().toLatin1().data());
                auto path = reinterpret_cast<QLineEdit *>(this->widgets[1].get())->text();

                // Set the dependency
                auto &dependency = value->get_dependency();
                dependency.tag_class_int = class_int;
                dependency.path = Invader::File::preferred_path_to_halo_path(path.toStdString());

                this->verify_dependency_path();
                break;
            }

            case Parser::ParserStructValue::ValueType::VALUE_TYPE_ENUM: {
                // Write the combobox stuff
                auto *combobox = reinterpret_cast<QComboBox *>(this->widgets[0].get());
                value->write_enum(combobox->property(INTERNAL_VALUE).toStringList()[combobox->currentIndex()].toLatin1().data());
                break;
            }

            case Parser::ParserStructValue::ValueType::VALUE_TYPE_BITMASK: {
                auto *list = reinterpret_cast<QListWidget *>(this->widgets[0].get());
                auto possible_values = value->list_enum();
                auto value_count = possible_values.size();
                for(std::size_t i = 0; i < value_count; i++) {
                    value->write_bitfield(possible_values[i], list->item(i)->checkState() == Qt::Checked);
                }
                break;
            }

            default: {
                // Set the number value
                auto widget_count = this->textbox_widgets.size();
                std::vector<Parser::ParserStructValue::Number> numbers(widget_count);

                // Go through each number
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
        }
        this->value_changed();
    }

    void TagEditorEditWidget::verify_dependency_path() {
        // Find it!
        auto &dependency = this->get_struct_value()->get_dependency();
        bool found = false;
        auto preferred_path = File::halo_path_to_preferred_path(dependency.path);

        // If an empty path was given, don't color it red then
        if(preferred_path == "") {
            found = true;
        }
        else {
            for(auto &t : this->get_editor_window()->get_parent_window()->get_all_tags()) {
                if(t.tag_class_int == dependency.tag_class_int && File::split_tag_class_extension(File::halo_path_to_preferred_path(t.tag_path)).value().path == preferred_path) {
                    found = true;
                    break;
                }
            }
        }

        // Get the path
        auto *textbox = textbox_widgets[0];
        if(found) {
            textbox->setStyleSheet("");
        }
        else {
            textbox->setStyleSheet("color: #FF0000");
        }
    }

    void TagEditorEditWidget::find_dependency() {
        TagTreeDialog dialog(this, this->get_editor_window()->get_parent_window(), this->get_struct_value()->get_allowed_classes());
        dialog.exec();
        auto &tag = dialog.get_tag();
        if(tag.has_value()) {
            auto &tag_val = tag.value();
            this->textbox_widgets[0]->setText(File::split_tag_class_extension(File::halo_path_to_preferred_path(tag_val.tag_path))->path.c_str());
            reinterpret_cast<QComboBox *>(this->widgets[0].get())->setCurrentText(HEK::tag_class_to_extension(tag_val.tag_class_int));
            this->on_change();
        }
    }
}
