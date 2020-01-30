// SPDX-License-Identifier: GPL-3.0-only

#ifndef INVADER__EDIT__QT__TAG_EDITOR_TEXTBOX_WIDGET_HPP
#define INVADER__EDIT__QT__TAG_EDITOR_TEXTBOX_WIDGET_HPP

#include "tag_editor_abstract_widget.hpp"
#include <cstdint>

class QLineEdit;

namespace Invader::EditQt {
    class TagEditorTextboxWidget : public TagEditorAbstractWidget {
        Q_OBJECT
    public:
        enum TextboxSize {
            SMALL,
            MEDIUM,
            LARGE
        };

        /**
         * Instantiate a TagEditorTextboxWidget
         * @param parent         parent widget
         * @param text           element text
         * @param size           size of the textbox
         * @param count          number of textboxes
         * @param labels         labels to add
         */
        TagEditorTextboxWidget(QWidget *parent, const char *text, TextboxSize size, std::size_t count, const std::optional<std::vector<std::string>> &labels = std::nullopt);

        /**
         * Get the string value
         * @param  textbox textbox to get string from
         * @return         string
         */
        QString get_string(std::size_t textbox) const;

        /**
         * Get the float value
         * @param  textbox textbox to get float from
         * @param  success optional pointer to a boolean to indicate success or failure
         * @return         float
         */
        float get_float(std::size_t textbox, bool *success = nullptr) const;

        /**
         * Get the int value
         * @param  textbox textbox to get int from
         * @param  success optional pointer to a boolean to indicate success or failure
         * @return         int
         */
        std::int32_t get_int(std::size_t textbox, bool *success = nullptr) const;
    private:
        std::vector<QLineEdit *> text_boxes;
    };
}

#endif
