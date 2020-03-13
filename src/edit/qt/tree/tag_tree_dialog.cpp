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
#include <QHBoxLayout>
#include <QComboBox>
#include <QLineEdit>
#include <QPushButton>
#include <QAction>
#include <QInputDialog>

namespace Invader::EditQt {
    TagTreeDialog::TagTreeDialog(QWidget *parent, TagTreeWindow *parent_window, const std::optional<std::vector<HEK::TagClassInt>> &classes, std::optional<HEK::TagClassInt> save_class) : QDialog(parent), save_class(save_class) {
        // Make a layout and set our flags
        auto *vbox_layout = new QVBoxLayout();
        vbox_layout->setMargin(4);
        this->setWindowFlags(Qt::Dialog | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);

        // Handle refreshing
        auto *refresh = new QAction();
        refresh->setShortcut(QKeySequence::Refresh);
        connect(refresh, &QAction::triggered, parent_window, &TagTreeWindow::refresh_view);
        this->addAction(refresh);

        // Add the widget
        if(save_class.has_value()) {
            // Add tag paths
            this->tag_paths = new QComboBox();
            auto &tag_tree_directories = parent_window->get_tag_directories();
            for(auto &p : tag_tree_directories) {
                this->tag_paths->addItem(p.string().c_str());
            }
            this->tag_paths->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);
            vbox_layout->addWidget(this->tag_paths);

            // Add some path stuff
            this->path_to_enter = new QLineEdit();
            this->tree_widget = new TagTreeWidget(nullptr, parent_window, std::vector<HEK::TagClassInt>(&*save_class, &*save_class + 1), std::nullopt, true);
            connect(this->tree_widget, &TagTreeWidget::itemClicked, this, &TagTreeDialog::on_click);
            connect(this->tree_widget, &TagTreeWidget::itemDoubleClicked, this, &TagTreeDialog::on_double_click);
        }
        else {
            this->tree_widget = new TagTreeWidget(nullptr, parent_window, classes);
            connect(this->tree_widget, &TagTreeWidget::itemDoubleClicked, this, &TagTreeDialog::on_double_click);
        }

        // Set layout
        vbox_layout->addWidget(this->tree_widget);

        // Next, add a textbox to type a path in, a button to create directories in, and a combo box of tag paths
        if(save_class.has_value()) {
            auto *hbox_layout = new QHBoxLayout();
            this->path_to_enter = new QLineEdit();

            hbox_layout->addWidget(this->path_to_enter);

            // Add a new directory button because memes
            auto *new_directory = new QPushButton("New folder");
            connect(new_directory, &QPushButton::clicked, this, &TagTreeDialog::new_folder);
            hbox_layout->addWidget(new_directory);

            auto *save = new QPushButton("Save");
            connect(save, &QPushButton::clicked, this, &TagTreeDialog::do_save_as);
            hbox_layout->addWidget(save);
            new_directory->setAutoDefault(false);
            save->setAutoDefault(true);

            // Add our thing
            auto *widget = new QWidget();
            widget->setLayout(hbox_layout);
            vbox_layout->addWidget(widget);
        }

        this->tree_widget->setMinimumSize(640, 480);

