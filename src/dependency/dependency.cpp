// SPDX-License-Identifier: GPL-3.0-only

#include <vector>
#include <string>
#include <filesystem>
#include <invader/version.hpp>
#include <invader/printf.hpp>
#include <invader/tag/compiled_tag.hpp>
#include <invader/dependency/found_tag_dependency.hpp>
#include <invader/build/build_workload.hpp>
#include <invader/map/map.hpp>
#include <invader/command_line_option.hpp>
#include <invader/file/file.hpp>

int main(int argc, char * const *argv) {
    std::vector<Invader::CommandLineOption> options;
    options.emplace_back("info", 'i', 0, "Show credits, source info, and other info.");
    options.emplace_back("tags", 't', 1, "Use the specified tags directory. Use multiple times to add more directories, ordered by precedence.", "<dir>");
    options.emplace_back("reverse", 'V', 0, "Find all tags that depend on the tag, instead.");
    options.emplace_back("recursive", 'r', 0, "Recursively get all depended tags.");
    options.emplace_back("fs-path", 'P', 0, "Use a filesystem path for the tag.");

    static constexpr char DESCRIPTION[] = "Check dependencies for a tag.";
    static constexpr char USAGE[] = "[options] <tag.class>";

    struct DependencyOption {
        bool reverse = false;
        bool recursive = false;
        std::vector<std::string> tags;
        std::string output;
        bool use_filesystem_path = false;
    } dependency_options;

    auto remaining_arguments = Invader::CommandLineOption::parse_arguments<DependencyOption &>(argc, argv, options, USAGE, DESCRIPTION, 1, 1, dependency_options, [](char opt, const auto &arguments, auto &dependency_options) {
        switch(opt) {
            case 't':
                dependency_options.tags.push_back(arguments[0]);
                break;
            case 'i':
                Invader::show_version_info();
                std::exit(EXIT_FAILURE);
            case 'V':
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
    std::vector<char> tag_path_to_find_data;
    if(dependency_options.use_filesystem_path) {
        auto tag_path_maybe = Invader::File::file_path_to_tag_path(remaining_arguments[0], dependency_options.tags, true);

        if(tag_path_maybe.has_value()) {
            auto &file_path = tag_path_maybe.value();
            tag_path_to_find_data = std::vector<char>(file_path.begin(), file_path.end());
        }
        else {
            eprintf("Failed to find a valid tag %s in the tags directory\n", remaining_arguments[0]);
            return EXIT_FAILURE;
        }
    }
    else {
        tag_path_to_find_data = std::vector<char>(remaining_arguments[0], remaining_arguments[0] + strlen(remaining_arguments[0]));
    }

    // Add a null terminator
    tag_path_to_find_data.emplace_back(0);

    char *tag_path_to_find = tag_path_to_find_data.data();

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
    auto found_tags = Invader::FoundTagDependency::find_dependencies(tag_path_to_find, tag_int_to_find, dependency_options.tags, dependency_options.reverse, dependency_options.recursive, success);

    if(!success) {
        return EXIT_FAILURE;
    }

    // See what depended on it or what depends on this
    for(auto &tag : found_tags) {
        oprintf("%s.%s%s\n", Invader::File::halo_path_to_preferred_path(tag.path).data(), Invader::HEK::tag_class_to_extension(tag.class_int), tag.broken ? " [BROKEN]" : "");
    }
}
