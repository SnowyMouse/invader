// SPDX-License-Identifier: GPL-3.0-only

#include <vector>
#include <string>
#include <filesystem>
#include <archive.h>
#include <archive_entry.h>
#include <invader/version.hpp>
#include <invader/printf.hpp>
#include <invader/build/build_workload.hpp>
#include <invader/map/map.hpp>
#include <invader/dependency/found_tag_dependency.hpp>
#include <invader/command_line_option.hpp>
#include <invader/file/file.hpp>

int main(int argc, const char **argv) {
    struct ArchiveOptions {
        bool single_tag = false;
        std::vector<std::filesystem::path> tags;
        std::string output;
        bool use_filesystem_path = false;
        bool copy = false;
        std::optional<Invader::HEK::CacheFileEngine> engine;
    } archive_options;

    static constexpr char DESCRIPTION[] = "Generate .tar.xz archives of the tags required to build a cache file.";
    static constexpr char USAGE[] = "[options] <scenario | -s tag.class>";

    std::vector<Invader::CommandLineOption> options;
    options.emplace_back("info", 'i', 0, "Show credits, source info, and other info.");
    options.emplace_back("single-tag", 's', 0, "Archive a tag tree instead of a cache file.");
    options.emplace_back("tags", 't', 1, "Use the specified tags directory. Use multiple times to add more directories, ordered by precedence.", "<dir>");
    options.emplace_back("output", 'o', 1, "Output to a specific file. Extension must be .tar.xz unless using --copy which then it's a directory.", "<file>");
    options.emplace_back("fs-path", 'P', 0, "Use a filesystem path for the tag.");
    options.emplace_back("copy", 'C', 0, "Copy instead of making an archive.");
    options.emplace_back("game-engine", 'g', 1, "Specify the game engine. This option is required if -s is not specified. Valid engines are: custom, demo, retail, xbox, native", "<id>");

    auto remaining_arguments = Invader::CommandLineOption::parse_arguments<ArchiveOptions &>(argc, argv, options, USAGE, DESCRIPTION, 1, 1, archive_options, [](char opt, const auto &arguments, auto &archive_options) {
        switch(opt) {
            case 't':
                archive_options.tags.push_back(arguments[0]);
                break;
            case 'o':
                archive_options.output = arguments[0];
                break;
            case 'i':
                Invader::show_version_info();
                std::exit(EXIT_SUCCESS);
            case 's':
                archive_options.single_tag = true;
                break;
            case 'P':
                archive_options.use_filesystem_path = true;
                break;
            case 'g':
                if(std::strcmp(arguments[0], "custom") == 0) {
                    archive_options.engine = Invader::HEK::CacheFileEngine::CACHE_FILE_CUSTOM_EDITION;
                }
                else if(std::strcmp(arguments[0], "retail") == 0) {
                    archive_options.engine = Invader::HEK::CacheFileEngine::CACHE_FILE_RETAIL;
                }
                else if(std::strcmp(arguments[0], "demo") == 0) {
                    archive_options.engine = Invader::HEK::CacheFileEngine::CACHE_FILE_DEMO;
                }
                else if(std::strcmp(arguments[0], "native") == 0) {
                    archive_options.engine = Invader::HEK::CacheFileEngine::CACHE_FILE_NATIVE;
                }
                else if(std::strcmp(arguments[0], "xbox") == 0) {
                    archive_options.engine = Invader::HEK::CacheFileEngine::CACHE_FILE_XBOX;
                }
                break;
            case 'C':
                archive_options.copy = true;
                break;
        }
    });

    // Figure out our engine target
    if(!archive_options.engine.has_value() && !archive_options.single_tag) {
        eprintf_error("No engine target specified for map archival. Use -h for more information.");
        return EXIT_FAILURE;
    }

    // No tags folder? Use tags in current directory
    if(archive_options.tags.size() == 0) {
        archive_options.tags.emplace_back("tags");
    }

    // Require a tag
    std::vector<char> base_tag;
    if(archive_options.use_filesystem_path) {
        // See if the tag path is valid
        std::optional<std::string> base_tag_maybe;
        if(archive_options.single_tag) {
            base_tag_maybe = Invader::File::file_path_to_tag_path(remaining_arguments[0], archive_options.tags, true);
        }
        else {
            base_tag_maybe = Invader::File::file_path_to_tag_path_with_extension(remaining_arguments[0], archive_options.tags, std::string(".scenario"));
        }
        if(base_tag_maybe.has_value()) {
            const char *str = (*base_tag_maybe).c_str();
            base_tag.insert(base_tag.end(), str, str + (*base_tag_maybe).size());
        }
        else {
            eprintf_error("Failed to find a valid%stag %s in the tags directory", archive_options.single_tag ? " " : " scenario ", remaining_arguments[0]);
            if(!archive_options.single_tag && Invader::File::file_path_to_tag_path(remaining_arguments[0], archive_options.tags, true).has_value()) {
                eprintf_error("A file was detected there, but it isn't a .scenario tag, so you need to use -s");
            }
            return EXIT_FAILURE;
        }
    }
    else {
        base_tag.insert(base_tag.end(), remaining_arguments[0], remaining_arguments[0] + std::strlen(remaining_arguments[0]));
    }
    base_tag.emplace_back();

    // Variables to hold this
    std::vector<std::pair<std::filesystem::path, std::string>> archive_list;

    // If no output filename was given, make one
    static const char extension[] = ".tar.xz";
    if(archive_options.output.size() == 0) {
        // Set output
        archive_options.output = Invader::File::base_name(base_tag.data()) + ((archive_options.copy) ? "" : extension);
    }
    else {
        bool fail = true;
        if(archive_options.output.size() > sizeof(extension)) {
            fail = std::strcmp(archive_options.output.c_str() + archive_options.output.size() - sizeof(extension) + 1, extension) != 0;
        }

        if(!archive_options.copy) {
            if(fail) {
                eprintf_error("Invalid output file path %s. This should end with %s.\n", archive_options.output.c_str(), extension);
                return EXIT_FAILURE;
            }
        }
        else {
            if(!fail) {
                eprintf_warn("Output directory path %s ends with %s.\nThis is technically valid, but you probably didn't want to do this.", archive_options.output.c_str(), extension);
            }
        }
    }

    // Fix this a bit
    Invader::File::halo_path_to_preferred_path_chars(base_tag.data());
    Invader::File::remove_duplicate_slashes_chars(base_tag.data());

    if(!archive_options.single_tag) {
        // Build the map
        std::vector<std::byte> map;

        try {
            Invader::BuildWorkload::BuildParameters parameters(*archive_options.engine);
            parameters.scenario = base_tag.data();
            parameters.tags_directories = archive_options.tags;
            if(archive_options.engine == Invader::HEK::CacheFileEngine::CACHE_FILE_XBOX) {
                parameters.details.build_compress = true;
                parameters.details.build_compression_level = 0;
            }
            else {
                parameters.details.build_compress = false;
            }
            if(archive_options.engine != Invader::HEK::CacheFileEngine::CACHE_FILE_NATIVE) {
                parameters.details.build_maximum_cache_file_size = UINT32_MAX;
            }
            parameters.verbosity = Invader::BuildWorkload::BuildParameters::BUILD_VERBOSITY_QUIET;
            
            map = Invader::BuildWorkload::compile_map(parameters);
        }
        catch(std::exception &e) {
            eprintf_error("Failed to compile scenario %s into a map", base_tag.data());
            return EXIT_FAILURE;
        }

        // Parse the map
        std::unique_ptr<Invader::Map> parsed_map;
        try {
            parsed_map = std::make_unique<Invader::Map>(Invader::Map::map_with_move(std::move(map)));
        }
        catch(std::exception &e) {
            eprintf_error("Failed to parse the map file generated with scenario %s: %s", base_tag.data(), e.what());
            return EXIT_FAILURE;
        }
        auto tag_count = parsed_map->get_tag_count();

        // Go through each tag and see if we can find everything.
        archive_list.reserve(tag_count);
        for(std::size_t i = 0; i < tag_count; i++) {
            // Get the tag path information
            auto &tag = parsed_map->get_tag(i);
            std::string full_tag_path = Invader::File::halo_path_to_preferred_path(std::string(tag.get_path()) + "." + tag_class_to_extension(tag.get_tag_class_int()));

            // Check each tags directory if it exists. If so, archive it
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
                eprintf_error("Failed to find %s. Archive could not be made.", full_tag_path.c_str());
                return EXIT_FAILURE;
            }
        }
    }
    else {
        // Turn it into something the filesystem can understand
        auto base_tag_split_maybe = Invader::File::split_tag_class_extension_chars(base_tag.data());

        // Split the extension
        if(!base_tag_split_maybe.has_value()) {
            eprintf_error("%s is not a valid tag. Archive could not be made.", base_tag.data());
            return EXIT_FAILURE;
        }

        // Add it
        bool exists = false;
        for(auto &dir : archive_options.tags) {
            std::filesystem::path tag_path = std::filesystem::path(dir) / base_tag.data();
            if(std::filesystem::exists(tag_path)) {
                exists = true;
                archive_list.emplace_back(tag_path.string(), base_tag.data());
                break;
            }
        }

        if(!exists) {
            eprintf_error("Failed to find %s. Archive could not be made.", base_tag.data());
            return EXIT_FAILURE;
        }

        // Now find its dependencies
        bool success;
        auto &base_tag_split = base_tag_split_maybe.value();
        auto dependencies = Invader::FoundTagDependency::find_dependencies(base_tag_split.path.c_str(), base_tag_split.class_int, archive_options.tags, false, true, success);
        if(!success) {
            eprintf_error("Failed to find dependencies for %s. Archive could not be made.", base_tag.data());
            return EXIT_FAILURE;
        }

        // Make sure there aren't any broken dependencies
        for(auto &dependency : dependencies) {
            if(dependency.broken) {
                eprintf_error("%s.%s is missing (broken dependency). Archive could not be made.", dependency.path.c_str(), tag_class_to_extension(dependency.class_int));
                return EXIT_FAILURE;
            }

            std::string path_copy = Invader::File::halo_path_to_preferred_path(dependency.path + "." + tag_class_to_extension(dependency.class_int));
            archive_list.emplace_back(*dependency.file_path, path_copy);
        }
    }
    
    // Archive
    if(!archive_options.copy) {
        // Begin making the archive
        auto *archive = archive_write_new();
        archive_write_add_filter_xz(archive);
        archive_write_set_format_pax_restricted(archive);
        archive_write_open_filename(archive, archive_options.output.c_str());

        // Go through each tag path we got
        for(std::size_t i = 0; i < archive_list.size(); i++) {
            auto str_path = archive_list[i].first.string();
            const char *path = str_path.c_str();

            // Begin
            auto *entry = archive_entry_new();
            archive_entry_set_pathname(entry, archive_list[i].second.c_str());
            archive_entry_set_perm(entry, 0644);
            archive_entry_set_filetype(entry, AE_IFREG);

            auto data_maybe = Invader::File::open_file(path);
            if(!data_maybe.has_value()) {
                eprintf_error("Failed to open %s\n", path);
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
        
        oprintf("Saved %s\n", archive_options.output.c_str());
    }
    // Copy
    else {
        // Make the directory if it doesn't yet exist
        auto base_path = std::filesystem::path(archive_options.output.c_str());
        if(!std::filesystem::is_directory(base_path)) {
            try {
                std::filesystem::create_directory(base_path);
            }
            catch(std::exception &e) {
                eprintf_error("Failed to create directory %s: %s", base_path.string().c_str(), e.what());
                return EXIT_FAILURE;
            }
        }
        
        // Go through each file to archive
        for(std::size_t i = 0; i < archive_list.size(); i++) {
            auto old_path = std::filesystem::path(archive_list[i].first.c_str());
            auto new_path = base_path / std::filesystem::path(archive_list[i].second.c_str());
            
            // Copy function
            auto place_if_possible = [&new_path, &old_path]() -> bool {
                // If it exists, continue
                if(std::filesystem::exists(new_path)) {
                    return false;
                }
                
                // Try to see if we need to create the directory
                auto up_one_dir = new_path.parent_path();
                if(!std::filesystem::exists(up_one_dir)) {
                    try {
                        std::filesystem::create_directories(up_one_dir);
                    }
                    catch(std::exception &e) {
                        eprintf_error("Failed to create directory %s: %s", up_one_dir.string().c_str(), e.what());
                        return false;
                    }
                }
                
                // Now copy
                try {
                    std::filesystem::copy_file(old_path, new_path);
                }
                catch(std::exception &e) {
                    eprintf_error("Failed to create copy %s to %s: %s", old_path.string().c_str(), new_path.string().c_str(), e.what());
                    return false;
                }
                
                return true;
            };
            
            if(place_if_possible()) {
                oprintf_success("Saved %s", new_path.string().c_str());
            }
            else {
                eprintf_warn("Skipping %s...", new_path.string().c_str());
            }
        }
    }

    return EXIT_SUCCESS;
}
