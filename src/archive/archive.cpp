/*
 * Invader (c) 2018 Kavawuvi
 *
 * This program is free software under the GNU General Public License v3.0 or later. See LICENSE for more information.
 */

#include <vector>
#include <string>
#include <getopt.h>
#include <filesystem>
#include "../eprintf.hpp"

#include "../build/build_workload.hpp"
#include "../map/map.hpp"

int main(int argc, const char **argv) {
    if(argc < 3) {
        eprintf("Usage: %s <scenario> <tags> [tags2 [...]]\n", argv[0]);
        return EXIT_FAILURE;
    }

    // Add tags to the array
    std::vector<std::string> tags;
    for(int i = 2; i < argc; i++) {
        tags.emplace_back(argv[i]);
    }

    // Build the map
    auto map = Invader::BuildWorkload::compile_map(argv[1], tags, "", std::vector<std::tuple<Invader::HEK::TagClassInt, std::string>>(), true, false, false);

    // Parse the map
    auto parsed_map = Invader::Map::map_with_pointer(map.data(), map.size());
    auto tag_count = parsed_map.tag_count();

    // Go through each tag and see if we can find everything.
    std::vector<std::string> archive_list;
    archive_list.reserve(tag_count);
    for(std::size_t i = 0; i < tag_count; i++) {
        // Get the tag path information
        auto &tag = parsed_map.get_tag(i);
        std::string full_tag_path = std::string(tag.path()) + "." + tag_class_to_extension(tag.tag_class_int());

        // Replace all backslashes with forward slashes if needed
        #ifndef _WIN32
        for(char &c : full_tag_path) {
            if(c == '\\') c = '/';
        }
        #endif

        // Check each tag directory if it exists. If so, archive it
        bool exists = false;
        for(auto &dir : tags) {
            std::filesystem::path tag_path = std::filesystem::path(dir) / full_tag_path;
            if(std::filesystem::exists(tag_path)) {
                exists = true;
                archive_list.push_back(tag_path.string());
                break;
            }
        }

        if(!exists) {
            eprintf("Failed to find %s.\n", full_tag_path.data());
        }
    }

    return EXIT_SUCCESS;
}
