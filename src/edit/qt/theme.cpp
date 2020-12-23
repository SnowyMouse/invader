// SPDX-License-Identifier: GPL-3.0-only

#include <QSettings>
#include <QApplication>
#include <QStyleFactory>
#include <QPalette>
#include "theme.hpp"

namespace SixShooter {
    #ifdef _WIN32
    void Theme::set_win32_theme() {
        QSettings settings("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize", QSettings::NativeFormat);
        auto setting = settings.value("AppsUseLightTheme");
        if(!setting.isNull() && setting == 0) {
            qApp->setStyle(QStyleFactory::create("Fusion"));
            QPalette dark_palette;
            
            QColor background_color_darker = QColor(0x20, 0x20, 0x20);
            QColor background_color = QColor(0x30, 0x30, 0x30);
            QColor disabled_color = QColor(0x7F, 0x7F, 0x7F);
            QColor text_color = QColor(0xEE, 0xEE, 0xEE);
            QColor link_color = QColor(0x4E, 0x4E, 0xEE);
            QColor bright_text_color = QColor(0xEE, 0x4E, 0x2E);
            
            dark_palette.setColor(QPalette::Window, background_color);
            dark_palette.setColor(QPalette::WindowText, text_color);
            dark_palette.setColor(QPalette::Base, background_color_darker);
            dark_palette.setColor(QPalette::AlternateBase, background_color);
            dark_palette.setColor(QPalette::ToolTipBase, text_color);
            dark_palette.setColor(QPalette::ToolTipText, text_color);
            dark_palette.setColor(QPalette::Text, text_color);
            
            dark_palette.setColor(QPalette::Button, background_color);
            dark_palette.setColor(QPalette::ButtonText, text_color);
            
            dark_palette.setColor(QPalette::BrightText, bright_text_color);
            dark_palette.setColor(QPalette::Link, link_color);
            dark_palette.setColor(QPalette::Highlight, text_color);
            dark_palette.setColor(QPalette::HighlightedText, background_color_darker);
            
            dark_palette.setColor(QPalette::Disabled, QPalette::Text, disabled_color);
            dark_palette.setColor(QPalette::Disabled, QPalette::ButtonText, disabled_color);
            dark_palette.setColor(QPalette::Disabled, QPalette::HighlightedText, disabled_color);

            qApp->setPalette(dark_palette);
        }
    }
    #else
    void Theme::set_theme() {}
    #endif
}
