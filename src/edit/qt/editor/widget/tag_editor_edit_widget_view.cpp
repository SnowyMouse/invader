// SPDX-License-Identifier: GPL-3.0-only

#include "tag_editor_edit_widget_view.hpp"
#include "tag_editor_array_widget.hpp"
#include <QApplication>

namespace Invader::EditQt {
    int TagEditorEditWidgetView::y_for_item(const char *item) const noexcept {
        for(auto *i : this->widgets) {
            auto *widget = dynamic_cast<TagEditorWidget *>(i);
            if(widget) {
                if(std::strcmp(item,widget->get_struct_value()->get_name()) == 0) {
                    return widget->geometry().y();
                }
            }
        }
        
        return -1;
    }
    
    TagEditorEditWidgetView::TagEditorEditWidgetView(QWidget *parent, const std::vector<Parser::ParserStructValue> &values, TagEditorWindow *editor_window, bool primary, QWidget *extra_widget) : QFrame(parent), values(values), editor_window(editor_window) {
        auto *vbox_layout = new QVBoxLayout();
        vbox_layout->setMargin(0);
        vbox_layout->setSpacing(0);

        // Set the layout
        this->setLayout(vbox_layout);

        bool alternate = false;
        auto &widgets = this->widgets;

        auto add_widget = [&alternate, &vbox_layout, &widgets](QWidget *widget) {
            QPalette palette;
            widget->setAutoFillBackground(true);

            if(alternate) {
                palette.setColor(QPalette::Window, QApplication::palette().color(QPalette::Midlight));
            }
            else {
                palette.setColor(QPalette::Window, QApplication::palette().color(QPalette::Window));
            }

            widget->setPalette(palette);
            vbox_layout->addWidget(widget);
            widgets.emplace_back(widget);

            alternate = !alternate;
        };

        if(extra_widget) {
            add_widget(extra_widget);
        }

        for(auto &value : this->values) {
            auto *widget = TagEditorWidget::generate_widget(nullptr, &value, editor_window, dynamic_cast<TagEditorArrayWidget *>(parent));
            if(widget) {
                add_widget(widget);
            }
        }

        // Add a spacer so it doesn't try to evenly space everything if we're too big
        if(primary) {
            vbox_layout->addStretch(1);
        }
        else {
            this->setFrameStyle(QFrame::Panel | QFrame::Raised);
            this->setLineWidth(2);
        }
    }
}
