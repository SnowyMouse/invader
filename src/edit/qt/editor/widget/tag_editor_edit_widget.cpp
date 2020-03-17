// SPDX-License-Identifier: GPL-3.0-only

#include <invader/tag/parser/parser.hpp>
#include <QLineEdit>
#include <QComboBox>
#include <QScrollBar>
#include <QPushButton>
#include <QWheelEvent>
#include <QListWidget>
#include <QListWidgetItem>
#include <QStandardItemModel>
#include <QStandardItem>
#include <QCheckBox>
#include <QGraphicsView>
#include <QGraphicsScene>
#include "tag_editor_edit_widget.hpp"
#include "../../tree/tag_tree_window.hpp"
#include "../../tree/tag_tree_dialog.hpp"
#include <invader/bitmap/color_plate_pixel.hpp>
#include "tag_editor_array_widget.hpp"

#define INTERNAL_VALUE "internal-value"

namespace Invader::EditQt {
    /**
     * Enum box to prevent accidentally changing values by scrolling over them
     */
    class EnumComboBox : public QComboBox {
        void wheelEvent(QWheelEvent *event) override {
            event->ignore();
        }
    };

    TagEditorEditWidget::TagEditorEditWidget(QWidget *parent, Parser::ParserStructValue *value, TagEditorWindow *editor_window, TagEditorArrayWidget *array_widget) :
        TagEditorWidget(parent, value, editor_window), array_widget(array_widget) {

        auto *comment = value->get_comment();
        if(comment) {
            this->setToolTip(comment);
        }

        this->read_only = value->is_read_only() && editor_window->get_parent_window()->safeguards();

        auto *title_label = new QLabel(value->get_name());
        int standard_width = title_label->fontMetrics().boundingRect("MMMM").width();
        int prefix_label_width = standard_width / 2;
        int label_width = standard_width * 7;

        title_label->setSizePolicy(QSizePolicy::Policy::Fixed, QSizePolicy::Policy::Fixed);
        title_label->setAlignment(Qt::AlignLeft | Qt::AlignTop);

        auto *layout = new QHBoxLayout();
        layout->addWidget(title_label);
        this->setLayout(layout);

        std::size_t value_index = 0;
        auto &widgets_array = this->widgets;
        auto values = value->get_values();

        auto &textbox_widgets = this->textbox_widgets;

        auto &read_only = this->read_only;
        auto &auxiliary_widget = this->auxiliary_widget;
        auto &auxiliary_checkbox = this->auxiliary_checkbox;
        auxiliary_widget = nullptr;

        auto add_widget = [&value_index, &value, &widgets_array, &layout, &values, &label_width, &textbox_widgets, &standard_width, &prefix_label_width, &read_only, &auxiliary_widget, &auxiliary_checkbox]() {
            auto add_single_textbox = [&value, &value_index, &widgets_array, &layout, &values, &label_width, &textbox_widgets, &standard_width, &prefix_label_width, &read_only](int size, const char *prefix = nullptr) -> QLineEdit * {
                // Make our textbox
                auto *textbox = reinterpret_cast<QLineEdit *>(widgets_array.emplace_back(new QLineEdit()));

                // If we've got a prefix, set it
                if(prefix) {
                    auto *label = reinterpret_cast<QLabel *>(widgets_array.emplace_back(new QLabel()));
                    label->setText(prefix);
                    label->setMaximumWidth(prefix_label_width);
                    label->setMinimumWidth(prefix_label_width);
                    label->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
                    layout->addWidget(label);

                    // Align it
                    if(value_index == 0) {
                        label_width -= prefix_label_width + layout->spacing();
                    }
                }

                // Parameters for textbox
                int width = standard_width * size;
                textbox_widgets.emplace_back(textbox);
                textbox->setMinimumWidth(width);
                layout->addWidget(textbox);

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
                    textbox->setMaxLength(sizeof(HEK::TagString::string) - 1);
                }

                if(read_only) {
                    textbox->setEnabled(false);
                }

                value_index++;
                return textbox;
            };

            // Make a color widget thing
            auto make_color_widget = [&auxiliary_widget, &layout, &auxiliary_checkbox]() {
                auto *new_auxiliary_widget = new QGraphicsView();
                new_auxiliary_widget->setMaximumHeight(QLineEdit().minimumSizeHint().height());
                new_auxiliary_widget->setMaximumWidth(new_auxiliary_widget->maximumHeight());
                new_auxiliary_widget->setMinimumHeight(new_auxiliary_widget->maximumHeight());
                new_auxiliary_widget->setMinimumWidth(new_auxiliary_widget->maximumWidth());
                new_auxiliary_widget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
                new_auxiliary_widget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
                auxiliary_widget = new_auxiliary_widget;
                layout->addWidget(auxiliary_widget);

                auxiliary_checkbox = new QCheckBox("Preview alpha");
                auxiliary_checkbox->setCheckState(Qt::Checked);
                layout->addWidget(auxiliary_checkbox);
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
                    add_single_textbox(8);
                    break;

                // Some more complex stuff with multiple boxes
                case Parser::ParserStructValue::VALUE_TYPE_COLORARGBINT:
                    add_single_textbox(1, "a:");
                    add_single_textbox(1, "r:");
                    add_single_textbox(1, "g:");
                    add_single_textbox(1, "b:");
                    make_color_widget();
                    auxiliary_checkbox->setCheckState(Qt::Unchecked);
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
                    make_color_widget();
                    break;
                case Parser::ParserStructValue::VALUE_TYPE_COLORRGB:
                    add_single_textbox(3, "r:");
                    add_single_textbox(3, "g:");
                    add_single_textbox(3, "b:");
                    make_color_widget();
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
                    auto *combobox = reinterpret_cast<QComboBox *>(widgets_array.emplace_back(new EnumComboBox()));
                    combobox->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);

                    // Use a QStandardItemModel - it's a bit faster than adding directly, especially on Windows for whatever reason
                    auto *model = new QStandardItemModel(combobox);
                    auto &allowed_classes = value->get_allowed_classes();
                    std::size_t count = allowed_classes.size();
                    if(count) {
                        for(std::size_t i = 0; i < count; i++) {
                            model->appendRow(new QStandardItem(HEK::tag_class_to_extension(allowed_classes[i])));
                        }
                    }
                    else {
                        auto all_classes = Parser::ParserStruct::all_tag_classes(true);
                        for(auto c : all_classes) {
                            model->appendRow(new QStandardItem(HEK::tag_class_to_extension(c)));
                        }
                    }
                    combobox->setModel(model);

                    layout->addWidget(combobox);

                    // Next, the textbox
                    auto *textbox = add_single_textbox(8);

                    // Next, get our thing
                    auto &dependency = value->get_dependency();

                    // Lastly, set the dependency
                    textbox->setText(Invader::File::halo_path_to_preferred_path(dependency.path).c_str());
                    combobox->setCurrentText(HEK::tag_class_to_extension(dependency.tag_class_int));

                    break;
                }

