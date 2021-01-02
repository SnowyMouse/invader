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

#define INDEX_EXTENSION ".tag_indices"

int main(int argc, char * const *argv) {
    EXIT_IF_INVADER_EXTRACT_HIDDEN_VALUES

    std::vector<Invader::CommandLineOption> options;
    options.emplace_back("info", 'i', 0, "Show license and credits.");
    options.emplace_back("tags", 't', 1, "Use the specified tags directory.", "<dir>");
    options.emplace_back("data", 'd', 1, "Use the specified data directory.", "<dir>");
    options.emplace_back("fs-path", 'P', 0, "Use a filesystem path for the text file.");

    static constexpr char DESCRIPTION[] = "Generate tag_collection tags.";
    static constexpr char USAGE[] = "[options] <tag>";

    struct CollectionOptions {
        std::filesystem::path data = "data";
        std::optional<std::filesystem::path> tags;
        bool use_filesystem_path = false;
    } collection_options;

    auto remaining_arguments = Invader::CommandLineOption::parse_arguments<CollectionOptions &>(argc, argv, options, USAGE, DESCRIPTION, 1, 1, collection_options, [](char opt, const std::vector<const char *> &arguments, auto &collection) {
        switch(opt) {
            case 't':
                if(collection.tags.has_value()) {
                    eprintf_error("This tool does not support multiple tags directories.");
                    std::exit(EXIT_FAILURE);
                }
                collection.tags = arguments[0];
                break;
            case 'i':
                Invader::show_version_info();
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
        auto string_tag_maybe = Invader::File::file_path_to_tag_path_with_extension(remaining_arguments[0], data, INDEX_EXTENSION);
        if(string_tag_maybe.has_value()) {
            string_tag = string_tag_maybe.value();
        }
        else {
            eprintf_error("Failed to find a valid %s file %s in the data directory", INDEX_EXTENSION, remaining_arguments[0]);
            return EXIT_FAILURE;
        }
    }
    else {
        string_tag = remaining_arguments[0];
    }

    // Ensure it's lowercase
    for(const char *c = string_tag.c_str(); *c; c++) {
        if(*c >= 'A' && *c <= 'Z') {
            eprintf_error("Invalid tag path %s. Tag paths must be lowercase.", string_tag.c_str());
            return EXIT_FAILURE;
        }
    }

    std::filesystem::path tags_path(*collection_options.tags);
    if(!std::filesystem::is_directory(tags_path)) {
        eprintf_error("Directory %s was not found or is not a directory", collection_options.tags->string().c_str());
        return EXIT_FAILURE;
    }
    std::filesystem::path data_path(collection_options.data);

    auto input_path = (data_path / string_tag).string() + INDEX_EXTENSION;
    auto output_path = (tags_path / string_tag).string() + "." + Invader::HEK::tag_class_to_extension(Invader::TagClassInt::TAG_CLASS_TAG_COLLECTION);

    // Open a file
    std::ifstream input_file = std::ifstream(input_path);
    if(!input_file.is_open()) {
        eprintf_error("Failed to open %s", input_path.c_str());
        return EXIT_FAILURE;
    }
    std::string line;
    Invader::Parser::TagCollection tag;
    std::size_t line_number = 0;
    while(std::getline(input_file, line)) {
        line_number++;
        if(line.size()) {
            auto &entry = tag.tags.emplace_back();
            auto whole_tag = Invader::File::split_tag_class_extension(line);
            if(!whole_tag.has_value()) {
                eprintf_error("Invalid tag path in line #%zu: %s", line_number, line.c_str());
                return EXIT_FAILURE;
            }
            entry.reference.path = Invader::File::preferred_path_to_halo_path(whole_tag->path);
            entry.reference.tag_class_int = whole_tag->class_int;
        }
    }
    auto final_data = tag.generate_hek_tag_data(Invader::TagClassInt::TAG_CLASS_TAG_COLLECTION, true);

    // Write it all
    std::filesystem::path tag_path(output_path);

    // Create missing directories if needed
    std::error_code ec;
    std::filesystem::create_directories(tag_path.parent_path(), ec);

    if(!Invader::File::save_file(output_path.c_str(), final_data)) {
        eprintf_error("Error: Failed to write to %s.", output_path.c_str());
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
