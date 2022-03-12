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

#include "../build/build.hpp"

struct Format {
    const char *name;
    const char *extension;
    int (*filter)(archive *a);
    int (*format)(archive *a);
};

static const constexpr Format formats[] = {
    {"7z", ".7z", nullptr, archive_write_set_format_7zip},
    {"tar-gz", ".tar.xz", archive_write_add_filter_gzip, archive_write_set_format_pax_restricted},
    {"tar-xz", ".tar.xz", archive_write_add_filter_xz, archive_write_set_format_pax_restricted},
    {"tar-zst", ".tar.zst", archive_write_add_filter_zstd, archive_write_set_format_pax_restricted},
    {"zip", ".zip", nullptr, archive_write_set_format_zip}
};

static std::string list_formats() {
    std::string f;
    for(auto &format : formats) {
        if(!f.empty()) {
            f = f + ", ";
        }
        f = f + format.name;
    }
    return f;
}

int main(int argc, const char **argv) {
    set_up_color_term();
    
    using namespace Invader;
    
    struct ArchiveOptions {
        bool single_tag = false;
        std::vector<std::filesystem::path> tags;
        std::vector<std::filesystem::path> tags_excluded;
        std::vector<std::filesystem::path> tags_excluded_same;
        std::string output;
        bool use_filesystem_path = false;
        bool copy = false;
        bool verbose = false;
        bool overwrite = false;
        std::optional<HEK::GameEngine> engine;
        const Format *format = &formats[0];
    } archive_options;

    static constexpr char DESCRIPTION[] = "Generate .tar.xz archives of the tags required to build a cache file.";
    static constexpr char USAGE[] = "[options] <scenario | -s tag.class>";
    
    std::string game_engine_arguments = std::string("Specify the game engine. This option is required. Valid engines are: ") + Build::get_comma_separated_game_engine_shorthands();
    std::string formats_argument = std::string("Specify format. Valid formats are: ") + list_formats() + ". Default format is 7z";
    
    const CommandLineOption options[] {
        CommandLineOption("format", 'F', 1, formats_argument.c_str(), "<format>"),
        CommandLineOption("info", 'i', 0, "Show credits, source info, and other info."),
        CommandLineOption("single-tag", 's', 0, "Archive a tag tree instead of a cache file."),
        CommandLineOption("tags", 't', 1, "Use the specified tags directory. Use multiple times to add more directories, ordered by precedence.", "<dir>"),
        CommandLineOption("exclude-matched", 'E', 1, "Exclude copying any tags that are also located in the specified directory and are functionally the same. Use multiple times to exclude multiple directories."),
        CommandLineOption("overwrite", 'O', 0, "Overwrite tags if they already exist if using --copy"),
        CommandLineOption("exclude", 'e', 1, "Exclude copying any tags that share a path with a tag in specified directory. Use multiple times to exclude multiple directories.", "<dir>"),
        CommandLineOption("output", 'o', 1, "Output to a specific file. Extension must be .tar.xz unless using --copy which then it's a directory.", "<file>"),
        CommandLineOption("fs-path", 'P', 0, "Use a filesystem path for the tag."),
        CommandLineOption("copy", 'C', 0, "Copy instead of making an archive."),
        CommandLineOption("verbose", 'v', 0, "Print whether or not tags are omitted,  Do verbose comparisons."),
        CommandLineOption("game-engine", 'g', 1, game_engine_arguments.c_str(), "<id>")
    };

    auto remaining_arguments = CommandLineOption::parse_arguments<ArchiveOptions &>(argc, argv, options, USAGE, DESCRIPTION, 1, 1, archive_options, [](char opt, const auto &arguments, auto &archive_options) {
        switch(opt) {
            case 'F': {
                bool found = false;
                for(auto &f : formats) {
                    if(std::strcmp(arguments[0], f.name) == 0) {
                        archive_options.format = &f;
                        found = true;
                        break;
                    }
                }
                if(!found) {
                    eprintf_error("Unknown archive format %s", arguments[0]);
                    std::exit(EXIT_FAILURE);
                }
                break;
            }
            case 't':
                archive_options.tags.push_back(arguments[0]);
                break;
            case 'e':
                archive_options.tags_excluded.push_back(arguments[0]);
                break;
            case 'E':
                archive_options.tags_excluded_same.push_back(arguments[0]);
                break;
            case 'o':
                archive_options.output = arguments[0];
                break;
            case 'O':
                archive_options.overwrite = true;
                break;
            case 'v':
                archive_options.verbose = true;
                break;
            case 'i':
                show_version_info();
                std::exit(EXIT_SUCCESS);
            case 's':
                archive_options.single_tag = true;
                break;
            case 'P':
                archive_options.use_filesystem_path = true;
                break;
            case 'g': {
                if(const auto *engine_maybe = HEK::GameEngineInfo::get_game_engine_info(arguments[0])) {
                    archive_options.engine = engine_maybe->engine;
                }
                else {
                    eprintf_error("Unknown engine %s", arguments[0]);
                    std::exit(EXIT_FAILURE);
                }
                
                break;
            }
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
    std::string base_tag;
    if(archive_options.use_filesystem_path) {
        // See if the tag path is valid
        std::optional<std::string> base_tag_maybe;
        if(std::filesystem::exists(remaining_arguments[0])) {
            base_tag_maybe = File::file_path_to_tag_path(remaining_arguments[0], archive_options.tags);
        }
        if(base_tag_maybe.has_value()) {
            base_tag = *base_tag_maybe;

            // Remove extension if necessary
            if(!archive_options.single_tag) {
                auto path_test = std::filesystem::path(base_tag);
                if(path_test.extension() != ".scenario") {
                    eprintf_error("This function only accepts scenario tags. To use other tags, use -s");
                    return EXIT_FAILURE;
                }
                base_tag = path_test.replace_extension().string();
            }
        }
        else {
            eprintf_error("Failed to find a valid%stag %s in the tags directory", archive_options.single_tag ? " " : " scenario ", remaining_arguments[0]);
            return EXIT_FAILURE;
        }
    }
    else {
        base_tag.insert(base_tag.end(), remaining_arguments[0], remaining_arguments[0] + std::strlen(remaining_arguments[0]));
    }

    // Variables to hold this
    std::vector<std::pair<std::filesystem::path, std::string>> archive_list;

    // If no output filename was given, make one
    const char *extension = archive_options.format->extension;
    if(archive_options.output.size() == 0) {
        // Set output
        archive_options.output = File::base_name(base_tag.data()) + ((archive_options.copy) ? "" : extension);
    }
    else {
        bool fail = true;
        auto extension_len = std::strlen(extension);
        if(archive_options.output.size() > extension_len) {
            fail = std::strcmp(archive_options.output.c_str() + archive_options.output.size() - extension_len + 1, extension) != 0;
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
    File::halo_path_to_preferred_path_chars(base_tag.data());
    File::remove_duplicate_slashes_chars(base_tag.data());

    if(!archive_options.single_tag) {
        // Build the map
        std::vector<std::byte> map;

        try {
            BuildWorkload::BuildParameters parameters(*archive_options.engine);
            parameters.scenario = base_tag;
            parameters.tags_directories = archive_options.tags;
            if(parameters.details.build_cache_file_engine == HEK::CacheFileEngine::CACHE_FILE_XBOX) {
                parameters.details.build_compression_level = 0;
            }
            if(parameters.details.build_cache_file_engine != HEK::CacheFileEngine::CACHE_FILE_NATIVE) {
                parameters.details.build_maximum_cache_file_size = UINT32_MAX;
            }
            parameters.verbosity = BuildWorkload::BuildParameters::BUILD_VERBOSITY_QUIET;

            map = BuildWorkload::compile_map(parameters);
        }
        catch(std::exception &e) {
            eprintf_error("Failed to compile scenario %s into a map: %s", base_tag.c_str(), e.what());
            return EXIT_FAILURE;
        }

        // Parse the map
        std::unique_ptr<Map> parsed_map;
        try {
            parsed_map = std::make_unique<Map>(Map::map_with_move(std::move(map)));
        }
        catch(std::exception &e) {
            eprintf_error("Failed to parse the map file generated with scenario %s: %s", base_tag.c_str(), e.what());
            return EXIT_FAILURE;
        }
        auto tag_count = parsed_map->get_tag_count();

        // Go through each tag and see if we can find everything.
        archive_list.reserve(tag_count + 64);
        
        auto archive_it = [&archive_options, &archive_list](const std::string &path, TagFourCC fourcc) {
            std::string full_tag_path = File::halo_path_to_preferred_path(path) + "." + tag_fourcc_to_extension(fourcc);

            // Check each tags directory if it exists. If so, archive it (todo: refactor to tag_path_to_file_path())
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
                std::exit(EXIT_FAILURE);
            }
        };
        
        for(std::size_t i = 0; i < tag_count; i++) {
            // Get the tag path information
            auto &tag = parsed_map->get_tag(i);
            archive_it(tag.get_path(), tag.get_tag_fourcc());
        }
        
        // Archive child scenarios
        try {
            auto path = Invader::File::tag_path_to_file_path(base_tag + ".scenario", archive_options.tags);
            auto scenario_data = Invader::File::open_file(path.value()).value();
            auto scenario_ptr = Invader::Parser::ParserStruct::parse_hek_tag_file(scenario_data.data(), scenario_data.size());
            auto &scenario = dynamic_cast<Invader::Parser::Scenario &>(*scenario_ptr);
            for(auto &child : scenario.child_scenarios) {
                if(!child.child_scenario.path.empty()) {
                    archive_it(child.child_scenario.path, child.child_scenario.tag_fourcc);
                }
            }
        }
        catch(std::exception &e) {
            eprintf_error("Failed to get dependencies of %s.scenario: %s", base_tag.c_str(), e.what());
            return EXIT_FAILURE;
        }
    }
    else {
        // Turn it into something the filesystem can understand
        auto base_tag_split_maybe = File::split_tag_class_extension_chars(base_tag.data());

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
        auto dependencies = FoundTagDependency::find_dependencies(base_tag_split.path.c_str(), base_tag_split.fourcc, archive_options.tags, false, true, success);
        if(!success) {
            eprintf_error("Failed to find dependencies for %s. Archive could not be made.", base_tag.data());
            return EXIT_FAILURE;
        }

        // Make sure there aren't any broken dependencies
        for(auto &dependency : dependencies) {
            if(dependency.broken) {
                eprintf_error("%s.%s is missing (broken dependency). Archive could not be made.", dependency.path.c_str(), tag_fourcc_to_extension(dependency.fourcc));
                return EXIT_FAILURE;
            }

            std::string path_copy = File::halo_path_to_preferred_path(dependency.path + "." + tag_fourcc_to_extension(dependency.fourcc));
            archive_list.emplace_back(*dependency.file_path, path_copy);
        }
    }

    // Don't archive anything that is in an excluded directory
    for(auto &i : archive_options.tags_excluded) {
        for(std::size_t t = 0; t < archive_list.size(); t++) {
            // First check if it exists
            auto path_to_test = i / File::halo_path_to_preferred_path(archive_list[t].second);
            if(std::filesystem::exists(path_to_test)) {
                // Exclude
                archive_list.erase(archive_list.begin() + t);
                t--;
            }
        }
    }
    
    for(auto &i : archive_options.tags_excluded_same) {
        for(std::size_t t = 0; t < archive_list.size(); t++) {
            // First check if it exists
            auto path_to_test = i / File::halo_path_to_preferred_path(archive_list[t].second);
            
            if(std::filesystem::exists(path_to_test)) {
                // Okay it exists. Open both then
                std::list<std::string> differences;
                    
                try {
                    auto tag_archive_data = File::open_file(archive_list[t].first).value();
                    auto tag_archive = Parser::ParserStruct::parse_hek_tag_file(tag_archive_data.data(), tag_archive_data.size(), true);

                    auto tag_exclude_data = File::open_file(path_to_test).value();
                    auto tag_exclude = Parser::ParserStruct::parse_hek_tag_file(tag_exclude_data.data(), tag_exclude_data.size(), true);

                    // Do a functional comparison
                    if(!tag_archive->compare(tag_exclude.get(), true, true, archive_options.verbose ? &differences : nullptr)) {
                        continue;
                    }
                }
                catch (std::exception &) {
                    eprintf_error("Failed to do a functional comparison of %s and %s\n", archive_list[t].first.string().c_str(), path_to_test.string().c_str());
                    std::exit(EXIT_FAILURE);
                }
                
                if(archive_options.verbose) {
                    std::printf("Omitting %s\n", archive_list[t].second.c_str());
                    
                    for(auto &i : differences) {
                        eprintf("%s\n", i.c_str());
                    }
                }
                
                archive_list.erase(archive_list.begin() + t);
                t--;
            }
        }
    }
    
    // If we eliminate all tags, don't bother archiving anything
    if(archive_list.size() == 0) {
        oprintf_success_warn("There were no tags to archive");
        return EXIT_SUCCESS;
    }

    // Archive
    if(!archive_options.copy) {
        // Begin making the archive
        auto *archive = archive_write_new();
        if(archive_options.format->filter) {
            archive_options.format->filter(archive);
        }
        if(archive_options.format->format) {
            archive_options.format->format(archive);
        }
        archive_write_open_filename(archive, archive_options.output.c_str());

        // Go through each tag path we got
        for(std::size_t i = 0; i < archive_list.size(); i++) {
            auto str_path = archive_list[i].first.string();
            for(char &c : str_path) {
                if(c == std::filesystem::path::preferred_separator) {
                    c = '/';
                }
            }
            const char *path = str_path.c_str();

            // Begin
            auto *entry = archive_entry_new();
            archive_entry_set_pathname(entry, archive_list[i].second.c_str());
            archive_entry_set_perm(entry, 0644);
            archive_entry_set_filetype(entry, AE_IFREG);

            auto data_maybe = File::open_file(path);
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
            auto place_if_possible = [&new_path, &old_path, &archive_options]() -> bool {
                // If it exists, continue
                if(!archive_options.overwrite && std::filesystem::exists(new_path)) {
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
                    std::filesystem::copy_file(old_path, new_path, std::filesystem::copy_options::overwrite_existing);
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
