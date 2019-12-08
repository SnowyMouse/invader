// SPDX-License-Identifier: GPL-3.0-only

#include <optional>
#include <invader/map/map.hpp>
#include <invader/file/file.hpp>
#include <invader/command_line_option.hpp>
#include <invader/crc/hek/crc.hpp>
#include <invader/version.hpp>
#include <invader/tag/parser/parser.hpp>

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
        DISPLAY_EXTERNAL_BITMAP_INDICES,
        DISPLAY_EXTERNAL_BITMAPS,
        DISPLAY_EXTERNAL_INDICES,
        DISPLAY_EXTERNAL_LOC,
        DISPLAY_EXTERNAL_LOC_INDICES,
        DISPLAY_EXTERNAL_SOUND_INDICES,
        DISPLAY_EXTERNAL_SOUNDS,
        DISPLAY_EXTERNAL_TAGS,
        DISPLAY_MAP_TYPE,
        DISPLAY_PROTECTED,
        DISPLAY_SCENARIO,
        DISPLAY_SCENARIO_PATH,
        DISPLAY_TAG_COUNT,
        DISPLAY_STUB_COUNT,
        DISPLAY_TAGS,
        DISPLAY_UNIVERSAL
    };

    // Options struct
    struct MapInfoOptions {
        DisplayType type = DISPLAY_OVERVIEW;
    } map_info_options;

    // Command line options
    std::vector<Invader::CommandLineOption> options;
    options.emplace_back("type", 'T', 1, "Set the type of data to show. Can be overview (default), build, compressed, compression-ratio, crc32, crc32-mismatched, dirty, engine, external-bitmap-indices, external-bitmaps, external-indices, external-loc, external-loc-indices, external-sound-indices, external-sounds, external-tags, protected, map-type, scenario, scenario-path, stub-count, tag-count, tags, universal", "<type>");
    options.emplace_back("info", 'i', 0, "Show credits, source info, and other info.");

    static constexpr char DESCRIPTION[] = "Display map metadata.";
    static constexpr char USAGE[] = "[option] <map>";

    // Do it!
    auto remaining_arguments = Invader::CommandLineOption::parse_arguments<MapInfoOptions &>(argc, argv, options, USAGE, DESCRIPTION, 1, 1, map_info_options, [](char opt, const auto &args, auto &map_info_options) {
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
                else if(std::strcmp(args[0], "stub-count") == 0) {
                    map_info_options.type = DISPLAY_STUB_COUNT;
                }
                else if(std::strcmp(args[0], "external-tags") == 0) {
                    map_info_options.type = DISPLAY_EXTERNAL_TAGS;
                }
                else if(std::strcmp(args[0], "external-indices") == 0) {
                    map_info_options.type = DISPLAY_EXTERNAL_INDICES;
                }
                else if(std::strcmp(args[0], "external-bitmaps") == 0) {
                    map_info_options.type = DISPLAY_EXTERNAL_BITMAPS;
                }
                else if(std::strcmp(args[0], "external-loc") == 0) {
                    map_info_options.type = DISPLAY_EXTERNAL_LOC;
                }
                else if(std::strcmp(args[0], "external-sounds") == 0) {
                    map_info_options.type = DISPLAY_EXTERNAL_SOUNDS;
                }
                else if(std::strcmp(args[0], "external-bitmap-indices") == 0) {
                    map_info_options.type = DISPLAY_EXTERNAL_BITMAP_INDICES;
                }
                else if(std::strcmp(args[0], "external-loc-indices") == 0) {
                    map_info_options.type = DISPLAY_EXTERNAL_LOC_INDICES;
                }
                else if(std::strcmp(args[0], "external-sound-indices") == 0) {
                    map_info_options.type = DISPLAY_EXTERNAL_SOUND_INDICES;
                }
                else if(std::strcmp(args[0], "universal") == 0) {
                    map_info_options.type = DISPLAY_UNIVERSAL;
                }
                else {
                    eprintf_error("Unknown type %s", args[0]);
                    std::exit(EXIT_FAILURE);
                }
                break;
            case 'i':
                Invader::show_version_info();
                std::exit(EXIT_SUCCESS);
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
        eprintf_error("Failed to parse %s: %s", remaining_arguments[0], e.what());
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
            if(tag.get_tag_class_int() == TagClassInt::TAG_CLASS_SCENARIO_STRUCTURE_BSP && tag.get_tag_data_index().tag_data != 0) {
                return true;
            }
        }
        return false;
    };

    // Does the map require external data?
    std::size_t bitmaps, sounds, loc, bitmap_indices, sound_indices, loc_indices, total_indices, total_tags;
    bool universal;
    auto uses_external_data = [&tag_count, &map, &bitmaps, &sounds, &loc, &bitmap_indices, &sound_indices, &loc_indices, &total_indices, &total_tags, &universal]() -> bool {
        bitmaps = 0;
        sounds = 0;
        loc = 0;
        bitmap_indices = 0;
        sound_indices = 0;
        loc_indices = 0;
        universal = true;
        for(std::size_t i = 0; i < tag_count; i++) {
            auto &tag = map->get_tag(i);
            if(tag.is_indexed()) {
                switch(tag.get_tag_class_int()) {
                    case TagClassInt::TAG_CLASS_BITMAP:
                        bitmaps++;
                        bitmap_indices++;
                        break;
                    case TagClassInt::TAG_CLASS_SOUND:
                        sounds++;
                        sound_indices++;
                        break;
                    default:
                        loc++;
                        loc_indices++;
                        break;
                }
                continue;
            }

            switch(tag.get_tag_class_int()) {
                case TagClassInt::TAG_CLASS_BITMAP: {
                    auto &bitmap_header = tag.get_base_struct<HEK::Bitmap>();
                    std::size_t bitmap_data_count = bitmap_header.bitmap_data.count;
                    auto *bitmap_data = tag.resolve_reflexive(bitmap_header.bitmap_data);
                    for(std::size_t b = 0; b < bitmap_data_count; b++) {
                        if(bitmap_data[b].flags.read().external) {
                            bitmaps++;
                            break;
                        }
                    }
                    break;
                }

                case TagClassInt::TAG_CLASS_SOUND: {
                    auto &sound_header = tag.get_base_struct<HEK::Sound>();
                    std::size_t pitch_range_count = sound_header.pitch_ranges.count;
                    auto *pitch_ranges = tag.resolve_reflexive(sound_header.pitch_ranges);
                    bool break_early = false;
                    for(std::size_t pr = 0; pr < pitch_range_count && !break_early; pr++) {
                        auto &pitch_range = pitch_ranges[pr];
                        auto *permutations = tag.resolve_reflexive(pitch_range.permutations);
                        std::size_t permutation_count = pitch_range.permutations.count;
                        for(std::size_t pe = 0; pe < permutation_count; pe++) {
                            if(permutations[pe].samples.external & 1) {
                                break_early = true;
                                sounds++;
                                break;
                            }
                        }
                    }
                    break;
                }

                default:
                    break;
            }
        }
        total_indices = loc_indices + bitmap_indices + sound_indices;
        total_tags = loc + bitmaps + sounds;
        universal = total_indices == total_tags;
        return total_tags != 0;
    };

    // Get stub count
    auto stub_count = [&tag_count, &map]() {
        std::size_t count = 0;
        for(std::size_t i = 0; i < tag_count; i++) {
            auto &tag = map->get_tag(i);
            if(tag.get_tag_class_int() != TagClassInt::TAG_CLASS_SCENARIO_STRUCTURE_BSP && tag.get_tag_data_index().tag_data == HEK::CacheFileTagDataBaseMemoryAddress::CACHE_FILE_STUB_MEMORY_ADDRESS) {
                count++;
            }
        }
        return count;
    };

    switch(map_info_options.type) {
        case DISPLAY_OVERVIEW: {
            oprintf("Scenario name:     %s\n", header.name.string);
            oprintf("Build:             %s\n", header.build.string);
            oprintf("Engine:            %s\n", engine_name(header.engine));
            oprintf("Map type:          %s\n", type_name(header.map_type));
            oprintf("Tags:              %zu / %zu (%.02f MiB", tag_count, static_cast<std::size_t>(65535), BYTES_TO_MiB(header.tag_data_size));
            auto stubbed = stub_count();
            if(stubbed) {
                oprintf(", %zu stubbed out", stubbed);
            }
            oprintf(")\n");

            // Get CRC
            auto crc = Invader::calculate_map_crc(map->get_data(), data_length);
            bool external_data_used = uses_external_data();
            bool unsupported_external_data = header.engine == HEK::CacheFileEngine::CACHE_FILE_DARK_CIRCLET || header.engine == HEK::CacheFileEngine::CACHE_FILE_XBOX;
            auto dirty = crc != header.crc32 || memed_by_refinery() || map->is_protected() || (unsupported_external_data && external_data_used);

            if(crc != header.crc32) {
                oprintf_success_warn("CRC32:             0x%08X (mismatched)", crc);
            }
            else {
                oprintf_success("CRC32:             0x%08X (matches)", crc);
            }

            if(dirty) {
                oprintf_success_warn("Integrity:         Dirty");
            }
            else {
                oprintf_success("Integrity:         Clean");
            }

            if(unsupported_external_data) {
                if(external_data_used) {
                    oprintf_success_warn("External data:     Yes (WARNING: This is unsupported by this engine!)");
                }
                else {
                    oprintf("External data:     N/A\n");
                }
            }
            else if(!external_data_used) {
                oprintf("External data:     0\n");
            }
            else if(header.engine == HEK::CacheFileEngine::CACHE_FILE_CUSTOM_EDITION) {
                oprintf("External tags:     %zu (%zu bitmaps.map, %zu loc.map, %zu sounds.map)\n", total_tags, bitmaps, loc, sounds);
                if(total_indices == 0) {
                    oprintf("Indexed tags:      0\n");
                }
                else {
                    oprintf("Indexed tags:      %zu (%zu bitmap%s, %zu loc, %zu sound%s)\n", total_indices, bitmap_indices, bitmap_indices == 1 ? "" : "s", loc_indices, sound_indices, sound_indices == 1 ? "" : "s");
                }
                if(!universal) {
                    oprintf_success_warn("Universal:         No (map will not work on some versions of the game)");
                }
                else {
                    oprintf_success("Universal:         Yes (map will work on all versions of the game)");
                }
            }
            else {
                oprintf("External data:     Yes (%zu bitmaps.map, %zu sounds.map)\n", bitmaps, sounds);
            }

            // Is it protected?
            if(!map->is_protected()) {
                oprintf_success("Protected:         No (probably)");
            }
            else {
                oprintf_success_warn("Protected:         Yes");
            }

            // Compress and compression ratio
            oprintf("Compressed:        %s", compressed ? "Yes" : "No\n");
            if(compressed) {
                oprintf(" (%.02f %%)\n", compression_ratio * 100.0F);
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
            oprintf("%s\n", File::halo_path_to_preferred_path(map->get_tag(map->get_scenario_tag_id()).get_path()).data());
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
                oprintf("%s.%s\n", File::halo_path_to_preferred_path(tag.get_path()).data(), tag_class_to_extension(tag.get_tag_class_int()));
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
        case DISPLAY_STUB_COUNT:
            oprintf("%zu\n", stub_count());
            break;
        case DISPLAY_EXTERNAL_TAGS:
            uses_external_data();
            oprintf("%zu\n", total_tags);
            break;
        case DISPLAY_EXTERNAL_BITMAPS:
            uses_external_data();
            oprintf("%zu\n", bitmaps);
            break;
        case DISPLAY_EXTERNAL_LOC:
            uses_external_data();
            oprintf("%zu\n", loc);
            break;
        case DISPLAY_EXTERNAL_SOUNDS:
            uses_external_data();
            oprintf("%zu\n", sounds);
            break;
        case DISPLAY_EXTERNAL_INDICES:
            uses_external_data();
            oprintf("%zu\n", total_indices);
            break;
        case DISPLAY_EXTERNAL_BITMAP_INDICES:
            uses_external_data();
            oprintf("%zu\n", bitmap_indices);
            break;
        case DISPLAY_EXTERNAL_LOC_INDICES:
            uses_external_data();
            oprintf("%zu\n", loc_indices);
            break;
        case DISPLAY_EXTERNAL_SOUND_INDICES:
            uses_external_data();
            oprintf("%zu\n", sound_indices);
            break;
        case DISPLAY_UNIVERSAL:
            uses_external_data();
            oprintf("%s\n", universal ? "yes" : "no");
            break;
    }
}
