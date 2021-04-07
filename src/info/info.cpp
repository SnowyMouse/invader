// SPDX-License-Identifier: GPL-3.0-only

#include <optional>
#include <invader/map/map.hpp>
#include <invader/file/file.hpp>
#include <invader/command_line_option.hpp>
#include <invader/crc/hek/crc.hpp>
#include <invader/version.hpp>
#include <invader/tag/parser/parser.hpp>
#include <invader/hek/map.hpp>
#include <invader/compress/compression.hpp>

#include "language/language.hpp"
#include "info_def.hpp"

#define BYTES_TO_MiB(bytes) ((bytes) / 1024.0 / 1024.0)

struct DisplayValue {
    const char * const name;
    void (* const calculate_value)(const Invader::Map &map);
};

#define MAKE_DISPLAY_VALUE(name) {# name, Invader::Info::name }

static std::byte header_cache[sizeof(Invader::HEK::NativeCacheFileHeader)];
static std::size_t file_size = 0;

// Calculating compression ratio:
//
//     1. Take the length of the data after the header, since that's what's compressed
//     2. Divide the length of that data by the length of the data after the header when uncompressed.
//
//        So, if a map is 15 MiB compressed and 20 MiB uncompressed, the compression ratio is 0.75.
//
static double calculate_compression_ratio(const Invader::Map &map) {
    auto uncompressed_length = map.get_data_length() - sizeof(Invader::HEK::CacheFileHeader);
    auto compressed_length = file_size - sizeof(Invader::HEK::CacheFileHeader);
    return static_cast<double>(compressed_length) / uncompressed_length;
}

namespace Invader::Info {
    void compression_ratio(const Invader::Map &map) {
        oprintf("%f\n", calculate_compression_ratio(map));
    }
    
