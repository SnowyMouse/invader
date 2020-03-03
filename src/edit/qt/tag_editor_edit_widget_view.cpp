// SPDX-License-Identifier: GPL-3.0-only

#include "tag_editor_edit_widget_view.hpp"

namespace Invader::EditQt {
    TagEditorEditWidgetView::TagEditorEditWidgetView(QWidget *parent, const std::vector<Parser::ParserStructValue> &values, TagEditorWindow *editor_window, bool primary) : QWidget(parent), values(values), editor_window(editor_window) {
        this->setLayout(&this->vbox_layout);

        for(auto &value : this->values) {
            auto *widget = TagEditorWidget::generate_widget(this, &value, editor_window);
            if(widget) {
                widgets_to_remove.emplace_back(widget);
                this->vbox_layout.addWidget(widget);
            }
        }

        // Add a spacer so it doesn't try to evenly space everything if we're too big
        if(primary) {
            auto *spacer = new QSpacerItem(0 ,0);
            this->vbox_layout.addSpacerItem(spacer);
        }
    }
}
