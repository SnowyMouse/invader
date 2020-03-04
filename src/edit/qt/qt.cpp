// SPDX-License-Identifier: GPL-3.0-only

#include <QApplication>
#include <invader/command_line_option.hpp>
#include <invader/version.hpp>
#include <invader/printf.hpp>
#include "tag_tree_window.hpp"

int main(int argc, char **argv) {
    using namespace Invader;

    std::vector<CommandLineOption> options;
    options.emplace_back("info", 'i', 0, "Show credits, source info, and other info.");
    options.emplace_back("tags", 't', 1, "Use the specified tags directory. Use multiple times to add more directories, ordered by precedence.", "<dir>");

    static constexpr char DESCRIPTION[] = "Edit tag files.";
    static constexpr char USAGE[] = "[options]";

    struct EditQtOption {
        // Tags directory
        std::vector<std::filesystem::path> tags;
    } edit_qt_options;

    auto remaining_arguments = CommandLineOption::parse_arguments<EditQtOption &>(argc, argv, options, USAGE, DESCRIPTION, 0, 0, edit_qt_options, [](char opt, const std::vector<const char *> &arguments, auto &resource_options) {
        switch(opt) {
            case 'i':
                show_version_info();
                std::exit(EXIT_SUCCESS);

            case 't':
                resource_options.tags.push_back(arguments[0]);
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
    w.show();

    return a.exec();
}
