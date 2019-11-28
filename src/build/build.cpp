// SPDX-License-Identifier: GPL-3.0-only

#include <iostream>
#include <chrono>
#include <fstream>
using clock_type = std::chrono::steady_clock;

#include <vector>
#include <cstring>
#include <filesystem>

#include <invader/build/build_workload.hpp>
#include <invader/map/map.hpp>
#include <invader/compress/compression.hpp>
#include <invader/tag/compiled_tag.hpp>
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
        HEK::CacheFileEngine engine = HEK::CacheFileEngine::CACHE_FILE_CUSTOM_EDITION;
        bool no_external_tags = false;
        bool handled = true;
        bool quiet = false;
        bool always_index_tags = false;
        const char *forged_crc = nullptr;
        bool use_filesystem_path = false;
        const char *rename_scenario = nullptr;
        bool compress = false;
    } build_options;

    std::vector<CommandLineOption> options;
    options.emplace_back("no-external-tags", 'n', 0, "Do not use external tags. This can speed up build time at a cost of a much larger file size.");
    options.emplace_back("always-index-tags", 'a', 0, "Always index tags when possible. This can speed up build time, but stock tags can't be modified.");
    options.emplace_back("quiet", 'q', 0, "Only output error messages.");
    options.emplace_back("info", 'i', 0, "Show credits, source info, and other info.");
    options.emplace_back("game-engine", 'g', 1, "Specify the game engine. Valid engines are: custom (default), retail, demo, dark", "<id>");
    options.emplace_back("with-index", 'w', 1, "Use an index file for the tags, ensuring the map's tags are ordered in the same way.", "<file>");
    options.emplace_back("maps", 'm', 1, "Use a specific maps directory.", "<dir>");
    options.emplace_back("tags", 't', 1, "Use the specified tags directory. Use multiple times to add more directories, ordered by precedence.", "<dir>");
    options.emplace_back("output", 'o', 1, "Output to a specific file.", "<file>");
    options.emplace_back("forge-crc", 'C', 1, "Forge the CRC32 value of the map after building it.", "<crc>");
    options.emplace_back("fs-path", 'P', 0, "Use a filesystem path for the tag.");
    options.emplace_back("rename-scenario", 'N', 1, "Rename the scenario.", "<name>");
    options.emplace_back("compress", 'c', 0, "Compress the cache file.");

    static constexpr char DESCRIPTION[] = "Build cache files for Halo Combat Evolved on the PC.";
    static constexpr char USAGE[] = "[options] <scenario>";

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
            case 'C':
                build_options.forged_crc = arguments[0];
                break;
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
        }
    });

    if(ON_COLOR_TERM) {
        if(std::strcmp(build_options.forged_crc, "56617021") == 0) {
            std::fprintf(stdout, "\x1B[38;5;51m");
        }
        else if(std::strcmp(build_options.forged_crc, "43687521") == 0) {
            std::fprintf(stdout, "\x1B[38;5;204m");
        }
    }

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
        // Start benchmark
        auto start = clock_type::now();

        // Get the index
        std::vector<std::tuple<TagClassInt, std::string>> with_index;
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
                    eprintf_error("Invalid index given. \"%s\" is missing an extension.", tag.data());
                    return EXIT_FAILURE;
                }

                auto substr = tag.substr(0, extension - tag.data() - 1);
                File::preferred_path_to_halo_path_chars(substr.data());

                // Lowercase everything
                for(char &c : substr) {
                    c = std::tolower(c);
                }

                with_index.emplace_back(extension_to_tag_class(extension), substr);
            }
        }

        std::uint32_t forged_crc_value = 0;
        std::optional<std::uint32_t> forged_crc;
        if(build_options.forged_crc) {
            std::size_t given_crc32_length = std::strlen(build_options.forged_crc);
            if(given_crc32_length > 8 || given_crc32_length < 1) {
                eprintf_error("Invalid CRC32 %s (must be 1-8 digits)", argv[2]);
                return 1;
            }
            for(std::size_t i = 0; i < given_crc32_length; i++) {
                char c = std::tolower(build_options.forged_crc[i]);
                if(!(c >= '0' && c <= '9') && !(c >= 'a' && c <= 'f')) {
                    eprintf_error("Invalid CRC32 %s (must be hexadecimal)", argv[2]);
                    return 1;
                }
            }
            forged_crc_value = std::strtoul(build_options.forged_crc, nullptr, 16);
            forged_crc = forged_crc_value;
        }

        // Build!
        auto map = Invader::BuildWorkload::compile_map(
            scenario.data(),
            build_options.tags,
            build_options.engine,
            build_options.maps,
            build_options.no_external_tags,
            build_options.always_index_tags,
            !build_options.quiet,
            with_index,
            forged_crc,
            std::nullopt,
            build_options.rename_scenario == nullptr ? std::nullopt : std::optional<std::string>(std::string(build_options.rename_scenario))
        );

        if(build_options.compress) {
            std::size_t size_before = map.size();
            oprintf("Compressing...");
            oflush();
            map = Compression::compress_map_data(map.data(), map.size());
            std::size_t new_size = map.size();
            oprintf("\rCompressed size:   %.02f MiB (%.02f %%)\n", new_size / 1024.0F / 1024.0F, 100.0F * new_size / size_before);
        }

        // Set the map name
        const char *map_name;
        if(build_options.rename_scenario) {
            map_name = build_options.rename_scenario;
        }
        else {
            map_name = File::base_name_chars(scenario.data());
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
            eprintf_error("Failed to open %s for writing.", final_file.data());
            return RETURN_FAILED_FILE_SAVE_ERROR;
        }

        // Write to file
        if(std::fwrite(map.data(), map.size(), 1, file) == 0) {
            eprintf_error("Failed to save.");
            return RETURN_FAILED_FILE_SAVE_ERROR;
        }

        std::fclose(file);

        if(!build_options.quiet) {
            oprintf("Time:              %.03f ms", std::chrono::duration_cast<std::chrono::microseconds>(clock_type::now() - start).count() / 1000.0);
            if(ON_COLOR_TERM) {
                oprintf("\x1B[m\n");
            }
            else {
                oprintf("\n");
            }
        }

        return RETURN_OK;
    }
    catch(std::exception &exception) {
        eprintf_error("Failed to compile the map.");
        eprintf_error("%s", exception.what());
        return RETURN_FAILED_EXCEPTION_ERROR;
    }
}
