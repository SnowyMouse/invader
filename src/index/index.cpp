// SPDX-License-Identifier: GPL-3.0-only

#include <vector>
#include <cstring>
#include <regex>

#include <invader/map/map.hpp>
#include <invader/resource/resource_map.hpp>
#include <invader/version.hpp>
#include <invader/printf.hpp>
#include <invader/file/file.hpp>
#include <invader/resource/hek/resource_map.hpp>
#include <invader/command_line_option.hpp>

int main(int argc, const char **argv) {
    using namespace Invader;
    using namespace Invader::Parser;

    std::vector<Invader::CommandLineOption> options;
    options.emplace_back("info", 'i', 0, "Show credits, source info, and other info.");

    static constexpr char DESCRIPTION[] = "Create a file listing the tags of a map.";
    static constexpr char USAGE[] = "[options] <input-map> <output-txt>";

    auto remaining_arguments = Invader::CommandLineOption::parse_arguments<std::nullptr_t>(argc, argv, options, USAGE, DESCRIPTION, 2, 2, nullptr, [](char opt, const auto &, std::nullptr_t) {
        switch(opt) {
            case 'i':
                Invader::show_version_info();
                std::exit(EXIT_SUCCESS);
        }
    });

    const char *output = remaining_arguments[1];
    const char *input = remaining_arguments[0];

    auto input_map_data = File::open_file(input);

    // Open input map
    if(!input_map_data.has_value()) {
        eprintf_error("Failed to read %s", input);
        return EXIT_FAILURE;
    }

    auto input_map = std::move(*input_map_data);

    // If it's a resource map, try parsing that
    if(input_map.size() >= 4 && *reinterpret_cast<std::uint32_t *>(input_map.data()) <= 3) {
        try {
            auto map = load_resource_map(input_map.data(), input_map.size());
            auto &header = *reinterpret_cast<ResourceMapHeader *>(input_map.data());
            
            // Get our extension
            const char *extension;
            switch(header.type) {
                case ResourceMapType::RESOURCE_MAP_BITMAP:
                    extension = ".bitmap";
                    break;
                case ResourceMapType::RESOURCE_MAP_SOUND:
                    extension = ".sound";
                    break;
                case ResourceMapType::RESOURCE_MAP_LOC:
                    extension = "";
                    break;
                default:
                    std::terminate();
            }
            
            auto tag_count = map.size();
            int skip = *reinterpret_cast<std::uint32_t *>(input_map.data()) != 3 ? 1 : 0;

            // Open the output!
            std::FILE *f = std::fopen(output, "wb");

            // Go through tags
            try {
                for(std::size_t i = skip; i < tag_count; i+= 1 + skip) {
                    auto &tag = map[i];
                    std::fprintf(f, "%s%s\n", tag.path.c_str(), extension);
                }
            }
            catch(std::exception &) {
                std::fclose(f);
                return EXIT_FAILURE;
            }
            std::fclose(f);
        }
        catch(std::exception &e) {
            eprintf_error("Exception %s", e.what());
            return EXIT_FAILURE;
        }
    }

    // If not, it's probably a cache file
    else {
        try {
            auto map = Map::map_with_move(std::move(input_map));

            // Open output
            std::FILE *f = std::fopen(output, "wb");
            if(!f) {
                eprintf_error("Failed to open %s", output);
                return EXIT_FAILURE;
            }

            try {
                auto tag_count = map.get_tag_count();
                for(std::size_t i = 0; i < tag_count; i++) {
                    auto &tag = map.get_tag(i);
                    std::fprintf(f, "%s.%s\n", tag.get_path().c_str(), tag_fourcc_to_extension(tag.get_tag_fourcc()));
                }
            }
            catch(std::exception &) {
                std::fclose(f);
                return EXIT_FAILURE;
            }
            std::fclose(f);
        }
        catch(std::exception &e) {
            eprintf_error("Exception: %s", e.what());
            return EXIT_FAILURE;
        }
    }
    
    oprintf("Created an index file at %s\n", output);
}
