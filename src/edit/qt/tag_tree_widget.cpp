// SPDX-License-Identifier: GPL-3.0-only

#include "tag_tree_widget.hpp"
#include "tag_window.hpp"

#include <invader/file/file.hpp>
#include <QFileIconProvider>
#include <invader/printf.hpp>
#include <QHeaderView>
#include <QMessageBox>

namespace Invader::EditQt {
    TagTreeWidget::TagTreeWidget(QWidget *parent, TagWindow *parent_window, const std::optional<std::vector<HEK::TagClassInt>> &classes) : QTreeWidget(parent), filter(classes), parent_window(parent_window) {
        this->setAlternatingRowColors(true);
        this->setHeaderHidden(true);
        this->setAnimated(false);
    }

    void TagTreeWidget::refresh_view(const std::optional<std::vector<std::size_t>> &tags) {
        this->clear();

        // Get the tags we have
        auto *parent = reinterpret_cast<TagWindow *>(this->parent_window);
        auto all_tags = parent->get_all_tags();
        std::size_t all_tags_size = all_tags.size();

        // Next, go through each tag and filter out anything we don't need (i.e. lower-priority tags, non-matching extensions)
        for(std::size_t i = 0; i < all_tags_size; i++) {
            bool remove = false;
            auto &tag = all_tags[i];

            // First, can we drop it simply because it's out of our current scope?
            if(tags.has_value()) {
                remove = true;
                for(auto t : *tags) {
                    if(tag.tag_directory == t) {
                        remove = false;
                        break;
                    }
                }
            }

            // Next, can we filter it out based on tag class alone?
            if(!remove && this->filter.has_value()) {
                remove = true;
                for(auto f : *this->filter) {
                    if(tag.tag_class_int == f) {
                        remove = false;
                        break;
                    }
                }
            }

            // Lastly, is this superceded by anything?
            if(!remove && tag.tag_directory > 0) {
                for(std::size_t j = 0; j < all_tags_size; j++) {
                    // If we're the same, continue
                    if(j == i) {
                        continue;
                    }

                    // If we're a lower priority, or the class isn't the same, continue too
                    auto &other_tag = all_tags[j];
                    if(other_tag.tag_directory > tag.tag_directory || other_tag.tag_class_int != tag.tag_class_int) {
                        continue;
                    }

                    // It all comes down to this. Do we have the same tag path?
                    if(other_tag.tag_path == tag.tag_path) {
                        remove = true;
                        break;
                    }
                }
            }

            if(remove) {
                all_tags.erase(all_tags.begin() + i);
                i--;
                all_tags_size--;
            }
        }

        // Add the tags to the view
        this->total_tags = all_tags.size();
        QIcon dir_icon = QFileIconProvider().icon(QFileIconProvider::Folder);
        QIcon file_icon = QFileIconProvider().icon(QFileIconProvider::File);

        for(auto &t : all_tags) {
            QTreeWidgetItem *dir_item = nullptr;
            std::size_t element_count = t.tag_path_separated.size();
            for(std::size_t e = 0; e < element_count; e++) {
                auto &element = t.tag_path_separated[e];
                bool found = false;

                // See if we have it
                if(dir_item == nullptr) {
                    int count = this->topLevelItemCount();
                    for(int i = 0; i < count; i++) {
                        auto *item = this->topLevelItem(i);
                        if(item->text(0) == element.data()) {
                            found = true;
                            dir_item = item;
                            break;
                        }
                    }
                }
                else {
                    int count = dir_item->childCount();
                    for(int i = 0; i < count; i++) {
                        auto *item = dir_item->child(i);
                        if(item->text(0) == element.data()) {
                            found = true;
                            dir_item = item;
                            break;
                        }
                    }
                }

                // If we don't have it, make it
                if(!found) {
                    auto *new_dir_item = new QTreeWidgetItem(QStringList(element.data()));
                    this->setContentsMargins(0, 0, 0, 0);
                    if(dir_item == nullptr) {
                        this->addTopLevelItem(new_dir_item);
                    }
                    else {
                        dir_item->addChild(new_dir_item);
                    }
                    dir_item = new_dir_item;
                }

                // If it's the last one, all is well then
                if(e + 1 == element_count) {
                    dir_item->setData(0, Qt::UserRole, QVariant(t.full_path.string().data()));
                    if(!found) {
                        dir_item->setIcon(0, file_icon);
                    }
                }
                else if(!found) {
                    dir_item->setIcon(0, dir_icon);
                }
            }
        }

        auto less_than = [](QTreeWidgetItem *item_i, QTreeWidgetItem *item_j) {
            bool i_is_dir = item_i->childCount() != 0;
            bool j_is_dir = item_j->childCount() != 0;

            return !(j_is_dir && !i_is_dir) && ((i_is_dir && !j_is_dir) || (item_i->text(0).compare(item_j->text(0), Qt::CaseInsensitive) < 0));
        };

        // First, sort the top level
        int top_level_count = this->topLevelItemCount();
        bool sorted;
        do {
            sorted = true;
            for(int i = 1; i < top_level_count; i++) {
                auto *item_i = this->topLevelItem(i);
                for(int j = 0; j < i; j++) {
                    auto *item_j = this->topLevelItem(j);
                    if(less_than(item_i, item_j)) {
                        sorted = false;
                        this->insertTopLevelItem(j, this->takeTopLevelItem(i));
                        break;
                    }
                }
            }
        }
        while(!sorted);

        // Lastly, sort all elements in element
        auto sort_elements = [&less_than](QTreeWidgetItem *item, auto &sort_elements) -> void {
            int children_count = item->childCount();

            // Sort this
            bool sorted;
            do {
                sorted = true;
                for(int i = 1; i < children_count; i++) {
                    auto *item_i = item->child(i);
                    for(int j = 0; j < i; j++) {
                        auto *item_j = item->child(j);
                        if(less_than(item_i, item_j)) {
                            sorted = false;
                            item->insertChild(j, item->takeChild(i));
                            break;
                        }
                    }
                }
            }
            while(!sorted);

            // Then sort the elements of this
            for(int i = 0; i < children_count; i++) {
                sort_elements(item->child(i), sort_elements);
            }
        };
        for(int i = 0; i < top_level_count; i++) {
            sort_elements(this->topLevelItem(i), sort_elements);
        }
    }

    std::size_t TagTreeWidget::get_total_tags() {
        return this->total_tags;
    }

    void TagTreeWidget::set_filter(const std::optional<std::vector<HEK::TagClassInt>> &classes) {
        this->filter = classes;
    }
}