                case Parser::ParserStructValue::VALUE_TYPE_TAGDATAOFFSET: {
                    auto *textbox = reinterpret_cast<QLineEdit *>(widgets_array.emplace_back(new QLineEdit()));
                    textbox->setText(QString::number(value->get_data_size()));
                    textbox->setEnabled(false);
                    textbox->setMinimumWidth(3 * standard_width);
                    textbox->setMaximumWidth(textbox->minimumWidth());
                    layout->addWidget(textbox);
                    layout->addWidget(widgets_array.emplace_back(new QLabel("bytes")));
                    break;
                }

                case Parser::ParserStructValue::VALUE_TYPE_ENUM: {
                    auto *combobox = reinterpret_cast<QComboBox *>(widgets_array.emplace_back(new EnumComboBox()));
                    combobox->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);

                    // Internal items
                    auto possible_values = value->list_enum();
                    QStringList internal_items = QList<QString>(possible_values.data(), possible_values.data() + possible_values.size());
                    combobox->setProperty(INTERNAL_VALUE, internal_items);

                    // "Pretty" items
                    auto pretty_values = value->list_enum_pretty();

                    // Use a QStandardItemModel - it's a bit faster than adding directly, especially on Windows for whatever reason
                    auto *model = new QStandardItemModel(combobox);
                    std::size_t count = pretty_values.size();
                    for(std::size_t i = 0; i < count; i++) {
                        model->appendRow(new QStandardItem(pretty_values[i]));
                    }
                    combobox->setModel(model);

