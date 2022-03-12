// SPDX-License-Identifier: GPL-3.0-only

#ifdef INVADER_WIN32_EXE_STATIC_LINK
#include <QtPlugin>
#endif

#include <QApplication>
#include <QMessageBox>
#include <invader/command_line_option.hpp>
#include <invader/version.hpp>
#include <invader/printf.hpp>
#include "tree/tag_tree_window.hpp"

#ifdef INVADER_WIN32_EXE_STATIC_LINK
Q_IMPORT_PLUGIN(QWindowsIntegrationPlugin)
Q_IMPORT_PLUGIN(QWindowsVistaStylePlugin)
Q_IMPORT_PLUGIN(QWindowsAudioPlugin)
#endif

int main(int argc, char **argv) {
    set_up_color_term();
    
    using namespace Invader;

    const CommandLineOption options[] {
        CommandLineOption::from_preset(CommandLineOption::PRESET_COMMAND_LINE_OPTION_INFO),
        CommandLineOption::from_preset(CommandLineOption::PRESET_COMMAND_LINE_OPTION_FS_PATH),
        CommandLineOption::from_preset(CommandLineOption::PRESET_COMMAND_LINE_OPTION_TAGS_MULTIPLE),
        CommandLineOption("no-safeguards", 'n', 0, "Allow all tag data to be edited (proceed at your own risk)"),
        CommandLineOption("listing-mode", 'L', 1, "Set the listing behavior. Can be: fast, recursive (default: fast)")
    };

    static constexpr char DESCRIPTION[] = "Edit tag files.";
    static constexpr char USAGE[] = "[options] [<tag1> [tag2] [...]]";

    struct EditQtOption {
        std::vector<std::filesystem::path> tags;
        bool void_warranty = false;
        bool disable_safeguards = false;
        bool fs_path = false;
        bool fast_listing = true;
    } edit_qt_options;

    auto remaining_arguments = CommandLineOption::parse_arguments<EditQtOption &>(argc, argv, options, USAGE, DESCRIPTION, 0, 65535, edit_qt_options, [](char opt, const std::vector<const char *> &arguments, auto &edit_qt_options) {
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

            case 'P':
                edit_qt_options.fs_path = true;
                break;

            case 'L':
                if(std::strcmp(arguments[0], "fast") == 0) {
                    edit_qt_options.fast_listing = true;
                }
                else if(std::strcmp(arguments[0], "recursive") == 0) {
                    edit_qt_options.fast_listing = false;
                }
                else {
                    eprintf_error("Unknown listing mode %s", arguments[0]);
                    std::exit(EXIT_FAILURE);
                }
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
    
    File::check_working_directory("./guerillabeta.map");
    
    // Instantiate the application
    QApplication a(argc, argv);
    a.setWindowIcon(QIcon(":icon/invader-edit-qt.ico"));

    // Instantiate the window
    Invader::EditQt::TagTreeWindow w;
    w.set_tag_directories(edit_qt_options.tags);
    
    if(edit_qt_options.fast_listing) {
        w.set_fast_listing_mode(true);
    }
    
    // Give a spiel
    if(edit_qt_options.disable_safeguards) {
        QMessageBox message(QMessageBox::Warning, "You think you want this, but you actually don't.", "WARNING: Safeguards have been disabled.\n\nManually editing data that is normally read-only will likely break your tags.\n\nRemember: If something is normally read-only, then there is a much better program for modifying it than a tag editor.", QMessageBox::Ok | QMessageBox::Cancel);
        message.setDefaultButton(QMessageBox::Cancel);
        message.setButtonText(QMessageBox::Ok, "Time to break some tags!");
        if(message.exec() != QMessageBox::Ok) {
            std::exit(EXIT_FAILURE);
        }
        w.set_safeguards(false);
    }

    // Show it!
    w.show();

    // Open tags
    std::vector<std::string> tags;
    for(auto &i : remaining_arguments) {
        tags.emplace_back(i);
    }
    w.open_tags_when_ready(tags, edit_qt_options.fs_path);

    return a.exec();
}
