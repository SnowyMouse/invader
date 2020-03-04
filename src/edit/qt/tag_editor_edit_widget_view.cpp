// SPDX-License-Identifier: GPL-3.0-only

#include "tag_editor_edit_widget_view.hpp"

namespace Invader::EditQt {
    TagEditorEditWidgetView::TagEditorEditWidgetView(QWidget *parent, const std::vector<Parser::ParserStructValue> &values, TagEditorWindow *editor_window, bool primary) : QFrame(parent), values(values), editor_window(editor_window) {
        auto *vbox_layout = new QVBoxLayout(this);
        this->setLayout(vbox_layout);

        for(auto &value : this->values) {
            auto *widget = TagEditorWidget::generate_widget(nullptr, &value, editor_window);
            if(widget) {
                vbox_layout->addWidget(widget);
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
