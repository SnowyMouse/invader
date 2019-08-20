/*
 * Invader (c) 2018 Kavawuvi
 *
 * This program is free software under the GNU General Public License v3.0 or later. See LICENSE for more information.
 */

#include <vector>
#include <string>
#include <getopt.h>
#include <filesystem>
#include <archive.h>
#include <archive_entry.h>
#include "../version.hpp"
#include "../eprintf.hpp"
#include "../tag/compiled_tag.hpp"
#include "found_tag_dependency.hpp"
#include "../build/build_workload.hpp"
#include "../map/map.hpp"

int main(int argc, char * const *argv) {
    static struct option options[] = {
        {"help",  no_argument, 0, 'h'},
        {"info", no_argument, 0, 'i' },
        {"tags", required_argument, 0, 't' },
        {"reverse", no_argument, 0, 'r'},
        {"recursive", no_argument, 0, 'R'},
        {0, 0, 0, 0 }
    };

    int opt;
    int longindex = 0;
    bool reverse = false;
    bool recursive = false;

    // Go through every argument
    std::vector<std::string> tags;
    std::string output;
    while((opt = getopt_long(argc, argv, "t:ihrR", options, &longindex)) != -1) {
        switch(opt) {
            case 't':
                tags.push_back(optarg);
                break;
            case 'i':
                INVADER_SHOW_INFO
                return EXIT_FAILURE;
            case 'r':
                reverse = true;
                break;
            case 'R':
                recursive = true;
                break;
            default:
                eprintf("Usage: %s [options] <tag.class>\n\n", argv[0]);
                eprintf("Check dependencies for a tag.\n\n");
                eprintf("Options:\n");
                eprintf("  --info,-i                    Show credits, source info, and other info.\n");
                eprintf("  --recursive, -R              Recursively get all depended tags.\n");
                eprintf("  --reverse, -r                Find all tags that depend on the tag, instead.\n");
                eprintf("  --tags,-t <dir>              Use the specified tags directory. Use multiple\n");
                eprintf("                               times to add more directories, ordered by\n");
                eprintf("                               precedence.\n");
                return EXIT_FAILURE;
        }
    }

    // No tags folder? Use tags in current directory
    if(tags.size() == 0) {
        tags.emplace_back("tags");
    }

    // Require a tag
    char *tag_path_to_find;
    if(optind == argc) {
        eprintf("%s: A scenario tag path is required. Use -h for help.\n", argv[0]);
        return EXIT_FAILURE;
    }
    else if(optind < argc - 1) {
        eprintf("%s: Unexpected argument %s\n", argv[0], argv[optind + 1]);
        return EXIT_FAILURE;
    }
    else {
        tag_path_to_find = argv[optind];
    }

    // Get the tag path and extension
    char *c = nullptr;
    for(std::size_t i = 0; tag_path_to_find[i] != 0; i++) {
        if(tag_path_to_find[i] == '.') {
            c = tag_path_to_find + i + 1;
        }
    }

    auto tag_int_to_find = Invader::HEK::extension_to_tag_class(c);
    if(c == nullptr) {
        eprintf("Invalid tag path %s. Missing extension.\n", tag_path_to_find);
        return EXIT_FAILURE;
    }
    else if(tag_int_to_find == Invader::HEK::TAG_CLASS_NULL) {
        eprintf("Invalid tag path %s. Unknown tag class %s.\n", tag_path_to_find, c);
        return EXIT_FAILURE;
    }

    // Split the tag extension
    *(c - 1) = 0;

    // Here's an array we can use to hold what we got
    bool success;
    auto found_tags = Invader::FoundTagDependency::find_dependencies(tag_path_to_find, tag_int_to_find, tags, reverse, recursive, success);

    if(!success) {
        return EXIT_FAILURE;
    }

    // See what depended on it or what depends on this
    for(auto &tag : found_tags) {
        std::printf("%s.%s%s\n", tag.path.data(), Invader::HEK::tag_class_to_extension(tag.class_int), tag.broken ? " [BROKEN]" : "");
    }
}
