// SPDX-License-Identifier: GPL-3.0-only

#ifndef INVADER__EDIT__QT__TAG_TREE_WIDGET_HPP
#define INVADER__EDIT__QT__TAG_TREE_WIDGET_HPP

#include <QTreeWidget>
#include <filesystem>

#include <invader/hek/class_int.hpp>

namespace Invader::EditQt {
    class TagTreeWidget : public QTreeWidget {
    public:
        /**
         * Instantiate a TagTreeWidget
         * @param parent  parent widget
         * @param classes optional array of classes to filter in
         */
        TagTreeWidget(QWidget *parent, const std::optional<std::vector<HEK::TagClassInt>> &classes = std::nullopt);

        /**
         * Set the filter, limiting the view to those classes and directories that contain the given classes
         * @param classes an optional array of classes; if none is given, then the filter is cleared
         */
        void set_filter(const std::optional<std::vector<HEK::TagClassInt>> &classes = std::nullopt);

        /**
         * Refresh the view with the given directories
         * @param directories directories to show, ordered from highest to lowest priority
         */
        void refresh_view(const std::vector<std::filesystem::path> &directories);

        /**
         * Refresh the view with the given directory
         * @param directories directory to show
         */
        void refresh_view(std::filesystem::path &directory);

        /**
         * Get the total tags found
         * @return total tags found
         */
        std::size_t get_total_tags();
    private:
        std::size_t total_tags = 0;
        std::optional<std::vector<HEK::TagClassInt>> filter;
    };
}

#endif
