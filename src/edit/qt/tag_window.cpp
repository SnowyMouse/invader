// SPDX-License-Identifier: GPL-3.0-only

#include <QMenuBar>
#include <QDialog>
#include <QLayout>
#include <QLabel>
#include <QTreeWidget>
#include <QFontDatabase>
#include <QStatusBar>
#include "tag_window.hpp"
#include "tag_tree_widget.hpp"
#include <invader/version.hpp>

namespace Invader::EditQt {
    TagWindow::TagWindow() {
        // Set some window stuff
        this->setWindowTitle("invader-edit-qt");
        this->setMinimumSize(800, 600);

        // Make and set our menu bar
        QMenuBar *bar = new QMenuBar(this);
        this->setMenuBar(bar);

        // View menu
        auto *view_menu = bar->addMenu("View");
        auto *refresh = view_menu->addAction("Refresh");
        refresh->setShortcut(QKeySequence::Refresh);
        connect(refresh, &QAction::triggered, this, &TagWindow::refresh_view);

        // Help menu
        auto *help_menu = bar->addMenu("Help");
        auto *about = help_menu->addAction("About");
        connect(about, &QAction::triggered, this, &TagWindow::show_about_window);

        // Now, set up the layout
        auto *central_widget = new QWidget(this);
        auto *vbox_layout = new QVBoxLayout(central_widget);
        this->tag_view = new TagTreeWidget(this);
        vbox_layout->addWidget(this->tag_view);
        vbox_layout->setMargin(0);
        central_widget->setLayout(vbox_layout);
        this->setCentralWidget(central_widget);

        // Next, set up the status bar
        QStatusBar *status_bar = new QStatusBar();
        this->tag_count_label = new QLabel();
        this->tag_location_label = new QLabel();
        status_bar->addWidget(this->tag_location_label, 1);
        status_bar->addWidget(this->tag_count_label, 0);
        this->setStatusBar(status_bar);

        // Default: Try to see if tags is in the current directory
        paths.emplace_back("tags");
        this->refresh_view();
    }

    void TagWindow::refresh_view() {
        if(current_tag_index == SHOW_ALL_MERGED) {
            this->tag_view->refresh_view(paths);
        }
        else {
            this->tag_view->refresh_view(paths[current_tag_index]);
        }

        char tag_count_str[256];
        auto tag_count = this->tag_view->get_total_tags();
        std::snprintf(tag_count_str, sizeof(tag_count_str), "%zu tag%s", tag_count, tag_count == 1 ? "" : "s");
        this->tag_count_label->setText(tag_count_str);
    }

    void TagWindow::show_about_window() {
        // Instantiate it
        QDialog dialog;
        dialog.setWindowTitle("About");
        dialog.setWindowFlags(Qt::Dialog | Qt::WindowTitleHint);

        // Make a layout
        auto *vbox_layout = new QVBoxLayout(&dialog);
        vbox_layout->setSizeConstraint(QLayout::SetFixedSize);

        // Show the version
        QLabel *label = new QLabel(full_version_and_credits());
        label->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
        label->setAlignment(Qt::AlignLeft | Qt::AlignTop);
        vbox_layout->addWidget(label);

        // Set our layout and disable resizing
        dialog.setLayout(vbox_layout);

        // Done. Show it!
        dialog.exec();
    }
}
