// SPDX-License-Identifier: GPL-3.0-only

#ifndef INVADER__EDIT__QT__TAG_TREE_WIDGET_HPP
#define INVADER__EDIT__QT__TAG_TREE_WIDGET_HPP

#include <QTreeWidget>
#include <filesystem>

namespace Invader::EditQt {
    class TagTreeWidget : public QTreeWidget {
    public:
        TagTreeWidget(QWidget *parent);

        void refresh_view(const std::vector<std::filesystem::path> &directories);
        void refresh_view(std::filesystem::path &directory);

        std::size_t get_total_tags();
    private:
        std::size_t total_tags = 0;
    };
}

#endif
