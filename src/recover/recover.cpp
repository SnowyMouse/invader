// SPDX-License-Identifier: GPL-3.0-only

#include <invader/command_line_option.hpp>
#include <invader/file/file.hpp>
#include <invader/tag/parser/parser_struct.hpp>
#include <invader/version.hpp>
#include <invader/tag/hek/header.hpp>
#include "recover_method.hpp"

using namespace Invader;
using namespace Invader::Parser;

int main(int argc, const char **argv) {
    struct RecoverOptions {
        std::filesystem::path tags = "tags";
        std::filesystem::path data = "data";
        bool filesystem_path = false;
        bool overwrite = false;
    } recover_options;

    std::vector<Invader::CommandLineOption> options;
    options.emplace_back("info", 'i', 0, "Show credits, source info, and other info.");
    options.emplace_back("tags", 't', 1, "Use the specified tags directory.", "<dir>");
    options.emplace_back("data", 'd', 1, "Use the specified data directory.", "<dir>");
    options.emplace_back("overwrite", 'O', 0, "Overwrite data if it already exists");
    options.emplace_back("fs-path", 'P', 0, "Use a filesystem path for the tag path directory.");

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
    
    // read it
    auto file_path = File::tag_path_to_file_path(tag, recover_options.tags);
    auto file = File::open_file(file_path);
    if(!file.has_value()) {
        eprintf_error("Failed to read %s", file_path.string().c_str());
        return EXIT_FAILURE;
    }
    
    // Load it
    auto tag_data = Parser::ParserStruct::parse_hek_tag_file(file->data(), file->size());
    Recover::recover(*tag_data, std::filesystem::path(tag).replace_extension().string(), recover_options.data, reinterpret_cast<const TagFileHeader *>(file->data())->tag_fourcc, recover_options.overwrite);
}
