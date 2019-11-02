// SPDX-License-Identifier: GPL-3.0-only

#include <vector>
#include <string>
#include <filesystem>
#include <archive.h>
#include <archive_entry.h>
#include "invader/version.hpp"
#include "invader/printf.hpp"

#include "invader/build/build_workload.hpp"
#include "invader/map/map.hpp"
#include "invader/dependency/found_tag_dependency.hpp"
#include "invader/command_line_option.hpp"
#include "invader/file/file.hpp"

int main(int argc, const char **argv) {
    struct ArchiveOptions {
        const char *path;
        bool single_tag = false;
        std::vector<std::string> tags;
        std::string output;
        bool use_filesystem_path = false;
    } archive_options;
    archive_options.path = argv[0];

    static constexpr char DESCRIPTION[] = "Generate .tar.xz archives of the tags required to build a cache file.";
    static constexpr char USAGE[] = "[options] <scenario | -s tag.class>";

    std::vector<Invader::CommandLineOption> options;
    options.emplace_back("info", 'i', 0, "Show credits, source info, and other info.");
    options.emplace_back("single-tag", 's', 0, "Archive a tag tree instead of a cache file.");
    options.emplace_back("tags", 't', 1, "Use the specified tags directory. Use multiple times to add more directories, ordered by precedence.", "<dir>");
    options.emplace_back("output", 'o', 1, "Output to a specific file. Extension must be .tar.xz.", "<file>");
    options.emplace_back("fs-path", 'P', 0, "Use a filesystem path for the tag.");
    
    auto remaining_arguments = Invader::CommandLineOption::parse_arguments<ArchiveOptions &>(argc, argv, options, USAGE, DESCRIPTION, archive_options, [](char opt, const auto &arguments, auto &archive_options) {
        switch(opt) {
            case 't':
                archive_options.tags.push_back(arguments[0]);
                break;
            case 'o':
                archive_options.output = arguments[0];
                break;
            case 'i':
                Invader::show_version_info();
                std::exit(EXIT_FAILURE);
            case 's':
                archive_options.single_tag = true;
                break;
            case 'P':
                archive_options.use_filesystem_path = true;
                break;
        }
    });

    // No tags folder? Use tags in current directory
    if(archive_options.tags.size() == 0) {
        archive_options.tags.emplace_back("tags");
    }

    // Require a tag
    std::string base_tag;
    if(remaining_arguments.size() == 0) {
        eprintf("A %stag path is required. Use -h for help.\n", archive_options.single_tag ? "" : "scenario ");
        return EXIT_FAILURE;
    }
    else if(remaining_arguments.size() > 1) {
        eprintf("Unexpected argument %s\n", remaining_arguments[1]);
        return EXIT_FAILURE;
    }
    else if(archive_options.use_filesystem_path) {
        // See if the tag path is valid
        std::optional<std::string> base_tag_maybe;
        if(archive_options.single_tag) {
            base_tag_maybe = Invader::File::file_path_to_tag_path(remaining_arguments[0], archive_options.tags, true);
        }
        else {
            base_tag_maybe = Invader::File::file_path_to_tag_path_with_extension(remaining_arguments[0], archive_options.tags, std::string(".scenario"));
        }
        if(base_tag_maybe.has_value()) {
            base_tag = base_tag_maybe.value();
        }
        else {
            eprintf("Failed to find a valid%stag %s in the tags directory\n", archive_options.single_tag ? " " : " scenario ", remaining_arguments[0]);
            if(!archive_options.single_tag && Invader::File::file_path_to_tag_path(remaining_arguments[0], archive_options.tags, true).has_value()) {
                eprintf("A file was detected there, but it isn't a .scenario tag, so you need to use -s\n");
            }
            return EXIT_FAILURE;
        }
    }
    else {
        base_tag = Invader::File::preferred_path_to_halo_path(remaining_arguments[0]);
    }

    // Variables to hold this
    std::vector<std::pair<std::string, std::string>> archive_list;

    // If no output filename was given, make one
    static const char extension[] = ".tar.xz";
    if(archive_options.output.size() == 0) {
        // Set output
        archive_options.output = Invader::File::base_name(base_tag) + extension;
    }
    else {
        bool fail = true;
        if(archive_options.output.size() > sizeof(extension)) {
            fail = std::strcmp(archive_options.output.data() + archive_options.output.size() - sizeof(extension) + 1, extension) != 0;
        }

        if(fail) {
            eprintf("Invalid output file %s. This should end with .tar.xz.\n", archive_options.output.data());
            return EXIT_FAILURE;
        }
    }

    if(!archive_options.single_tag) {
        // Build the map
        std::vector<std::byte> map;

        try {
            map = Invader::BuildWorkload::compile_map(base_tag.data(), archive_options.tags);
        }
        catch(std::exception &e) {
            eprintf("Failed to compile scenario %s into a map\n", base_tag.data());
            return EXIT_FAILURE;
        }

        // Parse the map
        std::unique_ptr<Invader::Map> parsed_map;
        try {
            parsed_map = std::make_unique<Invader::Map>(Invader::Map::map_with_pointer(map.data(), map.size()));
        }
        catch(std::exception &e) {
            eprintf("Failed to parse the map file generated with scenario %s\n", base_tag.data());
            return EXIT_FAILURE;
        }
        auto tag_count = parsed_map->tag_count();

        // Go through each tag and see if we can find everything.
        archive_list.reserve(tag_count);
        for(std::size_t i = 0; i < tag_count; i++) {
            // Get the tag path information
            auto &tag = parsed_map->get_tag(i);
            std::string full_tag_path = Invader::File::halo_path_to_preferred_path(std::string(tag.path()) + "." + tag_class_to_extension(tag.tag_class_int()));

            // Check each tag directory if it exists. If so, archive it
            bool exists = false;
            for(auto &dir : archive_options.tags) {
                std::filesystem::path tag_path = std::filesystem::path(dir) / full_tag_path;
                if(std::filesystem::exists(tag_path)) {
                    exists = true;
                    archive_list.emplace_back(tag_path.string(), full_tag_path);
                    break;
                }
            }

            if(!exists) {
                eprintf("Failed to find %s. Archive could not be made.\n", full_tag_path.data());
                return EXIT_FAILURE;
            }
        }
    }
    else {
        // Turn it into something the filesystem can understand
        Invader::File::halo_path_to_preferred_path_chars(base_tag.data());

        // Split the extension
        char *tag_path_to_find = base_tag.data();
        char *c = nullptr;
        for(char *d = tag_path_to_find; *d; d++) {
            if(*d == '.') {
                c = d + 1;
            }
        }
        if(!c) {
            eprintf("No extension for %s. Archive could not be made.\n", tag_path_to_find);
            return EXIT_FAILURE;
        }
        Invader::HEK::TagClassInt tag_int_to_find = Invader::HEK::extension_to_tag_class(c);

        bool exists = false;
        for(auto &dir : archive_options.tags) {
            std::filesystem::path tag_path = std::filesystem::path(dir) / base_tag;
            if(std::filesystem::exists(tag_path)) {
                exists = true;
                archive_list.emplace_back(tag_path.string(), base_tag);
                break;
            }
        }

        if(!exists) {
            eprintf("Failed to find %s.%s. Archive could not be made.\n", tag_path_to_find, tag_class_to_extension(tag_int_to_find));
            return EXIT_FAILURE;
        }

        bool success;
        auto dependencies = Invader::FoundTagDependency::find_dependencies(base_tag.substr(0, c - tag_path_to_find - 1).data(), tag_int_to_find, archive_options.tags, false, true, success);
        if(!success) {
            eprintf("Failed to archive %s.%s. Archive could not be made.\n", tag_path_to_find, tag_class_to_extension(tag_int_to_find));
            return EXIT_FAILURE;
        }

        // Make sure there aren't any broken dependencies
        for(auto &dependency : dependencies) {
            if(dependency.broken) {
                eprintf("%s.%s is missing (broken dependency). Archive could not be made.\n", dependency.path.data(), tag_class_to_extension(dependency.class_int));
                return EXIT_FAILURE;
            }

            std::string path_copy = Invader::File::halo_path_to_preferred_path(dependency.path + "." + tag_class_to_extension(dependency.class_int));
            archive_list.emplace_back(dependency.file_path, path_copy);
        }
    }

    // Begin making the archive
    auto *archive = archive_write_new();
    archive_write_add_filter_xz(archive);
    archive_write_set_format_pax_restricted(archive);
    archive_write_open_filename(archive, archive_options.output.data());

    // Go through each tag path we got
    for(std::size_t i = 0; i < archive_list.size(); i++) {
        const char *path = archive_list[i].first.data();

        // Begin
        auto *entry = archive_entry_new();
        archive_entry_set_pathname(entry, archive_list[i].second.data());
        archive_entry_set_perm(entry, 0644);
        archive_entry_set_filetype(entry, AE_IFREG);

        auto data_maybe = Invader::File::open_file(path);
        if(!data_maybe.has_value()) {
            eprintf("Failed to open %s\n", path);
            return EXIT_FAILURE;
        }
        auto &data = data_maybe.value();

        // Get the filesystem path and write time
        std::filesystem::path path_path = path;

        // Get the modified time
        struct stat s;
        stat(path, &s);

        // Windows uses mtime which is a time_t rather than a struct with nanoseconds
        #ifdef _WIN32
        archive_entry_set_mtime(entry, s.st_mtime, 0);
        #else
        archive_entry_set_mtime(entry, s.st_mtim.tv_sec, 0);
        #endif

        // Archive that bastard
        archive_entry_set_size(entry, data.size());
        archive_write_header(archive, entry);
        archive_write_data(archive, data.data(), data.size());

        // Close it
        archive_entry_free(entry);
    }

    // Save and close
    archive_write_close(archive);
    archive_write_free(archive);

    return EXIT_SUCCESS;
}
