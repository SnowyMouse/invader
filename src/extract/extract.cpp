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
#include <invader/compress/ceaflate.hpp>
#include <regex>

int main(int argc, const char **argv) {
    using namespace Invader;

    // Options struct
    struct ExtractOptions {
        std::string tags_directory = "tags";
        std::optional<std::string> maps_directory;
        std::optional<std::string> ipak;
        std::vector<std::string> tags_to_extract;
        std::vector<std::string> search_queries;
        bool search_all_tags = true;
        bool recursive = false;
        bool overwrite = false;
        bool non_mp_globals = false;
    } extract_options;

    // Command line options
    std::vector<Invader::CommandLineOption> options;
    options.emplace_back("maps", 'm', 1, "Use the specified maps directory.", "<dir>");
    options.emplace_back("tags", 't', 1, "Use the specified tags directory.", "<dir>");
    options.emplace_back("recursive", 'r', 0, "Extract tag dependencies");
    options.emplace_back("overwrite", 'O', 0, "Overwrite tags if they already exist");
    options.emplace_back("info", 'i', 0, "Show credits, source info, and other info");
    options.emplace_back("search", 's', 1, "Search for tags (* and ? are wildcards); use multiple times for multiple queries", "<expr>");
    options.emplace_back("non-mp-globals", 'n', 0, "Enable extraction of non-multiplayer .globals");
    options.emplace_back("use-ipak", 'p', 1, "Use the inplace1.ipak file", "<ipak>");

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
            case 'p':
                extract_options.ipak = args[0];
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
            eprintf_error("Directory %s was not found or is not a directory", extract_options.tags_directory.c_str());
        }
        return EXIT_FAILURE;
    }

    std::vector<std::byte> loc, bitmaps, sounds, ipak;

    // Find the asset data
    if(extract_options.ipak.has_value()) {
        using namespace Compression::Ceaflate;
        auto potential_file = Invader::File::open_file(extract_options.ipak->c_str());
        if(!potential_file.has_value()) {
            eprintf_error("Failed to open %s", extract_options.ipak->c_str());
            return EXIT_FAILURE;
        }
        ipak = *potential_file;
    }
    else {
        if(!extract_options.maps_directory.has_value()) {
            std::filesystem::path map = std::string(remaining_arguments[0]);
            auto maps_folder = std::filesystem::absolute(map).parent_path();
            if(std::filesystem::is_directory(maps_folder)) {
                extract_options.maps_directory = maps_folder.string();
            }
        }
        if(extract_options.maps_directory.has_value()) {
            std::filesystem::path maps_directory(*extract_options.maps_directory);
            auto open_map_possibly = [&maps_directory](const char *map, const char *map_alt, auto &open_map_possibly) -> std::vector<std::byte> {
                auto potential_map_path = (maps_directory / map).string();
                auto potential_map = Invader::File::open_file(potential_map_path.c_str());
                if(potential_map.has_value()) {
                    return *potential_map;
                }
                else if(map_alt) {
                    return open_map_possibly(map_alt, nullptr, open_map_possibly);
                }
                else {
                    return std::vector<std::byte>();
                }
            };

            // Get its header
            Invader::HEK::CacheFileHeader header;
            std::FILE *f = std::fopen(remaining_arguments[0], "rb");
            if(!f) {
                eprintf_error("Failed to open %s to determine its version", remaining_arguments[0]);
                return EXIT_FAILURE;
            }
            if(!std::fread(&header, sizeof(header), 1, f)) {
                eprintf_error("Failed to read %s to determine its version", remaining_arguments[0]);
                std::fclose(f);
                return EXIT_FAILURE;
            }
            std::fclose(f);

            // Check if we can do things to it
            if(header.valid()) {
                switch(header.engine.read()) {
                    case HEK::CACHE_FILE_DEMO:
                    case HEK::CACHE_FILE_DEMO_COMPRESSED:
                    case HEK::CACHE_FILE_RETAIL:
                    case HEK::CACHE_FILE_RETAIL_COMPRESSED:
                        bitmaps = open_map_possibly("bitmaps.map", nullptr, open_map_possibly);
                        sounds = open_map_possibly("sounds.map", nullptr, open_map_possibly);
                        break;
                    case HEK::CACHE_FILE_CUSTOM_EDITION:
                    case HEK::CACHE_FILE_CUSTOM_EDITION_COMPRESSED: {
                        loc = open_map_possibly("custom_loc.map", "loc.map", open_map_possibly);
                        bitmaps = open_map_possibly("custom_bitmaps.map", "bitmaps.map", open_map_possibly);
                        sounds = open_map_possibly("custom_sounds.map", "sounds.map", open_map_possibly);
                        break;
                    }
                    default:
                        break; // nothing else gets resource maps
                }
            }
            // Maybe it's a demo map?
            else if(reinterpret_cast<Invader::HEK::CacheFileDemoHeader *>(&header)->valid()) {
                bitmaps = open_map_possibly("bitmaps.map", nullptr, open_map_possibly);
                sounds = open_map_possibly("sounds.map", nullptr, open_map_possibly);
            }
            // I have no idea what it is
            else {
                eprintf_error("Failed to parse %s's header to determine its version", remaining_arguments[0]);
                return EXIT_FAILURE;
            }
        }
    }

    // Load map
    std::unique_ptr<Map> map;
    try {
        auto file = File::open_file(remaining_arguments[0]).value();
        map = std::make_unique<Map>(Map::map_with_move(std::move(file), std::move(bitmaps), std::move(loc), std::move(sounds), std::move(ipak)));
    }
    catch (std::exception &e) {
        eprintf_error("Failed to parse %s: %s", remaining_arguments[0], e.what());
        return EXIT_FAILURE;
    }

    ExtractionWorkload::extract_map(*map, extract_options.tags_directory, extract_options.search_queries, extract_options.recursive, extract_options.overwrite, extract_options.non_mp_globals);
}