    void overview(const Invader::Map &map) {
        #define PRINT_LINE(function, key, format, ...) function("%-19s" format, key, __VA_ARGS__)
        
        // Basic metadata
        PRINT_LINE(oprintf, "Scenario name:", "%s\n", map.get_scenario_name());
        PRINT_LINE(oprintf, "Build:", "%s\n", map.get_build());
        auto engine = map.get_engine();
        PRINT_LINE(oprintf, "Engine:", "%s\n", engine_name(engine));
        
        if(engine == HEK::CacheFileEngine::CACHE_FILE_NATIVE) {
            PRINT_LINE(oprintf, "Timestamp:", "%s\n", reinterpret_cast<Invader::HEK::NativeCacheFileHeader *>(header_cache)->timestamp.string);
        }
        
        auto map_type = map.get_type();
        
        if(map_type == map.get_header_type()) {
            PRINT_LINE(oprintf_success, "Map type:", "%s (matches header)", type_name(map_type));
        }
        else {
            PRINT_LINE(oprintf_success_warn, "Map type:", "%s (mismatched)", type_name(map_type));
        }
        PRINT_LINE(oprintf, "Tags:", "%zu / %zu (%.02f MiB)\n", map.get_tag_count(), HEK::CacheFileLimits::CACHE_FILE_MAX_TAG_COUNT, BYTES_TO_MiB(map.get_tag_data_length()));
        
        auto crc = map.get_crc32();
        auto crc_matches = map.get_header_crc32() == crc;
        
        // TODODILE: Figure out how to check an Xbox map's integrity
        if(engine != HEK::CacheFileEngine::CACHE_FILE_XBOX) {
            // CRC32
            if(crc_matches) {
                PRINT_LINE(oprintf_success, "CRC32:", "0x%08X (matches header)", crc);
            }
            else {
                PRINT_LINE(oprintf_success_warn, "CRC32:", "0x%08X (mismatched)", crc);
            }
            
            // Dirty?
            if(map.is_clean()) {
                PRINT_LINE(oprintf_success, "Integrity:", "%s", "Clean");
            }
            else {
                PRINT_LINE(oprintf_success_warn, "Integrity:", "%s", "Dirty (map may be corrupted or modified)");
            }
        }
        
        // Protected?
        if(!map.is_protected()) {
            PRINT_LINE(oprintf_success, "Protected:", "%s", "No (probably)");
        }
        else {
            PRINT_LINE(oprintf_success_warn, "Protected:", "%s", "Yes");
        }
        
        std::size_t external_bitmaps = find_external_tags_indices(map, Map::DataMapType::DATA_MAP_BITMAP, true, true).size();
        std::size_t external_sounds = find_external_tags_indices(map, Map::DataMapType::DATA_MAP_SOUND, true, true).size();
        std::size_t external_loc = find_external_tags_indices(map, Map::DataMapType::DATA_MAP_LOC, true, true).size();
        std::size_t total_external = external_bitmaps + external_sounds + external_loc;
        
        if(engine != HEK::CacheFileEngine::CACHE_FILE_NATIVE && engine != HEK::CacheFileEngine::CACHE_FILE_XBOX) {
            if(total_external == 0) {
                PRINT_LINE(oprintf_success, "External tags:", "%s", "None");
            }
            else if(engine == HEK::CacheFileEngine::CACHE_FILE_CUSTOM_EDITION) {
                PRINT_LINE(oprintf_success_lesser_warn, "External tags:", "%zu (%zu bitmap%s, %zu loc, %zu sound%s)", total_external, external_bitmaps, external_bitmaps == 1 ? "" : "s", external_loc, external_sounds, external_sounds == 1 ? "" : "s");
                
                // If we're custom edition, we need to see if they're at least all indexed
                std::size_t indexed_bitmaps = find_external_tags_indices(map, Map::DataMapType::DATA_MAP_BITMAP, true, false).size();
                std::size_t indexed_sounds = find_external_tags_indices(map, Map::DataMapType::DATA_MAP_SOUND, true, false).size();
                std::size_t indexed_loc = find_external_tags_indices(map, Map::DataMapType::DATA_MAP_LOC, true, false).size();
                std::size_t total_indexed = indexed_bitmaps + indexed_sounds + indexed_loc;
                
                // If not, that's bad
                if(total_indexed != total_external) {
                    PRINT_LINE(oprintf_success_warn, "Indexed tags:", "%zu (%zu bitmap%s, %zu loc, %zu sound%s)", total_indexed, indexed_bitmaps, indexed_bitmaps == 1 ? "" : "s", indexed_loc, indexed_sounds, indexed_sounds == 1 ? "" : "s");
                }
                else {
                    PRINT_LINE(oprintf_success, "Indexed tags:", "%zu (%zu bitmap%s, %zu loc, %zu sound%s)", total_indexed, indexed_bitmaps, indexed_bitmaps == 1 ? "" : "s", indexed_loc, indexed_sounds, indexed_sounds == 1 ? "" : "s");
                }
            }
            else {
                PRINT_LINE(oprintf_success_warn, "External tags:", "%zu (%zu bitmap%s, %zu sound%s)", total_external, external_bitmaps, external_bitmaps == 1 ? "" : "s", external_sounds, external_sounds == 1 ? "" : "s");
            }
        }
        
        // Languages?
        if(engine == HEK::CacheFileEngine::CACHE_FILE_CUSTOM_EDITION) {
            bool any;
            auto languages = find_languages_for_map(map, any);
            if(any) {
                PRINT_LINE(oprintf_success, "Valid languages:", "%s", "Any (map will work on all original releases of the game)");
            }
            else if(languages.size() == 0) {
                if(!check_if_valid_indexed_tags_for_stock_custom_edition(map)) {
                    PRINT_LINE(oprintf_success_warn, "Valid languages:", "%s", "None (map contains invalid indices for stock resource maps)");
                }
                else {
                    PRINT_LINE(oprintf_success_warn, "Valid languages:", "%s", "None (map was built against custom resource maps)");
                }
            }
            else {
                std::string list;
                for(auto &i : languages) {
                    if(list.size() == 0) {
                        list = i;
                    }
                    else {
                        list += ", ";
                        list += i;
                    }
                }
                PRINT_LINE(oprintf_success_warn, "Valid languages:", "%s", list.c_str());
            }
        }
        
        // Compressed?
        Map::CompressionType compression_type;
        if((compression_type = map.get_compression_algorithm())) {
            const char *compression_algorithm = "";
            
            switch(compression_type) {
                case Map::CompressionType::COMPRESSION_TYPE_NONE:
                    std::terminate();
                case Map::CompressionType::COMPRESSION_TYPE_DEFLATE:
                    compression_algorithm = "Deflate";
                    break;
                case Map::CompressionType::COMPRESSION_TYPE_ZSTANDARD:
                    compression_algorithm = "Zstandard";
                    break;
                case Map::CompressionType::COMPRESSION_TYPE_MCC_DEFLATE:
                    compression_algorithm = "MCC-Deflate";
                    break;
            }
            
            PRINT_LINE(oprintf, "Compressed:", "Yes (%.02f %%) via %s\n", calculate_compression_ratio(map) * 100.0, compression_algorithm);
        }
        else {
            PRINT_LINE(oprintf, "Compressed:", "%s\n", "No");
        }
        
        // Uncompressed size
        std::optional<HEK::CacheFileLimits> max_uncompressed_size;
        switch(map.get_engine()) {
            case HEK::CacheFileEngine::CACHE_FILE_CUSTOM_EDITION:
            case HEK::CacheFileEngine::CACHE_FILE_RETAIL:
            case HEK::CacheFileEngine::CACHE_FILE_DEMO:
                max_uncompressed_size = HEK::CacheFileLimits::CACHE_FILE_MAXIMUM_FILE_LENGTH_PC;
                break;
            case HEK::CacheFileEngine::CACHE_FILE_XBOX:
                switch(map_type) {
                    case HEK::CacheFileType::SCENARIO_TYPE_SINGLEPLAYER:
                        max_uncompressed_size = HEK::CacheFileLimits::CACHE_FILE_MAXIMUM_FILE_LENGTH_XBOX_SINGLEPLAYER;
                        break;
                    case HEK::CacheFileType::SCENARIO_TYPE_MULTIPLAYER:
                        max_uncompressed_size = HEK::CacheFileLimits::CACHE_FILE_MAXIMUM_FILE_LENGTH_XBOX_MULTIPLAYER;
                        break;
                    case HEK::CacheFileType::SCENARIO_TYPE_USER_INTERFACE:
                        max_uncompressed_size = HEK::CacheFileLimits::CACHE_FILE_MAXIMUM_FILE_LENGTH_XBOX_USER_INTERFACE;
                        break;
                    default:
                        break;
                }
                break;
            case HEK::CacheFileEngine::CACHE_FILE_NATIVE:
                // pretty much useless to show max file size
                // max_uncompressed_size = HEK::CacheFileLimits::CACHE_FILE_MAXIMUM_FILE_LENGTH_NATIVE;
                break;
            default:
                break;
        }
        
        bool size_mismatched = map.get_data_length() != map.get_header_decompressed_file_size();
        
        if(max_uncompressed_size.has_value()) {
            auto num = BYTES_TO_MiB(map.get_data_length());
            auto den = BYTES_TO_MiB(*max_uncompressed_size);
            
            char uncompressed_size[128];
            std::snprintf(uncompressed_size, sizeof(uncompressed_size), "%.02f / %.02f MiB (%.02f %%)%s", num, den, 100.0 * num / den, size_mismatched ? " (mismatched)" : " (matches header)");
            
            if(num > den || size_mismatched) {
                PRINT_LINE(oprintf_success_warn, "Uncompressed size:", "%s", uncompressed_size);
            }
            else {
                PRINT_LINE(oprintf_success, "Uncompressed size:", "%s", uncompressed_size);
            }
        }
        else {
            if(size_mismatched) {
                PRINT_LINE(oprintf_success_warn, "Uncompressed size:", "%.02f MiB (mismatched)", BYTES_TO_MiB(map.get_data_length()));
            }
            else {
                PRINT_LINE(oprintf_success, "Uncompressed size:", "%.02f MiB (matches header)", BYTES_TO_MiB(map.get_data_length()));
            }
        }
        
        #undef PRINT_LINE
    }
}

