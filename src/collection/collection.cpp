// SPDX-License-Identifier: GPL-3.0-only

#include <vector>
#include <string>
#include <filesystem>
#include <invader/printf.hpp>
#include <invader/version.hpp>
#include <invader/command_line_option.hpp>
#include <invader/tag/parser/parser.hpp>
#include <invader/file/file.hpp>
#include <fstream>

#define INDEX_EXTENSION ".txt"

int main(int argc, char * const *argv) {
    set_up_color_term();
    
    using namespace Invader;
    
    const CommandLineOption options[] {
        CommandLineOption::from_preset(CommandLineOption::PRESET_COMMAND_LINE_OPTION_INFO),
        CommandLineOption::from_preset(CommandLineOption::PRESET_COMMAND_LINE_OPTION_DATA),
        CommandLineOption::from_preset(CommandLineOption::PRESET_COMMAND_LINE_OPTION_FS_PATH),
        CommandLineOption::from_preset(CommandLineOption::PRESET_COMMAND_LINE_OPTION_TAGS)
    };

    static constexpr char DESCRIPTION[] = "Generate tag_collection tags.";
    static constexpr char USAGE[] = "[options] <tag>";

    struct CollectionOptions {
        std::filesystem::path data = "data";
        std::optional<std::filesystem::path> tags;
        bool use_filesystem_path = false;
    } collection_options;

    auto remaining_arguments = CommandLineOption::parse_arguments<CollectionOptions &>(argc, argv, options, USAGE, DESCRIPTION, 1, 1, collection_options, [](char opt, const std::vector<const char *> &arguments, auto &collection) {
        switch(opt) {
            case 't':
                if(collection.tags.has_value()) {
                    eprintf_error("This tool does not support multiple tags directories.");
                    std::exit(EXIT_FAILURE);
                }
                collection.tags = arguments[0];
                break;
            case 'i':
                show_version_info();
                std::exit(EXIT_SUCCESS);
            case 'd':
                collection.data = arguments[0];
                break;
            case 'P':
                collection.use_filesystem_path = true;
                break;
        }
    });

    if(!collection_options.tags.has_value()) {
        collection_options.tags = "tags";
    }

    // Check if there's a string tag
    std::string string_tag;
    if(collection_options.use_filesystem_path) {
        std::vector<std::filesystem::path> data(&collection_options.data, &collection_options.data + 1);
        auto string_tag_maybe = File::file_path_to_tag_path(remaining_arguments[0], data);
        if(string_tag_maybe.has_value()) {
            string_tag = std::filesystem::path(*string_tag_maybe).replace_extension().string();
        }
        else {
            eprintf_error("Failed to find a valid %s file %s in the data directory", INDEX_EXTENSION, remaining_arguments[0]);
            return EXIT_FAILURE;
        }
    }
    else {
        string_tag = remaining_arguments[0];
    }

    std::filesystem::path tags_path(*collection_options.tags);
    if(!std::filesystem::is_directory(tags_path)) {
        eprintf_error("Directory %s was not found or is not a directory", collection_options.tags->string().c_str());
        return EXIT_FAILURE;
    }
    std::filesystem::path data_path(collection_options.data);

    auto input_path = (data_path / string_tag).string() + INDEX_EXTENSION;
    auto output_path = (tags_path / string_tag).string() + "." + HEK::tag_fourcc_to_extension(TagFourCC::TAG_FOURCC_TAG_COLLECTION);

    // Open a file
    std::ifstream input_file = std::ifstream(input_path);
    if(!input_file.is_open()) {
        eprintf_error("Failed to open %s", input_path.c_str());
        return EXIT_FAILURE;
    }
    std::string line;
    Parser::TagCollection tag;
    std::size_t line_number = 0;
    while(std::getline(input_file, line)) {
        line_number++;
        if(!line.empty()) {
            auto &entry = tag.tags.emplace_back();
            
            // First make sure it's a valid path
            auto whole_tag = File::split_tag_class_extension(line);
            if(!whole_tag.has_value()) {
                eprintf_error("Invalid tag path in line #%zu: %s (invalid or missing extension)", line_number, line.c_str());
                return EXIT_FAILURE;
            }
            
            entry.reference.path = File::preferred_path_to_halo_path(whole_tag->path);
            entry.reference.tag_fourcc = whole_tag->fourcc;
        }
    }
    auto final_data = tag.generate_hek_tag_data(TagFourCC::TAG_FOURCC_TAG_COLLECTION, true);

    // Write it all
    std::filesystem::path tag_path(output_path);

    // Create missing directories if needed
    std::error_code ec;
    std::filesystem::create_directories(tag_path.parent_path(), ec);

    if(!File::save_file(output_path.c_str(), final_data)) {
        eprintf_error("Error: Failed to write to %s.", output_path.c_str());
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
