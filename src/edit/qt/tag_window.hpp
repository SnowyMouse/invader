// SPDX-License-Identifier: GPL-3.0-only

#ifndef INVADER__EDIT__QT__TAG_WINDOW_HPP
#define INVADER__EDIT__QT__TAG_WINDOW_HPP

#include <QMainWindow>

namespace Invader::EditQt {
    class TagWindow : public QMainWindow {
    public:
        TagWindow();
    private:
        void show_about_window();
    };
}

#endif
