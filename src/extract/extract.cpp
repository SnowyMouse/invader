// SPDX-License-Identifier: GPL-3.0-only

#include <optional>
#include <filesystem>
#include <invader/map/map.hpp>
#include <invader/file/file.hpp>
#include <invader/command_line_option.hpp>
#include <invader/crc/hek/crc.hpp>
#include <invader/version.hpp>
#include <invader/extract/extraction.hpp>

bool string_matches(const char *string, const char *pattern) {
    for(const char *p = pattern;; p++) {
        if(*p == *string && *p == 0) {
            return true;
        }
        else if(*p == '?' || *p == *string) {
            string++;
            continue;
        }
        else if(*p == '*') {
            p++;
            if(*p == 0) {
                return true;
            }
            for(const char *s = string; *s; s++) {
                if(string_matches(s, p)) {
                    return true;
                }
            }
            return false;
        }
        else {
            return false;
        }
    }
    return true;
}

int main(int argc, const char **argv) {
    using namespace Invader;

    // Options struct
    struct ExtractOptions {
        std::string tags_directory = "tags";
        std::string maps_directory = "maps";
        std::vector<std::string> tags_to_extract;
        std::vector<std::string> search_queries;
        bool search_all_tags = true;
        bool recursive = false;
        bool overwrite = false;
        bool continue_extracting = false;
        bool no_external_tags = false;
    } extract_options;

    // Command line options
    std::vector<Invader::CommandLineOption> options;
    options.emplace_back("maps", 'm', 1, "Set the maps directory", "<dir>");
    options.emplace_back("tags", 't', 1, "Set the tags directory", "<dir>");
    //options.emplace_back("recursive", 'r', 0, "Extract tag dependencies");
    options.emplace_back("overwrite", 'O', 0, "Overwrite tags if they already exist");
    options.emplace_back("info", 'i', 0, "Show credits, source info, and other info");
    options.emplace_back("continue", 'c', 0, "Don't stop on error when possible");
    options.emplace_back("no-external-tags", 'n', 0, "Do not extract tags with external data");
    options.emplace_back("search", 's', 1, "Search for tags (* and ? are wildcards); use multiple times for multiple queries", "<expr>");

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
            case 'r':
                extract_options.recursive = true;
                break;
            case 'O':
                extract_options.overwrite = true;
                break;
            case 'c':
                extract_options.continue_extracting = true;
                break;
            case 'n':
                extract_options.no_external_tags = true;
                break;
            case 's':
                extract_options.search_queries.emplace_back(args[0]);
                extract_options.search_all_tags = false;
                break;
            case 'i':
                Invader::show_version_info();
                std::exit(EXIT_SUCCESS);
        }
    });

    auto start = std::chrono::steady_clock::now();
    std::filesystem::path maps_directory(extract_options.maps_directory);
    std::vector<std::byte> loc, bitmaps, sounds;

    // Load asset data
    if(!extract_options.no_external_tags) {
        auto open_map_possibly = [&maps_directory](const char *map) -> std::vector<std::byte> {
            auto potential_map_path = (maps_directory / map).string();
            auto potential_map = Invader::File::open_file(potential_map_path.data());
            if(potential_map.has_value()) {
                return *potential_map;
            }
            else {
                return std::vector<std::byte>();
            }
        };

        loc = open_map_possibly("loc.map");
        bitmaps = open_map_possibly("bitmaps.map");
        sounds = open_map_possibly("sounds.map");
    }

    std::unique_ptr<Map> map;
    try {
        auto file = File::open_file(remaining_arguments[0]).value();
        map = std::make_unique<Map>(Map::map_with_move(std::move(file), std::move(bitmaps), std::move(loc), std::move(sounds)));
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
    auto extract_tag = [&extracted_tags, &map, &tags, &extract_options](std::size_t tag_index) -> bool {
        if(extracted_tags[tag_index]) {
            return false;
        }

        // Get the tag path
        const auto &tag = map->get_tag(tag_index);

        // See if we can extract this
        const char *tag_extension = Invader::HEK::tag_class_to_extension(tag.get_tag_class_int());
        if(!tag.data_is_available()) {
            if(!extract_options.search_all_tags) {
                eprintf("Unable to extract %s.%s due to missing data\n", tag.get_path().data(), tag_extension);
                if(!extract_options.continue_extracting) {
                    eprintf("Use -c to override this.\n");
                    std::exit(1);
                }
            }
            return false;
        }

        auto tag_path_to_write_to = tags / (Invader::File::halo_path_to_preferred_path(tag.get_path()) + "." + tag_extension);
        if(!extract_options.overwrite && std::filesystem::exists(tag_path_to_write_to)) {
            return false;
        }

        // Get the tag data
        std::vector<std::byte> new_tag;
        try {
            new_tag = Invader::Extraction::extract_tag(tag);
        }
        catch (std::exception &e) {
            eprintf("Failed to extract %s.%s: %s\n", tag.get_path().data(), tag_extension, e.what());
            if(!extract_options.continue_extracting) {
                eprintf("Use -c to override this.\n");
                std::exit(1);
            }
            return false;
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
        return true;
    };

    std::vector<std::size_t> all_tags_to_extract;

    // Extract each tag?
    if(extract_options.search_all_tags) {
        for(std::size_t t = 0; t < tag_count; t++) {
            all_tags_to_extract.push_back(t);
        }
    }

    else {

        if(extract_options.tags_to_extract.size() != 0) {
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
        }

        // Regex search
        if(extract_options.search_queries.size() != 0) {
            std::size_t tag_count = map->get_tag_count();
            for(std::size_t t = 0; t < tag_count; t++) {
                // See if we already added
                bool already_added = false;
                for(auto &tag : all_tags_to_extract) {
                    if(tag == t) {
                        already_added = true;
                        break;
                    }
                }
                if(already_added) {
                    continue;
                }

                const auto &tag = map->get_tag(t);
                auto full_tag_path = Invader::File::halo_path_to_preferred_path(tag.get_path()) + "." + HEK::tag_class_to_extension(tag.get_tag_class_int());

                for(auto &query : extract_options.search_queries) {
                    if(string_matches(full_tag_path.data(), query.data())) {
                        all_tags_to_extract.emplace_back(t);
                        break;
                    }
                }
            }
        }

        if(all_tags_to_extract.size() == 0) {
            eprintf("No tags were found with the given search parameter(s).\n");
            return EXIT_FAILURE;
        }
    }

    for(auto &tag : all_tags_to_extract) {
        const auto &tag_map = map->get_tag(tag);
        if(extract_tag(tag)) {
            oprintf("Extracted %s.%s\n", Invader::File::halo_path_to_preferred_path(tag_map.get_path()).data(), HEK::tag_class_to_extension(tag_map.get_tag_class_int()));
        }
        else {
            oprintf("Skipped %s.%s\n", Invader::File::halo_path_to_preferred_path(tag_map.get_path()).data(), HEK::tag_class_to_extension(tag_map.get_tag_class_int()));
        }
    }

    auto end = std::chrono::steady_clock::now();
    oprintf("Extracted in %zu ms\n", std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count());
}
