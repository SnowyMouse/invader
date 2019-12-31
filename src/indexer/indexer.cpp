// SPDX-License-Identifier: GPL-3.0-only

#include <vector>
#include <cstring>
#include <regex>

#include <invader/map/map.hpp>
#include <invader/resource/resource_map.hpp>
#include <invader/version.hpp>
#include <invader/printf.hpp>
#include <invader/file/file.hpp>
#include <invader/command_line_option.hpp>

enum ReturnValue : int {
    RETURN_OK = 0,
    RETURN_FAILED_NOTHING_TO_DO = 1,
    RETURN_FAILED_ERROR = 2
};

int main(int argc, const char **argv) {
    using namespace Invader;
    using namespace Invader::HEK;

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

            // Go through tags
            try {
                for(std::size_t i = skip; i < tag_count; i+= 1 + skip) {
                    auto &tag = map[i];

                    // Replace double slashes (or more) with one slash
                    std::string path = std::regex_replace(tag.path, std::basic_regex<char>("\\\\{2,}"), "\\", std::regex_constants::match_default);

                    std::fprintf(f, "%s\n", path.c_str());
                }
            }
            catch(std::exception &) {
                std::fclose(f);
                return RETURN_FAILED_ERROR;
            }
            std::fclose(f);
        }
        catch(std::exception &e) {
            eprintf_error("Exception %s", e.what());
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
                eprintf_error("Failed to open %s", output);
                return RETURN_FAILED_ERROR;
            }

            try {
                auto tag_count = map.get_tag_count();
                for(std::size_t i = 0; i < tag_count; i++) {
                    auto &tag = map.get_tag(i);

                    // Replace double slashes (or more) with one slash
                    std::string path = std::regex_replace(tag.get_path(), std::basic_regex<char>("\\\\{2,}"), "\\", std::regex_constants::match_default);
                    std::fprintf(f, "%s.%s\n", path.c_str(), tag_class_to_extension(tag.get_tag_class_int()));
                }
            }
            catch(std::exception &) {
                std::fclose(f);
                return RETURN_FAILED_ERROR;
            }
            std::fclose(f);
        }
        catch(std::exception &e) {
            eprintf_error("Exception: %s", e.what());
            return RETURN_FAILED_ERROR;
        }
    }
}
