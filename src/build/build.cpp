/*
 * Invader (c) 2018 Kavawuvi
 *
 * This program is free software under the GNU General Public License v3.0 or later. See LICENSE for more information.
 */

#ifndef NO_OUTPUT
#include <iostream>
#include <chrono>
#include <fstream>
using clock_type = std::chrono::steady_clock;
#endif

#include <vector>
#include <cstring>

#include "build_workload.hpp"
#include "../map/map.hpp"
#include "../tag/compiled_tag.hpp"
#include "../version.hpp"

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

    // Default parameters
    const char *maps = nullptr;
    std::vector<std::string> tags;
    const char *output = nullptr;
    const char *scenario = nullptr;
    const char *last_argument = nullptr;
    const char *index = nullptr;
    bool no_indexed_tags = false;
    bool handled = true;
    bool quiet = false;
    std::uint32_t engine = 0x261;

    // Iterate through arguments
    for(int i = 1; i < argc; i++) {
        if(!handled) {
            if(std::strcmp(last_argument, "--maps") == 0) {
                maps = argv[i];
            }

            else if(std::strcmp(last_argument, "--output") == 0) {
                output = argv[i];
            }

            else if(std::strcmp(last_argument, "--tags") == 0) {
                tags.emplace_back(argv[i]);
            }

            else if(std::strcmp(last_argument, "--with-index") == 0) {
                index = argv[i];
            }

            else if(std::strcmp(last_argument, "--game-engine") == 0) {
                engine = std::strtol(argv[i], nullptr, 10);
            }

            handled = true;
            continue;
        }

        if(handled) {
            if(std::strcmp(argv[i], "--info") == 0 || std::strcmp(argv[i], "-i") == 0) {
                #ifndef NO_OUTPUT
                INVADER_SHOW_INFO
                #endif
                return RETURN_OK;
            }

            else if(std::strcmp(argv[i], "--maps") == 0 || std::strcmp(argv[i], "-m") == 0) {
                last_argument = "--maps";
                handled = false;
            }

            else if(std::strcmp(argv[i], "--tags") == 0 || std::strcmp(argv[i], "-t") == 0) {
                last_argument = "--tags";
                handled = false;
            }

            else if(std::strcmp(argv[i], "--output") == 0 || std::strcmp(argv[i], "-o") == 0) {
                last_argument = "--output";
                handled = false;
            }

            else if(std::strcmp(argv[i], "--with-index") == 0 || std::strcmp(argv[i], "-w") == 0) {
                last_argument = "--with-index";
                handled = false;
            }

            else if(std::strcmp(argv[i], "--game-engine") == 0 || std::strcmp(argv[i], "-g") == 0) {
                last_argument = "--game-engine";
                handled = false;
            }

            else if(std::strcmp(argv[i], "--no-indexed-tags") == 0 || std::strcmp(argv[i], "-n") == 0) {
                no_indexed_tags = true;
                last_argument = "";
            }

            else if(std::strcmp(argv[i], "--quiet") == 0 || std::strcmp(argv[i], "-q") == 0) {
                quiet = true;
                last_argument = "";
            }

            else if(last_argument && std::strcmp(last_argument, "--tags") == 0) {
                tags.emplace_back(argv[i]);
            }

            else if(!scenario) {
                scenario = argv[i];
            }

            else {
                #ifndef NO_OUTPUT
                std::cerr << "Unknown argument: " << argv[i] << "\n";
                #endif
                return RETURN_FAILED_UNKNOWN_ARGUMENT;
            }
        }
    }

    // If no scenario was given, show help
    if(!scenario) {
        #ifndef NO_OUTPUT
        std::cout << "invader-build [scenario] [options]"                                               << std::endl;
        std::cout << "Options:"                                                                         << std::endl;
        std::cout << "  --game-engine <id>                Specify the game engine. By default, Custom"  << std::endl;
        std::cout << "  -g                                Edition (609) is used."                       << std::endl;
        std::cout                                                                                       << std::endl;
        std::cout << "  --info                            Show credits, source info, and other info."   << std::endl;
        std::cout << "  -i"                                                                             << std::endl;
        std::cout                                                                                       << std::endl;
        std::cout << "  --maps <dir>                      Use a specific maps directory."               << std::endl;
        std::cout << "  -m"                                                                             << std::endl;
        std::cout                                                                                       << std::endl;
        std::cout << "  --no-indexed-tags                 Do not index tags. This can speed up build"   << std::endl;
        std::cout << "  -n                                time at the cost of a much larger file size." << std::endl;
        std::cout                                                                                       << std::endl;
        std::cout << "  --output <file>                   Output to a specific file."                   << std::endl;
        std::cout << "  -o"                                                                             << std::endl;
        std::cout                                                                                       << std::endl;
        std::cout << "  --quiet                           Only output error messages."                  << std::endl;
        std::cout << "  -q"                                                                             << std::endl;
        std::cout                                                                                       << std::endl;
        std::cout << "  --tags [dir1] [dir2] [...]        Use the specified tags directory(s). Specify" << std::endl;
        std::cout << "  -t                                directories in order of precedence."          << std::endl;
        std::cout                                                                                       << std::endl;
        std::cout << "  --with-index <file>               Use an index file for the tags, ensuring the" << std::endl;
        std::cout << "  -w                                map's tags are ordered in the same way."      << std::endl;
        std::cout                                                                                       << std::endl;
        #endif
        return RETURN_FAILED_NOTHING_TO_DO;
    }

    // If something is missing, let the user know
    if(!handled) {
        #ifndef NO_OUTPUT
        std::cerr << "Unhandled argument: " << last_argument << "\n";
        #endif
        return RETURN_FAILED_UNHANDLED_ARGUMENT;
    }

    // Use defaults
    if(tags.size() == 0) {
        tags.emplace_back("tags");
    }
    if(!maps || *maps == 0) {
        maps = "maps";
    }

    try {
        // Start benchmark
        #ifndef NO_OUTPUT
        auto start = clock_type::now();
        #endif

        // Get the index
        std::vector<std::tuple<HEK::TagClassInt, std::string>> with_index;
        if(index) {
            std::fstream index_file(index, std::ios_base::in);
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

        auto map = Invader::BuildWorkload::compile_map(scenario, tags, maps, with_index, !no_indexed_tags, !quiet);
        reinterpret_cast<Invader::HEK::CacheFileHeader *>(map.data())->engine = static_cast<Invader::HEK::CacheFileEngine>(engine);

        char *map_name = reinterpret_cast<char *>(map.data()) + 0x20;
        #ifdef _WIN32
        const char *separator = "\\";
        #else
        const char *separator = "/";
        #endif

        // Format path to maps/map_name.map if output not specified
        std::string final_file;
        if(!output) {
            final_file = std::string(maps) + (maps[std::strlen(maps) - 1] != *separator ? separator : "") + map_name + ".map";
        }
        else {
            final_file = output;
        }

        std::FILE *file = std::fopen(final_file.data(), "wb");

        // Check if file is open
        if(!file) {
            #ifndef NO_OUTPUT
            std::cerr << "Failed to open " << final_file << " for writing.\n";
            #endif
            return RETURN_FAILED_FILE_SAVE_ERROR;
        }

        // Write to file
        if(std::fwrite(map.data(), map.size(), 1, file) == 0) {
            #ifndef NO_OUTPUT
            std::cerr << "Failed to save.\n";
            #endif
            return RETURN_FAILED_FILE_SAVE_ERROR;
        }

        std::fclose(file);

        #ifndef NO_OUTPUT
        if(!quiet) {
            std::printf("Time:              %.03f ms\n", std::chrono::duration_cast<std::chrono::microseconds>(clock_type::now() - start).count() / 1000.0);
        }
        #endif

        return RETURN_OK;
    }
    catch(std::exception &exception) {
        #ifndef NO_OUTPUT
        std::cerr << "Failed to compile the map.\n";
        std::cerr << exception.what() << "\n";
        #endif
        return RETURN_FAILED_EXCEPTION_ERROR;
    }
}
