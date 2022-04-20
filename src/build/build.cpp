// SPDX-License-Identifier: GPL-3.0-only

#include <iostream>
#include <fstream>
#include <vector>
#include <cstring>
#include <filesystem>

#include <invader/build/build_workload.hpp>
#include <invader/compress/compression.hpp>
#include <invader/map/map.hpp>
#include <invader/version.hpp>
#include <invader/printf.hpp>
#include "../command_line_option.hpp"
#include <invader/file/file.hpp>
#include <invader/tag/index/index.hpp>

static std::uint32_t read_str32(const char *err, const char *s) {
    // Make sure it starts with '0x'
    if(std::strncmp(s, "0x", 2) != 0) {
        eprintf_error("%s %s (must be hexadecimal)", err, s);
        std::exit(EXIT_FAILURE);
    }
    
    // Check the string length
    std::size_t given_crc32_length = std::strlen(s);
    if(given_crc32_length > 10 || given_crc32_length < 3) {
        eprintf_error("%s %s (must be 1-8 digits)", err, s);
        std::exit(EXIT_FAILURE);
    }
    
    // Now, make sure it's all valid
    for(std::size_t i = 2; i < given_crc32_length; i++) {
        char c = std::tolower(s[i]);
        if(!(c >= '0' && c <= '9') && !(c >= 'a' && c <= 'f')) {
            eprintf_error("%s %s (must be hexadecimal)", err, s);
            std::exit(EXIT_FAILURE);
        }
    }
    
    return static_cast<std::uint32_t>(std::strtoul(s + 2, nullptr, 16));
}

