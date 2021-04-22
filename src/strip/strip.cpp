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

    static constexpr char DESCRIPTION[] = "Strips extra hidden data from tags.";
    static constexpr char USAGE[] = "[options] <-a | tag.class>";

    struct StripOptions {
        std::optional<std::filesystem::path> tags;
        bool use_filesystem_path = false;
        bool all = false;
    } strip_options;

    auto remaining_arguments = Invader::CommandLineOption::parse_arguments<StripOptions &>(argc, argv, options, USAGE, DESCRIPTION, 0, 1, strip_options, [](char opt, const std::vector<const char *> &arguments, auto &strip_options) {
        switch(opt) {
            case 't':
                if(strip_options.tags.has_value()) {
                    eprintf_error("This tool does not support multiple tags directories.");
                    std::exit(EXIT_FAILURE);
                }
                strip_options.tags = arguments[0];
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
        }
    });
    if(!strip_options.tags.has_value()) {
        strip_options.tags = "tags";
    }

    if(remaining_arguments.size() == 0) {
        if(!strip_options.all) {
            eprintf_error("Expected --all to be used OR a tag path. Use -h for more information.");
            return EXIT_FAILURE;
        }

        std::size_t success = 0;
        std::size_t total = 0;

        auto recursively_strip_dir = [&total, &success](const std::filesystem::path &dir, auto &recursively_strip_dir) -> void {
            for(auto i : std::filesystem::directory_iterator(dir)) {
                if(i.is_directory()) {
                    recursively_strip_dir(i, recursively_strip_dir);
                }
                else if(i.is_regular_file()) {
                    total++;
                    success += strip_tag(i.path().string().c_str()) == EXIT_SUCCESS;
                }
            }
        };

        recursively_strip_dir(std::filesystem::path(*strip_options.tags), recursively_strip_dir);

        oprintf("Stripped %zu out of %zu tag%s\n", success, total, total == 1 ? "" : "s");

        return EXIT_SUCCESS;
    }
    else if(strip_options.all) {
        eprintf_error("--all and a tag path cannot be used at the same time. Use -h for more information.");
        return EXIT_FAILURE;
    }
    else {
        std::filesystem::path file_path;
        if(strip_options.use_filesystem_path) {
            file_path = std::string(remaining_arguments[0]);
        }
        else {
            file_path = std::filesystem::path(*strip_options.tags) / Invader::File::halo_path_to_preferred_path(remaining_arguments[0]);
        }
        std::string file_path_str = file_path.string();
        return strip_tag(file_path_str.c_str());
    }
}
