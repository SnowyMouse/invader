// SPDX-License-Identifier: GPL-3.0-only

#include <optional>
#include <invader/map/map.hpp>
#include <invader/file/file.hpp>
#include <invader/command_line_option.hpp>
#include <invader/crc/hek/crc.hpp>

#define BYTES_TO_MiB(bytes) ((bytes) / 1024.0 / 1024.0)

int main(int argc, const char **argv) {
    using namespace Invader;

    // Display data type
    enum DisplayType {
        DISPLAY_OVERVIEW,
        DISPLAY_BUILD,
        DISPLAY_COMPRESSED,
        DISPLAY_COMPRESSION_RATIO,
        DISPLAY_CRC32,
        DISPLAY_CRC32_MISMATCHED,
        DISPLAY_DIRTY,
        DISPLAY_ENGINE,
        DISPLAY_MAP_TYPE,
        DISPLAY_PROTECTED,
        DISPLAY_SCENARIO,
        DISPLAY_SCENARIO_PATH,
        DISPLAY_TAG_COUNT,
        DISPLAY_TAGS
    };

    // Options struct
    struct MapInfoOptions {
        DisplayType type = DISPLAY_OVERVIEW;
    } map_info_options;

    // Command line options
    std::vector<Invader::CommandLineOption> options;
    options.emplace_back("type", 'T', 1, "Set the type of data to show. Can be overview (default), build, compressed, compression-ratio, crc32, crc32-mismatched, dirty, engine, protected, map-type, scenario, scenario-path, tag-count, tags", "<type>");

    static constexpr char DESCRIPTION[] = "Display map metadata.";
    static constexpr char USAGE[] = "[option] <map>";

    // Do it!
    auto remaining_arguments = Invader::CommandLineOption::parse_arguments<MapInfoOptions &>(argc, argv, options, USAGE, DESCRIPTION, 1, 1, map_info_options, [](char opt, const auto &args, MapInfoOptions &map_info_options) {
        switch(opt) {
            case 'T':
                if(std::strcmp(args[0], "overview") == 0) {
                    map_info_options.type = DISPLAY_OVERVIEW;
                }
                else if(std::strcmp(args[0], "crc32") == 0) {
                    map_info_options.type = DISPLAY_CRC32;
                }
                else if(std::strcmp(args[0], "crc32-mismatched") == 0) {
                    map_info_options.type = DISPLAY_CRC32_MISMATCHED;
                }
                else if(std::strcmp(args[0], "dirty") == 0) {
                    map_info_options.type = DISPLAY_DIRTY;
                }
                else if(std::strcmp(args[0], "scenario") == 0) {
                    map_info_options.type = DISPLAY_SCENARIO;
                }
                else if(std::strcmp(args[0], "scenario-path") == 0) {
                    map_info_options.type = DISPLAY_SCENARIO_PATH;
                }
                else if(std::strcmp(args[0], "tag-count") == 0) {
                    map_info_options.type = DISPLAY_TAG_COUNT;
                }
                else if(std::strcmp(args[0], "compressed") == 0) {
                    map_info_options.type = DISPLAY_COMPRESSED;
                }
                else if(std::strcmp(args[0], "engine") == 0) {
                    map_info_options.type = DISPLAY_ENGINE;
                }
                else if(std::strcmp(args[0], "map-type") == 0) {
                    map_info_options.type = DISPLAY_MAP_TYPE;
                }
                else if(std::strcmp(args[0], "protected") == 0) {
                    map_info_options.type = DISPLAY_PROTECTED;
                }
                else if(std::strcmp(args[0], "tags") == 0) {
                    map_info_options.type = DISPLAY_TAGS;
                }
                else if(std::strcmp(args[0], "compression-ratio") == 0) {
                    map_info_options.type = DISPLAY_COMPRESSION_RATIO;
                }
                else if(std::strcmp(args[0], "build") == 0) {
                    map_info_options.type = DISPLAY_BUILD;
                }
                else {
                    eprintf("Unknown type %s\n", args[0]);
                    std::exit(EXIT_FAILURE);
                }
                break;
        }
    });

    std::unique_ptr<Map> map;
    std::size_t file_size = 0;
    try {
        auto file = File::open_file(remaining_arguments[0]).value();
        file_size = file.size();
        map = std::make_unique<Map>(Map::map_with_move(std::move(file)));
    }
    catch (std::exception &e) {
        eprintf("Failed to parse %s: %s\n", remaining_arguments[0], e.what());
        return EXIT_FAILURE;
    }

    // Get the header
    auto &header = map->get_cache_file_header();
    auto data_length = map->get_data_length();
    bool compressed = map->is_compressed();
    auto compression_ratio = static_cast<float>(file_size) / data_length;
    auto tag_count = map->get_tag_count();

    // Was the map opened in Refinery at some point? If so, it's dirty regardless of if the CRC is correct.
    auto memed_by_refinery = [&tag_count, &map]() {
        for(std::size_t i = 0; i < tag_count; i++) {
            auto &tag = map->get_tag(i);
            if(tag.tag_class_int() == HEK::TagClassInt::TAG_CLASS_SCENARIO_STRUCTURE_BSP && tag.get_tag_data_index().tag_data != 0) {
                return true;
            }
        }
        return false;
    };

    switch(map_info_options.type) {
        case DISPLAY_OVERVIEW: {
            oprintf("Scenario name:     %s\n", header.name.string);
            oprintf("Build:             %s\n", header.build.string);
            oprintf("Engine:            %s\n", engine_name(header.engine));
            oprintf("Map type:          %s\n", type_name(header.map_type));
            oprintf("Tags:              %zu / %zu (%.02f MiB)\n", tag_count, static_cast<std::size_t>(65535), BYTES_TO_MiB(header.tag_data_size));

            // Get CRC
            auto crc = Invader::calculate_map_crc(map->get_data(), data_length);
            auto dirty = crc != header.crc32 || memed_by_refinery() || map->is_protected();
            oprintf("CRC32:             0x%08X%s\n", crc, (crc != header.crc32) ? " (mismatched)" : "");
            oprintf("Integrity:         %s\n", dirty ? "Dirty" : "Clean (probably)");

            // Is it protected?
            oprintf("Protected:         %s\n", map->is_protected() ? "Yes" : "No (probably)");

            // Compress and compression ratio
            oprintf("Compressed:        %s", compressed ? "Yes" : "No\n");
            if(compressed) {
                oprintf(" (%.01f %%)\n", compression_ratio * 100.0F);
            }

            // Uncompressed size
            oprintf("Uncompressed size: %.02f MiB / %.02f MiB (%.02f %%)\n", BYTES_TO_MiB(data_length), BYTES_TO_MiB(HEK::CACHE_FILE_MAXIMUM_FILE_LENGTH), static_cast<float>(data_length) / HEK::CACHE_FILE_MAXIMUM_FILE_LENGTH * 100.0F);
            break;
        }
        case DISPLAY_COMPRESSED:
            oprintf("%s\n", compressed ? "yes" : "no");
            break;
        case DISPLAY_CRC32:
            oprintf("%08X\n", Invader::calculate_map_crc(map->get_data(), data_length));
            break;
        case DISPLAY_DIRTY:
            oprintf("%s\n", (Invader::calculate_map_crc(map->get_data(), data_length) != header.crc32 || memed_by_refinery() || map->is_protected()) ? "yes" : "no");
            break;
        case DISPLAY_ENGINE:
            oprintf("%s\n", engine_name(header.engine));
            break;
        case DISPLAY_MAP_TYPE:
            oprintf("%s\n", type_name(header.map_type));
            break;
        case DISPLAY_SCENARIO:
            oprintf("%s\n", header.name.string);
            break;
        case DISPLAY_SCENARIO_PATH:
            oprintf("%s\n", File::halo_path_to_preferred_path(map->get_tag(map->get_scenario_tag_id()).path()).data());
            break;
        case DISPLAY_TAG_COUNT:
            oprintf("%zu\n", tag_count);
            break;
        case DISPLAY_PROTECTED:
            oprintf("%s\n", map->is_protected() ? "yes" : "no");
            break;
        case DISPLAY_TAGS:
            for(std::size_t t = 0; t < tag_count; t++) {
                auto &tag = map->get_tag(t);
                oprintf("%s.%s\n", File::halo_path_to_preferred_path(tag.path()).data(), tag_class_to_extension(tag.tag_class_int()));
            }
            break;
        case DISPLAY_COMPRESSION_RATIO:
            oprintf("%.05f\n", compression_ratio);
            break;
        case DISPLAY_BUILD:
            oprintf("%s\n", header.build.string);
            break;
        case DISPLAY_CRC32_MISMATCHED:
            oprintf("%s\n", (Invader::calculate_map_crc(map->get_data(), data_length) != header.crc32 ? "yes" : "no"));
            break;
    }
}
