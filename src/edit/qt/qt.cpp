// SPDX-License-Identifier: GPL-3.0-only

#include <QApplication>
#include <QMessageBox>
#include <invader/command_line_option.hpp>
#include <invader/version.hpp>
#include <invader/printf.hpp>
#include "tag_tree_window.hpp"

int main(int argc, char **argv) {
    using namespace Invader;

    std::vector<CommandLineOption> options;
    options.emplace_back("info", 'i', 0, "Show credits, source info, and other info.");
    options.emplace_back("tags", 't', 1, "Use the specified tags directory. Use multiple times to add more directories, ordered by precedence.", "<dir>");
    options.emplace_back("no-safeguards", 'n', 0, "Allow all tag data to be edited (proceed at your own risk)");

    static constexpr char DESCRIPTION[] = "Edit tag files.";
    static constexpr char USAGE[] = "[options]";

    struct EditQtOption {
        // Tags directory
        std::vector<std::filesystem::path> tags;

        bool void_warranty = false;

        bool disable_safeguards = false;
    } edit_qt_options;

    auto remaining_arguments = CommandLineOption::parse_arguments<EditQtOption &>(argc, argv, options, USAGE, DESCRIPTION, 0, 0, edit_qt_options, [](char opt, const std::vector<const char *> &arguments, auto &edit_qt_options) {
        switch(opt) {
            case 'i':
                show_version_info();
                std::exit(EXIT_SUCCESS);

            case 't':
                edit_qt_options.tags.push_back(arguments[0]);
                break;

            case 'n':
                edit_qt_options.disable_safeguards = true;
                break;
        }
    });

    if(edit_qt_options.tags.size() == 0) {
        edit_qt_options.tags.push_back("tags");
    }

    for(auto &t : edit_qt_options.tags) {
        if(!std::filesystem::is_directory(t)) {
            eprintf_error("Error: %s does not exist or is not a valid directory", t.string().c_str());
            return EXIT_FAILURE;
        }
    }

    QApplication a(argc, argv);

    Invader::EditQt::TagTreeWindow w;
    w.set_tag_directories(edit_qt_options.tags);
    if(edit_qt_options.disable_safeguards) {
        QMessageBox message(QMessageBox::Warning, "You think you want this, but you actually don't.", "WARNING: Safeguards have been disabled.\n\nManually editing data that is normally read-only will likely break your tags.\n\nRemember: If something is normally read-only, then there is a much better program for modifying it than a tag editor.", QMessageBox::Ok | QMessageBox::Cancel);
        message.setDefaultButton(QMessageBox::Cancel);
        message.setButtonText(QMessageBox::Ok, "Time to break some tags!");
        if(message.exec() != QMessageBox::Ok) {
            std::exit(EXIT_FAILURE);
        }
        w.set_safeguards(false);
    }
    w.show();

    return a.exec();
}
