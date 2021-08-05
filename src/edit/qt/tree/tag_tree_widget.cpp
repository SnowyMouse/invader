// SPDX-License-Identifier: GPL-3.0-only

#include "tag_tree_widget.hpp"
#include "tag_tree_window.hpp"

#include <invader/file/file.hpp>
#include <QFileIconProvider>
#include <invader/printf.hpp>
#include <QHeaderView>
#include <QMessageBox>

using namespace Invader::Parser;

namespace Invader::EditQt {
    TagTreeWidget::TagTreeWidget(QWidget *parent, TagTreeWindow *parent_window, const std::optional<std::vector<TagFourCC>> &classes, const std::optional<std::vector<std::size_t>> &tags_directories, bool show_directories) : QTreeWidget(parent), filter(classes), tag_arrays_to_show(tags_directories), show_directories(show_directories) {
        this->setColumnCount(1);
        this->setAlternatingRowColors(true);
        this->setHeaderHidden(true);
        this->setAnimated(false);
        this->refresh_view(parent_window);
        connect(parent_window, &TagTreeWindow::tags_reloaded, this, &TagTreeWidget::refresh_view);
    }

    void TagTreeWidget::refresh_view(TagTreeWindow *window) {
        this->clear();
        this->last_window = window;
        
        QIcon dir_icon = QFileIconProvider().icon(QFileIconProvider::Folder);
        QIcon file_icon = QFileIconProvider().icon(QFileIconProvider::File);
        
        // If fast listing mode is enabled
        if(!window->fast_listing_mode()) {
            // Get the tags we have
            const auto &all_tags = window->get_all_tags();
            std::size_t all_tags_size = all_tags.size();
            std::vector<bool> all_tags_ignored(all_tags_size, false);

            // Next, go through each tag and filter out anything we don't need (i.e. lower-priority tags, non-matching extensions)
            for(std::size_t i = 0; i < all_tags_size; i++) {
                bool remove = false;
                auto &tag = all_tags[i];

                // First, can we drop it simply because it's out of our current scope?
                if(this->tag_arrays_to_show.has_value()) {
                    remove = true;
                    for(auto t : *this->tag_arrays_to_show) {
                        if(tag.tag_directory == t) {
                            remove = false;
                            break;
                        }
                    }
                }

                // Next, can we filter it out based on tag class alone?
                if(!remove && this->filter.has_value() && this->filter->size() > 0) {
                    remove = true;
                    for(auto f : *this->filter) {
                        if(tag.tag_fourcc == f) {
                            remove = false;
                            break;
                        }
                    }
                }
                
                // Also, do we have this in our filters list?
                if(!remove && this->expressions.has_value()) {
                    remove = true;
                    for(auto &f : *this->expressions) {
                        if(File::path_matches(tag.tag_path.c_str(), f.c_str())) {
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
                        if(other_tag.tag_directory > tag.tag_directory || other_tag.tag_fourcc != tag.tag_fourcc) {
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
                    all_tags_ignored[i] = true;
                }
            }

            // Add the tags to the view
            this->total_tags = all_tags.size();

            for(std::size_t i = 0; i < all_tags_size; i++) {
                bool hide = all_tags_ignored[i];
                if(hide && !show_directories) {
                    continue;
                }

                auto &t = all_tags[i];
                QTreeWidgetItem *dir_item = nullptr;

                std::vector<std::string> separate_tag_path;
                auto prep = File::preferred_path_to_halo_path(t.tag_path);
                std::size_t last_separator = 0;
                std::size_t length = prep.size();

                for(std::size_t i = 0; i < length; i++) {
                    if(prep[i] == '\\') {
                        separate_tag_path.emplace_back(prep.c_str() + last_separator, (i - last_separator));
                        last_separator = ++i;
                    }
                }
                separate_tag_path.emplace_back(prep.c_str() + last_separator, (length - last_separator));

                std::size_t element_count = separate_tag_path.size();
                for(std::size_t e = 0; e < element_count; e++) {
                    auto &element = separate_tag_path[e];
                    bool found = false;

                    // See if we have it
                    if(dir_item == nullptr) {
                        int count = this->topLevelItemCount();
                        for(int i = 0; i < count; i++) {
                            auto *item = this->topLevelItem(i);
                            if(item->text(0) == element.c_str()) {
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
                            if(item->text(0) == element.c_str()) {
                                found = true;
                                dir_item = item;
                                break;
                            }
                        }
                    }

                    // Bail early if we're hiding this
                    if(e + 1 == element_count && hide) {
                        break;
                    }

                    // If we don't have it, make it
                    if(!found) {
                        auto *new_dir_item = new QTreeWidgetItem(QStringList(element.c_str()));
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
                        dir_item->setData(0, Qt::UserRole, QVariant::fromValue(reinterpret_cast<std::uintptr_t>(&t)));
                        if(!found) {
                            dir_item->setIcon(0, file_icon);
                        }
                        
                        // Make size text
                        char size[12];
                        std::uint64_t file_size;
                        try {
                            file_size = std::filesystem::file_size(t.full_path);
                        }
                        catch(std::exception &e) {
                            eprintf_error("Failed to get file size for %s: %s", t.full_path.string().c_str(), e.what());
                            file_size = 0;
                        }
                        if(file_size > 1024 * 1024) {
                            std::snprintf(size, sizeof(size), "%.02f MiB", file_size / 1024.0 / 1024.0);
                        }
                        else if(file_size > 1024) {
                            std::snprintf(size, sizeof(size), "%.02f KiB", file_size / 1024.0);
                        }
                        else if(file_size > 0) {
                            std::snprintf(size, sizeof(size), "%zu byte%s", file_size, file_size == 1 ? "" : "s");
                        }
                        else {
                            std::strcpy(size, "Unknown");
                        }
                        
                        // Make hover text
                        char text[1024];
                        std::snprintf(text, sizeof(text),
                            "Virtual path: %s\n"
                            "File path: %s\n"
                            "File size: %s"
                        , t.tag_path.c_str(),  t.full_path.string().c_str(), size);
                        dir_item->setToolTip(0, text);
                        dir_item->setToolTip(1, text);
                        dir_item->setText(1, size);
                    }
                    else if(!found) {
                        dir_item->setIcon(0, dir_icon);
                    }
                }
            }
        }
        
        // Otherwise, show the top level first, and we'll worry about the rest later
        else {
            this->load_directories(nullptr);
        }

        // Connect this in case we expand any items while in fast mode
        connect(this, &TagTreeWidget::itemExpanded, this, &TagTreeWidget::load_directories);
        
        // Done!
        this->resort_elements();
    }
    
    void TagTreeWidget::load_directories(QTreeWidgetItem *item) {
        // If fast listing mode is disabled, do nothing
        if(!this->last_window->fast_listing_mode()) {
            return;
        }
        
        std::filesystem::path start;
        auto recursive_backwards_memery = [&start](QTreeWidgetItem *item, auto &recursion) -> void {
            if(item) {
                recursion(item->parent(), recursion);
                start = start / item->text(0).toStdString();
            }
        };
        recursive_backwards_memery(item, recursive_backwards_memery);
        
        // Remove anything deemed unsatisfactory to the state
        if(item) {
            while(item->childCount()) {
                item->removeChild(item->child(0));
            }
        }
        else {
            this->clear();
        }
        
        std::vector<std::string> top_level_directories;
        std::vector<std::string> top_level_tags;
        
        QIcon dir_icon = QFileIconProvider().icon(QFileIconProvider::Folder);
        QIcon file_icon = QFileIconProvider().icon(QFileIconProvider::File);
        
        // Go through each tag directory
        for(auto &t : this->last_window->get_tag_directories()) {
            std::error_code ec;
            for(auto &q : std::filesystem::directory_iterator(t / start, ec)) {
                QTreeWidgetItem *new_item;
                bool directory = q.is_directory();
                auto relative = std::filesystem::relative(q.path(), t);
                if(!directory) {
                    auto path_split = File::split_tag_class_extension(relative.string());
                    
                    // No valid extension? Don't show it then.
                    if(!path_split.has_value()) {
                        continue;
                    }
                    
                    new_item = new QTreeWidgetItem(QStringList(relative.filename().string().c_str()));
                    new_item->setIcon(0, file_icon);
                    new_item->setData(0, Qt::UserRole, QVariant::fromValue(QString(relative.string().c_str())));
                    
                    if(this->filter.has_value() && this->filter->size() > 0) {
                        new_item->setDisabled(true);
                        for(auto &i : *this->filter) {
                            if(i == path_split->fourcc) {
                                new_item->setDisabled(false);
                                break;
                            }
                        }
                    }
                }
                else {
                    new_item = new QTreeWidgetItem(QStringList(relative.filename().string().c_str()));
                    new_item->setIcon(0, dir_icon);
                    new_item->addChild(new QTreeWidgetItem());
                }
                
                if(item) {
                    bool duplicate = false;
                    for(int i = 0; i < item->childCount(); i++) {
                        if(item->child(i)->text(0) == new_item->text(0)) {
                            duplicate = true;
                            break;
                        }
                    }
                    
                    // If it already exists, delete it. Otherwise add it.
                    if(duplicate) {
                        delete new_item;
                    }
                    else {
                        item->addChild(new_item);
                    }
                }
                else {
                    bool duplicate = false;
                    for(int i = 0; i < this->topLevelItemCount(); i++) {
                        if(this->topLevelItem(i)->text(0) == new_item->text(0)) {
                            duplicate = true;
                            break;
                        }
                    }
                    
                    // If it already exists, delete it. Otherwise add it.
                    if(duplicate) {
                        delete new_item;
                    }
                    else {
                        this->addTopLevelItem(new_item);
                    }
                }
            }
        }
        
        this->resort_elements();
    }

    void TagTreeWidget::resort_elements() {
        auto less_than = [](QTreeWidgetItem *item_i, QTreeWidgetItem *item_j) {
            bool i_is_dir = !item_i->data(0, Qt::UserRole).isValid();
            bool j_is_dir = !item_j->data(0, Qt::UserRole).isValid();

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

    void TagTreeWidget::set_filter(const std::optional<std::vector<TagFourCC>> &classes, const std::optional<std::vector<std::size_t>> &tags_directories, const std::optional<std::vector<std::string>> &expression_filters) {
        bool change_made = false;
        
        // Did we change classes?
        if(this->filter != classes) {
            this->filter = classes;
            change_made = true;
        }
        
        // Tag arrays changed?
        if(this->tag_arrays_to_show != tags_directories) {
            this->tag_arrays_to_show = tags_directories;
            change_made = true;
        }
        
        auto showing_everything_array = [](auto &expression_filters) {
            bool show_everything = true;
            if(expression_filters.has_value()) {
                for(auto &i : *expression_filters) {
                    for(auto &c : i) {
                        if(c != '*') {
                            show_everything = false;
                        }
                    }
                }
            }
            return show_everything;
        };
        
        // Expression filters?
        if(this->expressions != expression_filters) {
            bool currently_showing_everything = showing_everything_array(this->expressions);
            
            if(!currently_showing_everything || currently_showing_everything != showing_everything_array(expression_filters)) {
                this->expressions = expression_filters;
                change_made = true;
            }
        }
        
        // It's yours, my friend, as long as you have enough rubies.
        if(change_made) {
            this->refresh_view(this->last_window);
        }
    }

    std::optional<File::TagFile> TagTreeWidget::get_selected_tag() const noexcept {
        auto selected_items = this->selectedItems();
        
        if(selected_items.size()) {
            auto data = selected_items[0]->data(0, Qt::UserRole);
            auto ptr = data.value<std::uintptr_t>();
            if(ptr) {
                return *reinterpret_cast<const File::TagFile *>(ptr);
            }
            auto path = data.value<QString>();
            if(!path.isEmpty()) {
                File::TagFile file;
                auto path_str = path.toStdString();
                auto split = File::split_tag_class_extension(path_str).value();
                file.tag_path = path_str;
                file.tag_fourcc = split.fourcc;
                
                std::optional<std::filesystem::path> full_path_maybe;
                std::size_t directory = 0;
                
                // Find it... again
                auto &dirs = this->last_window->get_tag_directories();
                for(auto &t : dirs) {
                    auto path = t / path_str;
                    if(std::filesystem::exists(path)) {
                        full_path_maybe = path;
                        directory = &t - dirs.data();
                        break;
                    }
                }
                
                // Not found? Guess it was removed lol
                if(!full_path_maybe.has_value()) {
                    delete selected_items[0];
                    return std::nullopt;
                }
                
                file.tag_directory = directory;
                file.full_path = *full_path_maybe;
                return file;
            }
            return std::nullopt;
        }
        else {
            return std::nullopt;
        }
    }
    
    std::optional<std::string> TagTreeWidget::get_selected_directory() const noexcept {
        // If we have a tag selected, do nothing
        if(this->get_selected_tag().has_value()) {
            return std::nullopt;
        }
        
        auto selected_items = this->selectedItems();
        if(selected_items.size()) {
            auto *item = selected_items[0];
            std::string path;
            while(item) {
                path = item->text(0).toStdString() + path;
                item = item->parent();
                
                // Don't have a trailing path separator.
                if(item) {
                    path = std::string { INVADER_PREFERRED_PATH_SEPARATOR } + path;
                }
            }
            return path;
        }
        else {
            return std::nullopt;
        }
    }
}
