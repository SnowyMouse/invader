// SPDX-License-Identifier: GPL-3.0-only

#include <vector>
#include <cstring>
#include <regex>

#include "invader/map/map.hpp"
#include "invader/resource/resource_map.hpp"
#include "invader/tag/compiled_tag.hpp"
#include "invader/version.hpp"
#include "invader/printf.hpp"
#include "invader/file/file.hpp"

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

    auto input_map_data = File::open_file(input);

    // Open input map
    if(!input_map_data.has_value()) {
        eprintf("Failed to read %s\n", input);
        return RETURN_FAILED_ERROR;
    }

    auto input_map = std::move(*input_map_data);

    // If it's a resource map, try parsing that
    if(input_map.size() >= 4 && *reinterpret_cast<std::uint32_t *>(input_map.data()) <= 3) {
        try {
            auto map = load_resource_map(input_map.data(), input_map.size());
            auto tag_count = map.size();
            int skip = *reinterpret_cast<std::uint32_t *>(input_map.data()) != 3 ? 1 : 0;

            // Open the output!
            std::FILE *f = std::fopen(output, "wb");
            std::fprintf(f, "%zu\n", tag_count / (1 + skip));

            // Go through tags
            try {
                for(std::size_t i = skip; i < tag_count; i+= 1 + skip) {
                    auto &tag = map[i];

                    // Replace double slashes (or more) with one slash
                    std::string path = std::regex_replace(tag.path, std::basic_regex<char>("\\\\{2,}"), "\\", std::regex_constants::match_default);

                    std::fprintf(f, "%s\n", path.data());
                }
            }
            catch(std::exception &) {
                std::fclose(f);
                return RETURN_FAILED_ERROR;
            }
            std::fclose(f);
        }
        catch(std::exception &e) {
            eprintf("Exception %s\n", e.what());
            return RETURN_FAILED_ERROR;
        }
    }

    // If not, it's probably a cache file
    else {
        try {
            auto map = Map::map_with_move(std::move(input_map));

            // Open output
            std::FILE *f = std::fopen(output, "wb");
            if(!f) {
                eprintf("Failed to open %s\n", output);
                return RETURN_FAILED_ERROR;
            }

            try {
                auto tag_count = map.tag_count();
                std::fprintf(f, "%zu\n", tag_count);
                for(std::size_t i = 0; i < tag_count; i++) {
                    auto &tag = map.get_tag(i);

                    // Replace double slashes (or more) with one slash
                    std::string path = std::regex_replace(tag.path(), std::basic_regex<char>("\\\\{2,}"), "\\", std::regex_constants::match_default);
                    std::fprintf(f, "%u\t%s\n", tag.tag_class_int(), path.data());
                }
            }
            catch(std::exception &) {
                std::fclose(f);
                return RETURN_FAILED_ERROR;
            }
            std::fclose(f);
        }
        catch(std::exception &e) {
            eprintf("Exception: %s\n", e.what());
            return RETURN_FAILED_ERROR;
        }
    }
}
