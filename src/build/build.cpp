// SPDX-License-Identifier: GPL-3.0-only

#include <iostream>
#include <fstream>
#include <vector>
#include <cstring>
#include <filesystem>

#include <invader/build/build_workload.hpp>
#include <invader/map/map.hpp>
#include <invader/version.hpp>
#include <invader/printf.hpp>
#include <invader/command_line_option.hpp>
#include <invader/file/file.hpp>

enum ReturnValue : int {
    RETURN_OK = 0,
    RETURN_FAILED_NOTHING_TO_DO = 1,
    RETURN_FAILED_UNKNOWN_ARGUMENT = 2,
    RETURN_FAILED_UNHANDLED_ARGUMENT = 3,
    RETURN_FAILED_FILE_SAVE_ERROR = 4,
    RETURN_FAILED_EXCEPTION_ERROR = 5
};

int main(int argc, const char **argv) {
    using namespace Invader;
    using namespace Invader::HEK;

    // Parameters
    struct BuildOptions {
        std::string maps = "maps";
        std::vector<std::string> tags;
        std::string output;
        std::string last_argument;
        std::string index;
        std::optional<HEK::CacheFileEngine> engine;
        bool no_external_tags = false;
        bool handled = true;
        bool quiet = false;
        bool always_index_tags = false;
        std::optional<std::uint32_t> forged_crc;
        bool use_filesystem_path = false;
        const char *rename_scenario = nullptr;
        bool compress = false;
        bool optimize_space = false;
        bool hide_pedantic_warnings = false;
    } build_options;

    std::vector<CommandLineOption> options;
    options.emplace_back("no-external-tags", 'n', 0, "Do not use external tags. This can speed up build time at a cost of a much larger file size.");
    options.emplace_back("always-index-tags", 'a', 0, "Always index tags when possible. This can speed up build time, but stock tags can't be modified.");
    options.emplace_back("quiet", 'q', 0, "Only output error messages.");
    options.emplace_back("info", 'i', 0, "Show credits, source info, and other info.");
    options.emplace_back("game-engine", 'g', 1, "Specify the game engine. This option is required. Valid engines are: custom, demo, retail", "<id>");
    options.emplace_back("with-index", 'w', 1, "Use an index file for the tags, ensuring the map's tags are ordered in the same way.", "<file>");
    options.emplace_back("maps", 'm', 1, "Use a specific maps directory.", "<dir>");
    options.emplace_back("tags", 't', 1, "Use the specified tags directory. Use multiple times to add more directories, ordered by precedence.", "<dir>");
    options.emplace_back("output", 'o', 1, "Output to a specific file.", "<file>");
    options.emplace_back("forge-crc", 'C', 1, "Forge the CRC32 value of the map after building it.", "<crc>");
    options.emplace_back("fs-path", 'P', 0, "Use a filesystem path for the tag.");
    options.emplace_back("rename-scenario", 'N', 1, "Rename the scenario.", "<name>");
    options.emplace_back("compress", 'c', 0, "Compress the cache file.");
    options.emplace_back("optimize", 'O', 0, "Optimize tag space. This will drastically increase the amount of time required to build the cache file.");
    options.emplace_back("hide-pedantic-warnings", 'H', 0, "Don't show minor warnings.");

    static constexpr char DESCRIPTION[] = "Build cache files for Halo Combat Evolved on the PC.";
    static constexpr char USAGE[] = "[options] -g <target> <scenario>";

    auto remaining_arguments = CommandLineOption::parse_arguments<BuildOptions &>(argc, argv, options, USAGE, DESCRIPTION, 1, 1, build_options, [](char opt, const auto &arguments, auto &build_options) {
        switch(opt) {
            case 'n':
                build_options.no_external_tags = true;
                break;
            case 'q':
                build_options.quiet = true;
                break;
            case 'w':
                build_options.index = std::string(arguments[0]);
                break;
            case 't':
                build_options.tags.emplace_back(arguments[0]);
                break;
            case 'o':
                build_options.output = std::string(arguments[0]);
                break;
            case 'm':
                build_options.maps = std::string(arguments[0]);
                break;
            case 'a':
                build_options.always_index_tags = true;
                break;
            case 'g':
                if(std::strcmp(arguments[0], "custom") == 0) {
                    build_options.engine = HEK::CacheFileEngine::CACHE_FILE_CUSTOM_EDITION;
                }
                else if(std::strcmp(arguments[0], "retail") == 0) {
                    build_options.engine = HEK::CacheFileEngine::CACHE_FILE_RETAIL;
                }
                else if(std::strcmp(arguments[0], "demo") == 0) {
                    build_options.engine = HEK::CacheFileEngine::CACHE_FILE_DEMO;
                }
                else if(std::strcmp(arguments[0], "dark") == 0) {
                    build_options.engine = HEK::CacheFileEngine::CACHE_FILE_DARK_CIRCLET;
                }
                else {
                    eprintf_error("Unknown engine type %s.", arguments[0]);
                    std::exit(EXIT_FAILURE);
                }
                break;
            case 'C': {
                std::size_t given_crc32_length = std::strlen(arguments[0]);
                if(given_crc32_length > 8 || given_crc32_length < 1) {
                    eprintf_error("Invalid CRC32 %s (must be 1-8 digits)", arguments[0]);
                    std::exit(EXIT_FAILURE);
                }
                for(std::size_t i = 0; i < given_crc32_length; i++) {
                    char c = std::tolower(arguments[0][i]);
                    if(!(c >= '0' && c <= '9') && !(c >= 'a' && c <= 'f')) {
                        eprintf_error("Invalid CRC32 %s (must be hexadecimal)", arguments[0]);
                        std::exit(EXIT_FAILURE);
                    }
                }
                build_options.forged_crc = static_cast<std::uint32_t>(std::strtoul(arguments[0], nullptr, 16));
                break;
            }
            case 'c':
                build_options.compress = true;
                break;
            case 'P':
                build_options.use_filesystem_path = true;
                break;
            case 'i':
                show_version_info();
                std::exit(EXIT_SUCCESS);
                break;
            case 'N':
                build_options.rename_scenario = arguments[0];
                break;
            case 'O':
                build_options.optimize_space = true;
                break;
            case 'H':
                build_options.hide_pedantic_warnings = true;
                break;
        }
    });

    if(build_options.always_index_tags && build_options.no_external_tags) {
        eprintf_error("%s: --no-index-tags conflicts with --always-index-tags.", argv[0]);
        return EXIT_FAILURE;
    }

    std::string scenario;

    // By default, just use tags
    if(build_options.tags.size() == 0) {
        build_options.tags.emplace_back("tags");
    }

    if(build_options.use_filesystem_path) {
        auto scenario_maybe = Invader::File::file_path_to_tag_path_with_extension(remaining_arguments[0], build_options.tags, ".scenario");
        if(scenario_maybe.has_value()) {
            scenario = scenario_maybe.value();
        }
        else {
            eprintf_error("Failed to find a valid tag %s in the tags directory", remaining_arguments[0]);
            return RETURN_FAILED_UNHANDLED_ARGUMENT;
        }
    }
    else {
        scenario = File::halo_path_to_preferred_path(remaining_arguments[0]);
    }

    try {
        // Get the index
        std::vector<std::pair<TagClassInt, std::string>> with_index;
        if(build_options.index.size()) {
            std::fstream index_file(build_options.index, std::ios_base::in);
            std::string tag;
            while(std::getline(index_file, tag)) {
                // Check if empty
                if(tag.size() == 0) {
                    break;
                }

                // Get the extension
                const char *extension = nullptr;
                for(char &c : tag) {
                    if(c == '.') {
                        extension = &c + 1;
                    }
                }

                if(!extension) {
                    eprintf_error("Invalid index given. \"%s\" is missing an extension.", tag.c_str());
                    return EXIT_FAILURE;
                }

                auto substr = tag.substr(0, extension - tag.c_str() - 1);
                const char *substr_c = substr.c_str();
                std::vector<char> substr_v(substr_c, substr_c + substr.size() + 1);
                File::preferred_path_to_halo_path_chars(substr_v.data());

                // Lowercase everything
                for(char &c : substr) {
                    c = std::tolower(c);
                }

                with_index.emplace_back(extension_to_tag_class(extension), substr_v.data());
            }
        }

        // Figure out our engine target
        if(!build_options.engine.has_value()) {
            eprintf_error("No engine target specified. Use -h for more information.");
            return 1;
        }

        // Build!
        auto map = Invader::BuildWorkload::compile_map(
            scenario.c_str(),
            build_options.tags,
            build_options.engine.value(),
            build_options.maps,
            build_options.no_external_tags,
            build_options.always_index_tags,
            !build_options.quiet,
            with_index,
            build_options.forged_crc,
            std::nullopt,
            build_options.rename_scenario == nullptr ? std::nullopt : std::optional<std::string>(std::string(build_options.rename_scenario)),
            build_options.optimize_space,
            build_options.compress,
            build_options.hide_pedantic_warnings
        );

        // Set the map name
        const char *map_name;
        if(build_options.rename_scenario) {
            map_name = build_options.rename_scenario;
        }
        else {
            map_name = File::base_name_chars(scenario.c_str());
        }

        // Format path to maps/map_name.map if output not specified
        std::string final_file;
        if(!build_options.output.size()) {
            final_file = (std::filesystem::path(build_options.maps) / (std::string(map_name) + ".map")).string();
        }
        else {
            final_file = build_options.output;
        }

        std::FILE *file = std::fopen(final_file.c_str(), "wb");

        // Check if file is open
        if(!file) {
            eprintf_error("Failed to open %s for writing.", final_file.c_str());
            return RETURN_FAILED_FILE_SAVE_ERROR;
        }

        // Write to file
        if(std::fwrite(map.data(), map.size(), 1, file) == 0) {
            eprintf_error("Failed to save.");
            return RETURN_FAILED_FILE_SAVE_ERROR;
        }

        std::fclose(file);

        return RETURN_OK;
    }
    catch(std::exception &exception) {
        eprintf_error("Failed to compile the map.");
        eprintf_error("%s", exception.what());
        return RETURN_FAILED_EXCEPTION_ERROR;
    }
}
