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

int strip_tag(const std::filesystem::path &file_path) {
    // Open the tag
    auto tag = Invader::File::open_file(file_path);
    if(!tag.has_value()) {
        eprintf_error("Failed to open %s", file_path.string().c_str());
        return EXIT_FAILURE;
    }

    // Get the header
    std::vector<std::byte> file_data;
    try {
        const auto *header = reinterpret_cast<const Invader::HEK::TagFileHeader *>(tag->data());
        Invader::HEK::TagFileHeader::validate_header(header, tag->size());
        file_data = Invader::Parser::ParserStruct::parse_hek_tag_file(tag->data(), tag->size())->generate_hek_tag_data(header->tag_fourcc, true);
    }
    catch(std::exception &e) {
        eprintf_error("Error: Failed to strip %s: %s", file_path.string().c_str(), e.what());
        return EXIT_FAILURE;
    }

    if(!Invader::File::save_file(file_path, file_data)) {
        eprintf_error("Error: Failed to write to %s.", file_path.string().c_str());
        return EXIT_FAILURE;
    }

    oprintf_success("Stripped %s", file_path.string().c_str());

    return EXIT_SUCCESS;
}

int main(int argc, char * const *argv) {
    EXIT_IF_INVADER_EXTRACT_HIDDEN_VALUES

    std::vector<Invader::CommandLineOption> options;
    options.emplace_back("info", 'i', 0, "Show license and credits.");
    options.emplace_back("tags", 't', 1, "Use the specified tags directory.", "<dir>");
    options.emplace_back("fs-path", 'P', 0, "Use a filesystem path for the tag path if specifying a tag.");
    options.emplace_back("all", 'a', 0, "Strip all tags in the tags directory.");
    options.emplace_back("search", 's', 1, "Search for tags (* and ? are wildcards); use multiple times for multiple queries", "<expr>");

    static constexpr char DESCRIPTION[] = "Strips extra hidden data from tags.";
    static constexpr char USAGE[] = "<options>";

    struct StripOptions {
        std::vector<std::filesystem::path> tags;
        std::vector<std::string> search;
        bool use_filesystem_path = false;
        bool all = false;
    } strip_options;

    auto remaining_arguments = Invader::CommandLineOption::parse_arguments<StripOptions &>(argc, argv, options, USAGE, DESCRIPTION, 0, 0, strip_options, [](char opt, const std::vector<const char *> &arguments, auto &strip_options) {
        switch(opt) {
            case 't':
                strip_options.tags.emplace_back(arguments[0]);
                break;
            case 'i':
                Invader::show_version_info();
                std::exit(EXIT_SUCCESS);
            case 'P':
                strip_options.use_filesystem_path = true;
                break;
            case 'a':
                strip_options.all = true;
                break;
            case 's':
                strip_options.search.emplace_back(Invader::File::halo_path_to_preferred_path(arguments[0]));
                break;
        }
    });
    if(strip_options.tags.empty()) {
        strip_options.tags = { "tags" };
    }
    
    if(strip_options.all) {
        strip_options.search = { "*" };
    }
    
    if(strip_options.search.empty()) {
        eprintf_error("No tags to strip. Use -h for more information.");
        return EXIT_FAILURE;
    }
    
    std::size_t success = 0;
    std::size_t total = 0;
    
    auto virtual_tags_directory = Invader::File::load_virtual_tag_folder(strip_options.tags);
    for(auto &i : virtual_tags_directory) {
        bool matches = false;
        auto *path = i.tag_path.c_str();
        for(auto &s : strip_options.search) {
            if((matches = Invader::File::path_matches(path, s.c_str()))) {
                break;
            }
        }
        if(matches) {
            total++;
            success += strip_tag(i.full_path) == EXIT_SUCCESS ? 1 : 0;
        }
    }
    oprintf("Stripped %zu out of %zu tag%s\n", success, total, total == 1 ? "" : "s");

    return success == total ? EXIT_SUCCESS : EXIT_FAILURE;
}
