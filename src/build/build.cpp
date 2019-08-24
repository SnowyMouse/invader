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
    const char *forged_crc = nullptr;

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
        {"forge-crc", required_argument, 0, 'c' },
        {0, 0, 0, 0 }
    };

    // Go through every argument
    while((opt = getopt_long(argc, argv, "naqhiw:m:t:o:g:c:", options, &longindex)) != -1) {
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
            case 'c':
                forged_crc = optarg;
                break;
            case 'i':
                INVADER_SHOW_INFO
                return EXIT_FAILURE;
            default:
                eprintf("Usage: %s [options] <scenario>\n\n", argv[0]);
                eprintf("Build cache files for Halo Custom Edition.\n\n");
                eprintf("Options:\n");
                eprintf("  --game-engine,-g <id>        Specify the game engine. Valid engines are: ce,\n");
                eprintf("                               (default), retail\n");
                eprintf("  --info,-i                    Show credits, source info, and other info.\n");
                eprintf("  --maps,-m <dir>              Use a specific maps directory.\n");
                eprintf("  --no-indexed-tags,-n         Do not index tags. This can speed up build time\n");
                eprintf("                               at the cost of a much larger file size.\n");
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
        eprintf("Unhandled argument: %s\n", last_argument.data());
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

        std::uint32_t forged_crc_value = 0;
        std::uint32_t *forged_crc_ptr = nullptr;
        if(forged_crc) {
            std::size_t given_crc32_length = std::strlen(forged_crc);
            if(given_crc32_length > 8 || given_crc32_length < 1) {
                eprintf("Invalid CRC32 %s (must be 1-8 digits)\n", argv[2]);
                return 1;
            }
            for(std::size_t i = 0; i < given_crc32_length; i++) {
                char c = std::tolower(forged_crc[i]);
                if(!(c >= '0' && c <= '9') && !(c >= 'a' && c <= 'f')) {
                    eprintf("Invalid CRC32 %s (must be hexadecimal)\n", argv[2]);
                    return 1;
                }
            }
            forged_crc_value = std::strtoul(forged_crc, nullptr, 16);
            forged_crc_ptr = &forged_crc_value;
        }

        auto map = Invader::BuildWorkload::compile_map(scenario.data(), tags, maps, with_index, no_indexed_tags, always_index_tags, !quiet, forged_crc_ptr);

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
            eprintf("Failed to open %s for writing.\n", final_file.data());
            return RETURN_FAILED_FILE_SAVE_ERROR;
        }

        // Write to file
        if(std::fwrite(map.data(), map.size(), 1, file) == 0) {
            eprintf("Failed to save.\n");
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
        eprintf("Failed to compile the map.\n");
        eprintf("%s\n", exception.what());
        return RETURN_FAILED_EXCEPTION_ERROR;
    }
}