static DisplayValue all_values[] = {
    MAKE_DISPLAY_VALUE(overview),
    MAKE_DISPLAY_VALUE(build),
    MAKE_DISPLAY_VALUE(compressed),
    MAKE_DISPLAY_VALUE(compression_ratio),
    MAKE_DISPLAY_VALUE(crc32),
    MAKE_DISPLAY_VALUE(crc32_mismatched),
    MAKE_DISPLAY_VALUE(dirty),
    MAKE_DISPLAY_VALUE(engine),
    MAKE_DISPLAY_VALUE(external_bitmap_indices),
    MAKE_DISPLAY_VALUE(external_bitmaps),
    MAKE_DISPLAY_VALUE(external_indices),
    MAKE_DISPLAY_VALUE(external_loc_indices),
    MAKE_DISPLAY_VALUE(external_pointers),
    MAKE_DISPLAY_VALUE(external_sound_indices),
    MAKE_DISPLAY_VALUE(external_sounds),
    MAKE_DISPLAY_VALUE(external_tags),
    MAKE_DISPLAY_VALUE(languages),
    MAKE_DISPLAY_VALUE(map_type),
    MAKE_DISPLAY_VALUE(protection),
    MAKE_DISPLAY_VALUE(scenario),
    MAKE_DISPLAY_VALUE(scenario_path),
    MAKE_DISPLAY_VALUE(tag_count),
    MAKE_DISPLAY_VALUE(stub_count),
    MAKE_DISPLAY_VALUE(tags),
    MAKE_DISPLAY_VALUE(tags_external_bitmap_indices),
    MAKE_DISPLAY_VALUE(tags_external_loc_indices),
    MAKE_DISPLAY_VALUE(tags_external_pointers),
    MAKE_DISPLAY_VALUE(tags_external_sound_indices),
    MAKE_DISPLAY_VALUE(tags_external_indices),
    MAKE_DISPLAY_VALUE(uncompressed_size)
};