int main(int argc, const char **argv) {
    set_up_color_term();

    using namespace Invader;
    using namespace Invader::HEK;
    
    using RawDataHandling = BuildWorkload::BuildParameters::BuildParametersDetails::RawDataHandling;

    // Parameters
    struct BuildOptions {
        std::filesystem::path maps = "maps";
        std::filesystem::path data = "data";
        std::optional<std::filesystem::path> resource_map_path;
        std::vector<std::filesystem::path> tags;
        std::optional<std::filesystem::path> output;
        std::string last_argument;
        std::string index;
        std::optional<const HEK::GameEngineInfo *> engine;
        bool handled = true;
        bool quiet = false;
        RawDataHandling raw_data_handling = RawDataHandling::RAW_DATA_HANDLING_RETAIN_ALL;
        std::optional<std::uint32_t> forged_crc;
        bool use_filesystem_path = false;
        std::optional<std::string> rename_scenario;
        bool optimize_space = false;
        bool hide_pedantic_warnings = false;
        std::optional<int> compression_level;
        bool increased_file_size_limits = false;
        std::optional<std::string> build_version;
        bool check_custom_edition_resource_bounds = false;
        std::optional<std::uint64_t> max_tag_space;
        bool auto_forge = false;
        bool do_not_auto_forge = false;
        bool use_anniverary_mode = false;
        bool use_tags_for_script_source = false;
    } build_options;
    
    const CommandLineOption options[] = {
        CommandLineOption::from_preset(CommandLineOption::PRESET_COMMAND_LINE_OPTION_INFO),
        CommandLineOption::from_preset(CommandLineOption::PRESET_COMMAND_LINE_OPTION_MAPS),
        CommandLineOption::from_preset(CommandLineOption::PRESET_COMMAND_LINE_OPTION_DATA),
        CommandLineOption::from_preset(CommandLineOption::PRESET_COMMAND_LINE_OPTION_TAGS_MULTIPLE),
        CommandLineOption::from_preset(CommandLineOption::PRESET_COMMAND_LINE_OPTION_FS_PATH),
        CommandLineOption::from_preset(CommandLineOption::PRESET_COMMAND_LINE_OPTION_GAME_ENGINE),
        CommandLineOption("quiet", 'q', 0, "Only output error messages."),
        CommandLineOption("script-source", 'S', 1, "Specify the script source data location. Can be \"data\" or \"tags\". Default: data"),
        CommandLineOption("with-index", 'w', 1, "Use an index file for the tags, ensuring the map's tags are ordered in the same way.", "<file>"),
        CommandLineOption("output", 'o', 1, "Output to a specific file.", "<file>"),
        CommandLineOption("auto-forge", 'A', 0, "Ensure the map will be network compatible with the target engine's stock maps."),
        CommandLineOption("forge-crc", 'C', 1, "Forge the CRC32 value of the map after building it.", "<crc>"),
        CommandLineOption("rename-scenario", 'N', 1, "Rename the scenario.", "<name>"),
        CommandLineOption("level", 'l', 1, "Set the compression level (Xbox maps only). Must be between 0 and 9. Default: 9", "<level>"),
        CommandLineOption("optimize", 'O', 0, "Optimize tag space. This will drastically increase the amount of time required to build the cache file."),
        CommandLineOption("hide-pedantic-warnings", 'H', 0, "Don't show minor warnings."),
        CommandLineOption("extend-file-limits", 'E', 0, "Extend file size limits to 2 GiB regardless of if the target engine will support the cache file."),
        CommandLineOption("build-string", 'B', 1, "Set the build string in the header.", "<ver>"),
        CommandLineOption("stock-resource-bounds", 'b', 0, "Only index tags if the tag's index is within stock Custom Edition's resource map bounds. (Custom Edition only)"),
        CommandLineOption("anniversary-mode", 'a', 0, "Enable anniversary graphics and audio (CEA only)"),
        CommandLineOption("resource-maps", 'R', 1, "Specify the directory for loading resource maps. (by default this is the maps directory)", "<dir>"),
        CommandLineOption("tag-space", 'T', 1, "Override the tag space. This may result in a map that does not work with the stock games. You can specify the number of bytes, optionally suffixing with K (for KiB) or M (for MiB), or specify in hexadecimal the number of bytes (e.g. 0x1000).", "<size>"),
        CommandLineOption("resource-usage", 'r', 1, "Specify the behavior for using resource maps. Must be: none (don't use resource maps), check (check resource maps), always (always index tags in resource maps - Custom Edition only). Default: none", "<usage>")
    };

    static constexpr char DESCRIPTION[] = "Build a cache file.";
    static constexpr char USAGE[] = "[options] -g <target> <scenario>";

    auto remaining_arguments = CommandLineOption::parse_arguments<BuildOptions &>(argc, argv, options, USAGE, DESCRIPTION, 1, 1, build_options, [](char opt, const auto &arguments, auto &build_options) {
        switch(opt) {
            case 'r':
                if(std::strcmp(arguments[0], "none") == 0) {
                    build_options.raw_data_handling = RawDataHandling::RAW_DATA_HANDLING_RETAIN_ALL;
                }
                else if(std::strcmp(arguments[0], "check") == 0) {
                    build_options.raw_data_handling = RawDataHandling::RAW_DATA_HANDLING_RETAIN_AUTOMATICALLY;
                }
                else if(std::strcmp(arguments[0], "always") == 0) {
                    build_options.raw_data_handling = RawDataHandling::RAW_DATA_HANDLING_ALWAYS_INDEX;
                }
                else {
                    eprintf_error("Unknown resource-usage parameter %s. Use -h for more information.", arguments[0]);
                    std::exit(EXIT_FAILURE);
                }
                break;
            case 'q':
                build_options.quiet = true;
                break;
            case 'a':
                build_options.use_anniverary_mode = true;
                break;
            case 'w':
                build_options.index = std::string(arguments[0]);
                break;
            case 't':
                build_options.tags.emplace_back(arguments[0]);
                break;
            case 'o':
                build_options.output = std::string(arguments[0]);
                break;
            case 'm':
                build_options.maps = std::string(arguments[0]);
                break;
            case 'R':
                build_options.resource_map_path = std::string(arguments[0]);
                break;
            case 'B':
                build_options.build_version = std::string(arguments[0]);
                if(build_options.build_version->size() >= 32) {
                    eprintf_error("Build string must be fewer than 32 characters.");
                    std::exit(EXIT_FAILURE);
                }
                break;
            case 'b':
                build_options.check_custom_edition_resource_bounds = true;
                break;
            case 'E':
                build_options.increased_file_size_limits = true;
                break;
            case 'l':
                try {
                    build_options.compression_level = std::stoi(arguments[0]);
                    if(build_options.compression_level < 0 || build_options.compression_level > 9) {
                        eprintf_error("Compression level must be between 0 and 9");
                        std::exit(EXIT_FAILURE);
                    }
                }
                catch(std::exception &) {
                    eprintf_error("Invalid compression level %s. Use -h for more information.", arguments[0]);
                    std::exit(EXIT_FAILURE);
                }
                break;
                
            case 'S':
                if(std::strcmp(arguments[0], "tags") == 0) {
                    build_options.use_tags_for_script_source = true;
                }
                else if(std::strcmp(arguments[0], "data") == 0) {
                    build_options.use_tags_for_script_source = false;
                }
                else {
                    eprintf_error("Unknown script-source parameter %s. Use -h for more information.", arguments[0]);
                    std::exit(EXIT_FAILURE);
                }
                break;
                
            case 'A':
                build_options.auto_forge = true;
                break;
                
            case 'g': {
                if(const auto *engine_maybe = HEK::GameEngineInfo::get_game_engine_info(arguments[0])) {
                    build_options.engine = engine_maybe;
                }
                else {
                    eprintf_error("Unknown engine %s. Use -h for more information.", arguments[0]);
                    std::exit(EXIT_FAILURE);
                }
                
                break;
            }
            case 'C':
                build_options.forged_crc = read_str32("Invalid CRC32", arguments[0]);
                break;
            case 'P':
                build_options.use_filesystem_path = true;
                break;
            case 'i':
                show_version_info();
                std::exit(EXIT_SUCCESS);
                break;
            case 'N':
                build_options.rename_scenario = arguments[0];
                break;
            case 'O':
                build_options.optimize_space = true;
                break;
            case 'H':
                build_options.hide_pedantic_warnings = true;
                break;
            case 'd':
                build_options.data = arguments[0];
                break;
            case 'T':
                try {
                    std::string arg = arguments[0];
                    std::size_t after = 0;
                    if(arg.size() >= 2 && arg.substr(0, 2) == "0x") {
                        build_options.max_tag_space = std::stoull(arguments[0] + 2, &after, 16);
                        if(arg.size() != 2 + after) {
                            throw std::exception(); // nope!
                        }
                    }
                    else {
                        unsigned long long tag_space_int = std::stoull(arg, &after);
                        unsigned long long multiplier = 1;

                        if(arg.size() != after) {
                            if(arg.size() != after + 1) {
                                throw std::exception(); // nope!
                            }
                            char s = arg[after];
                            switch(s) {
                                case 'K':
                                    multiplier = 1024;
                                    break;
                                case 'M':
                                    multiplier = 1024 * 1024;
                                    break;
                                default:
                                    throw std::exception();
                            }
                        }

                        // Check if we overflowed
                        unsigned long long final_tag_space = tag_space_int * multiplier;
                        if(final_tag_space / multiplier != tag_space_int) {
                            eprintf_error("overflowed");
                            throw std::exception();
                        }

                        // Done
                        build_options.max_tag_space = final_tag_space;
                    }
                }
                catch(std::exception &) {
                    eprintf_error("Invalid tag space size %s", arguments[0]);
                    std::exit(EXIT_FAILURE);
                }
                break;
        }
    });
    
    std::string scenario;

    // By default, just use tags
    if(build_options.tags.size() == 0) {
        build_options.tags.emplace_back("tags");
    }

    if(build_options.use_filesystem_path) {
        auto scenario_maybe = Invader::File::file_path_to_tag_path(remaining_arguments[0], build_options.tags);
        if(scenario_maybe.has_value()) std::printf("%s\n", scenario_maybe->c_str());
        if(scenario_maybe.has_value() && std::filesystem::exists(remaining_arguments[0])) {
            scenario = std::filesystem::path(*scenario_maybe).replace_extension().string();
        }
        else {
            eprintf_error("Failed to find a valid tag %s in the tags directory", remaining_arguments[0]);
            return EXIT_FAILURE;
        }
    }
    else {
        scenario = File::halo_path_to_preferred_path(remaining_arguments[0]);
    }

    try {
        // Get the index
        std::optional<std::vector<File::TagFilePath>> with_index;
        if(build_options.index.size()) {
            with_index = std::vector<File::TagFilePath>();
            
            std::fstream index_file(build_options.index, std::ios_base::in);
            std::string tag;
            
            if(!index_file.is_open()) {
                eprintf_error("Failed to open index file %s", build_options.index.c_str());
                return EXIT_FAILURE;
            }
            
            while(std::getline(index_file, tag)) {
                // Check if empty
                if(tag.size() == 0) {
                    break;
                }

                // Get the extension
                const char *extension = nullptr;
                for(char &c : tag) {
                    if(c == '.') {
                        extension = &c + 1;
                    }
                }

                if(!extension) {
                    eprintf_error("Invalid index given. \"%s\" is missing an extension.", tag.c_str());
                    return EXIT_FAILURE;
                }

                auto substr = tag.substr(0, extension - tag.c_str() - 1);
                const char *substr_c = substr.c_str();
                std::vector<char> substr_v(substr_c, substr_c + substr.size() + 1);
                File::preferred_path_to_halo_path_chars(substr_v.data());

                // Lowercase everything
                for(char &c : substr) {
                    c = std::tolower(c);
                }

                with_index->emplace_back(substr_v.data(), tag_extension_to_fourcc(extension));
            }
        }

        // Figure out our engine target
        if(!build_options.engine.has_value()) {
            eprintf_error("No engine target specified. Use -h for more information.");
            return EXIT_FAILURE;
        }
        
        const auto &engine_info = *(*build_options.engine);
        BuildWorkload::BuildParameters parameters(engine_info.engine);
        
        parameters.use_tags_for_script_data = build_options.use_tags_for_script_source;
        parameters.tags_directories = build_options.tags;
        parameters.data_directory = build_options.data;
        parameters.scenario = scenario;
        parameters.rename_scenario = build_options.rename_scenario;
        parameters.optimize_space = build_options.optimize_space;
        parameters.forge_crc = build_options.forged_crc;
        parameters.index = with_index;
        
        if(build_options.max_tag_space.has_value()) {
            parameters.details.build_maximum_tag_space = *build_options.max_tag_space;
        }
        
        if(build_options.build_version.has_value()) {
            parameters.details.build_version = *build_options.build_version;
        }
        
        if(!build_options.use_anniverary_mode) {
            parameters.details.build_flags_cea = HEK::CacheFileHeaderCEAFlags::CACHE_FILE_HEADER_CEA_FLAGS_USE_BITMAPS_CACHE | HEK::CacheFileHeaderCEAFlags::CACHE_FILE_HEADER_CEA_FLAGS_USE_SOUNDS_CACHE | HEK::CacheFileHeaderCEAFlags::CACHE_FILE_HEADER_CEA_FLAGS_CLASSIC_ONLY;
        }
        
        parameters.details.build_check_custom_edition_resource_map_bounds = build_options.check_custom_edition_resource_bounds;
        
        if(build_options.increased_file_size_limits) {
            if(engine_info.engine != HEK::GameEngine::GAME_ENGINE_NATIVE) {
                parameters.details.build_maximum_cache_file_size = static_cast<std::uint32_t>(INT32_MAX); // 2 GiB minus 1 byte
            }
        }
        if(build_options.compression_level.has_value()) {
            parameters.details.build_compression_level = build_options.compression_level;
        }
        
        // Do we need resource maps?
        bool require_resource_maps = engine_info.supports_external_resource_maps();
        
        // Don't allow resource maps if we can't use them!
        if(!require_resource_maps && build_options.raw_data_handling != RawDataHandling::RAW_DATA_HANDLING_RETAIN_ALL) {
            eprintf_error("Resource maps are not used for the target engine");
            return EXIT_FAILURE;
        }
        
        // Set this to false
        if(require_resource_maps && build_options.raw_data_handling == RawDataHandling::RAW_DATA_HANDLING_RETAIN_ALL) {
            require_resource_maps = false;
        }
        
        parameters.details.build_raw_data_handling = build_options.raw_data_handling;
        
        // Load resource maps
        if(require_resource_maps) {
            bool error = false;
            
            auto try_open = [](const std::filesystem::path &path) {
                auto file = File::open_file(path);
                if(!file.has_value()) {
                    eprintf_error("Failed to open %s", path.string().c_str());
                    std::exit(EXIT_FAILURE);
                }
                try {
                    return load_resource_map(file->data(), file->size());
                }
                catch(std::exception &e) {
                    eprintf_error("Failed to read %s: %s", path.string().c_str(), e.what());
                    std::exit(EXIT_FAILURE);
                }
            };
            
            // Where are the memes located?
            std::filesystem::path resource_target_path;
            const char *resource_error_prefix;
            if(!build_options.resource_map_path.has_value()) {
                resource_target_path = std::filesystem::path(build_options.maps);
                resource_error_prefix = "Maps";
            }
            else {
                resource_target_path = *build_options.resource_map_path;
                resource_error_prefix = "Resource";
            }
            
            auto bitmaps = resource_target_path / "bitmaps.map";
            auto sounds = resource_target_path / "sounds.map";
            if(parameters.details.build_cache_file_engine == HEK::CacheFileEngine::CACHE_FILE_CUSTOM_EDITION) {
                // Use loc for Custom Edition
                auto loc = resource_target_path / "loc.map";
                
                // Well, guess that's that
                if(!std::filesystem::is_regular_file(bitmaps) || !std::filesystem::is_regular_file(sounds) || !std::filesystem::is_regular_file(loc)) {
                    eprintf_error("%s directory is missing either bitmaps.map, sounds.map, OR loc.map", resource_error_prefix);
                    error = true;
                    goto show_me_the_spaghetti_code_error;
                }
                
                parameters.bitmap_data = try_open(bitmaps);
                parameters.sound_data = try_open(sounds);
                parameters.loc_data = try_open(loc);
            }
            else {
                // Well, guess that's that
                if(!std::filesystem::is_regular_file(bitmaps) || !std::filesystem::is_regular_file(sounds)) {
                    eprintf_error("%s directory is missing either bitmaps.map OR sounds.map", resource_error_prefix);
                    error = true;
                    goto show_me_the_spaghetti_code_error;
                }
                
                parameters.bitmap_data = try_open(bitmaps);
                parameters.sound_data = try_open(sounds);
            }
            
            show_me_the_spaghetti_code_error:
            if(error) {
                eprintf_error("Use -n if no resource maps are required.");
                return EXIT_FAILURE;
            }
        }
        
        if(build_options.hide_pedantic_warnings) {
            parameters.verbosity = BuildWorkload::BuildParameters::BuildVerbosity::BUILD_VERBOSITY_HIDE_PEDANTIC;
        }
        
        if(build_options.quiet) {
            parameters.verbosity = BuildWorkload::BuildParameters::BuildVerbosity::BUILD_VERBOSITY_QUIET;
        }

        // Set the map name
        std::string map_name;
        if(build_options.rename_scenario) {
            map_name = *build_options.rename_scenario;
        }
        else {
            map_name = File::base_name(scenario.c_str());
        }
        
        // CRC32 spoofing, indexing, etc.
        if(build_options.auto_forge) {
            if(!parameters.index.has_value()) {
                switch(engine_info.engine) {
                    case HEK::GameEngine::GAME_ENGINE_GEARBOX_RETAIL:
                        parameters.index = retail_indices(map_name.c_str());
                        break;
                    case HEK::GameEngine::GAME_ENGINE_GEARBOX_CUSTOM_EDITION:
                        parameters.index = custom_edition_indices(map_name.c_str());
                        
                        if(!parameters.forge_crc.has_value()) {
                            if(map_name == "beavercreek") {
                                parameters.forge_crc = 0x07B3876A;
                            }
                            else if(map_name == "bloodgulch") {
                                parameters.forge_crc = 0x7B309554;
                            }
                            else if(map_name == "boardingaction") {
                                parameters.forge_crc = 0xF4DEEF94;
                            }
                            else if(map_name == "carousel") {
                                parameters.forge_crc = 0x9C301A08;
                            }
                            else if(map_name == "chillout") {
                                parameters.forge_crc = 0x93C53C27;
                            }
                            else if(map_name == "damnation") {
                                parameters.forge_crc = 0x0FBA059D;
                            }
                            else if(map_name == "dangercanyon") {
                                parameters.forge_crc = 0xC410CD74;
                            }
                            else if(map_name == "deathisland") {
                                parameters.forge_crc = 0x1DF8C97F;
                            }
                            else if(map_name == "gephyrophobia") {
                                parameters.forge_crc = 0xD2872165;
                            }
                            else if(map_name == "hangemhigh") {
                                parameters.forge_crc = 0xA7C8B9C6;
                            }
                            else if(map_name == "icefields") {
                                parameters.forge_crc = 0x5EC1DEB7;
                            }
                            else if(map_name == "infinity") {
                                parameters.forge_crc = 0x0E7F7FE7;
                            }
                            else if(map_name == "longest") {
                                parameters.forge_crc = 0xC8F48FF6;
                            }
                            else if(map_name == "prisoner") {
                                parameters.forge_crc = 0x43B81A8B;
                            }
                            else if(map_name == "putput") {
                                parameters.forge_crc = 0xAF2F0B84;
                            }
                            else if(map_name == "ratrace") {
                                parameters.forge_crc = 0xF7F8E14C;
                            }
                            else if(map_name == "sidewinder") {
                                parameters.forge_crc = 0xBD95CF55;
                            }
                            else if(map_name == "timberland") {
                                parameters.forge_crc = 0x54446470;
                            }
                            else if(map_name == "wizard") {
                                parameters.forge_crc = 0xCF3359B1;
                            }
                        }
                        
                        break;
                    case HEK::GameEngine::GAME_ENGINE_GEARBOX_DEMO:
                        parameters.index = demo_indices(map_name.c_str());
                        break;
                    case HEK::GameEngine::GAME_ENGINE_MCC_COMBAT_EVOLVED_ANNIVERSARY:
                        parameters.index = mcc_cea_indices(map_name.c_str());
                        break;
                    default: break;
                }
            }
        }

        // Build!
        auto map = Invader::BuildWorkload::compile_map(parameters);
        
        static const char MAP_EXTENSION[] = ".map"; 
        auto map_name_with_extension = std::string(map_name) + MAP_EXTENSION;

        // Format path to maps/map_name.map if output not specified
        std::filesystem::path final_file;
        if(!build_options.output.has_value()) {
            final_file = std::filesystem::path(build_options.maps) / map_name_with_extension;
        }
        else {
            final_file = *build_options.output;
            auto final_file_name_no_extension = final_file.filename().replace_extension();
            auto final_file_name_no_extension_string = final_file_name_no_extension.string();
            
            // If it's not a .map, warn
            if(final_file.extension() != MAP_EXTENSION) {
                eprintf_warn("The base file extension is not \"%s\" which is required by the target engine", MAP_EXTENSION);
            }
            
            // If we are not building for MCC and the scenario name is mismatched, warn
            if(final_file_name_no_extension_string != map_name && engine_info.scenario_name_and_file_name_must_be_equal) {
                eprintf_warn("The base name (%s) does not match the scenario (%s)", final_file_name_no_extension_string.c_str(), map_name.c_str());
                eprintf_warn("The map will fail to load correctly in the target engine with this file name.");
                
                bool incorrect_case = false;
                for(char &c : final_file_name_no_extension_string) {
                    if(std::tolower(c) != c) {
                        incorrect_case = true;
                        break;
                    }
                }
                if(!incorrect_case) {
                    eprintf_warn("Did you intend to use --rename-scenario \"%s\"", final_file_name_no_extension_string.c_str());
                }
            }
        }
        
        // Save the file
        if(!File::save_file(final_file, map)) {
            eprintf_error("Failed to save %s", final_file.string().c_str());
            return EXIT_FAILURE;
        }

        return EXIT_SUCCESS;
    }
    catch(std::exception &exception) {
        eprintf_error("Failed to compile the map.");
        eprintf_error("%s", exception.what());
        return EXIT_FAILURE;
    }
}
