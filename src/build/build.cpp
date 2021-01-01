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

int main(int argc, const char **argv) {
    using namespace Invader;
    using namespace Invader::HEK;
    
    using RawDataHandling = BuildWorkload::BuildParameters::BuildParametersDetails::RawDataHandling;

    // Parameters
    struct BuildOptions {
        std::filesystem::path maps = "maps";
        std::vector<std::filesystem::path> tags;
        std::optional<std::filesystem::path> output;
        std::string last_argument;
        std::string index;
        std::optional<HEK::CacheFileEngine> engine;
        bool handled = true;
        bool quiet = false;
        std::optional<RawDataHandling> raw_data_handling;
        std::optional<std::uint32_t> forged_crc;
        bool use_filesystem_path = false;
        std::optional<std::string> rename_scenario;
        std::optional<bool> compress;
        bool optimize_space = false;
        bool hide_pedantic_warnings = false;
        std::optional<int> compression_level;
        bool mcc = false;
        bool increased_file_size_limits = false;
        std::optional<std::string> build_version;
    } build_options;

    std::vector<CommandLineOption> options;
    options.emplace_back("no-external-tags", 'n', 0, "Do not use external tags. This can speed up build time at a cost of a much larger file size.");
    options.emplace_back("always-index-tags", 'a', 0, "Always index tags when possible. This can speed up build time, but stock tags can't be modified.");
    options.emplace_back("quiet", 'q', 0, "Only output error messages.");
    options.emplace_back("info", 'i', 0, "Show credits, source info, and other info.");
    options.emplace_back("game-engine", 'g', 1, "Specify the game engine. This option is required. Valid engines are: custom, demo, native, retail, xbox, mcc-custom", "<id>");
    options.emplace_back("with-index", 'w', 1, "Use an index file for the tags, ensuring the map's tags are ordered in the same way.", "<file>");
    options.emplace_back("maps", 'm', 1, "Use the specified maps directory.", "<dir>");
    options.emplace_back("tags", 't', 1, "Use the specified tags directory. Use multiple times to add more directories, ordered by precedence.", "<dir>");
    options.emplace_back("output", 'o', 1, "Output to a specific file.", "<file>");
    options.emplace_back("forge-crc", 'C', 1, "Forge the CRC32 value of the map after building it.", "<crc>");
    options.emplace_back("fs-path", 'P', 0, "Use a filesystem path for the tag.");
    options.emplace_back("rename-scenario", 'N', 1, "Rename the scenario.", "<name>");
    options.emplace_back("compress", 'c', 0, "Compress the cache file.");
    options.emplace_back("level", 'l', 1, "Set the compression level. Must be between 0 and 19. If compressing an Xbox or MCC map, this will be clamped from 0 to 9. Default: 19", "<level>");
    options.emplace_back("uncompressed", 'u', 0, "Do not compress the cache file. This is default for demo, retail, and custom engines.");
    options.emplace_back("optimize", 'O', 0, "Optimize tag space. This will drastically increase the amount of time required to build the cache file.");
    options.emplace_back("hide-pedantic-warnings", 'H', 0, "Don't show minor warnings.");
    options.emplace_back("extend-file-limits", 'E', 0, "Extend file size limits beyond what is allowed by the target engine to its theoretical maximum size. This may create a map that will not work without a mod.");
    options.emplace_back("build-version", 'b', 1, "Set the build version. This is used on the Xbox version of the game (by default it's 01.10.12.2276 on Xbox and the Invader version on other engines)");

    static constexpr char DESCRIPTION[] = "Build a cache file for a version of Halo: Combat Evolved.";
    static constexpr char USAGE[] = "[options] -g <target> <scenario>";

    auto remaining_arguments = CommandLineOption::parse_arguments<BuildOptions &>(argc, argv, options, USAGE, DESCRIPTION, 1, 1, build_options, [](char opt, const auto &arguments, auto &build_options) {
        switch(opt) {
            case 'n':
                build_options.raw_data_handling = RawDataHandling::RAW_DATA_HANDLING_RETAIN_ALL;
                break;
            case 'a':
                build_options.raw_data_handling = RawDataHandling::RAW_DATA_HANDLING_ALWAYS_INDEX;
                break;
            case 'q':
                build_options.quiet = true;
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
            case 'b':
                build_options.build_version = std::string(arguments[0]);
                break;
            case 'E':
                build_options.increased_file_size_limits = true;
                break;
            case 'l':
                try {
                    build_options.compression_level = std::stoi(arguments[0]);
                    if(build_options.compression_level < 0 || build_options.compression_level > 19) {
                        eprintf_error("Compression level must be between 0 and 19");
                        std::exit(EXIT_FAILURE);
                    }
                }
                catch(std::exception &) {
                    eprintf_error("Invalid compression level %s", arguments[0]);
                    std::exit(EXIT_FAILURE);
                }
                break;
            case 'g':
                build_options.mcc = false;
                if(std::strcmp(arguments[0], "custom") == 0) {
                    build_options.engine = HEK::CacheFileEngine::CACHE_FILE_CUSTOM_EDITION;
                }
                else if(std::strcmp(arguments[0], "mcc-custom") == 0) {
                    build_options.engine = HEK::CacheFileEngine::CACHE_FILE_CUSTOM_EDITION;
                    build_options.mcc = true;
                }
                else if(std::strcmp(arguments[0], "retail") == 0) {
                    build_options.engine = HEK::CacheFileEngine::CACHE_FILE_RETAIL;
                }
                else if(std::strcmp(arguments[0], "demo") == 0) {
                    build_options.engine = HEK::CacheFileEngine::CACHE_FILE_DEMO;
                }
                else if(std::strcmp(arguments[0], "xbox") == 0) {
                    build_options.engine = HEK::CacheFileEngine::CACHE_FILE_XBOX;
                }
                else if(std::strcmp(arguments[0], "native") == 0) {
                    build_options.engine = HEK::CacheFileEngine::CACHE_FILE_NATIVE;
                }
                else {
                    eprintf_error("Unknown engine type: %s", arguments[0]);
                    std::exit(EXIT_FAILURE);
                }
                break;
            case 'C':
                build_options.forged_crc = read_str32("Invalid CRC32", arguments[0]);
                break;
            case 'c':
                build_options.compress = true;
                break;
            case 'u':
                build_options.compress = false;
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
        auto scenario_maybe = Invader::File::file_path_to_tag_path_with_extension(remaining_arguments[0], build_options.tags, ".scenario");
        if(scenario_maybe.has_value()) {
            scenario = scenario_maybe.value();
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

                with_index->emplace_back(substr_v.data(), extension_to_tag_class(extension));
            }
        }

        // Figure out our engine target
        if(!build_options.engine.has_value()) {
            eprintf_error("No engine target specified. Use -h for more information.");
            return EXIT_FAILURE;
        }
        
        BuildWorkload::BuildParameters parameters(*build_options.engine);
        
        parameters.tags_directories = build_options.tags;
        parameters.scenario = scenario;
        parameters.rename_scenario = build_options.rename_scenario;
        parameters.optimize_space = build_options.optimize_space;
        parameters.forge_crc = build_options.forged_crc;
        parameters.index = with_index;
        
        if(build_options.build_version.has_value()) {
            parameters.details.build_version = *build_options.build_version;
        }
        
        // Block this
        if((build_options.mcc || build_options.engine == HEK::CacheFileEngine::CACHE_FILE_XBOX) && !build_options.compress.value_or(true)) {
            eprintf_error("Uncompressed maps are not supported by the target engine.");
            return EXIT_FAILURE;
        }
        
        // MCC stuff
        if(build_options.mcc) {
            parameters.details.build_compress = true;
            parameters.details.build_compress_mcc = true;
            build_options.increased_file_size_limits = true;
        }
        
        if(build_options.increased_file_size_limits) {
            if(build_options.engine != HEK::CacheFileEngine::CACHE_FILE_NATIVE) {
                parameters.details.build_maximum_cache_file_size = UINT32_MAX;
            }
        }
        
        if(build_options.compress.has_value()) {
            parameters.details.build_compress = *build_options.compress;
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
        
        // Handle raw data handling
        if(build_options.raw_data_handling.has_value()) {
            // Don't allow resource maps if we can't use them!
            if(!require_resource_maps && build_options.raw_data_handling != RawDataHandling::RAW_DATA_HANDLING_RETAIN_ALL) {
                eprintf_error("Resource maps are not used for the target engine");
                return EXIT_FAILURE;
            }
            
            // Set this to false
            if(require_resource_maps && build_options.raw_data_handling == RawDataHandling::RAW_DATA_HANDLING_RETAIN_ALL) {
                require_resource_maps = false;
            }
            
            parameters.details.build_raw_data_handling = *build_options.raw_data_handling;
        }
        
        // Load resource maps
        if(require_resource_maps) {
            // Check if we have a custom_* resource maps or not (Custom Edition only)
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
            
            if(parameters.details.build_cache_file_engine == HEK::CacheFileEngine::CACHE_FILE_CUSTOM_EDITION) {
                // Try custom_*
                auto bitmaps = build_options.maps / "custom_bitmaps.map";
                auto sounds = build_options.maps / "custom_sounds.map";
                auto loc = build_options.maps / "custom_loc.map";
                
                // No? Okay, try regular
                if(!std::filesystem::is_regular_file(bitmaps) || !std::filesystem::is_regular_file(sounds) || !std::filesystem::is_regular_file(loc)) {
                    bitmaps = build_options.maps / "bitmaps.map";
                    sounds = build_options.maps / "sounds.map";
                    loc = build_options.maps / "loc.map";
                }
                
                // Well, guess that's that
                if(!std::filesystem::is_regular_file(bitmaps) || !std::filesystem::is_regular_file(sounds) || !std::filesystem::is_regular_file(loc)) {
                    eprintf_error("Maps folder does not contain either set of files:");
                    eprintf_error("- custom_bitmaps.map, custom_sounds.map, AND custom_loc.map");
                    eprintf_error("- bitmaps.map, sounds.map, AND loc.map");
                    error = true;
                    goto show_me_the_spaghetti_code_error;
                }
                
                parameters.bitmap_data = try_open(bitmaps);
                parameters.sound_data = try_open(sounds);
                parameters.loc_data = try_open(loc);
            }
            else {
                // Do it!
                auto bitmaps = build_options.maps / "bitmaps.map";
                auto sounds = build_options.maps / "sounds.map";
                
                // Well, guess that's that
                if(!std::filesystem::is_regular_file(bitmaps) || !std::filesystem::is_regular_file(sounds)) {
                    eprintf_error("Maps folder is missing a bitmaps.map OR sounds.map");
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
        
        // CRC32 spoofing, indices, etc.
        if(!build_options.mcc) {
            if(!parameters.index.has_value()) {
                switch(*build_options.engine) {
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
            if(!parameters.forge_crc.has_value()) {
                if(*build_options.engine == HEK::CacheFileEngine::CACHE_FILE_CUSTOM_EDITION) {
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
        }
        else {
            if(!parameters.index.has_value()) {
                parameters.index = demo_indices(map_name.c_str());
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
            
            // Memes
            if(build_options.mcc && final_file.extension() == ".fmeta") {
                eprintf_error("FATAL ERROR: You have committed crimes against Skyrim and her people.\nWhat say you in your defense?\n");
                eprintf_error(" > I submit. Take me to jail.");
                eprintf_error("   I'd rather die than go to prison!\n");
                return EXIT_FAILURE;
            }
            
            // If we are not building for MCC and the scenario name is mismatched, warn
            if(final_file_name_no_extension_string != map_name) {
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
        
        // Make an fmeta?
        if(build_options.mcc) {
            auto fmeta_path = final_file.replace_extension(".fmeta");
            
            std::vector<std::byte> new_fmeta(0x4408);
            
            // Get the uncompressed size
            std::uint64_t uncompressed_file_size;
            if(parameters.details.build_compress) {
                uncompressed_file_size = *Compression::ceaflate_compression_size(map.data(), map.size());
            }
            else {
                uncompressed_file_size = map.size();
            }
            
            // Set these ints
            *reinterpret_cast<HEK::LittleEndian<std::uint64_t> *>(new_fmeta.data() + 0x0) = 2;
            *reinterpret_cast<HEK::LittleEndian<std::uint64_t> *>(new_fmeta.data() + 0x110) = UINT64_MAX;
            *reinterpret_cast<HEK::LittleEndian<std::uint32_t> *>(new_fmeta.data() + 0x21C) = 1;
            *reinterpret_cast<HEK::LittleEndian<std::uint64_t> *>(new_fmeta.data() + 0x220) = uncompressed_file_size;
            
            // Set this... or bad things happen
            std::snprintf(reinterpret_cast<char *>(new_fmeta.data() + 0x118), 37, "%s.map", map_name.c_str());
            
            if(!File::save_file(fmeta_path, new_fmeta)) {
                eprintf_error("Failed to save %s", fmeta_path.string().c_str());
                return EXIT_FAILURE;
            }
        }

        return EXIT_SUCCESS;
    }
    catch(std::exception &exception) {
        eprintf_error("Failed to compile the map.");
        eprintf_error("%s", exception.what());
        return EXIT_FAILURE;
    }
}
