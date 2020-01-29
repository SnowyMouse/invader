// SPDX-License-Identifier: GPL-3.0-only

#include "tag_tree_dialog.hpp"
#include "tag_tree_widget.hpp"
#include "tag_tree_window.hpp"

#include <invader/file/file.hpp>
#include <QFileIconProvider>
#include <invader/printf.hpp>
#include <QHeaderView>
#include <QMessageBox>
#include <QVBoxLayout>

namespace Invader::EditQt {
    TagTreeDialog::TagTreeDialog(QWidget *parent, TagTreeWindow *parent_window, const std::optional<std::vector<HEK::TagClassInt>> &classes) : QDialog(parent) {
        // Make a layout and set our flags
        auto *vbox_layout = new QVBoxLayout(this);
        this->setWindowFlags(Qt::Dialog | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);

        // Add the widget
        this->tree_widget = new TagTreeWidget(this, parent_window, classes);
        this->tree_widget->setMinimumSize(640, 480);
        vbox_layout->addWidget(this->tree_widget);

        // Set our layout
        this->setLayout(vbox_layout);
        this->change_title(classes);
    }

    const std::optional<TagFile> &TagTreeDialog::get_tag() const noexcept {
        return this->tag;
    }

    void TagTreeDialog::set_filter(const std::optional<std::vector<HEK::TagClassInt>> &classes) {
        this->tree_widget->set_filter(classes);
        this->change_title(classes);
    }

    void TagTreeDialog::change_title(const std::optional<std::vector<HEK::TagClassInt>> &classes) {
        std::size_t class_count = classes.has_value() ? classes->size() : 0;

        // If we have classes filtered, format them into a list
        if(class_count) {
            char title_text[512];
            auto &classes_v = *classes;

            std::size_t pos = std::snprintf(title_text, sizeof(title_text), "Select a");
            std::size_t i = 0;
            while(pos < sizeof(title_text) && i < class_count) {
                pos += std::snprintf(title_text + pos, sizeof(title_text) - pos, " %s", HEK::tag_class_to_extension(classes_v[i]));

                if(pos + 3 < sizeof(title_text) && i + 2 <= class_count && class_count > 2) {
                    pos += std::snprintf(title_text + pos, sizeof(title_text) - pos, ",");
                }

                if(pos + 4 < sizeof(title_text) && i + 2 == class_count) {
                    pos += std::snprintf(title_text + pos, sizeof(title_text) - pos, " or");
                }

                i++;
            }

            if(pos + 5 < sizeof(title_text)) {
                pos += std::snprintf(title_text + pos, sizeof(title_text) - pos, " tag");
            }

            this->setWindowTitle(title_text);
        }
        else {
            this->setWindowTitle("Select a tag");
        }
    }
}