int main(int argc, const char **argv) {
    using namespace Invader;

    // Options struct
    struct MapInfoOptions {
        const DisplayValue *type = &all_values[0];
    } map_info_options;
    
    // Form the options list
    std::string options_list;
    bool overview_added = false;
    for(auto &i : all_values) {
        if(!overview_added) {
            options_list += "Set the type of data to show. Can be overview (default)";
            overview_added = true;
        }
        else {
            options_list += ", ";
            options_list += i.name;
        }
    }

    // Command line options
    std::vector<Invader::CommandLineOption> options;
    options.emplace_back("type", 'T', 1, options_list.c_str(), "<type>");
    options.emplace_back("info", 'i', 0, "Show credits, source info, and other info.");

    static constexpr char DESCRIPTION[] = "Display map metadata.";
    static constexpr char USAGE[] = "[option] <map>";

    // Do it!
    auto remaining_arguments = Invader::CommandLineOption::parse_arguments<MapInfoOptions &>(argc, argv, options, USAGE, DESCRIPTION, 1, 1, map_info_options, [](char opt, const auto &args, auto &map_info_options) {
        switch(opt) {
            case 'T': {
                bool found = false;
                
                for(auto &i : all_values) {
                    if(std::strcmp(args[0], i.name) == 0) {
                        map_info_options.type = &i;
                        found = true;
                        break;
                    }
                }
                
                if(!found) {
                    eprintf_error("Unknown type %s", args[0]);
                    std::exit(EXIT_FAILURE);
                }
                break;
            }
            case 'i':
                Invader::show_version_info();
                std::exit(EXIT_SUCCESS);
        }
    });

    // Load it
    std::unique_ptr<Map> map;
    try {
        auto file = File::open_file(remaining_arguments[0]).value();
        file_size = file.size();
        if(file_size >= sizeof(header_cache)) {
            std::memcpy(header_cache, file.data(), sizeof(header_cache));
        }
        
        map = std::make_unique<Map>(Map::map_with_move(std::move(file)));
    }
    catch (std::exception &e) {
        eprintf_error("Failed to parse %s: %s", remaining_arguments[0], e.what());
        return EXIT_FAILURE;
    }
    
    // Do it!
    map_info_options.type->calculate_value(*map);
    
    return EXIT_SUCCESS;
}
