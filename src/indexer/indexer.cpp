/*
 * Invader (c) 2018 Kavawuvi
 *
 * This program is free software under the GNU General Public License v3.0 or later. See LICENSE for more information.
 */

#ifndef NO_OUTPUT
#include <chrono>
using clock_type = std::chrono::steady_clock;
#include <iostream>
#endif

#include <vector>
#include <cstring>
#include <regex>

#include "../map/map.hpp"
#include "../resource/resource_map.hpp"
#include "../tag/compiled_tag.hpp"
#include "../version.hpp"

enum ReturnValue : int {
    RETURN_OK = 0,
    RETURN_FAILED_NOTHING_TO_DO = 1,
    RETURN_FAILED_ERROR = 2
};

int main(int argc, const char **argv) {
    using namespace Invader;
    using namespace Invader::HEK;

    if(argc < 3) {
        eprintf("Usage: %s <input map> <output index>\n", argv[0]);
        return RETURN_FAILED_NOTHING_TO_DO;
    }

    const char *output = argv[2];
    const char *input = argv[1];

    // Open input map
    FILE *f = std::fopen(input, "rb");
    if(!f) {
        eprintf("Failed to open %s.\n", input);
        return RETURN_FAILED_ERROR;
    }
    std::fseek(f, 0, SEEK_END);
    std::size_t size = std::ftell(f);
    std::unique_ptr<std::byte []> map_data(new std::byte[size]);
    std::fseek(f, 0, SEEK_SET);
    if(std::fread(map_data.get(), size, 1, f) != 1) {
        eprintf("Failed to read %s\n", argv[1]);
        return RETURN_FAILED_ERROR;
    }
    std::fclose(f);

    f = nullptr;

    // If it's a resource map, try parsing that
    if(size >= 4 && *reinterpret_cast<std::uint32_t *>(map_data.get()) <= 3) {
        try {
            auto map = load_resource_map(map_data.get(), size);
            auto tag_count = map.size();
            int skip = *reinterpret_cast<std::uint32_t *>(map_data.get()) != 3 ? 1 : 0;

            // Open the output!
            f = std::fopen(output, "wb");
            std::fprintf(f, "%zu\n", tag_count / (1 + skip));

            // Go through tags
            for(std::size_t i = skip; i < tag_count; i+= 1 + skip) {
                auto &tag = map[i];

                // Replace double slashes (or more) with one slash
                std::string path = std::regex_replace(tag.name, std::basic_regex<char>("\\\\{2,}"), "\\", std::regex_constants::match_default);

                std::fprintf(f, "%s\n", path.data());
            }

            std::fclose(f);
        }
        catch(std::exception &e) {
            eprintf("Exception %s\n", e.what());
            std::fclose(f);
            return RETURN_FAILED_ERROR;
        }
    }

    // If not, it's probably a cache file
    else {
        try {
            auto map = Map::map_with_pointer(map_data.get(), size);

            // Open output
            f = std::fopen(output, "wb");
            if(!f) {
                eprintf("Failed to open %s\n", output);
                return RETURN_FAILED_ERROR;
            }

            auto tag_count = map.tag_count();
            std::fprintf(f, "%zu\n", tag_count);
            for(std::size_t i = 0; i < tag_count; i++) {
                auto &tag = map.get_tag(i);

                // Replace double slashes (or more) with one slash
                std::string path = std::regex_replace(tag.path(), std::basic_regex<char>("\\\\{2,}"), "\\", std::regex_constants::match_default);

                std::fprintf(f, "%u\t%s\n", tag.tag_class_int(), path.data());
            }

            std::fclose(f);
        }
        catch(std::exception &e) {
            eprintf("Exception: %s\n", e.what());
            std::fclose(f);
            return RETURN_FAILED_ERROR;
        }
    }
}
