// SPDX-License-Identifier: GPL-3.0-only

#include <optional>
#include <filesystem>
#include <invader/map/map.hpp>
#include <invader/file/file.hpp>
#include <invader/command_line_option.hpp>
#include <invader/crc/hek/crc.hpp>
#include <invader/version.hpp>
#include <invader/extract/extraction.hpp>

int main(int argc, const char **argv) {
    using namespace Invader;

    // Options struct
    struct ExtractOptions {
        std::string tags_directory = "tags";
        std::string maps_directory = "maps";
        std::vector<std::vector<char>> tags_to_extract;
        bool recursive = false;
        bool overwrite = false;
    } extract_options;

    // Command line options
    std::vector<Invader::CommandLineOption> options;
    options.emplace_back("maps", 'm', 1, "Set the maps directory", "<dir>");
    options.emplace_back("tags", 't', 1, "Set the tags directory", "<dir>");
    options.emplace_back("tag", 'T', 1, "Extract a specific tag. Use multiple times to specify multiple tags.", "<tag.class>");
    options.emplace_back("recursive", 'r', 0, "Extract tag dependencies");
    options.emplace_back("overwrite", 'O', 0, "Overwrite tags if they already exist");
    options.emplace_back("info", 'i', 0, "Show credits, source info, and other info.");

    static constexpr char DESCRIPTION[] = "Extract data from cache files.";
    static constexpr char USAGE[] = "[options] <map>";

    // Do it!
    auto remaining_arguments = Invader::CommandLineOption::parse_arguments<ExtractOptions &>(argc, argv, options, USAGE, DESCRIPTION, 1, 1, extract_options, [](char opt, const auto &args, auto &extract_options) {
        switch(opt) {
            case 'm':
                extract_options.maps_directory = args[0];
                break;
            case 't':
                extract_options.tags_directory = args[0];
                break;
            case 'T':
                extract_options.tags_to_extract.emplace_back(args[0], args[0] + std::strlen(args[0]) + 1);
                break;
            case 'r':
                extract_options.recursive = true;
                break;
            case 'O':
                extract_options.overwrite = true;
                break;
            case 'i':
                Invader::show_version_info();
                std::exit(EXIT_SUCCESS);
        }
    });

    std::unique_ptr<Map> map;
    std::size_t file_size = 0;
    try {
        auto file = File::open_file(remaining_arguments[0]).value();
        file_size = file.size();
        map = std::make_unique<Map>(Map::map_with_move(std::move(file)));
    }
    catch (std::exception &e) {
        eprintf("Failed to parse %s: %s\n", remaining_arguments[0], e.what());
        return EXIT_FAILURE;
    }

    // Already extracted tags (so we don't need to re-extract them)
    auto tag_count = map->get_tag_count();
    std::vector<bool> extracted_tags(tag_count);

    // Also here's the tags directory
    std::filesystem::path tags(extract_options.tags_directory);

    // Extract a tag
    auto extract_tag = [&extracted_tags, &map, &tags, &extract_options](std::size_t tag_index) -> void {
        if(extracted_tags[tag_index]) {
            return;
        }

        // Get the tag path
        const auto &tag = map->get_tag(tag_index);
        const char *tag_extension = Invader::HEK::tag_class_to_extension(tag.get_tag_class_int());
        auto tag_path_to_write_to = tags / (Invader::File::halo_path_to_preferred_path(tag.get_path()) + "." + tag_extension);
        if(extract_options.overwrite && std::filesystem::exists(tag_path_to_write_to)) {
            return;
        }

        // Get the tag data
        std::vector<std::byte> new_tag;
        try {
            new_tag = Invader::Extraction::extract_tag(tag);
        }
        catch (std::exception &e) {
            eprintf("Failed to extract %s.%s: %s\n", tag.get_path().data(), tag_extension, e.what());
            return;
        }

        // Create directories along the way
        std::filesystem::create_directories(tag_path_to_write_to.parent_path());

        // Save it
        auto tag_path_str = tag_path_to_write_to.string();
        if(!Invader::File::save_file(tag_path_str.data(), new_tag)) {
            eprintf("Failed to save extracted tag to %s\n", tag_path_str.data());
            std::exit(1);
        }

        extracted_tags[tag_index] = true;
    };

    // Extract individual tags?
    if(extract_options.tags_to_extract.size() != 0) {
        std::vector<std::size_t> all_tags_to_extract;
        for(auto &tag : extract_options.tags_to_extract) {
            // Find the dot
            char *dot = nullptr;
            for(char &c : tag) {
                if(c == '.') {
                    dot = &c;
                }
            }
            if(dot == nullptr) {
                eprintf("Tag is missing an extension: %s\n", tag.data());
                return EXIT_FAILURE;
            }
            *dot = 0;

            // Get the extension
            const char *extension = dot + 1;
            auto tag_class_int = HEK::extension_to_tag_class(extension);
            if(tag_class_int == TagClassInt::TAG_CLASS_NULL) {
                eprintf("Invalid tag class: %s\n", extension);
                return EXIT_FAILURE;
            }

            // Replace forward slashes with backslashes
            File::preferred_path_to_halo_path_chars(tag.data());

            // Get the index
            auto index = map->find_tag(tag.data(), tag_class_int);
            if(!index.has_value()) {
                eprintf("Tag not in cache file: %s.%s\n", tag.data(), extension);
                return EXIT_FAILURE;
            }

            all_tags_to_extract.push_back(*index);
        }

        for(auto &tag : all_tags_to_extract) {
            extract_tag(tag);
        }
    }

    // Extract each tag?
    else {
        for(std::size_t t = 0; t < tag_count; t++) {
            extract_tag(t);
        }
    }
}
