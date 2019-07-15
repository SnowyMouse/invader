/*
 * Invader (c) 2019 Kavawuvi
 *
 * This program is free software under the GNU General Public License v3.0 or later. See LICENSE for more information.
 */

#include <cstdio>
#include <cstdlib>
#include <getopt.h>
#include "../version.hpp"
#include "hek/resource_map.hpp"

int main(int argc, char *argv[]) {
    using namespace Invader::HEK;

    // Long options
    int longindex = 0;
    static struct option options[] = {
        {"info", no_argument, 0, 'i'},
        {"help", no_argument, 0, 'h'},
        {"type", required_argument, 0, 'T' },
        {"tags", required_argument, 0, 't' },
        {"maps", required_argument, 0, 'm' },
        {0, 0, 0, 0 }
    };

    // Tags directory
    const char *tags = "tags/";

    // Maps directory
    const char *maps = "maps/";

    // Resource map type
    ResourceMapType type = ResourceMapType::RESOURCE_MAP_BITMAP;
    bool resource_map_set = false;

    int opt;

    // Go through each argument
    while((opt = getopt_long(argc, argv, "hit:T:m:", options, &longindex)) != -1) {
        switch(opt) {
            case 'i':
                INVADER_SHOW_INFO
                return EXIT_FAILURE;

            case 't':
                tags = optarg;
                break;

            case 'm':
                maps = optarg;
                break;

            case 'T':
                if(std::strcmp(optarg, "bitmaps") == 0) {
                    type = ResourceMapType::RESOURCE_MAP_BITMAP;
                }
                else if(std::strcmp(optarg, "sounds") == 0) {
                    type = ResourceMapType::RESOURCE_MAP_SOUND;
                }
                else if(std::strcmp(optarg, "loc") == 0) {
                    type = ResourceMapType::RESOURCE_MAP_LOC;
                }
                else {
                    eprintf("Invalid type %s. Use --help for more information.\n", optarg);
                    return EXIT_FAILURE;
                }
                resource_map_set = true;
                break;

            default:
                eprintf("Usage: %s <options>\n\n", *argv);
                eprintf("Create or modify a bitmap tag.\n\n");
                eprintf("Options:\n");
                eprintf("    --info,-i                  Show license and credits.\n");
                eprintf("    --help,-h                  Show help\n\n");
                eprintf("Directory options:\n");
                eprintf("    --maps,-m <path>           Set the maps directory.\n");
                eprintf("    --tags,-t <path>           Set the tags directory. Use multiple times to use\n");
                eprintf("                               multiple directories in order of precedence.\n\n");
                eprintf("Resource options:\n");
                eprintf("    --type,-T <type>           Set the resource map. This option is required for\n");
                eprintf("                               creating maps. Can be: bitmaps, sounds, or loc.\n\n");
                return EXIT_FAILURE;
        }
    }

    if(!resource_map_set) {
        eprintf("No resource map type was given. Use --help for more information.\n");
        return EXIT_FAILURE;
    }
}
