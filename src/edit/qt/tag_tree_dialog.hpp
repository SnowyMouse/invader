// SPDX-License-Identifier: GPL-3.0-only

#ifndef INVADER__EDIT__QT__TAG_TREE_DIALOG_HPP
#define INVADER__EDIT__QT__TAG_TREE_DIALOG_HPP

#include <QDialog>
#include "tag_tree_widget.hpp"

namespace Invader::EditQt {
    class TagTreeWindow;

    class TagTreeDialog : public QDialog {
    public:
        /**
         * Instantiate a TagFileDialog
         * @param parent         parent widget
         * @param parent_window  parent window
         * @param classes        an optional array of classes to filter
         */
        TagTreeDialog(QWidget *parent, TagTreeWindow *parent_window, const std::optional<std::vector<HEK::TagClassInt>> &classes = std::nullopt);

        /**
         * Set the filter, limiting the view to those classes and directories that contain the given classes
         * @param classes an optional array of classes; if none is given, then the filter is cleared
         */
        void set_filter(const std::optional<std::vector<HEK::TagClassInt>> &classes = std::nullopt);

        /**
         * Get the resulting tag file
         * @return tag file result
         */
        const std::optional<File::TagFile> &get_tag() const noexcept;

    private:
        std::optional<File::TagFile> tag;
        TagTreeWidget *tree_widget;
        void change_title(const std::optional<std::vector<HEK::TagClassInt>> &classes);
        void done(int r);
        void on_double_click(QTreeWidgetItem *item, int column);
    };
}

#endif
