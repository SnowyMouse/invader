// SPDX-License-Identifier: GPL-3.0-only

#include <QApplication>
#include "tag_window.hpp"

int main(int argc, char **argv) {
    QApplication a(argc, argv);

    Invader::EditQt::TagWindow w;
    w.show();

    return a.exec();
}
