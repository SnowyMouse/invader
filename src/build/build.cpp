// SPDX-License-Identifier: GPL-3.0-only

#include <iostream>
#include <chrono>
#include <fstream>
using clock_type = std::chrono::steady_clock;

#include <vector>
#include <cstring>
#include <filesystem>

#include "invader/build/build_workload.hpp"
#include "invader/map/map.hpp"
#include "invader/tag/compiled_tag.hpp"
#include "invader/version.hpp"
#include "invader/printf.hpp"
#include "invader/command_line_option.hpp"
#include "invader/file/file.hpp"

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
        const char *path;
        std::string maps = "maps";
        std::vector<std::string> tags;
        std::string output;
        std::string last_argument;
        std::string index;
        HEK::CacheFileEngine engine = HEK::CacheFileEngine::CACHE_FILE_CUSTOM_EDITION;
        bool no_external_tags = false;
        bool handled = true;
        bool quiet = false;
        bool always_index_tags = false;
        const char *forged_crc = nullptr;
        bool use_filesystem_path = false;
    } build_options;

    build_options.path = argv[0];

    std::vector<CommandLineOption> options;
    options.emplace_back("help", 'h', 0);
    options.emplace_back("no-external-tags", 'n', 0);
    options.emplace_back("always-index-tags", 'a', 0);
    options.emplace_back("quiet", 'q', 0);
    options.emplace_back("info", 'i', 0);
    options.emplace_back("game-engine", 'g', 1);
    options.emplace_back("with-index", 'w', 1);
    options.emplace_back("maps", 'm', 1);
    options.emplace_back("tags", 't', 1);
    options.emplace_back("output", 'o', 1);
    options.emplace_back("forge-crc", 'c', 1);
    options.emplace_back("fs-path", 'P', 0);

    auto remaining_arguments = CommandLineOption::parse_arguments<BuildOptions &>(argc, argv, options, 'h', build_options, [](char opt, const auto &arguments, BuildOptions &build_options) {
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
                break;
            case 'c':
                build_options.forged_crc = arguments[0];
                break;
            case 'P':
                build_options.use_filesystem_path = true;
                break;
            case 'i':
                show_version_info();
                std::exit(EXIT_FAILURE);
            default:
                eprintf("Usage: %s [options] <scenario>\n\n", build_options.path);
                eprintf("Build cache files for Halo Custom Edition.\n\n");
                eprintf("Options:\n");
                eprintf("  --info,-i                    Show credits, source info, and other info.\n");
                eprintf("  --maps,-m <dir>              Use a specific maps directory.\n");
                eprintf("  --fs-path,-P                 Use a filesystem path for the scenario tag.\n");
                eprintf("  --game-engine,-g <id>        Specify the game engine. Valid engines are:\n");
                eprintf("                               custom (default), retail, demo, dark\n");
                eprintf("  --no-external-tags,-n        Do not use external tags. This can speed up build\n");
                eprintf("                               time at the cost of a much larger file size.\n");
                eprintf("  --always-index-tags,-a       Always index tags when possible. This can speed\n");
                eprintf("                               up build time, but stock tags can't be modified.\n");
                eprintf("  --forge-crc,-c <crc>         Forge the CRC32 value of a map. This is useful\n");
                eprintf("                               for multiplayer.\n");
                eprintf("  --output,-o <file>           Output to a specific file.\n");
                eprintf("  --quiet,-q                   Only output error messages.\n");
                eprintf("  --tags,-t <dir>              Use the specified tags directory. Use multiple\n");
                eprintf("                               times to add more directories, ordered by\n");
                eprintf("                               precedence.\n");
                eprintf("  --with-index,-w <file>       Use an index file for the tags, ensuring the\n");
                eprintf("                               map's tags are ordered in the same way.\n\n");
                std::exit(EXIT_FAILURE);
        }
    });

    if(build_options.always_index_tags && build_options.no_external_tags) {
        eprintf("%s: --no-index-tags conflicts with --always-index-tags.\n", argv[0]);
        return EXIT_FAILURE;
    }

    std::string scenario;

    // By default, just use tags
    if(build_options.tags.size() == 0) {
        build_options.tags.emplace_back("tags");
    }

    // Check if there's a scenario tag
    if(remaining_arguments.size() == 0) {
        eprintf("A scenario tag path is required. Use -h for help.\n");
        return RETURN_FAILED_NOTHING_TO_DO;
    }
    else if(remaining_arguments.size() > 1) {
        eprintf("Unexpected argument %s\n", remaining_arguments[1]);
        return RETURN_FAILED_UNHANDLED_ARGUMENT;
    }
    else if(build_options.use_filesystem_path) {
        auto scenario_maybe = Invader::File::file_path_to_tag_path_with_extension(remaining_arguments[0], build_options.tags, ".scenario");
        if(scenario_maybe.has_value()) {
            scenario = scenario_maybe.value();
        }
        else {
            eprintf("Failed to find a valid tag %s in the tags directory\n", remaining_arguments[0]);
            return RETURN_FAILED_UNHANDLED_ARGUMENT;
        }
    }
    else {
        scenario = File::halo_path_to_preferred_path(remaining_arguments[0]);
    }

    try {
        // Start benchmark
        auto start = clock_type::now();

        // Get the index
        std::vector<std::tuple<HEK::TagClassInt, std::string>> with_index;
        if(build_options.index.size()) {
            std::fstream index_file(build_options.index, std::ios_base::in);
            std::size_t tag_count;

            // Get the tag count
            index_file >> tag_count;

            // Go through and do stuff
            with_index.reserve(tag_count);
            for(std::size_t i = 0; i < tag_count; i++) {
                std::size_t tag_class;
                std::string tag_path;

                index_file >> tag_class;
                index_file.ignore();
                std::getline(index_file, tag_path);

                with_index.emplace_back(static_cast<HEK::TagClassInt>(tag_class), tag_path);
            }
        }

        std::uint32_t forged_crc_value = 0;
        std::optional<std::uint32_t> forged_crc;
        if(build_options.forged_crc) {
            std::size_t given_crc32_length = std::strlen(build_options.forged_crc);
            if(given_crc32_length > 8 || given_crc32_length < 1) {
                eprintf("Invalid CRC32 %s (must be 1-8 digits)\n", argv[2]);
                return 1;
            }
            for(std::size_t i = 0; i < given_crc32_length; i++) {
                char c = std::tolower(build_options.forged_crc[i]);
                if(!(c >= '0' && c <= '9') && !(c >= 'a' && c <= 'f')) {
                    eprintf("Invalid CRC32 %s (must be hexadecimal)\n", argv[2]);
                    return 1;
                }
            }
            forged_crc_value = std::strtoul(build_options.forged_crc, nullptr, 16);
            forged_crc = forged_crc_value;
        }

        auto map = Invader::BuildWorkload::compile_map(scenario.data(), build_options.tags, build_options.engine, build_options.maps, with_index, build_options.no_external_tags, build_options.always_index_tags, !build_options.quiet, forged_crc);

        const char *map_name = scenario.data();
        for(const char &c : scenario) {
            if(c == std::filesystem::path::preferred_separator) {
                map_name = &c + 1;
            }
        }

        // Format path to maps/map_name.map if output not specified
        std::string final_file;
        if(!build_options.output.size()) {
            final_file = (std::filesystem::path(build_options.maps) / (std::string(map_name) + ".map")).string();
        }
        else {
            final_file = build_options.output;
        }

        std::FILE *file = std::fopen(final_file.data(), "wb");

        // Check if file is open
        if(!file) {
            eprintf("Failed to open %s for writing.\n", final_file.data());
            return RETURN_FAILED_FILE_SAVE_ERROR;
        }

        // Write to file
        if(std::fwrite(map.data(), map.size(), 1, file) == 0) {
            eprintf("Failed to save.\n");
            return RETURN_FAILED_FILE_SAVE_ERROR;
        }

        std::fclose(file);

        if(!build_options.quiet) {
            oprintf("Time:              %.03f ms\n", std::chrono::duration_cast<std::chrono::microseconds>(clock_type::now() - start).count() / 1000.0);
        }

        return RETURN_OK;
    }
    catch(std::exception &exception) {
        eprintf("Failed to compile the map.\n");
        eprintf("%s\n", exception.what());
        return RETURN_FAILED_EXCEPTION_ERROR;
    }
}
