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
#include <QColorDialog>
#include <QStandardItem>
#include <QCheckBox>
#include <QGraphicsView>
#include <QGraphicsScene>
#include "tag_editor_edit_widget.hpp"
#include "../../tree/tag_tree_window.hpp"
#include "../../tree/tag_tree_dialog.hpp"
#include <invader/bitmap/pixel.hpp>
#include "tag_editor_array_widget.hpp"
#include <cassert>

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
    
    /**
     * Graphics view that opens a color picker when clicked
     */
    class ColorPickerGraphicsView : public QGraphicsView {
    public:
        ColorPickerGraphicsView(TagEditorEditWidget *parent) : QGraphicsView(parent), parent(parent) {}
    private:
        TagEditorEditWidget *parent;
        void mousePressEvent(QMouseEvent *) override {
            parent->activate_auxiliary_widget();
        }
    };
    
    static int compare_number(const Parser::ParserStructValue::Number &a, const Parser::ParserStructValue::Number &b) {
        auto compare_to = [&b](auto a) -> int {
            auto do_compare = [&a](auto b) -> int {
                if(a == b) {
                    return 0;
                }
                else if(a > b) {
                    return 1;
                }
                else {
                    return -1;
                }
            };
            
            if(auto *b_int = std::get_if<std::int64_t>(&b)) {
                return do_compare(*b_int);
            }
            else {
                return do_compare(std::get<double>(b));
            }
        };
        
        if(auto *a_int = std::get_if<std::int64_t>(&a)) {
            return compare_to(*a_int);
        }
        else {
            return compare_to(std::get<double>(a));
        }
    }
    
    static void set_error_level_textbox(QLineEdit *textbox, int error);

    TagEditorEditWidget::TagEditorEditWidget(QWidget *parent, Parser::ParserStructValue *value, TagEditorWindow *editor_window, TagEditorArrayWidget *array_widget) :
        TagEditorWidget(parent, value, editor_window), array_widget(array_widget) {

        auto *this_widget = this;
        this->setToolTip(QString(value->get_comment()).replace("\n","\n\n"));
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

        auto add_widget = [&value_index, &value, &widgets_array, &layout, &values, &label_width, &textbox_widgets, &standard_width, &prefix_label_width, &read_only, &auxiliary_widget, &auxiliary_checkbox, &this_widget]() {
            auto add_simple_textbox = [&widgets_array, &standard_width, &textbox_widgets](int size, QLayout *layout) -> QLineEdit * {
                // Make our textbox
                auto *textbox = reinterpret_cast<QLineEdit *>(widgets_array.emplace_back(new QLineEdit()));
                int width = standard_width * size;
                textbox_widgets.emplace_back(textbox);
                textbox->setMinimumWidth(width);
                layout->addWidget(textbox);
                return textbox;
            };
            
            auto add_single_textbox = [&value, &value_index, &widgets_array, &values, &label_width, &prefix_label_width, &read_only, &add_simple_textbox](int size, QLayout *layout, const char *prefix = nullptr) -> QLineEdit * {
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
                
                // Make our textbox
                auto *textbox = add_simple_textbox(size, layout);
                
                // Get the minimum and maximum
                auto min = value->get_minimum();
                auto max = value->get_maximum();
                
                assert(value_index < values.size());
                
                auto &current_value = values[value_index];
                set_error_level_textbox(textbox, ((min.has_value() && compare_number(current_value, min.value()) < 0) || (max.has_value() && compare_number(current_value, max.value()) > 0)) ? 2 : 0);

                auto value_type = value->get_type();
                if(value->get_number_format() == Parser::ParserStructValue::NumberFormat::NUMBER_FORMAT_FLOAT) {
                    // Radians get converted to degrees
                    switch(value_type) {
                        case Parser::ParserStructValue::VALUE_TYPE_ANGLE:
                        case Parser::ParserStructValue::VALUE_TYPE_EULER2D:
                        case Parser::ParserStructValue::VALUE_TYPE_EULER3D:
                            textbox->setText(QString::number(RADIANS_TO_DEGREES(std::get<double>(current_value))));
                            break;
                        default:
                            textbox->setText(QString::number(std::get<double>(current_value)));
                            break;
                    }
                }
                // Indices can be blank (null)
                else if(value->get_type() == Parser::ParserStructValue::VALUE_TYPE_INDEX) {
                    if(std::get<std::int64_t>(current_value) != NULL_INDEX) {
                        textbox->setText(QString::number(std::get<std::int64_t>(current_value)));
                    }
                    textbox->setPlaceholderText("NULL");
                }
                else if(value->get_number_format() == Parser::ParserStructValue::NumberFormat::NUMBER_FORMAT_INT) {
                    textbox->setText(QString::number(std::get<std::int64_t>(current_value)));
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
            auto make_color_widget = [&auxiliary_widget, &layout, &auxiliary_checkbox, &this_widget]() {
                auto *new_auxiliary_widget = new ColorPickerGraphicsView(this_widget);
                new_auxiliary_widget->setMaximumHeight(QLineEdit().minimumSizeHint().height());
                new_auxiliary_widget->setMaximumWidth(new_auxiliary_widget->maximumHeight());
                new_auxiliary_widget->setMinimumHeight(new_auxiliary_widget->maximumHeight());
                new_auxiliary_widget->setMinimumWidth(new_auxiliary_widget->maximumWidth());
                new_auxiliary_widget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
                new_auxiliary_widget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
                auxiliary_widget = new_auxiliary_widget;
                layout->addWidget(auxiliary_widget);

                auxiliary_checkbox = new QCheckBox("Preview alpha");
                layout->addWidget(auxiliary_checkbox);
            };

            switch(value->get_type()) {
                // Simple value types; just textboxes
                case Parser::ParserStructValue::VALUE_TYPE_INT8:
                case Parser::ParserStructValue::VALUE_TYPE_UINT8:
                    add_single_textbox(1, layout);
                    break;

                case Parser::ParserStructValue::VALUE_TYPE_INT16:
                case Parser::ParserStructValue::VALUE_TYPE_UINT16:
                    add_single_textbox(2, layout);
                    break;

                case Parser::ParserStructValue::VALUE_TYPE_INDEX:
                    add_single_textbox(2, layout);
                    break;

                case Parser::ParserStructValue::VALUE_TYPE_INT32:
                case Parser::ParserStructValue::VALUE_TYPE_UINT32:
                case Parser::ParserStructValue::VALUE_TYPE_FLOAT:
                case Parser::ParserStructValue::VALUE_TYPE_FRACTION:
                case Parser::ParserStructValue::VALUE_TYPE_ANGLE:
                    add_single_textbox(3, layout);
                    break;

                case Parser::ParserStructValue::VALUE_TYPE_TAGSTRING:
                    add_simple_textbox(8, layout);
                    break;

                // Some more complex stuff with multiple boxes
                case Parser::ParserStructValue::VALUE_TYPE_COLORARGBINT:
                    add_single_textbox(1, layout, "a:");
                    add_single_textbox(1, layout, "r:");
                    add_single_textbox(1, layout, "g:");
                    add_single_textbox(1, layout, "b:");
                    make_color_widget();
                    break;
                case Parser::ParserStructValue::VALUE_TYPE_POINT2DINT:
                    add_single_textbox(2, layout, "x:");
                    add_single_textbox(2, layout, "y:");
                    break;
                case Parser::ParserStructValue::VALUE_TYPE_RECTANGLE2D:
                    add_single_textbox(2, layout, "t:");
                    add_single_textbox(2, layout, "r:");
                    add_single_textbox(2, layout, "b:");
                    add_single_textbox(2, layout, "l:");
                    break;
                case Parser::ParserStructValue::VALUE_TYPE_COLORARGB:
                    add_single_textbox(3, layout, "a:");
                    add_single_textbox(3, layout, "r:");
                    add_single_textbox(3, layout, "g:");
                    add_single_textbox(3, layout, "b:");
                    make_color_widget();
                    break;
                case Parser::ParserStructValue::VALUE_TYPE_COLORRGB:
                    add_single_textbox(3, layout, "r:");
                    add_single_textbox(3, layout, "g:");
                    add_single_textbox(3, layout, "b:");
                    make_color_widget();
                    auxiliary_checkbox->setHidden(true);
                    break;
                case Parser::ParserStructValue::VALUE_TYPE_VECTOR2D:
                    add_single_textbox(3, layout, "i:");
                    add_single_textbox(3, layout, "j:");
                    break;
                case Parser::ParserStructValue::VALUE_TYPE_VECTOR3D:
                    add_single_textbox(3, layout, "i:");
                    add_single_textbox(3, layout, "j:");
                    add_single_textbox(3, layout, "k:");
                    break;
                case Parser::ParserStructValue::VALUE_TYPE_EULER2D:
                    add_single_textbox(3, layout, "y:");
                    add_single_textbox(3, layout, "p:");
                    break;
                case Parser::ParserStructValue::VALUE_TYPE_EULER3D:
                    add_single_textbox(3, layout, "y:");
                    add_single_textbox(3, layout, "p:");
                    add_single_textbox(3, layout, "r:");
                    break;
                case Parser::ParserStructValue::VALUE_TYPE_PLANE2D:
                    add_single_textbox(3, layout, "x:");
                    add_single_textbox(3, layout, "y:");
                    add_single_textbox(3, layout, "w:");
                    break;
                case Parser::ParserStructValue::VALUE_TYPE_POINT2D:
                    add_single_textbox(3, layout, "x:");
                    add_single_textbox(3, layout, "y:");
                    break;
                case Parser::ParserStructValue::VALUE_TYPE_POINT3D:
                    add_single_textbox(3, layout, "x:");
                    add_single_textbox(3, layout, "y:");
                    add_single_textbox(3, layout, "z:");
                    break;
                case Parser::ParserStructValue::VALUE_TYPE_PLANE3D:
                    add_single_textbox(3, layout, "x:");
                    add_single_textbox(3, layout, "y:");
                    add_single_textbox(3, layout, "z:");
                    add_single_textbox(3, layout, "w:");
                    break;

                case Parser::ParserStructValue::VALUE_TYPE_QUATERNION:
                    add_single_textbox(3, layout, "i:");
                    add_single_textbox(3, layout, "j:");
                    add_single_textbox(3, layout, "k:");
                    add_single_textbox(3, layout, "w:");
                    break;
                case Parser::ParserStructValue::VALUE_TYPE_MATRIX: {
                    // Set up layouts
                    auto *top = new QWidget();
                    auto *top_layout = new QHBoxLayout();
                    top->setLayout(top_layout);
                    auto *mid = new QWidget();
                    auto *mid_layout = new QHBoxLayout();
                    mid->setLayout(mid_layout);
                    auto *bottom = new QWidget();
                    auto *bottom_layout = new QHBoxLayout();
                    bottom->setLayout(bottom_layout);
                    auto *rows = new QWidget();
                    auto *row_layout = new QVBoxLayout();
                    rows->setLayout(row_layout);
                    row_layout->addWidget(top);
                    row_layout->addWidget(mid);
                    row_layout->addWidget(bottom);
                    layout->addWidget(rows);
                    
                    // Set up widgets
                    add_single_textbox(3, top_layout,    "x<sub><strong>1</strong></sub>:");
                    add_single_textbox(3, top_layout,    "y<sub><strong>1</strong></sub>:");
                    add_single_textbox(3, top_layout,    "z<sub><strong>1</strong></sub>:");
                    add_single_textbox(3, mid_layout,    "x<sub><strong>2</strong></sub>:");
                    add_single_textbox(3, mid_layout,    "y<sub><strong>2</strong></sub>:");
                    add_single_textbox(3, mid_layout,    "z<sub><strong>2</strong></sub>:");
                    add_single_textbox(3, bottom_layout, "x<sub><strong>3</strong></sub>:");
                    add_single_textbox(3, bottom_layout, "y<sub><strong>3</strong></sub>:");
                    add_single_textbox(3, bottom_layout, "z<sub><strong>3</strong></sub>:");
                    break;
                }

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
                            model->appendRow(new QStandardItem(HEK::tag_fourcc_to_extension(allowed_classes[i])));
                        }
                    }
                    else {
                        auto all_classes = Parser::ParserStruct::all_tag_classes(true);
                        for(auto c : all_classes) {
                            model->appendRow(new QStandardItem(HEK::tag_fourcc_to_extension(c)));
                        }
                    }
                    combobox->setModel(model);

                    layout->addWidget(combobox);

                    // Next, the textbox
                    auto *textbox = add_simple_textbox(8, layout);

                    // Next, get our thing
                    auto &dependency = value->get_dependency();

                    // Lastly, set the dependency
                    textbox->setText(Invader::File::halo_path_to_preferred_path(dependency.path).c_str());
                    combobox->setCurrentText(HEK::tag_fourcc_to_extension(dependency.tag_fourcc));

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
                case Parser::ParserStructValue::VALUE_TYPE_TAGID:
                case Parser::ParserStructValue::VALUE_TYPE_GROUP_START:
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
        if(unit == nullptr) {
            switch(value->get_type()) {
                case Parser::ParserStructValue::VALUE_TYPE_ANGLE:
                case Parser::ParserStructValue::VALUE_TYPE_EULER2D:
                case Parser::ParserStructValue::VALUE_TYPE_EULER3D:
                    unit = "degrees";
                    break;
                default:
                    break;
            }
        }
        if(unit) {
            layout->addWidget(widgets_array.emplace_back(new QLabel(unit)));
        }
        
        // Show the range if needed
        auto min = value->get_minimum();
        auto max = value->get_maximum();
        if(min.has_value() || max.has_value()) {
            // Get the min/max text
            auto make_text = [](const std::optional<Parser::ParserStructValue::Number> &number) -> QString {
                if(number.has_value()) {
                    // If it's an int, get this
                    if(auto *get_int = std::get_if<std::int64_t>(&number.value())) {
                        return QString::number(*get_int);
                    }
                    // Otherwise it's a float
                    else {
                        return QString::number(std::get<double>(number.value()));
                    }
                }
                return QString();
            };
            QString min_text = make_text(min);
            QString max_text = make_text(max);  
            
            // Format the text
            char min_max_text[256] = {};
            if(min.has_value() && !max.has_value()) {
                std::snprintf(min_max_text, sizeof(min_max_text), "[minimum: %s]", min_text.toLatin1().data());
            }
            else if(!min.has_value() && max.has_value()) {
                std::snprintf(min_max_text, sizeof(min_max_text), "[maximum: %s]", max_text.toLatin1().data());
            }
            else {
                std::snprintf(min_max_text, sizeof(min_max_text), "[%s - %s]", min_text.toLatin1().data(), max_text.toLatin1().data());
            }
            
            // Make the label
            layout->addWidget(reinterpret_cast<QLabel *>(widgets_array.emplace_back(new QLabel(min_max_text))));
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
        layout->setContentsMargins(6, 6, 6, 6);
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
                Pixel pixel = { 255, 255, 255, 255 };
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

                Pixel triangle_up = (Pixel { 255, 255, 255, 255 }).alpha_blend(pixel);
                Pixel triangle_down = (Pixel { 0, 0, 0, 255 }).alpha_blend(pixel);
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
        auto value_type = value->get_type();

        // Don't worry about it
        if(this->read_only) {
            return;
        }

        switch(value_type) {
            case Parser::ParserStructValue::ValueType::VALUE_TYPE_TAGSTRING:
                value->set_string(this->textbox_widgets[0]->text().toLatin1().data());
                break;

            case Parser::ParserStructValue::ValueType::VALUE_TYPE_DEPENDENCY: {
                // Get the dependency stuff
                auto fourcc = HEK::tag_extension_to_fourcc(reinterpret_cast<QComboBox *>(this->widgets[0])->currentText().toLower().toLatin1().data());
                auto path = reinterpret_cast<QLineEdit *>(this->widgets[1])->text();

                // Set the dependency
                auto &dependency = value->get_dependency();
                dependency.tag_fourcc = fourcc;
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
                bool can_set = true;
                
                // Get the minimum and maximum
                auto min = value->get_minimum();
                auto max = value->get_maximum();
                auto number_format = value->get_number_format();
                
                if(number_format == Parser::ParserStructValue::NumberFormat::NUMBER_FORMAT_INT) {
                    std::int64_t actual_min = 0, actual_max;
                    
                    // Get the theoretical mins and maxes
                    switch(value_type) {
                        case Parser::ParserStructValue::VALUE_TYPE_INT8:
                            actual_min = INT8_MIN;
                            actual_max = INT8_MAX;
                            break;
                            
                        case Parser::ParserStructValue::VALUE_TYPE_UINT8:
                            actual_max = UINT8_MAX;
                            break;

                        case Parser::ParserStructValue::VALUE_TYPE_INT16:
                            actual_min = INT16_MIN;
                            actual_max = INT16_MAX;
                            break;
                            
                        case Parser::ParserStructValue::VALUE_TYPE_UINT16:
                        case Parser::ParserStructValue::VALUE_TYPE_INDEX:
                            actual_max = UINT16_MAX;
                            break;

                        case Parser::ParserStructValue::VALUE_TYPE_INT32:
                            actual_min = INT32_MIN;
                            actual_max = INT32_MAX;
                            break;
                            
                        case Parser::ParserStructValue::VALUE_TYPE_UINT32:
                            actual_max = UINT32_MAX;
                            break;
                            
                        case Parser::ParserStructValue::VALUE_TYPE_RECTANGLE2D:
                        case Parser::ParserStructValue::VALUE_TYPE_POINT2DINT:
                            actual_max = INT16_MAX;
                            actual_min = INT16_MIN;
                            break;
                            
                        case Parser::ParserStructValue::VALUE_TYPE_COLORARGBINT:
                            actual_max = UINT8_MAX;
                            actual_min = 0;
                            break;
                            
                        default:
                            std::terminate();
                    }
                    
                    if(!min.has_value()) {
                        min = actual_min;
                    }
                    
                    if(!max.has_value()) {
                        max = actual_max;
                    }
                }

                // Go through each number
                for(std::size_t w = 0; w < widget_count; w++) {
                    auto *widget = this->textbox_widgets[w];
                    bool ok = false;
                    
                    // Set the value based on bass
                    auto &number = numbers[w];
                    switch(number_format) {
                        case Parser::ParserStructValue::NumberFormat::NUMBER_FORMAT_FLOAT: {
                            double double_value = widget->text().toDouble(&ok);
                            
                            // Modify the value if we're inputting into something that takes degrees but stores as radians
                            switch(value_type) {
                                case Parser::ParserStructValue::VALUE_TYPE_EULER2D:
                                case Parser::ParserStructValue::VALUE_TYPE_EULER3D:
                                case Parser::ParserStructValue::ValueType::VALUE_TYPE_ANGLE:
                                    double_value = DEGREES_TO_RADIANS(double_value);
                                    break;
                                default:
                                    break;
                            }
                            number = double_value;
                            break;
                        }
                        
                        case Parser::ParserStructValue::NumberFormat::NUMBER_FORMAT_INT:
                            if(value_type == Parser::ParserStructValue::ValueType::VALUE_TYPE_INDEX && (widget->text().isEmpty() || widget->text().toLower() == "null")) {
                                number = static_cast<std::int64_t>(NULL_INDEX);
                                ok = true;
                            }
                            else {
                                number = static_cast<std::int64_t>(widget->text().toLongLong(&ok));
                                if(!ok && widget->text().startsWith("0x")) {
                                    number = static_cast<std::int64_t>(widget->text().toLongLong(&ok, 16)); // try hexadecimal
                                }
                            }
                            break;
                        default:
                            // What???
                            std::terminate();
                    }
                    
                    // Make sure we're within range
                    ok = ok && !(min.has_value() && compare_number(number, min.value()) < 0);
                    ok = ok && !(max.has_value() && compare_number(number, max.value()) > 0);
                    set_error_level_textbox(widget, ok ? 0 : 2);
                    
                    // If it's NOT ok, then we shall not save anything.
                    can_set = can_set && ok;
                }
                
                // If anything we entered was invalid, don't set. Otherwise, set.
                if(can_set) {
                    value->set_values(numbers);
                }
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
        
        // If fast listing mode is disabled, check if it exists
        else if(!this->get_editor_window()->get_parent_window()->fast_listing_mode()) {
            // First pass: Check to see if the exact path exists
            for(auto &t : this->get_editor_window()->get_parent_window()->get_all_tags()) {
                if(t.tag_fourcc == dependency.tag_fourcc && File::split_tag_class_extension(File::halo_path_to_preferred_path(t.tag_path)).value().path == preferred_path) {
                    found = true;
                    break;
                }
            }
            
            // Second pass: Check to see if it's an actual, explicit path
            if(!found) {
                auto *textbox = this->textbox_widgets[0];
                    
                // Are we at the end of the textbox?
                if(textbox->cursorPosition() == textbox->text().size()) {
                    auto split_path = File::split_tag_class_extension(preferred_path);
                    if(split_path.has_value()) {
                        auto fourcc = split_path->fourcc;
                        bool is_allowed = false;
                        for(auto &ac : this->get_struct_value()->get_allowed_classes()) {
                            if(ac == fourcc) {
                                is_allowed = true;
                                break;
                            }
                        }
                        
                        // If we got a path, see if we can find it again
                        if(is_allowed) {
                            // First, Jason Jones the path in, accounting for the extension
                            dependency.path = split_path->path;
                            dependency.tag_fourcc = split_path->fourcc;
                            textbox->setText(split_path->path.c_str());
                            textbox->setCursorPosition(textbox->text().size());
                            reinterpret_cast<QComboBox *>(this->widgets[0])->setCurrentText(HEK::tag_fourcc_to_extension(split_path->fourcc));
                            
                            // Then look for it
                            for(auto &t : this->get_editor_window()->get_parent_window()->get_all_tags()) {
                                if(t.tag_fourcc == fourcc && File::split_tag_class_extension(File::halo_path_to_preferred_path(t.tag_path)).value().path == split_path->path) {
                                    found = true;
                                    break;
                                }
                            }
                        }
                    }
                }
            }
        }
        
        // If fast listing mode is enabled, query the filesystem
        else {
            auto path_with_extension = preferred_path + "." + HEK::tag_fourcc_to_extension(dependency.tag_fourcc);
            for(auto &i : this->get_editor_window()->get_parent_window()->get_tag_directories()) {
                if(std::filesystem::exists(i / path_with_extension)) {
                    found = true;
                    break;
                }
            }
        }

        // Color based on if we found it or it's empty, or we didn't find it
        set_error_level_textbox(textbox_widgets[0], found ? 0 : 1);

        // Set our button as enabled
        this->widgets[3]->setEnabled(found && preferred_path != "");
    }

    void TagEditorEditWidget::find_dependency() {
        TagTreeDialog dialog(nullptr, this->get_editor_window()->get_parent_window(), this->get_struct_value()->get_allowed_classes(), std::filesystem::path(this->get_editor_window()->get_file().tag_path).parent_path().string().c_str());
        dialog.exec();
        auto &tag = dialog.get_tag();
        if(tag.has_value()) {
            auto &tag_val = tag.value();
            this->textbox_widgets[0]->setText(File::split_tag_class_extension(File::halo_path_to_preferred_path(tag_val.tag_path))->path.c_str());
            reinterpret_cast<QComboBox *>(this->widgets[0])->setCurrentText(HEK::tag_fourcc_to_extension(tag_val.tag_fourcc));
            this->on_change();
        }
    }

    void TagEditorEditWidget::open_dependency() {
        char path_to_open[1024];
        std::snprintf(path_to_open, sizeof(path_to_open), "%s.%s", this->textbox_widgets[0]->text().toLatin1().data(), reinterpret_cast<QComboBox *>(this->widgets[0])->currentText().toLower().toLatin1().data());
        this->get_editor_window()->get_parent_window()->open_tag(path_to_open, false);
    }
    
    static void set_error_level_textbox(QLineEdit *textbox, int error) {
        if(error == 0) {
            textbox->setStyleSheet("");
        }
        else if(error == 1) {
            textbox->setStyleSheet("color: #FF7F00");
        }
        else if(error == 2) {
            textbox->setStyleSheet("color: #FF0000");
        }
    }
    
    void TagEditorEditWidget::activate_auxiliary_widget() {
        TagEditorEditWidget *this_widget = this;
        auto show_color = [&this_widget](bool use_int, bool use_alpha) {
            QColorDialog dialog;
            dialog.setOption(QColorDialog::ColorDialogOption::ShowAlphaChannel, use_alpha);
            Parser::ParserStructValue::Number numbers[4];
            this_widget->get_struct_value()->get_values(numbers);
            
            // Set the default color to our current color
            QColor color;
            if(use_int) {
                color.setAlpha(std::get<std::int64_t>(numbers[0]));
                color.setRed(std::get<std::int64_t>(numbers[1]));
                color.setGreen(std::get<std::int64_t>(numbers[2]));
                color.setBlue(std::get<std::int64_t>(numbers[3]));
            }
            else if(use_alpha) {
                color.setAlphaF(std::get<double>(numbers[0]));
                color.setRedF(std::get<double>(numbers[1]));
                color.setGreenF(std::get<double>(numbers[2]));
                color.setBlueF(std::get<double>(numbers[3]));
            }
            else {
                color.setRedF(std::get<double>(numbers[0]));
                color.setGreenF(std::get<double>(numbers[1]));
                color.setBlueF(std::get<double>(numbers[2]));
            }
            dialog.setCurrentColor(color);
            
            // Open the dialog and cancel if the user cancelled
            if(dialog.exec() == QDialog::Rejected) {
                return;
            }
            
            // When we're done, get the new color
            color = dialog.selectedColor();
            if(use_int) {
                this_widget->textbox_widgets[0]->setText(QString::number(color.alpha()));
                this_widget->textbox_widgets[1]->setText(QString::number(color.red()));
                this_widget->textbox_widgets[2]->setText(QString::number(color.green()));
                this_widget->textbox_widgets[3]->setText(QString::number(color.blue()));
            }
            else if(use_alpha) {
                this_widget->textbox_widgets[0]->setText(QString::number(color.alphaF()));
                this_widget->textbox_widgets[1]->setText(QString::number(color.redF()));
                this_widget->textbox_widgets[2]->setText(QString::number(color.greenF()));
                this_widget->textbox_widgets[3]->setText(QString::number(color.blueF()));
            }
            else {
                this_widget->textbox_widgets[0]->setText(QString::number(color.redF()));
                this_widget->textbox_widgets[1]->setText(QString::number(color.greenF()));
                this_widget->textbox_widgets[2]->setText(QString::number(color.blueF()));
            }
            
            this_widget->on_change();
        };
        
        switch(this->get_struct_value()->get_type()) {
            case Parser::ParserStructValue::VALUE_TYPE_COLORARGB:
                show_color(false, true);
                break;
            case Parser::ParserStructValue::VALUE_TYPE_COLORARGBINT:
                show_color(true, true);
                break;
            case Parser::ParserStructValue::VALUE_TYPE_COLORRGB:
                show_color(false, false);
                break;
            default:
                eprintf_error("Unknown type for aux widget");
                std::terminate();
        }
        QColorDialog dialog;
        
    }
}
