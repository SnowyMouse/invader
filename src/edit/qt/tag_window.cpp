// SPDX-License-Identifier: GPL-3.0-only

#include <QMenuBar>
#include <QDialog>
#include <QLayout>
#include <QLabel>
#include <QFontDatabase>
#include "tag_window.hpp"
#include <invader/version.hpp>

namespace Invader::EditQt {
    TagWindow::TagWindow() {
        this->setWindowTitle(QString(full_version()));

        QMenuBar *bar = new QMenuBar(this);
        auto *help_menu = bar->addMenu("Help");
        auto *about = help_menu->addAction("About");
        connect(about, &QAction::triggered, this, &TagWindow::show_about_window);
        this->setMenuBar(bar);
    }

    void TagWindow::show_about_window() {
        // Instantiate it
        QDialog dialog;
        dialog.setWindowTitle("About");
        dialog.setWindowFlags(Qt::Dialog | Qt::WindowTitleHint);

        // Make a layout
        QVBoxLayout *layout = new QVBoxLayout();

        // Show the version
        QLabel *label = new QLabel(full_version_and_credits());
        label->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
        label->setAlignment(Qt::AlignLeft | Qt::AlignTop);
        layout->addWidget(label);

        // Set our layout and disable resizing
        dialog.setLayout(layout);
        layout->setSizeConstraint(QLayout::SetFixedSize);

        // Done. Show it!
        dialog.exec();

        delete layout;
    }
}
