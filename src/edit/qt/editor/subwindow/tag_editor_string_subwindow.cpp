// SPDX-License-Identifier: GPL-3.0-only

#include <QVBoxLayout>
#include <QPlainTextEdit>
#include <QComboBox>
#include <QLabel>

#include <invader/tag/parser/parser.hpp>
#include "tag_editor_string_subwindow.hpp"
#include "../tag_editor_window.hpp"

namespace Invader::EditQt {
    void TagEditorStringSubwindow::update() {
        auto *data = this->get_parent_window()->get_parser_data();
        auto &that = *this;
        auto &combo_box = this->combo_box;
        
        // Set the list stuff
        auto set_lists = [&combo_box, &that](auto &lists, auto *list) {
            lists.clear();
            combo_box->blockSignals(true);
            combo_box->clear();
            std::size_t strcount = list->strings.size();
            for(std::size_t i = 0; i < strcount; i++) {
                auto *data = lists.emplace_back(&list->strings[i].string);
                combo_box->addItem(QString::number(i) + ". " + that.decode_string(data, true));
            }
            combo_box->blockSignals(false);
            that.refresh_string();
        };
        
        // Depending on if we're unicode or regular string list, set up the data
        auto *strlist = dynamic_cast<Parser::StringList *>(data);
        if(strlist) {
            this->utf16 = false;
            set_lists(this->lists, strlist);
            this->refresh_string();
            return;
        }
        
        auto *ustrlist = dynamic_cast<Parser::UnicodeStringList *>(data);
        if(ustrlist) {
            this->utf16 = true;
            set_lists(this->lists, ustrlist);
            this->refresh_string();
            return;
        }
        
        std::terminate();
    }

    TagEditorStringSubwindow::TagEditorStringSubwindow(TagEditorWindow *parent) : TagEditorSubwindow(parent) {
        // Set up the rest of the layout (add our header and a text view)
        auto *vlayout = new QVBoxLayout();
        vlayout->addWidget((this->combo_box = new QComboBox()));
        vlayout->addWidget((this->text_view = new QPlainTextEdit()));
        this->text_view->setReadOnly(true);
        connect(this->combo_box, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &TagEditorStringSubwindow::refresh_string);
        
        // Set it as our central widget
        auto *vwidget = new QWidget();
        vwidget->setLayout(vlayout);
        this->setCentralWidget(vwidget);
        
        // Center the window
        this->center_window();
        
        // Update!
        this->update();
    }
    
    QString TagEditorStringSubwindow::decode_string(const std::vector<std::byte> *data, bool first_line_only) {
        // Check if it's zero bytes in length, incorrect length, or not null terminated
        if(data->size() == 0 || (this->utf16 && ((data->size() % sizeof(char16_t)) != 0 || reinterpret_cast<const char16_t *>(data->data() + data->size())[-1] != 0))) {
            return QString("ERROR ??????????");
        }
        
        if(first_line_only) {
            return this->decode_string(data, false).split("\r\n")[0];
        }
        
        if(this->utf16) {
            return QString::fromUtf16(reinterpret_cast<const char16_t *>(data->data()), data->size() / sizeof(char16_t) - 1);
        }
        else {
            return QString::fromLatin1(reinterpret_cast<const char *>(data->data()), data->size() / sizeof(char) - 1);
        }
    }
    
    void TagEditorStringSubwindow::refresh_string() {
        int index = this->combo_box->currentIndex();
        if(index == -1) {
            this->text_view->setEnabled(false);
            this->text_view->clear();
        }
        else {
            this->text_view->setEnabled(true);
            this->text_view->setPlainText(this->decode_string(this->lists[index], false));
        }
    }
}
