// SPDX-License-Identifier: GPL-3.0-only

#include <invader/command_line_option.hpp>
#include <invader/file/file.hpp>
#include <invader/tag/parser/parser.hpp>
#include <invader/version.hpp>
#include <invader/tag/hek/header.hpp>
#include "recover_method.hpp"

int main(int argc, const char **argv) {
    set_up_color_term();
    
    using namespace Invader;
    using namespace Invader::HEK;
    
    struct RecoverOptions {
        std::filesystem::path tags = "tags";
        std::filesystem::path data = "data";
        bool filesystem_path = false;
        bool overwrite = false;
    } recover_options;

    const CommandLineOption options[] = {
        CommandLineOption::from_preset(CommandLineOption::PRESET_COMMAND_LINE_OPTION_INFO),
        CommandLineOption::from_preset(CommandLineOption::PRESET_COMMAND_LINE_OPTION_FS_PATH),
        CommandLineOption::from_preset(CommandLineOption::PRESET_COMMAND_LINE_OPTION_DATA),
        CommandLineOption::from_preset(CommandLineOption::PRESET_COMMAND_LINE_OPTION_TAGS),
        CommandLineOption("overwrite", 'O', 0, "Overwrite data if it already exists"),
    };

    static constexpr char DESCRIPTION[] = "Recover source data from tags.";
    static constexpr char USAGE[] = "[options] <tag.class>";

    auto remaining_arguments = Invader::CommandLineOption::parse_arguments<RecoverOptions &>(argc, argv, options, USAGE, DESCRIPTION, 1, 1, recover_options, [](char opt, const auto &args, RecoverOptions &recover_options) {
        switch(opt) {
            case 'i':
                Invader::show_version_info();
                std::exit(EXIT_SUCCESS);
            case 'P':
                recover_options.filesystem_path = true;
                break;
            case 'd':
                recover_options.data = args[0];
                break;
            case 'O':
                recover_options.overwrite = true;
                break;
            case 't':
                recover_options.tags = args[0];
                break;
        }
    });
    
    if(!std::filesystem::is_directory(recover_options.data)) {
        eprintf_error("Data folder %s does not exist or is not a valid directory", recover_options.data.string().c_str());
        return EXIT_FAILURE;
    }
    
    // Handle -P
    std::string tag;
    if(recover_options.filesystem_path) {
        auto tag_maybe = File::file_path_to_tag_path(remaining_arguments[0], recover_options.tags);
        if(tag_maybe.has_value() && std::filesystem::exists(remaining_arguments[0])) {
            tag = *tag_maybe;
        }
        else {
            eprintf_error("Failed to find a valid tag %s in %s.", remaining_arguments[0], recover_options.tags.string().c_str());
            return EXIT_FAILURE;
        }
    }
    else {
        tag = remaining_arguments[0];
    }
    tag = File::halo_path_to_preferred_path(tag);
    
    // read it
    auto file_path = File::tag_path_to_file_path(tag, recover_options.tags);
    auto file = File::open_file(file_path);
    if(!file.has_value()) {
        eprintf_error("Failed to read %s", file_path.string().c_str());
        return EXIT_FAILURE;
    }
    
    // Load it
    auto tag_data = Parser::ParserStruct::parse_hek_tag_file(file->data(), file->size());
    Recover::recover(*tag_data, std::filesystem::path(tag).replace_extension().string(), recover_options.data, reinterpret_cast<const HEK::TagFileHeader *>(file->data())->tag_fourcc, recover_options.overwrite);
}
