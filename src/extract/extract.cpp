// SPDX-License-Identifier: GPL-3.0-only

#include <optional>
#include <filesystem>
#include <invader/map/map.hpp>
#include <invader/file/file.hpp>
#include <invader/command_line_option.hpp>
#include <invader/crc/hek/crc.hpp>
#include <invader/version.hpp>
#include <invader/extract/extraction.hpp>
#include <invader/build/build_workload.hpp>
#include <invader/tag/parser/parser.hpp>
#include <regex>

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
        std::optional<std::string> maps_directory;
        std::vector<std::string> tags_to_extract;
        std::vector<std::string> search_queries;
        bool search_all_tags = true;
        bool recursive = false;
        bool overwrite = false;
        bool non_mp_globals = false;
    } extract_options;

    // Command line options
    std::vector<Invader::CommandLineOption> options;
    options.emplace_back("maps", 'm', 1, "Set the maps directory", "<dir>");
    options.emplace_back("tags", 't', 1, "Set the tags directory", "<dir>");
    options.emplace_back("recursive", 'r', 0, "Extract tag dependencies");
    options.emplace_back("overwrite", 'O', 0, "Overwrite tags if they already exist");
    options.emplace_back("info", 'i', 0, "Show credits, source info, and other info");
    options.emplace_back("search", 's', 1, "Search for tags (* and ? are wildcards); use multiple times for multiple queries", "<expr>");
    options.emplace_back("non-mp-globals", 'n', 0, "Enable extraction of non-multiplayer .globals");

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
            case 'n':
                extract_options.non_mp_globals = true;
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

    // Check if the tags directory exists
    std::filesystem::path tags(extract_options.tags_directory);
    if(!std::filesystem::is_directory(tags)) {
        if(extract_options.tags_directory == "tags") {
            eprintf_error("No tags directory was given, and \"tags\" was not found or is not a directory.");
        }
        else {
            eprintf_error("Directory %s was not found or is not a directory", extract_options.tags_directory.data());
        }
        return EXIT_FAILURE;
    }

    auto start = std::chrono::steady_clock::now();
    std::vector<std::byte> loc, bitmaps, sounds;

    // Load asset data
    if(extract_options.maps_directory.has_value()) {
        std::filesystem::path maps_directory(*extract_options.maps_directory);
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

    // Load map
    std::unique_ptr<Map> map;
    try {
        auto file = File::open_file(remaining_arguments[0]).value();
        map = std::make_unique<Map>(Map::map_with_move(std::move(file), std::move(bitmaps), std::move(loc), std::move(sounds)));
    }
    catch (std::exception &e) {
        eprintf_error("Failed to parse %s: %s", remaining_arguments[0], e.what());
        return EXIT_FAILURE;
    }

    // Warn if needed
    auto &header = map->get_cache_file_header();
    if(extract_options.maps_directory.has_value()) {
        using namespace Invader::HEK;
        switch(header.engine) {
            case CacheFileEngine::CACHE_FILE_CUSTOM_EDITION:
                if(loc.size() == 0) {
                    eprintf_error("Failed to find a loc.map");
                }
                // fallthrough
            case CacheFileEngine::CACHE_FILE_RETAIL:
            case CacheFileEngine::CACHE_FILE_DEMO:
                if(bitmaps.size() == 0) {
                    eprintf_error("Failed to find a bitmaps.map");
                }
                if(sounds.size() == 0) {
                    eprintf_error("Failed to find a sounds.map");
                }
                break;
            default:
                break;
        }
    }

    // Already extracted tags (so we don't need to re-extract them)
    auto tag_count = map->get_tag_count();
    std::vector<bool> extracted_tags(tag_count);

    // Also here's the tags directory
    std::vector<std::size_t> all_tags_to_extract;

    auto extract_tag = [&extracted_tags, &map, &tags, &extract_options, &all_tags_to_extract, &header](std::size_t tag_index) -> bool {
        // Used for bad paths
        static const std::regex BAD_PATH_DIRECTORY("(^|.*(\\\\|\\/))\\.{1,2}(\\\\|\\/).*");

        // Do it
        extracted_tags[tag_index] = true;

        // Get the tag path
        const auto &tag = map->get_tag(tag_index);

        // See if we can extract this
        auto tag_class_int = tag.get_tag_class_int();
        const char *tag_extension = Invader::HEK::tag_class_to_extension(tag_class_int);
        if(!tag.data_is_available()) {
            return false;
        }

        // Get the path
        auto path = Invader::File::halo_path_to_preferred_path(tag.get_path());

        // Lowercase everything
        for(char &c : path) {
            c = std::tolower(c);
        }

        // Make sure we don't have any dot slashes (i.e. ../ or ./) to prevent a potential directory traversal attack
        if(std::regex_match(path, BAD_PATH_DIRECTORY)) {
            eprintf_error("Error: %s.%s contains an unsafe path", path.data(), tag_extension);
            return false;
        }

        // Figure out the path we're writing to
        auto tag_path_to_write_to = tags / (path + "." + tag_extension);
        if(!extract_options.overwrite && std::filesystem::exists(tag_path_to_write_to)) {
            return false;
        }

        // Skip globals
        if(tag_class_int == Invader::TagClassInt::TAG_CLASS_GLOBALS && !extract_options.non_mp_globals && header.map_type != Invader::HEK::CacheFileType::CACHE_FILE_MULTIPLAYER) {
            eprintf_warn("Skipping the non-multiplayer map's globals tag");
            return false;
        }

        // Get the tag data
        std::vector<std::byte> new_tag;
        try {
            new_tag = Invader::Extraction::extract_tag(tag);

            // If we're recursive, we want to also get that stuff, too
            if(extract_options.recursive) {
                auto tag_compiled = Invader::BuildWorkload::compile_single_tag(new_tag.data(), new_tag.size(), std::vector<std::string>(), false);
                std::vector<std::pair<const std::string *, Invader::TagClassInt>> dependencies;
                for(auto &s : tag_compiled.structs) {
                    for(auto &d : s.dependencies) {
                        auto &tag = tag_compiled.tags[d.tag_index];
                        dependencies.emplace_back(&tag.path, tag.tag_class_int);
                    }
                }
                for(auto &d : dependencies) {
                    auto tag_index = map->find_tag(d.first->data(), d.second);
                    if(tag_index.has_value() && extracted_tags[*tag_index] == false) {
                        all_tags_to_extract.push_back(*tag_index);
                    }
                }
            }
        }
        catch (std::exception &e) {
            eprintf_error("Error: Failed to extract %s.%s: %s", Invader::File::halo_path_to_preferred_path(tag.get_path()).data(), tag_extension, e.what());
            return false;
        }

        // Jason Jones the tag
        if(header.map_type == Invader::HEK::CacheFileType::CACHE_FILE_SINGLEPLAYER) {
            switch(tag_class_int) {
                case Invader::TagClassInt::TAG_CLASS_WEAPON: {
                    if(path == "weapons\\pistol\\pistol") {
                        auto parsed = Invader::Parser::Weapon::parse_hek_tag_file(new_tag.data(), new_tag.size());
                        if(parsed.triggers.size() >= 1) {
                            auto &first_trigger = parsed.triggers[0];
                            first_trigger.minimum_error = DEGREES_TO_RADIANS(0.0F);
                            first_trigger.error_angle.from = DEGREES_TO_RADIANS(0.2F);
                            first_trigger.error_angle.to = DEGREES_TO_RADIANS(2.0F);
                        }
                        new_tag = parsed.generate_hek_tag_data(TagClassInt::TAG_CLASS_WEAPON);
                    }
                    else if(path == "weapons\\plasma rifle\\plasma rifle") {
                        auto parsed = Invader::Parser::Weapon::parse_hek_tag_file(new_tag.data(), new_tag.size());
                        if(parsed.triggers.size() >= 1) {
                            auto &first_trigger = parsed.triggers[0];
                            first_trigger.error_angle.from = DEGREES_TO_RADIANS(0.5F);
                            first_trigger.error_angle.to = DEGREES_TO_RADIANS(5.0F);
                        }
                        new_tag = parsed.generate_hek_tag_data(TagClassInt::TAG_CLASS_WEAPON);
                    }
                    break;
                }
                case Invader::TagClassInt::TAG_CLASS_DAMAGE_EFFECT:
                    if(path == "weapons\\pistol\\bullet") {
                        auto parsed = Invader::Parser::DamageEffect::parse_hek_tag_file(new_tag.data(), new_tag.size());
                        parsed.elite_energy_shield = 1.0F;
                        new_tag = parsed.generate_hek_tag_data(TagClassInt::TAG_CLASS_DAMAGE_EFFECT);
                    }
                    break;
                default:
                    break;
            }
        }

        // Create directories along the way
        std::filesystem::create_directories(tag_path_to_write_to.parent_path());

        // Save it
        auto tag_path_str = tag_path_to_write_to.string();
        if(!Invader::File::save_file(tag_path_str.data(), new_tag)) {
            eprintf_error("Error: Failed to save %s", tag_path_str.data());
            return false;
        }

        return true;
    };

    // Extract each tag?
    if(extract_options.search_all_tags) {
        for(std::size_t t = 0; t < tag_count; t++) {
            all_tags_to_extract.push_back(t);
        }
    }

    else {
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

        if(all_tags_to_extract.size() == 0) {
            eprintf_error("No tags were found with the given search parameter(s).");
            return EXIT_FAILURE;
        }
    }

    // Extract tags
    std::size_t total = 0;
    std::size_t extracted = 0;
    while(all_tags_to_extract.size() > 0) {
        std::size_t tag = all_tags_to_extract[0];
        all_tags_to_extract.erase(all_tags_to_extract.begin());
        if(extracted_tags[tag]) {
            continue;
        }
        total++;
        const auto &tag_map = map->get_tag(tag);
        if(extract_tag(tag)) {
            eprintf_success("Extracted %s.%s", Invader::File::halo_path_to_preferred_path(tag_map.get_path()).data(), HEK::tag_class_to_extension(tag_map.get_tag_class_int()));
            extracted++;
        }
        else {
            oprintf("Skipped %s.%s\n", Invader::File::halo_path_to_preferred_path(tag_map.get_path()).data(), HEK::tag_class_to_extension(tag_map.get_tag_class_int()));
        }
    }

    auto end = std::chrono::steady_clock::now();
    oprintf("Extracted %zu / %zu tags in %zu ms\n", extracted, total, std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count());
}
