// SPDX-License-Identifier: GPL-3.0-only

#include <vector>
#include <string>
#include <filesystem>
#include <invader/version.hpp>
#include <invader/printf.hpp>
#include <invader/dependency/found_tag_dependency.hpp>
#include <invader/build/build_workload.hpp>
#include <invader/map/map.hpp>
#include "../command_line_option.hpp"
#include <invader/file/file.hpp>

#define ERROR_PARSING_TAGS 197

int main(int argc, char * const *argv) {
    set_up_color_term();

    using namespace Invader;

    const CommandLineOption options[] {
        CommandLineOption::from_preset(CommandLineOption::PRESET_COMMAND_LINE_OPTION_INFO),
        CommandLineOption::from_preset(CommandLineOption::PRESET_COMMAND_LINE_OPTION_FS_PATH),
        CommandLineOption::from_preset(CommandLineOption::PRESET_COMMAND_LINE_OPTION_TAGS_MULTIPLE),
        CommandLineOption("reverse", 'R', 0, "Find all tags that depend on the tag, instead. The tag does not have to exist if not using --fs-path."),
        CommandLineOption("recursive", 'r', 0, "Recursively get all depended tags."),
    };

    static constexpr char DESCRIPTION[] = "Check dependencies for a tag.";
    static constexpr char USAGE[] = "[options] <tag.group>";

    struct DependencyOption {
        bool reverse = false;
        bool recursive = false;
        std::vector<std::filesystem::path> tags;
        bool use_filesystem_path = false;
    } dependency_options;

    auto remaining_arguments = CommandLineOption::parse_arguments<DependencyOption &>(argc, argv, options, USAGE, DESCRIPTION, 1, 1, dependency_options, [](char opt, const auto &arguments, auto &dependency_options) {
        switch(opt) {
            case 't':
                dependency_options.tags.push_back(arguments[0]);
                break;
            case 'i':
                show_version_info();
                std::exit(EXIT_SUCCESS);
            case 'R':
                dependency_options.reverse = true;
                break;
            case 'r':
                dependency_options.recursive = true;
                break;
            case 'P':
                dependency_options.use_filesystem_path = true;
                break;
        }
    });

    // No tags folder? Use tags in current directory
    if(dependency_options.tags.size() == 0) {
        dependency_options.tags.emplace_back("tags");
    }

    // Require a tag
    std::optional<std::string> tag_path;
    if(dependency_options.use_filesystem_path) {
        auto tag_path_maybe = File::file_path_to_tag_path(remaining_arguments[0], dependency_options.tags);
        if(tag_path_maybe.has_value()) {
            tag_path = File::preferred_path_to_halo_path(*tag_path_maybe);
        }
        else {
            eprintf_error("Failed to find a valid tag %s", remaining_arguments[0]);
            return EXIT_FAILURE;
        }
    }
    else {
        tag_path = File::halo_path_to_preferred_path(remaining_arguments[0]);
    }

    auto tag_path_split = File::split_tag_class_extension(*tag_path);
    if(!tag_path_split.has_value()) {
        eprintf_error("%s is not a valid tag path", remaining_arguments[0]);
        return EXIT_FAILURE;
    }

    // Here's an array we can use to hold what we got
    std::vector<FoundTagDependency> found_tags;
    try {
        bool success;
        found_tags = FoundTagDependency::find_dependencies(tag_path_split->path.c_str(), tag_path_split->fourcc, dependency_options.tags, dependency_options.reverse, dependency_options.recursive, success);
        if(!success) {
            return EXIT_FAILURE;
        }
    }
    catch(InvalidTagDataException &e) {
        eprintf_error("Failed to list dependencies due to invalid tag data (tags may be corrupt)");
        return ERROR_PARSING_TAGS;
    }
    catch(std::exception &e) {
        eprintf_error("Failed to list dependencies: %s\n", e.what());
        return EXIT_FAILURE;
    }

    // See what depended on it or what depends on this
    for(auto &tag : found_tags) {
        oprintf("%s.%s%s\n", File::halo_path_to_preferred_path(tag.path).c_str(), HEK::tag_fourcc_to_extension(tag.fourcc), tag.broken ? " [BROKEN]" : "");
    }

    return EXIT_SUCCESS;
}
