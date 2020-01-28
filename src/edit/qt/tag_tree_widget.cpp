// SPDX-License-Identifier: GPL-3.0-only

#include "tag_tree_widget.hpp"

#include <invader/file/file.hpp>
#include <QFileIconProvider>

namespace Invader::EditQt {
    TagTreeWidget::TagTreeWidget(QWidget *parent) : QTreeWidget(parent) {
        this->setHeaderHidden(true);
    }

    void TagTreeWidget::refresh_view(const std::vector<std::filesystem::path> &directories) {
        this->clear();

        // Build our tags directory
        std::vector<std::pair<std::filesystem::path, std::vector<std::string>>> all_tags;
        auto iterate_directories = [&all_tags](const std::vector<std::string> &the_story_thus_far, const std::filesystem::path &dir, auto &iterate_directories, int depth) -> void {
            if(++depth == 256) {
                return;
            }

            for(auto &d : std::filesystem::directory_iterator(dir)) {
                std::vector<std::string> add_dir = the_story_thus_far;
                auto file_path = d.path();
                add_dir.emplace_back(file_path.filename().string());
                if(d.is_directory()) {
                    iterate_directories(add_dir, d, iterate_directories, depth);
                }
                else {
                    bool found = false;
                    for(auto &t : all_tags) {
                        if(t.second == add_dir) {
                            found = true;
                            break;
                        }
                    }
                    if(!found) {
                        all_tags.emplace_back(file_path, add_dir);
                    }
                }
            }
        };
        for(auto &d : directories) {
            auto dir_str = d.string();
            iterate_directories(std::vector<std::string>(), d, iterate_directories, 0);
        }

        // Add the tags to the view
        for(auto &t : all_tags) {
            QTreeWidgetItem *dir_item = nullptr;
            std::size_t element_count = t.second.size();
            for(std::size_t e = 0; e < element_count; e++) {
                auto &element = t.second[e];
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
                    dir_item->setData(0, Qt::UserRole, QVariant(t.first.string().data()));
                }
                else if(!found) {
                    dir_item->setIcon(0, QFileIconProvider().icon(QFileIconProvider::Folder));
                }
            }
        }

        this->sortItems(0, Qt::SortOrder::AscendingOrder);
    }

    void TagTreeWidget::refresh_view(std::filesystem::path &directory) {
        refresh_view(std::vector<std::filesystem::path>(&directory, &directory + 1));
    }
}