                    combobox->setFocusPolicy(Qt::StrongFocus);

                    auto current_value = value->read_enum();

                    // Set values
                    for(auto &v : possible_values) {
                        if(std::strcmp(current_value, v) == 0) {
                            combobox->setCurrentIndex(&v - possible_values.data());
                            break;
                        }
                    }
                    layout->addWidget(combobox);

                    break;
                }
                case Parser::ParserStructValue::VALUE_TYPE_BITMASK: {
                    auto *list = reinterpret_cast<QListWidget *>(widgets_array.emplace_back(new QListWidget()));
                    layout->addWidget(list);

                    // Internal items
                    auto possible_values = value->list_enum();
                    auto value_count = possible_values.size();
                    QStringList internal_items = QList<QString>(possible_values.data(), possible_values.data() + possible_values.size());
                    list->setProperty(INTERNAL_VALUE, internal_items);
                    list->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
                    list->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

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
            auto *hyphen = reinterpret_cast<QLabel *>(widgets_array.emplace_back(new QLabel(" - ")));
            hyphen->setMinimumWidth(prefix_label_width);
            hyphen->setMaximumWidth(prefix_label_width);
            hyphen->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
            layout->addWidget(hyphen);
            add_widget();
        }

        if(value->get_type() == Parser::ParserStructValue::VALUE_TYPE_DEPENDENCY) {
            // Set up dependency stuff
            auto *combobox = reinterpret_cast<QComboBox *>(this->widgets[0]);
            int comboWidth = combobox->fontMetrics().boundingRect("shader_transparent_chicago_extended").width() * 5 / 4;
            combobox->setMaximumWidth(comboWidth);
            combobox->setMinimumWidth(comboWidth);

            auto *find_button = reinterpret_cast<QPushButton *>(widgets_array.emplace_back(new QPushButton("Find...")));
            layout->addWidget(find_button);

            auto *open_button = reinterpret_cast<QPushButton *>(widgets_array.emplace_back(new QPushButton("Open...")));
            layout->addWidget(open_button);
            connect(open_button, &QPushButton::clicked, this, &TagEditorEditWidget::open_dependency);

            // For read-only stuff, disable the find button and combobox
            if(read_only) {
                combobox->setEnabled(false);
                find_button->setEnabled(false);
            }
            else {
                connect(combobox, &QComboBox::currentTextChanged, this, &TagEditorEditWidget::on_change);
                connect(find_button, &QPushButton::clicked, this, &TagEditorEditWidget::find_dependency);
            }

            // Verify it's all there
            this->verify_dependency_path();
        }
        else if(value->get_type() == Parser::ParserStructValue::VALUE_TYPE_ENUM) {
            // Set up the enum
            auto *combobox = reinterpret_cast<QComboBox *>(this->widgets[0]);
            int comboWidth = standard_width * 8;
            combobox->setMaximumWidth(comboWidth);
            combobox->setMinimumWidth(comboWidth);

            // Block enum from being set if read only
            if(read_only) {
                combobox->setEnabled(false);
            }
            else {
                connect(combobox, &QComboBox::currentTextChanged, this, &TagEditorEditWidget::on_change);
            }
        }
        else if(value->get_type() == Parser::ParserStructValue::VALUE_TYPE_BITMASK) {
            // Set the list up
            auto *list = reinterpret_cast<QListWidget *>(this->widgets[0]);
            int listWidth = standard_width * 8;
            list->setMaximumWidth(listWidth);
            list->setMinimumWidth(listWidth);
            if(read_only) {
                list->setEnabled(false);
            }
            else {
                connect(list, &QListWidget::itemChanged, this, &TagEditorEditWidget::on_change);
            }
            list->setFocusPolicy(Qt::NoFocus);
            list->setSelectionMode(QAbstractItemView::NoSelection);
        }

        if(this->auxiliary_widget) {
            this->update_auxiliary_widget();
            if(this->auxiliary_checkbox) {
                connect(this->auxiliary_checkbox, &QCheckBox::stateChanged, this, &TagEditorEditWidget::update_auxiliary_widget);
            }
        }

        // Add any suffix if needed
        const char *unit = value->get_unit();
        if(unit == nullptr && value->get_type() == Parser::ParserStructValue::VALUE_TYPE_ANGLE) {
            unit = "degrees";
        }
        if(unit) {
            layout->addWidget(widgets_array.emplace_back(new QLabel(unit)));
        }

        title_label->setMinimumWidth(label_width);
        title_label->setMaximumWidth(title_label->minimumWidth());

        // Set this stuff for things that aren't read only
        if(!read_only) {
            for(auto *textbox_widget : this->textbox_widgets) {
                connect(textbox_widget, &QLineEdit::textEdited, this, &TagEditorEditWidget::on_change);
            }
        }

        if(value->get_type() != Parser::ParserStructValue::VALUE_TYPE_DEPENDENCY) {
            layout->addStretch(1);
        }
        layout->setMargin(6);
    }

    void TagEditorEditWidget::update_auxiliary_widget() {
        auto *value = this->get_struct_value();

        switch(value->get_type()) {
            case Parser::ParserStructValue::VALUE_TYPE_COLORARGBINT:
            case Parser::ParserStructValue::VALUE_TYPE_COLORARGB:
            case Parser::ParserStructValue::VALUE_TYPE_COLORRGB: {
                // Get our stuff
                auto *color_widget = dynamic_cast<QGraphicsView *>(this->auxiliary_widget);
                auto *scene = new QGraphicsScene();
                int width = color_widget->minimumWidth();
                int height = color_widget->minimumHeight();

                // Make the pixel stuff
                std::vector<std::uint32_t> colors(width * height);

                QPixmap map(width, height);
                auto value_count = value->get_value_count();
                std::vector<Parser::ParserStructValue::Number> numbers(value_count);
                value->get_values(numbers.data());

                // Make the pixel colors
                ColorPlatePixel pixel = { 255, 255, 255, 255 };
                switch(value->get_type()) {
                    case Parser::ParserStructValue::VALUE_TYPE_COLORARGBINT:
                        pixel.alpha = static_cast<int>(std::get<std::int64_t>(numbers[0]));
                        pixel.red =   static_cast<int>(std::get<std::int64_t>(numbers[1]));
                        pixel.green = static_cast<int>(std::get<std::int64_t>(numbers[2]));
                        pixel.blue =  static_cast<int>(std::get<std::int64_t>(numbers[3]));
                        break;
                    case Parser::ParserStructValue::VALUE_TYPE_COLORARGB:
                        pixel.alpha = static_cast<int>(std::get<double>(numbers[0]) * 255);
                        pixel.red =   static_cast<int>(std::get<double>(numbers[1]) * 255);
                        pixel.green = static_cast<int>(std::get<double>(numbers[2]) * 255);
                        pixel.blue =  static_cast<int>(std::get<double>(numbers[3]) * 255);
                        break;
                    case Parser::ParserStructValue::VALUE_TYPE_COLORRGB:
                        pixel.red =   static_cast<int>(std::get<double>(numbers[0]) * 255);
                        pixel.green = static_cast<int>(std::get<double>(numbers[1]) * 255);
                        pixel.blue =  static_cast<int>(std::get<double>(numbers[2]) * 255);
                        break;
                    default:
                        std::terminate();
                }

                if(this->auxiliary_checkbox->checkState() != Qt::Checked) {
                    pixel.alpha = 255;
                }

                ColorPlatePixel triangle_up = (ColorPlatePixel { 255, 255, 255, 255 }).alpha_blend(pixel);
                ColorPlatePixel triangle_down = (ColorPlatePixel { 0, 0, 0, 255 }).alpha_blend(pixel);
                std::uint32_t color_up = static_cast<std::uint32_t>(triangle_up.alpha) << 24 | static_cast<std::uint32_t>(triangle_up.red) << 16 | static_cast<std::uint32_t>(triangle_up.green) << 8 | triangle_up.blue;
                std::uint32_t color_down = static_cast<std::uint32_t>(triangle_down.alpha) << 24 | static_cast<std::uint32_t>(triangle_down.red) << 16 | static_cast<std::uint32_t>(triangle_down.green) << 8 | triangle_down.blue;

                // Copy the pixel colors
                for(int y = 0; y < height; y++) {
                    for(int x = 0; x < width; x++) {
                        colors[x + y * width] = (x > y) ? color_down : color_up;
                    }
                }

                // Convert!
                map.convertFromImage(QImage(reinterpret_cast<unsigned char *>(colors.data()), width, height, QImage::Format::Format_ARGB32));

                scene->addPixmap(map);
                color_widget->setScene(scene);
                break;
            }
            default:
                std::terminate();
        }
    }

    void TagEditorEditWidget::on_change() {
        auto *value = this->get_struct_value();

        // Don't worry about it
        if(this->read_only) {
            return;
        }

        switch(value->get_type()) {
            case Parser::ParserStructValue::ValueType::VALUE_TYPE_TAGSTRING:
                value->set_string(this->textbox_widgets[0]->text().toLatin1().data());
                break;

            case Parser::ParserStructValue::ValueType::VALUE_TYPE_DEPENDENCY: {
                // Get the dependency stuff
                auto class_int = HEK::extension_to_tag_class(reinterpret_cast<QComboBox *>(this->widgets[0])->currentText().toLower().toLatin1().data());
                auto path = reinterpret_cast<QLineEdit *>(this->widgets[1])->text();

                // Set the dependency
                auto &dependency = value->get_dependency();
                dependency.tag_class_int = class_int;
                dependency.path = Invader::File::preferred_path_to_halo_path(path.toStdString());

                this->verify_dependency_path();
                break;
            }

            case Parser::ParserStructValue::ValueType::VALUE_TYPE_ENUM: {
                // Write the combobox stuff
                auto *combobox = reinterpret_cast<QComboBox *>(this->widgets[0]);
                value->write_enum(combobox->property(INTERNAL_VALUE).toStringList()[combobox->currentIndex()].toLatin1().data());
                break;
            }

            case Parser::ParserStructValue::ValueType::VALUE_TYPE_BITMASK: {
                auto *list = reinterpret_cast<QListWidget *>(this->widgets[0]);
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

        if(this->auxiliary_widget) {
            this->update_auxiliary_widget();
        }

        if(this->array_widget) {
            this->array_widget->update_text();
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

        // Color based on if we found it or it's empty, or we didn't find it
        auto *textbox = textbox_widgets[0];
        if(found) {
            textbox->setStyleSheet("");
        }
        else {
            textbox->setStyleSheet("color: #FF0000");
        }

        // Set our button as enabled
        this->widgets[3]->setEnabled(found && preferred_path != "");
    }

    void TagEditorEditWidget::find_dependency() {
        TagTreeDialog dialog(this, this->get_editor_window()->get_parent_window(), this->get_struct_value()->get_allowed_classes());
        dialog.exec();
        auto &tag = dialog.get_tag();
        if(tag.has_value()) {
            auto &tag_val = tag.value();
            this->textbox_widgets[0]->setText(File::split_tag_class_extension(File::halo_path_to_preferred_path(tag_val.tag_path))->path.c_str());
            reinterpret_cast<QComboBox *>(this->widgets[0])->setCurrentText(HEK::tag_class_to_extension(tag_val.tag_class_int));
            this->on_change();
        }
    }

    void TagEditorEditWidget::open_dependency() {
        char path_to_open[1024];
        std::snprintf(path_to_open, sizeof(path_to_open), "%s.%s", this->textbox_widgets[0]->text().toLatin1().data(), reinterpret_cast<QComboBox *>(this->widgets[0])->currentText().toLower().toLatin1().data());
        this->get_editor_window()->get_parent_window()->open_tag(path_to_open, false);
    }
}
