// SPDX-License-Identifier: GPL-3.0-only

#ifndef INVADER__EDIT__QT__TAG_TREE_DIALOG_HPP
#define INVADER__EDIT__QT__TAG_TREE_DIALOG_HPP

#include <QDialog>
#include "tag_tree_widget.hpp"

class QLineEdit;
class QComboBox;

namespace Invader::EditQt {
    class TagTreeWindow;

    class TagTreeDialog : public QDialog {
        Q_OBJECT
        
    public:
        /**
         * Instantiate a TagFileDialog
         * @param parent         parent widget
         * @param parent_window  parent window
         * @param classes        an optional array of classes to filter
         */
        TagTreeDialog(QWidget *parent, TagTreeWindow *parent_window, const std::optional<std::vector<HEK::TagClassInt>> &classes = std::nullopt);

        /**
         * Instantiate a TagFileDialog for saving
         * @param parent         parent widget
         * @param parent_window  parent window
         * @param tag_class      tag class to save
         */
        TagTreeDialog(QWidget *parent, TagTreeWindow *parent_window, HEK::TagClassInt save_class);

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
        
        ~TagTreeDialog() = default;

    private:
        TagTreeDialog(QWidget *parent, TagTreeWindow *parent_window, const std::optional<std::vector<HEK::TagClassInt>> &classes, std::optional<HEK::TagClassInt> saving);
        std::optional<File::TagFile> tag;
        TagTreeWidget *tree_widget;
        void match_find_filter(const QString &filter);
        void change_title(const std::optional<std::vector<HEK::TagClassInt>> &classes);
        void change_title(HEK::TagClassInt save_class);
        void done(int r);
        void on_double_click(QTreeWidgetItem *item, int column);
        void on_click(QTreeWidgetItem *item, int column);
        void new_folder();
        void do_save_as();
        std::optional<HEK::TagClassInt> save_class;
        std::optional<std::vector<HEK::TagClassInt>> filter_classes;
        QString path_filter;
        void refresh_filter();

        QLineEdit *path_to_enter;
        QComboBox *tag_paths;
    };
}

#endif
