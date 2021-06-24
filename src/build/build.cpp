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
#include <invader/command_line_option.hpp>
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

enum XboxVariation {
    XBOX_JP,
    XBOX_TW
};

int main(int argc, const char **argv) {
    using namespace Invader;
    using namespace Invader::HEK;
    
    using RawDataHandling = BuildWorkload::BuildParameters::BuildParametersDetails::RawDataHandling;

    // Parameters
    struct BuildOptions {
        std::filesystem::path maps = "maps";
        std::optional<std::filesystem::path> resource_map_path;
        std::vector<std::filesystem::path> tags;
        std::optional<std::filesystem::path> output;
        std::string last_argument;
        std::string index;
        std::optional<HEK::CacheFileEngine> engine;
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
        std::optional<HEK::CacheFileEngine> auto_forge_target;
        bool do_not_auto_forge = false;
        bool cea_anniversary = false;
        
        std::optional<XboxVariation> variation;
    } build_options;
    
    #define VALID_ENGINES_LIST "mcc-cea, pc-custom, pc-demo, pc-retail, xbox-0009, xbox-0135, xbox-2276"

    std::vector<CommandLineOption> options;
    options.emplace_back("quiet", 'q', 0, "Only output error messages.");
    options.emplace_back("info", 'i', 0, "Show credits, source info, and other info.");
    options.emplace_back("game-engine", 'g', 1, "Specify the game engine. This option is required. Valid engines are: " VALID_ENGINES_LIST, "<engine>");
    options.emplace_back("with-index", 'w', 1, "Use an index file for the tags, ensuring the map's tags are ordered in the same way.", "<file>");
    options.emplace_back("maps", 'm', 1, "Use the specified maps directory.", "<dir>");
    options.emplace_back("resource-path", 'R', 1, "Specify the directory for loading resource maps. (by default this is the maps directory)", "<dir>");
    options.emplace_back("tags", 't', 1, "Use the specified tags directory. Use multiple times to add more directories, ordered by precedence.", "<dir>");
    options.emplace_back("output", 'o', 1, "Output to a specific file.", "<file>");
    options.emplace_back("auto-forge", 'A', 1, "Ensure the map will be network compatible with the given target engine. Valid engines are: " VALID_ENGINES_LIST, "<engine>");
    options.emplace_back("forge-crc", 'C', 1, "Forge the CRC32 value of the map after building it.", "<crc>");
    options.emplace_back("tag-space", 'T', 1, "Override the tag space. This may result in a map that does not work with the stock games. You can specify the number of bytes, optionally suffixing with K (for KiB), M (for MiB), or G (for GiB), or specify in hexadecimal the number of bytes (e.g. 0x1000).", "<size>");
    options.emplace_back("fs-path", 'P', 0, "Use a filesystem path for the tag.");
    options.emplace_back("rename-scenario", 'N', 1, "Rename the scenario.", "<name>");
    options.emplace_back("level", 'l', 1, "Set the compression level (Xbox maps only). Must be between 0 and 9. Default: 9", "<level>");
    options.emplace_back("optimize", 'O', 0, "Optimize tag space. This will drastically increase the amount of time required to build the cache file.");
    options.emplace_back("hide-pedantic-warnings", 'H', 0, "Don't show minor warnings.");
    options.emplace_back("extend-file-limits", 'E', 0, "Extend file size limits beyond what is allowed by the target engine to its theoretical maximum size. This may create a map that will not work without a mod.");
    options.emplace_back("build-version", 'B', 1, "Set the build version. This is used on the Xbox version of the game (by default it's 01.10.12.2276 on Xbox and the Invader version on other engines)");
    options.emplace_back("stock-resource-bounds", 'b', 0, "Only index tags if the tag's index is within stock Custom Edition's resource map bounds. (Custom Edition only)");
    options.emplace_back("anniversary", 'a', 0, "Enable anniversary graphics and audio (CEA only)");
    options.emplace_back("resource-maps", 'r', 1, "Specify the behavior for using resource maps. Must be: none (don't use resource maps), check (check tags), always (always index tags - Custom Edition only). Default: none", "<method>");

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
                break;
            case 'q':
                build_options.quiet = true;
                break;
            case '3':
                build_options.cea_anniversary = true;
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
                break;
            case 'b':
                build_options.check_custom_edition_resource_bounds = true;
                break;
            case 'E':
                build_options.increased_file_size_limits = true;
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
                                case 'G':
                                    multiplier = 1024 * 1024 * 1024;
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
            case 'l':
                try {
                    build_options.compression_level = std::stoi(arguments[0]);
                    if(build_options.compression_level < 0 || build_options.compression_level > 9) {
                        eprintf_error("Compression level must be between 0 and 9");
                        std::exit(EXIT_FAILURE);
                    }
                }
                catch(std::exception &) {
                    eprintf_error("Invalid compression level %s", arguments[0]);
                    std::exit(EXIT_FAILURE);
                }
                break;
            case 'A':
            case 'g': {
                HEK::CacheFileEngine engine;
                std::optional<XboxVariation> variation;
                
                variation = std::nullopt;
                if(std::strcmp(arguments[0], "mcc-cea") == 0) {
                    engine = HEK::CacheFileEngine::CACHE_FILE_MCC_CEA;
                }
                else if(std::strcmp(arguments[0], "pc-custom") == 0) {
                    engine = HEK::CacheFileEngine::CACHE_FILE_CUSTOM_EDITION;
                }
                else if(std::strcmp(arguments[0], "pc-retail") == 0) {
                    engine = HEK::CacheFileEngine::CACHE_FILE_RETAIL;
                }
                else if(std::strcmp(arguments[0], "pc-demo") == 0) {
                    engine = HEK::CacheFileEngine::CACHE_FILE_DEMO;
                }
                else if(std::strcmp(arguments[0], "xbox-2276") == 0) {
                    engine = HEK::CacheFileEngine::CACHE_FILE_XBOX;
                }
                else if(std::strcmp(arguments[0], "xbox-0009") == 0) {
                    engine = HEK::CacheFileEngine::CACHE_FILE_XBOX;
                    variation = XboxVariation::XBOX_JP;
                }
                else if(std::strcmp(arguments[0], "xbox-0135") == 0) {
                    engine = HEK::CacheFileEngine::CACHE_FILE_XBOX;
                    variation = XboxVariation::XBOX_TW;
                }
                else if(std::strcmp(arguments[0], "native") == 0) {
                    engine = HEK::CacheFileEngine::CACHE_FILE_NATIVE;
                }
                else if(std::strcmp(arguments[0], "none") == 0 && opt == 'A') {
                    build_options.do_not_auto_forge = true;
                    build_options.auto_forge_target = std::nullopt;
                    break;
                }
                else {
                    eprintf_error("Unknown engine type: %s", arguments[0]);
                    
                    // We changed it
                    if(std::strcmp(arguments[0], "custom") == 0) {
                        eprintf_error("Did you mean pc-custom?");
                    }
                    else if(std::strcmp(arguments[0], "retail") == 0) {
                        eprintf_error("Did you mean pc-retail?");
                    }
                    else if(std::strcmp(arguments[0], "demo") == 0) {
                        eprintf_error("Did you mean pc-demo?");
                    }
                    else if(std::strcmp(arguments[0], "xbox-en") == 0) {
                        eprintf_error("Did you mean xbox-2276?");
                    }
                    else if(std::strcmp(arguments[0], "xbox-jp") == 0) {
                        eprintf_error("Did you mean xbox-0009?");
                    }
                    else if(std::strcmp(arguments[0], "xbox-tw") == 0) {
                        eprintf_error("Did you mean xbox-0135?");
                    }
                    
                    std::exit(EXIT_FAILURE);
                }
                
                if(opt == 'g') {
                    build_options.engine = engine;
                    build_options.variation = variation;
                }
                if(opt == 'A' || (!build_options.auto_forge_target.has_value() && build_options.do_not_auto_forge)) {
                    build_options.auto_forge_target = engine;
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
        
        BuildWorkload::BuildParameters parameters(*build_options.engine);
        
        if(build_options.variation.has_value() && !build_options.max_tag_space.has_value()) {
            switch(*build_options.variation) {
                case XboxVariation::XBOX_JP:
                    build_options.max_tag_space = 0x1648000;
                    parameters.details.build_version = "01.03.14.0009";
                    break;
                case XboxVariation::XBOX_TW:
                    build_options.max_tag_space = 0x167D000;
                    parameters.details.build_version = "01.12.09.0135";
                    break;
            }
        }
        
        parameters.tags_directories = build_options.tags;
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
        
        if(!build_options.cea_anniversary) {
            parameters.details.build_flags_cea = HEK::CacheFileHeaderCEAFlags::CACHE_FILE_HEADER_CEA_FLAGS_USE_BITMAPS_CACHE | HEK::CacheFileHeaderCEAFlags::CACHE_FILE_HEADER_CEA_FLAGS_USE_SOUNDS_CACHE | HEK::CacheFileHeaderCEAFlags::CACHE_FILE_HEADER_CEA_FLAGS_CLASSIC_ONLY;
        }
        
        parameters.details.build_check_custom_edition_resource_map_bounds = build_options.check_custom_edition_resource_bounds;
        
        if(build_options.increased_file_size_limits) {
            if(build_options.engine != HEK::CacheFileEngine::CACHE_FILE_NATIVE) {
                parameters.details.build_maximum_cache_file_size = UINT32_MAX;
            }
        }
        if(build_options.compression_level.has_value()) {
            parameters.details.build_compression_level = build_options.compression_level;
        }
        
        // Do we need resource maps?
        bool require_resource_maps;
        switch(*build_options.engine) {
            case HEK::CacheFileEngine::CACHE_FILE_NATIVE:
            case HEK::CacheFileEngine::CACHE_FILE_XBOX:
                require_resource_maps = false;
                break;
            default:
                require_resource_maps = true;
        }
        
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
            if(parameters.details.build_cache_file_engine == HEK::CacheFileEngine::CACHE_FILE_CUSTOM_EDITION || parameters.details.build_cache_file_engine == HEK::CacheFileEngine::CACHE_FILE_MCC_CEA) {
                // Use loc for MCC and Custom Edition
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
        if(build_options.auto_forge_target.has_value()) {
            auto auto_forge_target = *build_options.auto_forge_target;
            
            if(!parameters.index.has_value()) {
                switch(auto_forge_target) {
                    case HEK::CacheFileEngine::CACHE_FILE_RETAIL:
                        parameters.index = retail_indices(map_name.c_str());
                        break;
                    case HEK::CacheFileEngine::CACHE_FILE_CUSTOM_EDITION:
                        parameters.index = custom_edition_indices(map_name.c_str());
                        break;
                    case HEK::CacheFileEngine::CACHE_FILE_DEMO:
                        parameters.index = demo_indices(map_name.c_str());
                        break;
                    default: break;
                }
            }
            
            if(!parameters.forge_crc.has_value() && auto_forge_target == HEK::CacheFileEngine::CACHE_FILE_CUSTOM_EDITION) {
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
            if(final_file_name_no_extension_string != map_name && build_options.engine != HEK::CacheFileEngine::CACHE_FILE_MCC_CEA) {
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
