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

#include <getopt.h>

#include "build_workload.hpp"
#include "../map/map.hpp"
#include "../tag/compiled_tag.hpp"
#include "../version.hpp"
#include "../eprintf.hpp"

enum ReturnValue : int {
    RETURN_OK = 0,
    RETURN_FAILED_NOTHING_TO_DO = 1,
    RETURN_FAILED_UNKNOWN_ARGUMENT = 2,
    RETURN_FAILED_UNHANDLED_ARGUMENT = 3,
    RETURN_FAILED_FILE_SAVE_ERROR = 4,
    RETURN_FAILED_EXCEPTION_ERROR = 5
};

int main(int argc, char * const argv[]) {
    using namespace Invader;
    using namespace Invader::HEK;

    // Default parameters
    std::string maps = "maps";
    std::vector<std::string> tags;
    std::string output;
    std::string scenario;
    std::string last_argument;
    std::string index;
    std::string engine = "ce";
    bool no_indexed_tags = false;
    bool handled = true;
    bool quiet = false;
    bool always_index_tags = false;

    int opt;
    int longindex = 0;
    static struct option options[] = {
        {"help",  no_argument, 0, 'h'},
        {"no-indexed-tags", no_argument, 0, 'n' },
        {"always-index-tags", no_argument, 0, 'a' },
        {"quiet", no_argument, 0, 'q' },
        {"info", no_argument, 0, 'i' },
        {"game-engine",  required_argument, 0, 'g' },
        {"with-index",  required_argument, 0, 'w' },
        {"maps", required_argument, 0, 'm' },
        {"tags", required_argument, 0, 't' },
        {"output", required_argument, 0, 'o' },
        {0, 0, 0, 0 }
    };

    // Go through every argument
    while((opt = getopt_long(argc, argv, "naqhiw:m:t:o:g:", options, &longindex)) != -1) {
        switch(opt) {
            case 'n':
                no_indexed_tags = true;
                break;
            case 'q':
                quiet = true;
                break;
            case 'w':
                index = std::string(optarg);
                break;
            case 't':
                tags.emplace_back(optarg);
                break;
            case 'o':
                output = std::string(optarg);
                break;
            case 'm':
                maps = std::string(optarg);
                break;
            case 'a':
                always_index_tags = true;
                break;
            case 'g':
                engine = std::string(optarg);
                if(engine != "ce" && engine != "retail") {
                    eprintf("%s: Invalid engine. Expected ce or retail.\n", argv[0]);
                    return EXIT_FAILURE;
                }
                // Implicitly -n them
                if(engine == "retail") {
                    no_indexed_tags = true;
                }
                break;
            case 'i':
                INVADER_SHOW_INFO
                return EXIT_FAILURE;
            default:
                eprintf("Usage: %s [options] <scenario>\n", argv[0]);
                eprintf("Options:\n");
                eprintf("  --game-engine <id>   Specify the game engine. Valid engines are: ce (default),\n");
                eprintf("  -g                   retail\n\n");
                eprintf("  --info               Show credits, source info, and other info.\n");
                eprintf("  -i\n\n");
                eprintf("  --maps <dir>         Use a specific maps directory.\n");
                eprintf("  -m\n\n");
                eprintf("  --no-indexed-tags    Do not index tags. This can speed up build time at the\n");
                eprintf("  -n                   cost of a much larger file size.\n\n");
                eprintf("  --always-index-tags  Always index tags when possible. This can speed up build\n");
                eprintf("  -a                   time, but modified stock bitmap tags may not apply.\n\n");
                eprintf("  --output <file>      Output to a specific file.\n");
                eprintf("  -o\n\n");
                eprintf("  --quiet              Only output error messages.\n");
                eprintf("  -q\n\n");
                eprintf("  --tags <dir>         Use the specified tags directory. Use multiple times to\n");
                eprintf("  -t                   add more directories, ordered by precedence.\n\n");
                eprintf("  --with-index <file>  Use an index file for the tags, ensuring the map's tags\n");
                eprintf("  -w                   are ordered in the same way.\n");
                return EXIT_FAILURE;
        }
    }

    if(always_index_tags && no_indexed_tags) {
        eprintf("%s: --no-index-tags conflicts with --always-index-tags.\n", argv[0]);
        return EXIT_FAILURE;
    }

    // Check if there's a scenario tag
    if(optind == argc) {
        eprintf("%s: A scenario tag path is required. Use -h for help.\n", argv[0]);
        return EXIT_FAILURE;
    }
    else if(optind < argc - 1) {
        eprintf("%s: Unexpected argument %s\n", argv[0], argv[optind + 1]);
        return EXIT_FAILURE;
    }
    else {
        scenario = argv[optind];
    }

    // If something is missing, let the user know
    if(!handled) {
        #ifndef NO_OUTPUT
        std::cerr << "Unhandled argument: " << last_argument << "\n";
        #endif
        return RETURN_FAILED_UNHANDLED_ARGUMENT;
    }

    // By default, just use tags
    if(tags.size() == 0) {
        tags.emplace_back("tags");
    }

    try {
        // Start benchmark
        #ifndef NO_OUTPUT
        auto start = clock_type::now();
        #endif

        // Get the index
        std::vector<std::tuple<HEK::TagClassInt, std::string>> with_index;
        if(index.size()) {
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

        auto map = Invader::BuildWorkload::compile_map(scenario.data(), tags, maps, with_index, no_indexed_tags, always_index_tags, !quiet);

        if(engine == "retail") {
            reinterpret_cast<Invader::HEK::CacheFileHeader *>(map.data())->engine = Invader::HEK::CACHE_FILE_RETAIL;
        }

        char *map_name = reinterpret_cast<char *>(map.data()) + 0x20;
        #ifdef _WIN32
        const char *separator = "\\";
        #else
        const char *separator = "/";
        #endif

        // Format path to maps/map_name.map if output not specified
        std::string final_file;
        if(!output.size()) {
            final_file = std::string(maps) + (maps[maps.size() - 1] != *separator ? separator : "") + map_name + ".map";
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
