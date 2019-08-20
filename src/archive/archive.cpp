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

#include "../build/build_workload.hpp"
#include "../map/map.hpp"
#include "../dependency/found_tag_dependency.hpp"

int main(int argc, char * const *argv) {
    static struct option options[] = {
        {"help",  no_argument, 0, 'h'},
        {"info", no_argument, 0, 'i' },
        {"tags", required_argument, 0, 't' },
        {"output", required_argument, 0, 'o' },
        {"single-tag", no_argument, 0, 's' },
        {0, 0, 0, 0 }
    };

    int opt;
    int longindex = 0;

    // Go through every argument
    std::vector<std::string> tags;
    std::string output;
    bool single_tag = false;
    while((opt = getopt_long(argc, argv, "t:o:ihs", options, &longindex)) != -1) {
        switch(opt) {
            case 't':
                tags.push_back(optarg);
                break;
            case 'o':
                output = optarg;
                break;
            case 'i':
                INVADER_SHOW_INFO
                return EXIT_FAILURE;
            case 's':
                single_tag = true;
                break;
            default:
                eprintf("Usage: %s [options] <scenario | -s tag.class>\n\n", argv[0]);
                eprintf("Generate .tar.xz archives of the tags required to build a cache file.\n\n");
                eprintf("Options:\n");
                eprintf("  --info,-i                    Show credits, source info, and other info.\n");
                eprintf("  --output,-o <file>           Output to a specific file. Extension must be\n");
                eprintf("                               .tar.xz.\n");
                eprintf("  --single-tag,-s              Archive a tag tree instead of a cache file.\n");
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
    char *base_tag;
    if(optind == argc) {
        eprintf("%s: A %stag path is required. Use -h for help.\n", single_tag ? "" : "scenario ", argv[0]);
        return EXIT_FAILURE;
    }
    else if(optind < argc - 1) {
        eprintf("%s: Unexpected argument %s\n", argv[0], argv[optind + 1]);
        return EXIT_FAILURE;
    }
    else {
        base_tag = argv[optind];
    }

    // Variables to hold this
    std::vector<std::pair<std::string, std::string>> archive_list;

    // If no output filename was given, make one
    static const char extension[] = ".tar.xz";
    if(output.size() == 0) {
        const char *base_name = base_tag;
        for(const char *c = base_name; *c; c++) {
            if(*c == '\\' || *c == '/') {
                base_name = c + 1;
            }
        }

        // Set output
        output = std::string(base_name) + extension;
    }
    else {
        bool fail = true;
        if(output.size() > sizeof(extension)) {
            fail = std::strcmp(output.data() + output.size() - sizeof(extension) + 1, extension) != 0;
        }

        if(fail) {
            eprintf("Invalid output file %s. This should end with .tar.xz.\n", output.data());
            return EXIT_FAILURE;
        }
    }

    if(!single_tag) {
        // Build the map
        auto map = Invader::BuildWorkload::compile_map(base_tag, tags, "", std::vector<std::tuple<Invader::HEK::TagClassInt, std::string>>(), true, false, false);

        // Parse the map
        auto parsed_map = Invader::Map::map_with_pointer(map.data(), map.size());
        auto tag_count = parsed_map.tag_count();

        // Go through each tag and see if we can find everything.
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
        #ifndef _WIN32
        for(char *c = base_tag; *c; c++) {
            if(*c == '\\') {
                *c = '/';
            }
        }
        #endif

        // Split the extension
        char *tag_path_to_find = base_tag;
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
        *(c - 1) = 0;
        Invader::HEK::TagClassInt tag_int_to_find = Invader::HEK::extension_to_tag_class(c);

        bool exists = false;
        for(auto &dir : tags) {
            auto name_with_extension = std::string(base_tag) + "." + c;
            std::filesystem::path tag_path = std::filesystem::path(dir) / name_with_extension;
            if(std::filesystem::exists(tag_path)) {
                exists = true;
                archive_list.emplace_back(tag_path.string(), name_with_extension);
                break;
            }
        }

        if(!exists) {
            eprintf("Failed to find %s.%s. Archive could not be made.\n", tag_path_to_find, tag_class_to_extension(tag_int_to_find));
            return EXIT_FAILURE;
        }

        bool success;
        auto dependencies = Invader::FoundTagDependency::find_dependencies(tag_path_to_find, tag_int_to_find, tags, false, true, success);
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

            std::string path_copy = dependency.path + "." + tag_class_to_extension(dependency.class_int);
            #ifndef _WIN32
            for(char &c : path_copy) {
                if(c == '\\') {
                    c = '/';
                }
            }
            #endif
            archive_list.emplace_back(dependency.file_path, path_copy);
        }
    }

    // Begin making the archive
    auto *archive = archive_write_new();
    archive_write_add_filter_xz(archive);
    archive_write_set_format_pax_restricted(archive);
    archive_write_open_filename(archive, output.data());

    // Go through each tag path we got
    for(std::size_t i = 0; i < archive_list.size(); i++) {
        // Begin
        auto *entry = archive_entry_new();
        archive_entry_set_pathname(entry, archive_list[i].second.data());
        archive_entry_set_perm(entry, 0644);
        archive_entry_set_filetype(entry, AE_IFREG);

        // Open the tag
        std::FILE *t = std::fopen(archive_list[i].first.data(), "rb");
        if(!t) {
            eprintf("Failed to open %s for reading.\n", archive_list[i].first.data());
            return EXIT_FAILURE;
        }
        std::fseek(t, 0, SEEK_END);
        long size = std::ftell(t);
        archive_entry_set_size(entry, size);
        std::fseek(t, 0, SEEK_SET);

        // Make a pointer and read the data
        std::unique_ptr<std::byte[]> data = std::make_unique<std::byte[]>(size);
        std::fread(data.get(), size, 1, t);
        std::fclose(t);

        // Archive that bastard
        archive_write_header(archive, entry);
        archive_write_data(archive, data.get(), size);

        // Close it
        archive_entry_free(entry);
    }

    // Save and close
    archive_write_close(archive);
    archive_write_free(archive);

    return EXIT_SUCCESS;
}
