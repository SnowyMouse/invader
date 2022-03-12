// SPDX-License-Identifier: GPL-3.0-only

#include <vector>
#include <string>
#include <filesystem>
#include <invader/printf.hpp>
#include <invader/version.hpp>
#include <invader/tag/hek/header.hpp>
#include <invader/tag/hek/definition.hpp>
#include <invader/command_line_option.hpp>
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
        CommandLineOption("search", 's', 1, "Search for tags (* and ? are wildcards) and strip these. Use multiple times for multiple queries. If unspecified, all tags will be stripped.", "<expr>"),
        CommandLineOption("search-exclude", 'e', 1, "Search for tags (* and ? are wildcards) and ignore these. Use multiple times for multiple queries. This takes precedence over --search.", "<expr>")
    };

    static constexpr char DESCRIPTION[] = "Strips extra hidden data from tags.";
    static constexpr char USAGE[] = "[options]";

    struct StripOptions {
        std::filesystem::path tags = "tags";
        std::vector<std::string> search;
        std::vector<std::string> search_exclude;
    } strip_options;

    auto remaining_arguments = CommandLineOption::parse_arguments<StripOptions &>(argc, argv, options, USAGE, DESCRIPTION, 0, 0, strip_options, [](char opt, const std::vector<const char *> &arguments, auto &strip_options) {
        switch(opt) {
            case 't':
                strip_options.tags = arguments[0];
                break;
            case 'i':
                show_version_info();
                std::exit(EXIT_SUCCESS);
            case 's':
                strip_options.search.emplace_back(File::preferred_path_to_halo_path(arguments[0]));
                break;
            case 'e':
                strip_options.search_exclude.emplace_back(File::preferred_path_to_halo_path(arguments[0]));
                break;
        }
    });

    std::size_t success = 0;
    std::size_t total = 0;
    
    for(auto &i : File::load_virtual_tag_folder( { strip_options.tags } )) {
        if(File::path_matches(i.tag_path.c_str(), strip_options.search, strip_options.search_exclude)) {
            total++;
            success += strip_tag(i.full_path.c_str(), i.tag_path) ? 1 : 0;
        }
    }

    oprintf("Stripped %zu out of %zu tag%s\n", success, total, total == 1 ? "" : "s");

    return EXIT_SUCCESS;
}