        // Set our layout
        this->setLayout(vbox_layout);
    }

    TagTreeDialog::TagTreeDialog(QWidget *parent, TagTreeWindow *parent_window, const std::optional<std::vector<HEK::TagClassInt>> &classes) : TagTreeDialog(parent, parent_window, classes, std::nullopt) {
        this->change_title(classes);
    }

    TagTreeDialog::TagTreeDialog(QWidget *parent, TagTreeWindow *parent_window, HEK::TagClassInt save_class) : TagTreeDialog(parent, parent_window, std::nullopt, save_class) {
        this->change_title(save_class);
    }

    const std::optional<File::TagFile> &TagTreeDialog::get_tag() const noexcept {
        return this->tag;
    }

    void TagTreeDialog::set_filter(const std::optional<std::vector<HEK::TagClassInt>> &classes) {
        this->tree_widget->set_filter(classes);
        this->change_title(classes);
    }

    void TagTreeDialog::change_title(HEK::TagClassInt save_class) {
        char title_text[512];
        std::snprintf(title_text, sizeof(title_text), "Select a location to save the %s tag", HEK::tag_class_to_extension(save_class));
        this->setWindowTitle(title_text);
    }

    void TagTreeDialog::new_folder() {
        auto items = this->tree_widget->selectedItems();
        auto dir_icon = QFileIconProvider().icon(QFileIconProvider::Folder);

        auto *item = items.size() == 0 ? nullptr : items[0];
        while(item && item->data(0, Qt::UserRole).isValid()) {
            item = item->parent();
        }

        // Ask for a directory
        QInputDialog dialog_ask;
        dialog_ask.setInputMode(QInputDialog::TextInput);
        dialog_ask.setWindowFlags(Qt::Dialog | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);
        dialog_ask.setLabelText("Enter the directory name");
        dialog_ask.setWindowTitle("New folder");

        QTreeWidgetItem *new_item;
        if(dialog_ask.exec() == QInputDialog::Accepted) {
            new_item = new QTreeWidgetItem();
            QString dialog_lower = dialog_ask.textValue().toLower();
            new_item->setIcon(0, dir_icon);
            new_item->setText(0, dialog_lower);

            if(item == nullptr) {
                for(int i = 0; i < this->tree_widget->topLevelItemCount(); i++) {
                    auto *item_test = this->tree_widget->topLevelItem(i);
                    if(item_test->text(0) == dialog_lower) {
                        this->tree_widget->clearSelection();
                        item_test->setSelected(true);
                        delete new_item;
                        this->on_click(item_test, 0);
                        return;
                    }
                }
                this->tree_widget->addTopLevelItem(new_item);
            }
            else {
                for(int i = 0; i < item->childCount(); i++) {
                    auto *item_test = item->child(i);
                    if(item_test->text(0) == dialog_lower) {
                        this->tree_widget->clearSelection();
                        item_test->setSelected(true);
                        delete new_item;
                        this->on_click(item_test, 0);
                        return;
                    }
                }
                item->addChild(new_item);
            }
        }
        else {
            return;
        }

        this->tree_widget->resort_elements();

        // Expand everything
        while(item) {
            item->setExpanded(true);
            item = item->parent();
        }

        // Select our new item
        this->tree_widget->clearSelection();
        new_item->setSelected(true);
        this->on_click(new_item, 0);
    }

    void TagTreeDialog::do_save_as() {
        auto potential_final_path_std_str = File::halo_path_to_preferred_path(this->path_to_enter->text().toLower().toLatin1().data());
        auto potential_final_path = QString(potential_final_path_std_str.c_str());
        static constexpr char PREFERRED_SEPARATOR[] = { std::filesystem::path::preferred_separator, 0 };

        if(potential_final_path.size() == 0 || potential_final_path.endsWith(PREFERRED_SEPARATOR) || potential_final_path.startsWith(PREFERRED_SEPARATOR) || this->path_to_enter->text() != potential_final_path) {
            QMessageBox(QMessageBox::Icon::Critical, "Invalid path", "The path given was invalid or empty. Paths cannot start or end with a path separator or contain any uppercase characters.", QMessageBox::Cancel).exec();
            return;
        }

        if(!potential_final_path.contains(PREFERRED_SEPARATOR)) {
            if(QMessageBox(QMessageBox::Icon::Warning, "You think you want it, but you actually don't", "You are creating a tag in the tag directory root. This is not recommended.\n\nAre you sure you want to continue?", QMessageBox::Yes | QMessageBox::Cancel, this).exec() == QMessageBox::Accepted) {
                return;
            }
        }

        // Generate the thing
        char extension[256];
        std::snprintf(extension, sizeof(extension), ".%s", HEK::tag_class_to_extension(*this->save_class));
        if(!potential_final_path.endsWith(extension)) {
            potential_final_path_std_str = potential_final_path_std_str + extension;
        }

        File::TagFile file;
        file.tag_path = potential_final_path_std_str;
        file.full_path = std::filesystem::path(this->tag_paths->currentText().toStdString()) / potential_final_path_std_str;
        file.tag_class_int = *this->save_class;
        file.tag_directory = this->tag_paths->currentIndex();
        this->tag = file;
        this->done(QDialog::Accepted);
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

    void TagTreeDialog::done(int r) {
        // If we're done, do it!
        if(!this->save_class.has_value()) {
            if(r == QDialog::Accepted) {
                auto *tag = this->tree_widget->get_selected_tag();
                if(tag) {
                    this->tag = *tag;
                }
                else {
                    this->tag = std::nullopt;
                }
            }
            else {
                this->tag = std::nullopt;
            }
        }

        QDialog::done(r);
    }

    void TagTreeDialog::on_double_click(QTreeWidgetItem *, int) {
        // Double clicked a tag
        auto *selected_tag = this->tree_widget->get_selected_tag();
        if(selected_tag) {
            if(save_class.has_value()) {
                this->path_to_enter->setText(Invader::File::split_tag_class_extension(selected_tag->tag_path.c_str())->path.c_str());
                this->do_save_as();
            }
            else {
                this->done(QDialog::Accepted);
            }
        }
    }

    void TagTreeDialog::on_click(QTreeWidgetItem *item, int) {
        // Clicked a tag
        auto *selected_tag = this->tree_widget->get_selected_tag();
        if(selected_tag) {
            this->path_to_enter->setText(Invader::File::split_tag_class_extension(selected_tag->tag_path.c_str())->path.c_str());
        }

        // Clicked a directory
        else if(item) {
            static constexpr char PREFERRED_SEPARATOR[] = { std::filesystem::path::preferred_separator, 0 };
            std::string name;

            while(item) {
                name = std::string(item->text(0).toLatin1().data()) + std::string(PREFERRED_SEPARATOR) + name;
                item = item->parent();
            }

            this->path_to_enter->setText(name.c_str());
            this->path_to_enter->setFocus();
            this->path_to_enter->setCursorPosition(name.size());
        }
    }
}
