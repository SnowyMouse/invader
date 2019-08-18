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
#include "../eprintf.hpp"

#include "../build/build_workload.hpp"
#include "../map/map.hpp"

int main(int argc, const char **argv) {
    if(argc < 4) {
        eprintf("Usage: %s <scenario> <output> <tags> [tags2 [...]]\n", argv[0]);
        return EXIT_FAILURE;
    }

    // Add tags to the array
    std::vector<std::string> tags;
    for(int i = 3; i < argc; i++) {
        tags.emplace_back(argv[i]);
    }

    // Build the map
    auto map = Invader::BuildWorkload::compile_map(argv[1], tags, "", std::vector<std::tuple<Invader::HEK::TagClassInt, std::string>>(), true, false, false);

    // Parse the map
    auto parsed_map = Invader::Map::map_with_pointer(map.data(), map.size());
    auto tag_count = parsed_map.tag_count();

    // Go through each tag and see if we can find everything.
    std::vector<std::string> archive_list;
    std::vector<std::string> archive_list_internal;
    archive_list.reserve(tag_count);
    archive_list_internal.reserve(tag_count);
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
                archive_list_internal.push_back(full_tag_path);
                break;
            }
        }

        if(!exists) {
            eprintf("Failed to find %s. Archive could not be made.\n", full_tag_path.data());
            return EXIT_FAILURE;
        }
    }

    // Begin making the archive
    auto *archive = archive_write_new();
    archive_write_add_filter_xz(archive);
    archive_write_set_format_pax_restricted(archive);
    archive_write_open_filename(archive, argv[2]);

    // Go through each tag path we got
    for(std::size_t i = 0; i < tag_count; i++) {
        // Begin
        auto *entry = archive_entry_new();
        archive_entry_set_pathname(entry, archive_list_internal[i].data());
        archive_entry_set_perm(entry, 0644);
        archive_entry_set_filetype(entry, AE_IFREG);

        // Open the tag
        std::FILE *t = std::fopen(archive_list[i].data(), "rb");
        if(!t) {
            eprintf("Failed to open %s for reading.\n", archive_list[i].data());
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
