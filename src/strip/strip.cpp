// SPDX-License-Identifier: GPL-3.0-only

#include <vector>
#include <string>
#include <filesystem>
#include <invader/printf.hpp>
#include <invader/version.hpp>
#include <invader/tag/hek/header.hpp>
#include <invader/tag/hek/definition.hpp>
#include "../command_line_option.hpp"
#include <invader/tag/parser/parser.hpp>
#include <invader/file/file.hpp>

using namespace Invader;

bool strip_tag(const std::filesystem::path &file_path, const std::string &tag_path) {
    // Open the tag
    auto tag = File::open_file(file_path);
    if(!tag.has_value()) {
        eprintf_error("Failed to open %s", file_path.string().c_str());
        return false;
    }

    // Get the header
    std::vector<std::byte> file_data;
    try {
        const auto *header = reinterpret_cast<const HEK::TagFileHeader *>(tag->data());
        HEK::TagFileHeader::validate_header(header, tag->size());
        file_data = Parser::ParserStruct::parse_hek_tag_file(tag->data(), tag->size())->generate_hek_tag_data(header->tag_fourcc, true);
    }
    catch(std::exception &e) {
        eprintf_error("Error: Failed to strip %s: %s", tag_path.c_str(), e.what());
        return false;
    }

    // Don't write if it matches
    if(file_data == *tag) {
        oprintf("Skipped %s\n", tag_path.c_str());
        return false;
    }
    
    if(!File::save_file(file_path, file_data)) {
        eprintf_error("Error: Failed to write to %s.", file_path.string().c_str());
        return false;
    }

    oprintf_success("Stripped %s", tag_path.c_str());

    return true;
}

int main(int argc, char * const *argv) {
    set_up_color_term();
    
    const CommandLineOption options[] {
        CommandLineOption::from_preset(CommandLineOption::PRESET_COMMAND_LINE_OPTION_INFO),
        CommandLineOption::from_preset(CommandLineOption::PRESET_COMMAND_LINE_OPTION_TAGS),
        CommandLineOption::from_preset(CommandLineOption::PRESET_COMMAND_LINE_OPTION_BATCH),
        CommandLineOption::from_preset(CommandLineOption::PRESET_COMMAND_LINE_OPTION_BATCH_EXCLUDE),
        CommandLineOption::from_preset(CommandLineOption::PRESET_COMMAND_LINE_OPTION_FS_PATH)
    };

    static constexpr char DESCRIPTION[] = "Strips extra hidden data from tags.";
    static constexpr char USAGE[] = "[options] <-b [expr] | <tag>>";

    struct StripOptions {
        std::filesystem::path tags = "tags";
        bool fs_path = false;
        std::vector<std::string> search;
        std::vector<std::string> search_exclude;
    } strip_options;

    auto remaining_arguments = CommandLineOption::parse_arguments<StripOptions &>(argc, argv, options, USAGE, DESCRIPTION, 0, 1, strip_options, [](char opt, const std::vector<const char *> &arguments, auto &strip_options) {
        switch(opt) {
            case 't':
                strip_options.tags = arguments[0];
                break;
            case 'i':
                show_version_info();
                std::exit(EXIT_SUCCESS);
            case 'b':
                strip_options.search.emplace_back(File::preferred_path_to_halo_path(arguments[0]));
                break;
            case 'e':
                strip_options.search_exclude.emplace_back(File::preferred_path_to_halo_path(arguments[0]));
                break;
            case 'P':
                strip_options.fs_path = true;
                break;
        }
    });
    
    std::optional<File::TagFilePath> single_tag;
    if(strip_options.search.empty() && strip_options.search_exclude.empty()) {
        if(remaining_arguments.size() == 1) {
            try {
                single_tag = File::split_tag_class_extension(!strip_options.fs_path ? remaining_arguments[0] : File::file_path_to_tag_path(remaining_arguments[0], strip_options.tags).value()).value();
            }
            catch(std::exception &) {
                eprintf_error("Invalid tag path %s", remaining_arguments[0]);
                std::exit(EXIT_FAILURE);
            }
        }
        else {
            eprintf_error("A tag path was expected. Use -h for more information.");
            return EXIT_FAILURE;
        }
    }
    else if(!remaining_arguments.empty()) {
        eprintf_error("Can't use an extra tag path and -b. Use -h for more information.");
        return EXIT_FAILURE;
    }

    std::size_t success = 0;
    std::size_t total = 0;
    
    if(single_tag.has_value()) {
        return strip_tag(File::tag_path_to_file_path(*single_tag, strip_options.tags).string().c_str(), File::halo_path_to_preferred_path(single_tag->join())) ? EXIT_SUCCESS : EXIT_FAILURE;
    }
    
    for(auto &i : File::load_virtual_tag_folder( { strip_options.tags } )) {
        if(File::path_matches(i.tag_path.c_str(), strip_options.search, strip_options.search_exclude)) {
            total++;
            success += strip_tag(i.full_path.c_str(), i.tag_path) ? 1 : 0;
        }
    }

    oprintf("Stripped %zu out of %zu tag%s\n", success, total, total == 1 ? "" : "s");

    return EXIT_SUCCESS;
}
