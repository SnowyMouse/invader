// SPDX-License-Identifier: GPL-3.0-only

#include <time.h>
#include <stdio.h>

#include <invader/build/build_workload.hpp>
#include <invader/hek/map.hpp>
#include <invader/file/file.hpp>
#include <invader/tag/hek/header.hpp>
#include <invader/version.hpp>
#include <invader/crc/hek/crc.hpp>
#include <invader/compress/compression.hpp>
#include <invader/tag/index/index.hpp>
#include <invader/tag/parser/compile/bitmap.hpp>
#include <invader/tag/parser/compile/sound.hpp>
#include "../crc/crc32.h"

namespace Invader {
    using namespace HEK;

    #define TAG_DATA_HEADER_STRUCT (structs[0])
    #define TAG_ARRAY_STRUCT (structs[1])

    bool BuildWorkload::BuildWorkloadStruct::can_dedupe(const BuildWorkload::BuildWorkloadStruct &other) const noexcept {
        if(this->unsafe_to_dedupe || other.unsafe_to_dedupe || (this->bsp.has_value() && this->bsp != other.bsp)) {
            return false;
        }

        std::size_t this_size = this->data.size();
        std::size_t other_size = other.data.size();

        if(this->dependencies == other.dependencies && this->pointers == other.pointers && this_size >= other_size) {
            return std::memcmp(this->data.data(), other.data.data(), other_size) == 0;
        }

        return false;
    }

    BuildWorkload::BuildWorkload() : ErrorHandler() {}

    std::vector<std::byte> BuildWorkload::compile_map (
        const char *scenario,
        const std::vector<std::string> &tags_directories,
        HEK::CacheFileEngine engine_target,
        std::string maps_directory,
        RawDataHandling raw_data_handling,
        bool verbose,
        const std::optional<std::vector<std::pair<TagClassInt, std::string>>> &with_index,
        const std::optional<std::uint32_t> &forge_crc,
        const std::optional<std::uint32_t> &tag_data_address,
        const std::optional<std::string> &rename_scenario,
        bool optimize_space,
        bool compress,
        bool hide_pedantic_warnings
    ) {
        BuildWorkload workload;

        // Start benchmark
        workload.start = std::chrono::steady_clock::now();

        // Hide these?
        if((workload.hide_pedantic_warnings = hide_pedantic_warnings)) {
            workload.set_reporting_level(REPORTING_LEVEL_HIDE_ALL_PEDANTIC_WARNINGS);
        }

        auto scenario_name_fixed = File::preferred_path_to_halo_path(scenario);
        workload.scenario = scenario_name_fixed.c_str();
        workload.tags_directories = &tags_directories;
        workload.engine_target = engine_target;
        workload.optimize_space = optimize_space;
        workload.verbose = verbose;
        workload.compress = compress;

        // Set defaults
        if(raw_data_handling == RawDataHandling::RAW_DATA_HANDLING_DEFAULT) {
            switch(engine_target) {
                case HEK::CacheFileEngine::CACHE_FILE_NATIVE:
                    raw_data_handling = RawDataHandling::RAW_DATA_HANDLING_RETAIN_ALL;
                    break;

                default:
                    break;
            }
        }

        // Native maps can only use these
        if(raw_data_handling != RawDataHandling::RAW_DATA_HANDLING_RETAIN_ALL && engine_target == HEK::CacheFileEngine::CACHE_FILE_NATIVE) {
            throw InvalidArgumentException();
        }

        // Only Custom Edition can use this
        if(raw_data_handling == RawDataHandling::RAW_DATA_HANDLING_ALWAYS_INDEX && engine_target != HEK::CacheFileEngine::CACHE_FILE_CUSTOM_EDITION) {
            throw InvalidArgumentException();
        }

        workload.raw_data_handling = raw_data_handling;

        // Attempt to open the resource map
        auto open_resource_map = [&maps_directory, &workload](const char *map, const char *map_alt = nullptr) -> std::vector<Resource> {
            if(workload.raw_data_handling == RawDataHandling::RAW_DATA_HANDLING_RETAIN_ALL || workload.raw_data_handling == RawDataHandling::RAW_DATA_HANDLING_REMOVE_ALL) {
                return std::vector<Resource>();
            }
            else {
                if(map_alt) {
                    oprintf("Reading %s or %s...", map, map_alt);
                }
                else {
                    oprintf("Reading %s...", map);
                }
                oflush();

                // Make two paths
                auto map_path = std::filesystem::path(maps_directory) / map;
                auto map_path_str = map_path.string();
                auto map_path_alt = std::filesystem::path(maps_directory) / (map_alt ? map_alt : map);
                auto map_path_alt_str = map_path_alt.string();

                // Try to open either
                auto map_data = Invader::File::open_file(map_path_str.c_str());
                if(!map_data.has_value() && map_alt) {
                    map_data = Invader::File::open_file(map_path_alt_str.c_str());
                }

                if(!map_data.has_value()) {
                    oprintf(" failed\n");
                    if(map_alt) {
                        REPORT_ERROR_PRINTF(workload, ERROR_TYPE_FATAL_ERROR, std::nullopt, "Failed to open %s or %s", map_path_str.c_str(), map_path_alt_str.c_str());
                    }
                    else {
                        REPORT_ERROR_PRINTF(workload, ERROR_TYPE_FATAL_ERROR, std::nullopt, "Failed to open %s", map_path_str.c_str());
                    }
                    throw FailedToOpenFileException();
                }
                oprintf(" done\n");
                return load_resource_map(map_data->data(), map_data->size());
            }
        };

        // Set the scenario name too
        if(rename_scenario.has_value()) {
            workload.set_scenario_name((*rename_scenario).c_str());
        }
        else {
            workload.set_scenario_name(scenario_name_fixed.c_str());
        }

        // If no index was provided, see if we can get one
        std::vector<std::pair<TagClassInt, std::string>> use_index;
        if(with_index.has_value()) {
            use_index = *with_index;
        }
        else {
            switch(engine_target) {
                case HEK::CacheFileEngine::CACHE_FILE_CUSTOM_EDITION:
                    use_index = custom_edition_indices(workload.scenario_name.string);
                    break;
                case HEK::CacheFileEngine::CACHE_FILE_RETAIL:
                    use_index = retail_indices(workload.scenario_name.string);
                    break;
                case HEK::CacheFileEngine::CACHE_FILE_DEMO:
                    use_index = demo_indices(workload.scenario_name.string);
                    break;
                default:
                    break;
            }
            if(use_index.size()) {
                oprintf_success_lesser_warn("Using built-in indices for %s...", workload.scenario_name.string);
            }
        }
        auto index_size = use_index.size();

        // If we have one, continue on
        if(index_size) {
            auto &error_reporter_tags = workload.get_tag_paths();
            error_reporter_tags.reserve(index_size);
            workload.tags.reserve(index_size);
            for(auto &tag : use_index) {
                auto &new_tag = workload.tags.emplace_back();
                new_tag.path = tag.second;
                new_tag.tag_class_int = tag.first;
                new_tag.stubbed = true;
                error_reporter_tags.emplace_back(tag.second, tag.first);
            }
        }

        // Next, if no CRC is passed and we need a CRC, press on
        if(!forge_crc.has_value() && engine_target == CacheFileEngine::CACHE_FILE_CUSTOM_EDITION) {
            if(std::strcmp(workload.scenario_name.string, "beavercreek") == 0) {
                workload.forge_crc = 0x07B3876A;
            }
            else if(std::strcmp(workload.scenario_name.string, "bloodgulch") == 0) {
                workload.forge_crc = 0x7B309554;
            }
            else if(std::strcmp(workload.scenario_name.string, "boardingaction") == 0) {
                workload.forge_crc = 0xF4DEEF94;
            }
            else if(std::strcmp(workload.scenario_name.string, "carousel") == 0) {
                workload.forge_crc = 0x9C301A08;
            }
            else if(std::strcmp(workload.scenario_name.string, "chillout") == 0) {
                workload.forge_crc = 0x93C53C27;
            }
            else if(std::strcmp(workload.scenario_name.string, "damnation") == 0) {
                workload.forge_crc = 0x0FBA059D;
            }
            else if(std::strcmp(workload.scenario_name.string, "dangercanyon") == 0) {
                workload.forge_crc = 0xC410CD74;
            }
            else if(std::strcmp(workload.scenario_name.string, "deathisland") == 0) {
                workload.forge_crc = 0x1DF8C97F;
            }
            else if(std::strcmp(workload.scenario_name.string, "gephyrophobia") == 0) {
                workload.forge_crc = 0xD2872165;
            }
            else if(std::strcmp(workload.scenario_name.string, "hangemhigh") == 0) {
                workload.forge_crc = 0xA7C8B9C6;
            }
            else if(std::strcmp(workload.scenario_name.string, "icefields") == 0) {
                workload.forge_crc = 0x5EC1DEB7;
            }
            else if(std::strcmp(workload.scenario_name.string, "infinity") == 0) {
                workload.forge_crc = 0x0E7F7FE7;
            }
            else if(std::strcmp(workload.scenario_name.string, "longest") == 0) {
                workload.forge_crc = 0xC8F48FF6;
            }
            else if(std::strcmp(workload.scenario_name.string, "prisoner") == 0) {
                workload.forge_crc = 0x43B81A8B;
            }
            else if(std::strcmp(workload.scenario_name.string, "putput") == 0) {
                workload.forge_crc = 0xAF2F0B84;
            }
            else if(std::strcmp(workload.scenario_name.string, "ratrace") == 0) {
                workload.forge_crc = 0xF7F8E14C;
            }
            else if(std::strcmp(workload.scenario_name.string, "sidewinder") == 0) {
                workload.forge_crc = 0xBD95CF55;
            }
            else if(std::strcmp(workload.scenario_name.string, "timberland") == 0) {
                workload.forge_crc = 0x54446470;
            }
            else if(std::strcmp(workload.scenario_name.string, "wizard") == 0) {
                workload.forge_crc = 0xCF3359B1;
            }
            if(workload.forge_crc.has_value()) {
                oprintf_success_lesser_warn("Using built-in CRC32 for %s...", workload.scenario_name.string);
            }
        }
        else {
            workload.forge_crc = forge_crc;
        }

        // Set the tag data address
        switch(engine_target) {
            case CacheFileEngine::CACHE_FILE_NATIVE:
                workload.tag_data_address = CacheFileTagDataBaseMemoryAddress::CACHE_FILE_NATIVE_BASE_MEMORY_ADDRESS;
                workload.tag_data_size = CacheFileLimits::CACHE_FILE_MEMORY_LENGTH_NATIVE;
                break;
            case CacheFileEngine::CACHE_FILE_DEMO:
                workload.tag_data_address = CacheFileTagDataBaseMemoryAddress::CACHE_FILE_DEMO_BASE_MEMORY_ADDRESS;
                workload.tag_data_size = CacheFileLimits::CACHE_FILE_MEMORY_LENGTH;
                workload.bitmaps = open_resource_map("bitmaps.map");
                workload.sounds = open_resource_map("sounds.map");
                break;
            case CacheFileEngine::CACHE_FILE_CUSTOM_EDITION:
                workload.tag_data_address = CacheFileTagDataBaseMemoryAddress::CACHE_FILE_PC_BASE_MEMORY_ADDRESS;
                workload.tag_data_size = CacheFileLimits::CACHE_FILE_MEMORY_LENGTH;
                workload.loc = open_resource_map("custom_loc.map", "loc.map");
                workload.bitmaps = open_resource_map("custom_bitmaps.map", "bitmaps.map");
                workload.sounds = open_resource_map("custom_sounds.map", "sounds.map");
                break;
            default:
                workload.tag_data_address = CacheFileTagDataBaseMemoryAddress::CACHE_FILE_PC_BASE_MEMORY_ADDRESS;
                workload.tag_data_size = CacheFileLimits::CACHE_FILE_MEMORY_LENGTH;
                workload.bitmaps = open_resource_map("bitmaps.map");
                workload.sounds = open_resource_map("sounds.map");
                break;
        }

        if(tag_data_address.has_value()) {
            workload.tag_data_address = *tag_data_address;
        }

        if(engine_target != CacheFileEngine::CACHE_FILE_NATIVE && (0x100000000ull - workload.tag_data_address) < workload.tag_data_size) {
            eprintf_error("Specified tag data address cannot contain the entire tag space");
            throw InvalidArgumentException();
        }

        return workload.build_cache_file();
    }

    #define BYTES_TO_MiB(bytes) (bytes / 1024.0 / 1024.0)

    std::vector<std::byte> BuildWorkload::build_cache_file() {
        // Yay
        File::check_working_directory("./toolbeta.map");

        // First, make our tag data header and array
        this->structs.resize(2);
        TAG_DATA_HEADER_STRUCT.unsafe_to_dedupe = true;
        TAG_ARRAY_STRUCT.unsafe_to_dedupe = true;

        // Add all of the tags
        if(this->verbose) {
            oprintf("Reading tags...\n");
        }
        this->add_tags();

        // Invalidate the raw data if we're removing everything
        if(this->raw_data_handling == RawDataHandling::RAW_DATA_HANDLING_REMOVE_ALL) {
            for(auto &t : this->tags) {
                switch(t.tag_class_int) {
                    case TagClassInt::TAG_CLASS_BITMAP:
                    case TagClassInt::TAG_CLASS_INVADER_BITMAP: {
                        auto &main_struct = this->structs[t.base_struct.value()];
                        auto &data = *reinterpret_cast<Parser::Bitmap::struct_little *>(main_struct.data.data());
                        std::size_t bitmap_data_count = data.bitmap_data.count.read();
                        if(bitmap_data_count) {
                            auto *bitmap_data = reinterpret_cast<Parser::BitmapData::struct_little *>(this->structs[*main_struct.resolve_pointer(&data.bitmap_data.pointer)].data.data());
                            for(std::size_t bd = 0; bd < bitmap_data_count; bd++) {
                                auto &bds = bitmap_data[bd];
                                bds.pixel_data_offset = 0xFFFFFFFF;
                                bds.flags = bds.flags.read() | HEK::BitmapDataFlagsFlag::BITMAP_DATA_FLAGS_FLAG_EXTERNAL;
                            }
                        }
                        break;
                    }

                    case TagClassInt::TAG_CLASS_SOUND:
                    case TagClassInt::TAG_CLASS_INVADER_SOUND: {
                        auto &main_struct = this->structs[*t.base_struct];
                        auto &data = *reinterpret_cast<Parser::Sound::struct_little *>(main_struct.data.data());
                        std::size_t pitch_range_count = data.pitch_ranges.count.read();
                        if(pitch_range_count) {
                            auto &pitch_range_struct = this->structs[*main_struct.resolve_pointer(&data.pitch_ranges.pointer)];
                            auto *pitch_ranges = reinterpret_cast<Parser::SoundPitchRange::struct_little *>(pitch_range_struct.data.data());
                            for(std::size_t pr = 0; pr < pitch_range_count; pr++) {
                                auto &prs = pitch_ranges[pr];
                                std::size_t permutation_count = prs.permutations.count.read();
                                if(permutation_count) {
                                    auto *permutations = reinterpret_cast<Parser::SoundPermutation::struct_little *>(this->structs[*pitch_range_struct.resolve_pointer(&prs.permutations.pointer)].data.data());
                                    for(std::size_t p = 0; p < permutation_count; p++) {
                                        auto &ps = permutations[p];
                                        ps.samples.file_offset = 0xFFFFFFFF;
                                        ps.samples.external = 1;
                                    }
                                }
                            }
                        }
                        break;
                    }

                    default:
                        break;
                }
            }
        }

        // If we have resource maps to check, check them
        if(this->bitmaps.size() != 0) {
            this->externalize_tags();
        }

        // Generate the tag array
        this->generate_tag_array();

        // Set the scenario tag thingy
        auto make_tag_data_header_struct = [](std::size_t scenario_index, auto &structs, auto size) {
            auto &scenario_tag_dependency = TAG_DATA_HEADER_STRUCT.dependencies.emplace_back();
            scenario_tag_dependency.tag_id_only = true;
            scenario_tag_dependency.tag_index = scenario_index;
            auto *header_struct_data_temp = reinterpret_cast<HEK::CacheFileTagDataHeader *>(TAG_DATA_HEADER_STRUCT.data.data());
            scenario_tag_dependency.offset = reinterpret_cast<const std::byte *>(&header_struct_data_temp->scenario_tag) - reinterpret_cast<const std::byte *>(header_struct_data_temp);
            TAG_DATA_HEADER_STRUCT.data.resize(size);
            auto &tag_data_ptr = TAG_DATA_HEADER_STRUCT.pointers.emplace_back();
            tag_data_ptr.offset = reinterpret_cast<const std::byte *>(&header_struct_data_temp->tag_array_address) - reinterpret_cast<const std::byte *>(header_struct_data_temp);
            tag_data_ptr.struct_index = 1;
            tag_data_ptr.limit_to_32_bits = true;
        };

        switch(this->engine_target) {
            case HEK::CacheFileEngine::CACHE_FILE_NATIVE:
                make_tag_data_header_struct(this->scenario_index, this->structs, sizeof(HEK::NativeCacheFileTagDataHeader));
                break;
            default:
                make_tag_data_header_struct(this->scenario_index, this->structs, sizeof(HEK::CacheFileTagDataHeaderPC));
                break;
        }

        auto errors = this->get_errors();
        if(errors) {
            auto warnings = this->get_warnings();
            if(this->verbose) {
                if(warnings) {
                    oprintf_fail("Build failed with %zu error%s and %zu warning%s", errors, errors == 1 ? "" : "s", warnings, warnings == 1 ? "" : "s");
                }
                else {
                    oprintf_fail("Build failed with %zu error%s", errors, errors == 1 ? "" : "s");
                }
            }
            throw InvalidTagDataException();
        }

        // Dedupe structs
        if(this->optimize_space) {
            this->dedupe_structs();
        }

        // Get the tag data
        if(this->verbose) {
            oprintf("Building tag data...");
            oflush();
        }
        std::size_t end_of_bsps = this->generate_tag_data();
        if(this->verbose) {
            oprintf(" done\n");
        }

        // Find the largest BSP
        std::size_t bsp_size = 0;
        std::size_t largest_bsp_size = 0;
        std::size_t largest_bsp_count = 0;

        bool bsp_size_affects_tag_space = this->engine_target != HEK::CacheFileEngine::CACHE_FILE_NATIVE;

        // Calculate total BSP size (pointless on native maps)
        std::vector<std::size_t> bsp_sizes(this->bsp_count);
        if(this->engine_target != HEK::CacheFileEngine::CACHE_FILE_NATIVE) {
            for(std::size_t i = 1; i <= this->bsp_count; i++) {
                // Determine the index.
                auto &this_bsp_size = bsp_sizes[i - 1];

                // Get the size of the BSP struct
                this_bsp_size = this->map_data_structs[i].size();

                // If this is now the largest BSP, mark it as such
                if(this_bsp_size > largest_bsp_size) {
                    largest_bsp_size = this_bsp_size;
                    largest_bsp_count = 1;
                }
                else if(this_bsp_size == largest_bsp_size) {
                    largest_bsp_count++;
                }

                // Add up
                bsp_size += this_bsp_size;
            }
        }

        // Get the bitmap and sound data in there
        if(this->raw_data_handling != RawDataHandling::RAW_DATA_HANDLING_REMOVE_ALL) {
            if(this->verbose) {
                oprintf("Building raw data...");
                oflush();
            }
            this->generate_bitmap_sound_data(end_of_bsps);
            if(this->verbose) {
                oprintf(" done\n");
            }
        }

        auto &workload = *this;
        auto generate_final_data = [&workload, &bsp_size_affects_tag_space, &bsp_size, &largest_bsp_size, &largest_bsp_count, &bsp_sizes](auto &header, auto max_size) {
            std::vector<std::byte> final_data;
            header = {};
            std::strncpy(header.build.string, full_version(), sizeof(header.build.string) - 1);
            header.engine = workload.engine_target;
            header.map_type = *workload.cache_file_type;
            header.name = workload.scenario_name;

            if(workload.verbose) {
                oprintf("Building cache file data...");
                oflush();
            }

            // Add header stuff
            final_data.resize(sizeof(HEK::CacheFileHeader));

            // Go through each BSP and add that stuff
            if(workload.engine_target != HEK::CacheFileEngine::CACHE_FILE_NATIVE) {
                for(std::size_t b = 0; b < workload.bsp_count; b++) {
                    final_data.insert(final_data.end(), workload.map_data_structs[b + 1].begin(), workload.map_data_structs[b + 1].end());
                }
            }
            workload.map_data_structs.resize(1);

            // Now add all the raw data
            final_data.insert(final_data.end(), workload.all_raw_data.begin(), workload.all_raw_data.end());
            auto raw_data_size = workload.all_raw_data.size();
            workload.all_raw_data = std::vector<std::byte>();

            // Let's get the model data there
            std::size_t model_offset = final_data.size() + REQUIRED_PADDING_32_BIT(final_data.size());
            final_data.resize(model_offset, std::byte());
            final_data.insert(final_data.end(), reinterpret_cast<std::byte *>(workload.model_vertices.data()), reinterpret_cast<std::byte *>(workload.model_vertices.data() + workload.model_vertices.size()));

            // Now add model indices
            std::size_t vertex_size = workload.model_vertices.size() * sizeof(*workload.model_vertices.data());
            final_data.insert(final_data.end(), reinterpret_cast<std::byte *>(workload.model_indices.data()), reinterpret_cast<std::byte *>(workload.model_indices.data() + workload.model_indices.size()));
            workload.model_vertices = decltype(workload.model_vertices)();
            workload.model_indices = decltype(workload.model_indices)();

            // We're almost there
            std::size_t tag_data_offset = final_data.size() + REQUIRED_PADDING_32_BIT(final_data.size());
            std::size_t model_data_size = tag_data_offset - model_offset;
            final_data.resize(tag_data_offset, std::byte());

            // Add tag data
            std::size_t tag_data_size = workload.map_data_structs[0].size();
            final_data.insert(final_data.end(), workload.map_data_structs[0].begin(), workload.map_data_structs[0].end());
            if(workload.engine_target == HEK::CacheFileEngine::CACHE_FILE_NATIVE) {
                auto &tag_data_struct = *reinterpret_cast<HEK::NativeCacheFileTagDataHeader *>(final_data.data() + tag_data_offset);
                tag_data_struct.tag_count = static_cast<std::uint32_t>(workload.tags.size());
                tag_data_struct.tags_literal = CacheFileLiteral::CACHE_FILE_TAGS;
                tag_data_struct.model_part_count = static_cast<std::uint32_t>(workload.part_count);
                tag_data_struct.model_data_file_offset = static_cast<std::uint32_t>(model_offset);
                tag_data_struct.vertex_size = static_cast<std::uint32_t>(vertex_size);
                tag_data_struct.model_data_size = static_cast<std::uint32_t>(model_data_size);
                tag_data_struct.raw_data_indices = workload.raw_data_indices_offset;
            }
            else {
                auto &tag_data_struct = *reinterpret_cast<HEK::CacheFileTagDataHeaderPC *>(final_data.data() + tag_data_offset);
                tag_data_struct.tag_count = static_cast<std::uint32_t>(workload.tags.size());
                tag_data_struct.tags_literal = CacheFileLiteral::CACHE_FILE_TAGS;
                tag_data_struct.model_part_count = static_cast<std::uint32_t>(workload.part_count);
                tag_data_struct.model_part_count_again = static_cast<std::uint32_t>(workload.part_count);
                tag_data_struct.model_data_file_offset = static_cast<std::uint32_t>(model_offset);
                tag_data_struct.vertex_size = static_cast<std::uint32_t>(vertex_size);
                tag_data_struct.model_data_size = static_cast<std::uint32_t>(model_data_size);
            }

            // Lastly, do the header
            header.tag_data_size = static_cast<std::uint32_t>(tag_data_size);
            header.tag_data_offset = static_cast<std::uint32_t>(tag_data_offset);
            if(workload.engine_target == HEK::CacheFileEngine::CACHE_FILE_DEMO) {
                header.head_literal = CacheFileLiteral::CACHE_FILE_HEAD_DEMO;
                header.foot_literal = CacheFileLiteral::CACHE_FILE_FOOT_DEMO;
                *reinterpret_cast<HEK::CacheFileDemoHeader *>(final_data.data()) = *reinterpret_cast<HEK::CacheFileHeader *>(&header);
            }
            else {
                header.head_literal = CacheFileLiteral::CACHE_FILE_HEAD;
                header.foot_literal = CacheFileLiteral::CACHE_FILE_FOOT;
                std::memcpy(final_data.data(), &header, sizeof(header));
            }

            if(workload.verbose) {
                oprintf(" done\n");
            }

            // Check to make sure we aren't too big
            std::size_t uncompressed_size = final_data.size();
            if(static_cast<std::uint64_t>(uncompressed_size) > max_size) {
                REPORT_ERROR_PRINTF(workload, ERROR_TYPE_FATAL_ERROR, std::nullopt, "Map file exceeds maximum size when uncompressed (%.04f MiB > %.04f MiB)", BYTES_TO_MiB(uncompressed_size), BYTES_TO_MiB(static_cast<std::size_t>(max_size)));
                throw MaximumFileSizeException();
            }

            // Make sure we don't go beyond the maximum tag space usage
            std::size_t tag_space_usage = workload.indexed_data_amount + tag_data_size;
            if(bsp_size_affects_tag_space) {
                tag_space_usage += largest_bsp_size;
            }
            if(tag_space_usage > workload.tag_data_size) {
                REPORT_ERROR_PRINTF(workload, ERROR_TYPE_FATAL_ERROR, std::nullopt, "Maximum tag space exceeded (%.04f MiB > %.04f MiB)", BYTES_TO_MiB(tag_space_usage), BYTES_TO_MiB(workload.tag_data_size));
                throw MaximumFileSizeException();
            }

            // Hold this here, of course
            auto &tag_file_checksums = reinterpret_cast<HEK::CacheFileTagDataHeader *>(final_data.data() + tag_data_offset)->tag_file_checksums;
            tag_file_checksums = workload.tag_file_checksums;
            
            // If we can calculate the CRC32, do it
            std::uint32_t new_crc = 0;
            bool can_calculate_crc = workload.engine_target != CacheFileEngine::CACHE_FILE_XBOX;
            if(can_calculate_crc) {
                if(workload.verbose) {
                    oprintf("Calculating CRC32...");
                    oflush();
                }
                
                // Calculate the CRC32 and/or forge one if we must
                if(workload.forge_crc.has_value()) {
                    std::uint32_t checksum_delta = 0;
                    new_crc = calculate_map_crc(final_data.data(), final_data.size(), &workload.forge_crc.value(), &checksum_delta);
                    tag_file_checksums = checksum_delta;
                }
                else {
                    new_crc = calculate_map_crc(final_data.data(), final_data.size());
                }
                
                header.crc32 = new_crc;
                if(workload.verbose) {
                    oprintf(" done\n");
                }
            }

            // Copy it again, this time with the new CRC32
            if(workload.engine_target == HEK::CacheFileEngine::CACHE_FILE_DEMO) {
                *reinterpret_cast<HEK::CacheFileDemoHeader *>(final_data.data()) = *reinterpret_cast<HEK::CacheFileHeader *>(&header);
            }
            else {
                std::memcpy(final_data.data(), &header, sizeof(header));
            }

            // Compress if needed
            if(workload.compress) {
                if(workload.verbose) {
                    oprintf("Compressing...");
                    oflush();
                }
                final_data = Compression::compress_map_data(final_data.data(), final_data.size());
                if(workload.verbose) {
                    oprintf(" done\n");
                }
            }
            // Set the file size in the header if needed
            else if(workload.engine_target == HEK::CacheFileEngine::CACHE_FILE_NATIVE) {
                header.decompressed_file_size = final_data.size();
            }

            // Display the scenario name and information
            if(workload.verbose) {
                auto warnings = workload.get_warnings();
                if(warnings) {
                    oprintf_success_warn("Built successfully with %zu warning%s", warnings, warnings == 1 ? "" : "s");
                }
                else {
                    oprintf_success("Built successfully");
                }

                // Chu
                bool easter_egg = false;
                if(ON_COLOR_TERM(stdout)) {
                    if(new_crc == 0x21706156) {
                        oprintf("\x1B[38;5;51m");
                        easter_egg = true;
                    }
                    else if(new_crc == 0x21756843) {
                        oprintf("\x1B[38;5;204m");
                        easter_egg = true;
                    }
                }

                // Show some useful metadata
                oprintf("Scenario:          %s\n", workload.scenario_name.string);
                oprintf("Engine:            %s\n", HEK::engine_name(workload.engine_target));
                oprintf("Map type:          %s\n", HEK::type_name(*workload.cache_file_type));
                oprintf("Tags:              %zu / %zu (%.02f MiB)", workload.tags.size(), static_cast<std::size_t>(UINT16_MAX), BYTES_TO_MiB(workload.map_data_structs[0].size()));
                if(workload.stubbed_tag_count) {
                    oprintf(", %zu stubbed", workload.stubbed_tag_count);
                }
                oprintf("\n");

                // Show the BSP count and/or size
                oprintf("BSPs:              %zu", workload.bsp_count);
                if(workload.engine_target == HEK::CacheFileEngine::CACHE_FILE_NATIVE) {
                    oprintf("\n");
                }
                else {
                    oprintf(" (%.02f MiB)\n", BYTES_TO_MiB(bsp_size));
                }

                // If we have BSPs, go through all of them
                if(workload.bsp_count > 0) {
                    auto &scenario_tag_struct = workload.structs[*workload.tags[workload.scenario_index].base_struct];
                    auto &scenario_tag_data = *reinterpret_cast<Parser::Scenario::struct_little *>(scenario_tag_struct.data.data());
                    auto *scenario_tag_bsps = reinterpret_cast<Parser::ScenarioBSP::struct_little *>(workload.map_data_structs[0].data() + *workload.structs[*scenario_tag_struct.resolve_pointer(&scenario_tag_data.structure_bsps.pointer)].offset);

                    // Go through the BSPs and print their names
                    for(std::size_t b = 0; b < workload.bsp_count; b++) {
                        auto &bsp = scenario_tag_bsps[b];
                        oprintf("                   %s", File::halo_path_to_preferred_path(workload.tags[bsp.structure_bsp.tag_id.read().index].path).c_str());

                        // If we're not on a native map, print the size (native maps don't have any meaningful way to get BSP size)
                        if(workload.engine_target != HEK::CacheFileEngine::CACHE_FILE_NATIVE) {
                            auto &bss = bsp_sizes[b];
                            oprintf(
                                " (%.02f MiB)%s",
                                BYTES_TO_MiB(bss),
                                (largest_bsp_count < workload.bsp_count && bss == largest_bsp_size) ? "*" : ""
                            );
                        }
                        oprintf("\n");
                    }

                    // And, if we're not on native maps and we have different BSP sizes, indicate the largest BSP
                    if(workload.engine_target != HEK::CacheFileEngine::CACHE_FILE_NATIVE && largest_bsp_count < workload.bsp_count) {
                        oprintf("                   * = Largest BSP%s%s\n", largest_bsp_count == 1 ? "" : "s", bsp_size_affects_tag_space ? " (affects final tag space usage)" : "");
                    }
                }

                // Show the total tag space (if applicable)
                if(workload.engine_target != HEK::CacheFileEngine::CACHE_FILE_NATIVE) {
                    oprintf("Tag space:         %.02f / %.02f MiB (%.02f %%)\n", BYTES_TO_MiB(tag_space_usage), BYTES_TO_MiB(workload.tag_data_size), 100.0 * tag_space_usage / workload.tag_data_size);
                }

                // Show some other data that might be useful
                oprintf("Models:            %zu (%.02f MiB)\n", workload.part_count, BYTES_TO_MiB(model_data_size));
                oprintf("Raw data:          %.02f MiB (%.02f MiB bitmaps, %.02f MiB sounds)\n", BYTES_TO_MiB(raw_data_size), BYTES_TO_MiB(workload.raw_bitmap_size), BYTES_TO_MiB(workload.raw_sound_size));

                // Show our CRC32
                if(can_calculate_crc) {
                    oprintf("CRC32 checksum:    0x%08X\n", new_crc);
                }

                // If we compressed it, how small did we get it?
                if(workload.compress) {
                    std::size_t compressed_size = final_data.size();
                    oprintf("Compressed size:   %.02f MiB (%.02f %%)\n", BYTES_TO_MiB(compressed_size), 100.0 * compressed_size / uncompressed_size);
                }

                // Show the original size
                oprintf("Uncompressed size: %.02f ", BYTES_TO_MiB(uncompressed_size));

                // If we have a 32-bit limit, show the limit
                if(workload.engine_target != HEK::CacheFileEngine::CACHE_FILE_NATIVE) {
                    oprintf("/ %.02f MiB (%.02f %%)\n", BYTES_TO_MiB(HEK::CACHE_FILE_MAXIMUM_FILE_LENGTH), 100.0 * uncompressed_size / HEK::CACHE_FILE_MAXIMUM_FILE_LENGTH);
                }
                else {
                    oprintf("MiB\n");
                }

                // And how long did we take
                oprintf("Time:              %.03f ms", std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - workload.start).count() / 1000.0);

                // Chu
                if(ON_COLOR_TERM(stdout)) {
                    if(easter_egg) {
                        oprintf("\x1B[m");
                    }
                }

                oprintf("\n");
            }

            return final_data;
        };

        if(this->engine_target == HEK::CacheFileEngine::CACHE_FILE_NATIVE) {
            HEK::NativeCacheFileHeader header;
            return generate_final_data(header, UINT64_MAX);
        }
        else {
            HEK::CacheFileHeader header;
            return generate_final_data(header, UINT32_MAX);
        }
    }

    void BuildWorkload::compile_tag_data_recursively(const std::byte *tag_data, std::size_t tag_data_size, std::size_t tag_index, std::optional<TagClassInt> tag_class_int) {
        #define COMPILE_TAG_CLASS(class_struct, class_int) case TagClassInt::class_int: { \
            do_compile_tag(std::move(Parser::class_struct::parse_hek_tag_file(tag_data, tag_data_size, true))); \
            break; \
        }

        auto *header = reinterpret_cast<const HEK::TagFileHeader *>(tag_data);

        if(!tag_class_int.has_value()) {
            tag_class_int = header->tag_class_int;
        }

        // Set this in case it's not set yet
        this->tags[tag_index].tag_class_int = *tag_class_int;

        // Check header and CRC32
        HEK::TagFileHeader::validate_header(header, tag_data_size, tag_class_int);
        HEK::BigEndian<std::uint32_t> expected_crc = ~crc32(0, header + 1, tag_data_size - sizeof(*header));
        
        // Make sure the header's CRC32 matches the calculated CRC32
        if(expected_crc != header->crc32) {
            REPORT_ERROR_PRINTF(*this, ERROR_TYPE_WARNING_PEDANTIC, tag_index, "%s.%s's CRC32 is incorrect. Tag may have been improperly modified.", File::halo_path_to_preferred_path(this->tags[tag_index].path).c_str(), tag_class_to_extension(tag_class_int.value()));
        }
        
        // Calculate checksums. This is to prevent collisions when bitmap/sound data is modified but nothing else, since this data isn't directly factored into the map checksum.
        // Also, unlike tool.exe, we're actually recalculating the CRC32 rather than just taking the CRC32 in the header (in case the tag is improperly modified).
        //
        // TODO: Although it accomplishes the same task, this is NOT the algorithm tool.exe uses.
        this->tag_file_checksums = crc32(this->tag_file_checksums, &expected_crc, sizeof(expected_crc)); 

        auto &structs = this->structs;
        auto &tags = this->tags;
        auto &workload = *this;
        auto do_compile_tag = [&structs, &tags, &workload, &tag_index](auto new_tag_struct) {
            auto &new_struct = structs.emplace_back();
            tags[tag_index].base_struct = &new_struct - structs.data();
            new_struct.data.resize(sizeof(typename decltype(new_tag_struct)::struct_little), std::byte());
            new_tag_struct.compile(workload, tag_index, &new_struct - structs.data());
        };

        switch(*tag_class_int) {
            COMPILE_TAG_CLASS(Actor, TAG_CLASS_ACTOR)
            COMPILE_TAG_CLASS(ActorVariant, TAG_CLASS_ACTOR_VARIANT)
            COMPILE_TAG_CLASS(Antenna, TAG_CLASS_ANTENNA)
            COMPILE_TAG_CLASS(ModelAnimations, TAG_CLASS_MODEL_ANIMATIONS)
            COMPILE_TAG_CLASS(Biped, TAG_CLASS_BIPED)
            COMPILE_TAG_CLASS(Bitmap, TAG_CLASS_BITMAP)
            COMPILE_TAG_CLASS(ModelCollisionGeometry, TAG_CLASS_MODEL_COLLISION_GEOMETRY)
            COMPILE_TAG_CLASS(ColorTable, TAG_CLASS_COLOR_TABLE)
            COMPILE_TAG_CLASS(Contrail, TAG_CLASS_CONTRAIL)
            COMPILE_TAG_CLASS(DeviceControl, TAG_CLASS_DEVICE_CONTROL)
            COMPILE_TAG_CLASS(Decal, TAG_CLASS_DECAL)
            COMPILE_TAG_CLASS(UIWidgetDefinition, TAG_CLASS_UI_WIDGET_DEFINITION)
            COMPILE_TAG_CLASS(InputDeviceDefaults, TAG_CLASS_INPUT_DEVICE_DEFAULTS)
            COMPILE_TAG_CLASS(DetailObjectCollection, TAG_CLASS_DETAIL_OBJECT_COLLECTION)
            COMPILE_TAG_CLASS(Effect, TAG_CLASS_EFFECT)
            COMPILE_TAG_CLASS(Equipment, TAG_CLASS_EQUIPMENT)
            COMPILE_TAG_CLASS(Flag, TAG_CLASS_FLAG)
            COMPILE_TAG_CLASS(Fog, TAG_CLASS_FOG)
            COMPILE_TAG_CLASS(Font, TAG_CLASS_FONT)
            COMPILE_TAG_CLASS(MaterialEffects, TAG_CLASS_MATERIAL_EFFECTS)
            COMPILE_TAG_CLASS(Garbage, TAG_CLASS_GARBAGE)
            COMPILE_TAG_CLASS(Glow, TAG_CLASS_GLOW)
            COMPILE_TAG_CLASS(GrenadeHUDInterface, TAG_CLASS_GRENADE_HUD_INTERFACE)
            COMPILE_TAG_CLASS(HUDMessageText, TAG_CLASS_HUD_MESSAGE_TEXT)
            COMPILE_TAG_CLASS(HUDNumber, TAG_CLASS_HUD_NUMBER)
            COMPILE_TAG_CLASS(HUDGlobals, TAG_CLASS_HUD_GLOBALS)
            COMPILE_TAG_CLASS(ItemCollection, TAG_CLASS_ITEM_COLLECTION)
            COMPILE_TAG_CLASS(DamageEffect, TAG_CLASS_DAMAGE_EFFECT)
            COMPILE_TAG_CLASS(LensFlare, TAG_CLASS_LENS_FLARE)
            COMPILE_TAG_CLASS(Lightning, TAG_CLASS_LIGHTNING)
            COMPILE_TAG_CLASS(DeviceLightFixture, TAG_CLASS_DEVICE_LIGHT_FIXTURE)
            COMPILE_TAG_CLASS(Light, TAG_CLASS_LIGHT)
            COMPILE_TAG_CLASS(SoundLooping, TAG_CLASS_SOUND_LOOPING)
            COMPILE_TAG_CLASS(DeviceMachine, TAG_CLASS_DEVICE_MACHINE)
            COMPILE_TAG_CLASS(Globals, TAG_CLASS_GLOBALS)
            COMPILE_TAG_CLASS(Meter, TAG_CLASS_METER)
            COMPILE_TAG_CLASS(LightVolume, TAG_CLASS_LIGHT_VOLUME)
            COMPILE_TAG_CLASS(GBXModel, TAG_CLASS_GBXMODEL)
            COMPILE_TAG_CLASS(Model, TAG_CLASS_MODEL)
            COMPILE_TAG_CLASS(MultiplayerScenarioDescription, TAG_CLASS_MULTIPLAYER_SCENARIO_DESCRIPTION)
            COMPILE_TAG_CLASS(Particle, TAG_CLASS_PARTICLE)
            COMPILE_TAG_CLASS(ParticleSystem, TAG_CLASS_PARTICLE_SYSTEM)
            COMPILE_TAG_CLASS(Physics, TAG_CLASS_PHYSICS)
            COMPILE_TAG_CLASS(Placeholder, TAG_CLASS_PLACEHOLDER)
            COMPILE_TAG_CLASS(PointPhysics, TAG_CLASS_POINT_PHYSICS)
            COMPILE_TAG_CLASS(Projectile, TAG_CLASS_PROJECTILE)
            COMPILE_TAG_CLASS(WeatherParticleSystem, TAG_CLASS_WEATHER_PARTICLE_SYSTEM)
            COMPILE_TAG_CLASS(Scenery, TAG_CLASS_SCENERY)
            COMPILE_TAG_CLASS(ShaderTransparentChicagoExtended, TAG_CLASS_SHADER_TRANSPARENT_CHICAGO_EXTENDED)
            COMPILE_TAG_CLASS(ShaderTransparentChicago, TAG_CLASS_SHADER_TRANSPARENT_CHICAGO)
            COMPILE_TAG_CLASS(Scenario, TAG_CLASS_SCENARIO)
            COMPILE_TAG_CLASS(ShaderEnvironment, TAG_CLASS_SHADER_ENVIRONMENT)
            COMPILE_TAG_CLASS(ShaderTransparentGlass, TAG_CLASS_SHADER_TRANSPARENT_GLASS)
            COMPILE_TAG_CLASS(Sky, TAG_CLASS_SKY)
            COMPILE_TAG_CLASS(ShaderTransparentMeter, TAG_CLASS_SHADER_TRANSPARENT_METER)
            COMPILE_TAG_CLASS(Sound, TAG_CLASS_SOUND)
            COMPILE_TAG_CLASS(SoundEnvironment, TAG_CLASS_SOUND_ENVIRONMENT)
            COMPILE_TAG_CLASS(ShaderModel, TAG_CLASS_SHADER_MODEL)
            COMPILE_TAG_CLASS(ShaderTransparentGeneric, TAG_CLASS_SHADER_TRANSPARENT_GENERIC)
            COMPILE_TAG_CLASS(TagCollection, TAG_CLASS_UI_WIDGET_COLLECTION)
            COMPILE_TAG_CLASS(ShaderTransparentPlasma, TAG_CLASS_SHADER_TRANSPARENT_PLASMA)
            COMPILE_TAG_CLASS(SoundScenery, TAG_CLASS_SOUND_SCENERY)
            COMPILE_TAG_CLASS(StringList, TAG_CLASS_STRING_LIST)
            COMPILE_TAG_CLASS(ShaderTransparentWater, TAG_CLASS_SHADER_TRANSPARENT_WATER)
            COMPILE_TAG_CLASS(TagCollection, TAG_CLASS_TAG_COLLECTION)
            COMPILE_TAG_CLASS(CameraTrack, TAG_CLASS_CAMERA_TRACK)
            COMPILE_TAG_CLASS(Dialogue, TAG_CLASS_DIALOGUE)
            COMPILE_TAG_CLASS(UnitHUDInterface, TAG_CLASS_UNIT_HUD_INTERFACE)
            COMPILE_TAG_CLASS(UnicodeStringList, TAG_CLASS_UNICODE_STRING_LIST)
            COMPILE_TAG_CLASS(VirtualKeyboard, TAG_CLASS_VIRTUAL_KEYBOARD)
            COMPILE_TAG_CLASS(Vehicle, TAG_CLASS_VEHICLE)
            COMPILE_TAG_CLASS(Weapon, TAG_CLASS_WEAPON)
            COMPILE_TAG_CLASS(Wind, TAG_CLASS_WIND)
            COMPILE_TAG_CLASS(WeaponHUDInterface, TAG_CLASS_WEAPON_HUD_INTERFACE)
            
            case TagClassInt::TAG_CLASS_OBJECT:
            case TagClassInt::TAG_CLASS_UNIT:
            case TagClassInt::TAG_CLASS_SHADER:
            case TagClassInt::TAG_CLASS_ITEM:
            case TagClassInt::TAG_CLASS_DEVICE:
                REPORT_ERROR_PRINTF(*this, ERROR_TYPE_FATAL_ERROR, std::nullopt, "%s tags are not real tags and are therefore unimplemented", tag_class_to_extension(*tag_class_int));
                throw UnimplementedTagClassException();

            // For invader sounds and bitmaps, downgrade if necessary
            case TagClassInt::TAG_CLASS_INVADER_BITMAP: {
                auto tag_data_parsed = Parser::InvaderBitmap::parse_hek_tag_file(tag_data, tag_data_size, true);
                if(this->engine_target == HEK::CacheFileEngine::CACHE_FILE_NATIVE) {
                    do_compile_tag(std::move(tag_data_parsed));
                }
                else {
                    this->tags[tag_index].alias = *tag_class_int;
                    this->tags[tag_index].tag_class_int = TagClassInt::TAG_CLASS_BITMAP;
                    do_compile_tag(downgrade_invader_bitmap(tag_data_parsed));
                }
                break;
            }
            case TagClassInt::TAG_CLASS_INVADER_SOUND: {
                auto tag_data_parsed = Parser::InvaderSound::parse_hek_tag_file(tag_data, tag_data_size, true);
                if(this->engine_target == HEK::CacheFileEngine::CACHE_FILE_NATIVE) {
                    do_compile_tag(std::move(tag_data_parsed));
                }
                else {
                    this->tags[tag_index].alias = *tag_class_int;
                    this->tags[tag_index].tag_class_int = TagClassInt::TAG_CLASS_SOUND;
                    do_compile_tag(downgrade_invader_sound(tag_data_parsed));
                }
                break;
            }

            // And, of course, BSP tags
            case TagClassInt::TAG_CLASS_SCENARIO_STRUCTURE_BSP: {
                // First thing's first - parse the tag data
                auto tag_data_parsed = Parser::ScenarioStructureBSP::parse_hek_tag_file(tag_data, tag_data_size, true);
                std::size_t bsp = this->bsp_count++;

                // Next, if we're making a native map, we need to only do this
                if(this->engine_target == HEK::CacheFileEngine::CACHE_FILE_NATIVE) {
                    do_compile_tag(std::move(tag_data_parsed));
                }

                // Otherwise, make the header struct
                else {
                    auto &new_bsp_header_struct = this->structs.emplace_back();
                    new_bsp_header_struct.bsp = bsp;
                    this->tags[tag_index].base_struct = &new_bsp_header_struct - this->structs.data();
                    Parser::ScenarioStructureBSPCompiledHeader::struct_little *bsp_data;
                    new_bsp_header_struct.data.resize(sizeof(*bsp_data), std::byte());
                    bsp_data = reinterpret_cast<decltype(bsp_data)>(new_bsp_header_struct.data.data());
                    auto &new_ptr = new_bsp_header_struct.pointers.emplace_back();
                    new_ptr.limit_to_32_bits = true;
                    new_ptr.offset = reinterpret_cast<std::byte *>(&bsp_data->pointer) - reinterpret_cast<std::byte *>(bsp_data);
                    bsp_data->signature = TagClassInt::TAG_CLASS_SCENARIO_STRUCTURE_BSP;

                    // Make the new BSP struct thingy and make the header point to it
                    auto &new_bsp_struct = this->structs.emplace_back();
                    new_ptr.struct_index = &new_bsp_struct - this->structs.data();
                    new_bsp_struct.data.resize(sizeof(Parser::ScenarioStructureBSP::struct_little), std::byte());
                    tag_data_parsed.compile(*this, tag_index, new_ptr.struct_index, bsp);
                }
                break;
            }

            // We don't have any way of handling these tags
            case TagClassInt::TAG_CLASS_PREFERENCES_NETWORK_GAME:
            case TagClassInt::TAG_CLASS_SPHEROID:
            case TagClassInt::TAG_CLASS_CONTINUOUS_DAMAGE_EFFECT:
            case TagClassInt::TAG_CLASS_INVADER_FONT:
            case TagClassInt::TAG_CLASS_INVADER_UI_WIDGET_DEFINITION:
            case TagClassInt::TAG_CLASS_INVADER_UNIT_HUD_INTERFACE:
            case TagClassInt::TAG_CLASS_INVADER_WEAPON_HUD_INTERFACE:
            case TagClassInt::TAG_CLASS_INVADER_SCENARIO:
            case TagClassInt::TAG_CLASS_SHADER_TRANSPARENT_GLSL:
                REPORT_ERROR_PRINTF(*this, ERROR_TYPE_FATAL_ERROR, std::nullopt, "%s tags are unimplemented at this current time", tag_class_to_extension(*tag_class_int));
                throw UnimplementedTagClassException();
                
            // And this, of course, we don't know
            default:
                throw UnknownTagClassException();
        }
    }

    std::size_t BuildWorkload::compile_tag_recursively(const char *tag_path, TagClassInt tag_class_int) {
        // Remove duplicate slashes
        auto fixed_path = Invader::File::remove_duplicate_slashes(tag_path);
        tag_path = fixed_path.c_str();

        // Search for the tag
        std::size_t return_value = this->tags.size();
        bool found = false;
        for(std::size_t i = 0; i < return_value; i++) {
            auto &tag = this->tags[i];
            if((tag.tag_class_int == tag_class_int || tag.alias == tag_class_int) && tag.path == tag_path) {
                if(tag.base_struct.has_value()) {
                    return i;
                }
                return_value = i;
                found = true;
                tag.stubbed = false;
                break;
            }
        }

        // Find it
        char formatted_path[256];
        std::optional<std::string> new_path;
        if(tag_class_int != TagClassInt::TAG_CLASS_OBJECT) {
            std::snprintf(formatted_path, sizeof(formatted_path), "%s.%s", tag_path, tag_class_to_extension(tag_class_int));
            Invader::File::halo_path_to_preferred_path_chars(formatted_path);
            new_path = Invader::File::tag_path_to_file_path(formatted_path, *this->tags_directories, true);
        }
        else {
            #define TRY_THIS(new_int) if(!new_path.has_value()) { \
                std::snprintf(formatted_path, sizeof(formatted_path), "%s.%s", tag_path, tag_class_to_extension(new_int)); \
                Invader::File::halo_path_to_preferred_path_chars(formatted_path); \
                new_path = Invader::File::tag_path_to_file_path(formatted_path, *this->tags_directories, true); \
                tag_class_int = new_int; \
            }
            TRY_THIS(TagClassInt::TAG_CLASS_BIPED);
            TRY_THIS(TagClassInt::TAG_CLASS_VEHICLE);
            TRY_THIS(TagClassInt::TAG_CLASS_WEAPON);
            TRY_THIS(TagClassInt::TAG_CLASS_EQUIPMENT);
            TRY_THIS(TagClassInt::TAG_CLASS_GARBAGE);
            TRY_THIS(TagClassInt::TAG_CLASS_SCENERY);
            TRY_THIS(TagClassInt::TAG_CLASS_PLACEHOLDER);
            TRY_THIS(TagClassInt::TAG_CLASS_SOUND_SCENERY);
            TRY_THIS(TagClassInt::TAG_CLASS_DEVICE_CONTROL);
            TRY_THIS(TagClassInt::TAG_CLASS_DEVICE_MACHINE);
            TRY_THIS(TagClassInt::TAG_CLASS_DEVICE_LIGHT_FIXTURE);
            #undef TRY_THIS
            if(!new_path.has_value()) {
                tag_class_int = TagClassInt::TAG_CLASS_OBJECT;
                std::snprintf(formatted_path, sizeof(formatted_path), "%s.%s", tag_path, tag_class_to_extension(tag_class_int));
            }
            else {
                // Look for it again
                for(std::size_t i = 0; i < return_value; i++) {
                    auto &tag = this->tags[i];
                    if(tag.tag_class_int == tag_class_int && tag.path == tag_path) {
                        if(tag.base_struct.has_value()) {
                            return i;
                        }
                        return_value = i;
                        found = true;
                        tag.stubbed = false;
                        break;
                    }
                }
            }
        }

        // If it wasn't found in the current array list, add it to the list and let's begin
        if(!found) {
            auto &tag = this->tags.emplace_back();
            tag.path = tag_path;
            tag.tag_class_int = tag_class_int;
            this->get_tag_paths().emplace_back(tag_path, tag_class_int);
        }

        // And we're done! Maybe?
        if(this->disable_recursion && return_value > 0) {
            return return_value;
        }

        if(!new_path.has_value()) {
            eprintf_error("Failed to find %s", formatted_path);
            throw InvalidTagPathException();
        }

        // Open it
        auto tag_file = Invader::File::open_file(new_path->data());
        if(!tag_file.has_value()) {
            eprintf_error("Failed to open %s\n", formatted_path);
            throw FailedToOpenFileException();
        }
        auto &tag_file_data = *tag_file;

        try {
            this->compile_tag_data_recursively(tag_file_data.data(), tag_file_data.size(), return_value, tag_class_int);
        }
        catch(std::exception &e) {
            eprintf("Failed to compile tag %s\n", formatted_path);
            throw;
        }

        return return_value;
    }

    void BuildWorkload::add_tags() {
        this->building_stock_map = std::strcmp(this->scenario_name.string, "a10") == 0 ||
                                   std::strcmp(this->scenario_name.string, "a30") == 0 ||
                                   std::strcmp(this->scenario_name.string, "a50") == 0 ||
                                   std::strcmp(this->scenario_name.string, "b30") == 0 ||
                                   std::strcmp(this->scenario_name.string, "b40") == 0 ||
                                   std::strcmp(this->scenario_name.string, "beavercreek") == 0 ||
                                   std::strcmp(this->scenario_name.string, "bloodgulch") == 0 ||
                                   std::strcmp(this->scenario_name.string, "boardingaction") == 0 ||
                                   std::strcmp(this->scenario_name.string, "c10") == 0 ||
                                   std::strcmp(this->scenario_name.string, "c20") == 0 ||
                                   std::strcmp(this->scenario_name.string, "c40") == 0 ||
                                   std::strcmp(this->scenario_name.string, "carousel") == 0 ||
                                   std::strcmp(this->scenario_name.string, "chillout") == 0 ||
                                   std::strcmp(this->scenario_name.string, "d20") == 0 ||
                                   std::strcmp(this->scenario_name.string, "d40") == 0 ||
                                   std::strcmp(this->scenario_name.string, "damnation") == 0 ||
                                   std::strcmp(this->scenario_name.string, "dangercanyon") == 0 ||
                                   std::strcmp(this->scenario_name.string, "deathisland") == 0 ||
                                   std::strcmp(this->scenario_name.string, "gephyrophobia") == 0 ||
                                   std::strcmp(this->scenario_name.string, "hangemhigh") == 0 ||
                                   std::strcmp(this->scenario_name.string, "icefields") == 0 ||
                                   std::strcmp(this->scenario_name.string, "infinity") == 0 ||
                                   std::strcmp(this->scenario_name.string, "longest") == 0 ||
                                   std::strcmp(this->scenario_name.string, "prisoner") == 0 ||
                                   std::strcmp(this->scenario_name.string, "putput") == 0 ||
                                   std::strcmp(this->scenario_name.string, "ratrace") == 0 ||
                                   std::strcmp(this->scenario_name.string, "sidewinder") == 0 ||
                                   std::strcmp(this->scenario_name.string, "timberland") == 0 ||
                                   std::strcmp(this->scenario_name.string, "ui") == 0 ||
                                   std::strcmp(this->scenario_name.string, "wizard") == 0;

        this->scenario_index = this->compile_tag_recursively(this->scenario, TagClassInt::TAG_CLASS_SCENARIO);
        std::string full_scenario_path = this->tags[this->scenario_index].path;
        const char *first_char = full_scenario_path.c_str();
        const char *last_slash = first_char;
        for(const char *i = first_char; *i; i++) {
            if(*i == '\\') {
                last_slash = i + 1;
            }
        }
        this->tags[this->scenario_index].path = std::string(first_char, last_slash - first_char) + this->scenario_name.string;

        this->compile_tag_recursively("globals\\globals", TagClassInt::TAG_CLASS_GLOBALS);
        this->compile_tag_recursively("ui\\ui_tags_loaded_all_scenario_types", TagClassInt::TAG_CLASS_TAG_COLLECTION);

        // Load the correct tag collection tag
        switch(*this->cache_file_type) {
            case ScenarioType::SCENARIO_TYPE_SINGLEPLAYER:
                this->compile_tag_recursively("ui\\ui_tags_loaded_solo_scenario_type", TagClassInt::TAG_CLASS_TAG_COLLECTION);
                break;
            case ScenarioType::SCENARIO_TYPE_MULTIPLAYER:
                this->compile_tag_recursively("ui\\ui_tags_loaded_multiplayer_scenario_type", TagClassInt::TAG_CLASS_TAG_COLLECTION);
                break;
            case ScenarioType::SCENARIO_TYPE_USER_INTERFACE:
                this->compile_tag_recursively("ui\\ui_tags_loaded_mainmenu_scenario_type", TagClassInt::TAG_CLASS_TAG_COLLECTION);
                break;
            case ScenarioType::SCENARIO_TYPE_ENUM_COUNT:
                std::terminate();
        }

        // These are required for UI elements and other things
        this->compile_tag_recursively("sound\\sfx\\ui\\cursor", TagClassInt::TAG_CLASS_SOUND);
        this->compile_tag_recursively("sound\\sfx\\ui\\back", TagClassInt::TAG_CLASS_SOUND);
        this->compile_tag_recursively("sound\\sfx\\ui\\flag_failure", TagClassInt::TAG_CLASS_SOUND);
        this->compile_tag_recursively("ui\\shell\\main_menu\\mp_map_list", TagClassInt::TAG_CLASS_UNICODE_STRING_LIST);
        this->compile_tag_recursively("ui\\shell\\strings\\loading", TagClassInt::TAG_CLASS_UNICODE_STRING_LIST);
        this->compile_tag_recursively("ui\\shell\\bitmaps\\trouble_brewing", TagClassInt::TAG_CLASS_BITMAP);
        this->compile_tag_recursively("ui\\shell\\bitmaps\\background", TagClassInt::TAG_CLASS_BITMAP);

        // Mark stubs
        std::size_t warned = 0;
        for(auto &tag : this->tags) {
            if(tag.stubbed) {
                // Object tags and damage effects are referenced directly over the netcode
                if(*this->cache_file_type == HEK::CacheFileType::SCENARIO_TYPE_MULTIPLAYER && (IS_OBJECT_TAG(tag.tag_class_int) || tag.tag_class_int == TagClassInt::TAG_CLASS_DAMAGE_EFFECT)) {
                    REPORT_ERROR_PRINTF(*this, ERROR_TYPE_WARNING, &tag - this->tags.data(), "%s.%s was stubbed out due to not being referenced.", File::halo_path_to_preferred_path(tag.path).c_str(), tag_class_to_extension(tag.tag_class_int));
                    warned++;
                }

                tag.path = "MISSINGNO.";
                tag.tag_class_int = TagClassInt::TAG_CLASS_NONE;
                this->stubbed_tag_count++;
            }
        }

        // If we stubbed, explain why
        if(warned) {
            eprintf_warn("An exception error will occur if %s used by the server.", warned == 1 ? "this tag is" : "these tags are");
            eprintf_warn("You can fix this by referencing %s in some already referenced tag in the map.", warned == 1 ? "it" : "them");
        }
    }

    void BuildWorkload::dedupe_structs() {
        bool found_something = true;
        std::size_t total_savings = 0;
        std::size_t struct_count = this->structs.size();

        oprintf("Optimizing tag space...");
        oflush();

        while(found_something) {
            found_something = false;
            for(std::size_t i = 0; i < struct_count; i++) {
                if(this->structs[i].unsafe_to_dedupe) {
                    continue;
                }
                for(std::size_t j = i + 1; j < struct_count; j++) {
                    if(this->structs[j].unsafe_to_dedupe) {
                        continue;
                    }

                    // Check if the structs are the same
                    if(this->structs[i].can_dedupe(this->structs[j])) {
                        // If so, go through every struct pointer. If they equal j, set to i. If they're greater than j, decrement
                        for(std::size_t k = 0; k < struct_count; k++) {
                            for(auto &pointer : this->structs[k].pointers) {
                                auto &struct_index = pointer.struct_index;
                                if(struct_index == j) {
                                    struct_index = i;
                                }
                            }
                        }

                        // Also go through every tag, too
                        for(auto &tag : this->tags) {
                            if(tag.base_struct.has_value()) {
                                auto &base_struct = tag.base_struct.value();
                                if(base_struct == j) {
                                    base_struct = i;
                                }
                            }
                        }

                        total_savings += this->structs[j].data.size();
                        this->structs[j].unsafe_to_dedupe = true;

                        found_something = true;
                    }
                }
            }
        }
        oprintf(" done; reduced tag space usage by %.02f MiB\n", BYTES_TO_MiB(total_savings));
    }

    BuildWorkload BuildWorkload::compile_single_tag(const std::byte *tag_data, std::size_t tag_data_size, const std::vector<std::string> &tags_directories, bool recursion) {
        BuildWorkload workload = {};
        workload.set_reporting_level(ErrorHandler::ReportingLevel::REPORTING_LEVEL_HIDE_EVERYTHING);
        workload.disable_recursion = !recursion;
        workload.cache_file_type = HEK::CacheFileType::SCENARIO_TYPE_MULTIPLAYER;
        workload.tags_directories = &tags_directories;
        workload.tags.emplace_back();
        workload.hide_pedantic_warnings = true;
        workload.compile_tag_data_recursively(tag_data, tag_data_size, 0);
        return workload;
    }

    BuildWorkload BuildWorkload::compile_single_tag(const char *tag, TagClassInt tag_class_int, const std::vector<std::string> &tags_directories, bool recursion) {
        BuildWorkload workload = {};
        workload.set_reporting_level(ErrorHandler::ReportingLevel::REPORTING_LEVEL_HIDE_EVERYTHING);
        workload.disable_recursion = !recursion;
        workload.cache_file_type = HEK::CacheFileType::SCENARIO_TYPE_MULTIPLAYER;
        workload.tags_directories = &tags_directories;
        workload.hide_pedantic_warnings = true;
        workload.compile_tag_recursively(tag, tag_class_int);
        return workload;
    }

    template <typename Tag, HEK::Pointer64 stub_address, bool native> static void do_generate_tag_array(std::size_t tag_count, std::vector<BuildWorkload::BuildWorkloadTag> &tags, std::vector<BuildWorkload::BuildWorkloadStruct> &structs) {
        TAG_ARRAY_STRUCT.data.resize(sizeof(Tag) * tag_count);

        // Reserve tag paths
        std::size_t potential_size = 0;
        for(std::size_t t = 0; t < tag_count; t++) {
            potential_size += tags[t].path.size() + 1;
        }
        TAG_ARRAY_STRUCT.data.reserve(TAG_ARRAY_STRUCT.data.size() + potential_size);

        // Tag path
        for(std::size_t t = 0; t < tag_count; t++) {
            bool found = false;
            auto &tag = tags[t];
            for(std::size_t t2 = 0; t2 < t; t2++) {
                auto &tag2 = tags[t2];
                if(tag2.path == tag.path) {
                    tag.path_offset = tag2.path_offset;
                    found = true;
                    break;
                }
            }
            if(!found) {
                tag.path_offset = TAG_ARRAY_STRUCT.data.size();
                const std::byte *tag_path_str = reinterpret_cast<const std::byte *>(tag.path.c_str());
                TAG_ARRAY_STRUCT.data.insert(TAG_ARRAY_STRUCT.data.end(), tag_path_str, tag_path_str + 1 + tag.path.size());
            }
        }
        TAG_ARRAY_STRUCT.data.resize(TAG_ARRAY_STRUCT.data.size() + REQUIRED_PADDING_32_BIT(TAG_ARRAY_STRUCT.data.size()));

        auto *tag_array = reinterpret_cast<Tag *>(TAG_ARRAY_STRUCT.data.data());

        // Set tag classes, paths, etc.
        for(std::size_t t = 0; t < tag_count; t++) {
            auto &tag_index = tag_array[t];
            auto &tag = tags[t];

            // Tag data
            auto primary_class = tag.tag_class_int;
            if(!native) {
                reinterpret_cast<HEK::CacheFileTagDataTag *>(&tag_index)->indexed = tag.resource_index.has_value();
            }

            if(tag.stubbed) {
                tag_index.tag_data = stub_address;
            }
            else if(tag.resource_index.has_value() && !tag.base_struct.has_value()) {
                tag_index.tag_data = *tag.resource_index;
            }
            else if(primary_class != TagClassInt::TAG_CLASS_SCENARIO_STRUCTURE_BSP || native) {
                auto &tag_data_ptr = TAG_ARRAY_STRUCT.pointers.emplace_back();
                tag_data_ptr.offset = reinterpret_cast<std::byte *>(&tag_index.tag_data) - reinterpret_cast<std::byte *>(tag_array);
                tag_data_ptr.struct_index = *tag.base_struct;
            }

            // Tag ID
            auto &tag_id = TAG_ARRAY_STRUCT.dependencies.emplace_back();
            tag_id.tag_id_only = true;
            tag_id.offset = reinterpret_cast<std::byte *>(&tag_index.tag_id) - reinterpret_cast<std::byte *>(tag_array);
            tag_id.tag_index = t;

            // Not strictly required to set the secondary or tertiary classes, but we do it anyway
            tag_index.primary_class = tag.tag_class_int;
            tag_index.secondary_class = TagClassInt::TAG_CLASS_NONE;
            tag_index.tertiary_class = TagClassInt::TAG_CLASS_NONE;
            switch(tag.tag_class_int) {
                case TagClassInt::TAG_CLASS_BIPED:
                case TagClassInt::TAG_CLASS_VEHICLE:
                    tag_index.secondary_class = TagClassInt::TAG_CLASS_UNIT;
                    tag_index.tertiary_class = TagClassInt::TAG_CLASS_OBJECT;
                    break;
                case TagClassInt::TAG_CLASS_WEAPON:
                case TagClassInt::TAG_CLASS_GARBAGE:
                case TagClassInt::TAG_CLASS_EQUIPMENT:
                    tag_index.secondary_class = TagClassInt::TAG_CLASS_ITEM;
                    tag_index.tertiary_class = TagClassInt::TAG_CLASS_OBJECT;
                    break;
                case TagClassInt::TAG_CLASS_DEVICE_CONTROL:
                case TagClassInt::TAG_CLASS_DEVICE_LIGHT_FIXTURE:
                case TagClassInt::TAG_CLASS_DEVICE_MACHINE:
                    tag_index.secondary_class = TagClassInt::TAG_CLASS_DEVICE;
                    tag_index.tertiary_class = TagClassInt::TAG_CLASS_OBJECT;
                    break;
                case TagClassInt::TAG_CLASS_SCENERY:
                case TagClassInt::TAG_CLASS_SOUND_SCENERY:
                case TagClassInt::TAG_CLASS_PLACEHOLDER:
                case TagClassInt::TAG_CLASS_PROJECTILE:
                    tag_index.secondary_class = TagClassInt::TAG_CLASS_OBJECT;
                    break;
                case TagClassInt::TAG_CLASS_SHADER_ENVIRONMENT:
                case TagClassInt::TAG_CLASS_SHADER_MODEL:
                case TagClassInt::TAG_CLASS_SHADER_TRANSPARENT_GENERIC:
                case TagClassInt::TAG_CLASS_SHADER_TRANSPARENT_CHICAGO:
                case TagClassInt::TAG_CLASS_SHADER_TRANSPARENT_CHICAGO_EXTENDED:
                case TagClassInt::TAG_CLASS_SHADER_TRANSPARENT_WATER:
                case TagClassInt::TAG_CLASS_SHADER_TRANSPARENT_PLASMA:
                case TagClassInt::TAG_CLASS_SHADER_TRANSPARENT_METER:
                case TagClassInt::TAG_CLASS_SHADER_TRANSPARENT_GLASS:
                    tag_index.secondary_class = TagClassInt::TAG_CLASS_SHADER;
                    break;
                default:
                    break;
            }
        }
    }

    void BuildWorkload::generate_tag_array() {
        if(this->engine_target == HEK::CacheFileEngine::CACHE_FILE_NATIVE) {
            do_generate_tag_array<HEK::NativeCacheFileTagDataTag, HEK::CacheFileTagDataBaseMemoryAddress::CACHE_FILE_STUB_MEMORY_ADDRESS_NATIVE, true>(this->tags.size(), this->tags, this->structs);
        }
        else {
            do_generate_tag_array<HEK::CacheFileTagDataTag, HEK::CacheFileTagDataBaseMemoryAddress::CACHE_FILE_STUB_MEMORY_ADDRESS, false>(this->tags.size(), this->tags, this->structs);
        }
    }

    std::size_t BuildWorkload::generate_tag_data() {
        auto &structs = this->structs;
        auto &tags = this->tags;
        std::size_t tag_count = tags.size();

        // Pointer offset to what struct
        using PointerInternal = std::pair<std::size_t, std::size_t>;
        std::vector<PointerInternal> pointers;
        std::vector<PointerInternal> pointers_64_bit;
        auto name_tag_data_pointer = this->tag_data_address;
        auto &tag_array_struct = TAG_ARRAY_STRUCT;

        auto pointer_of_tag_path = [&tags, &name_tag_data_pointer, &tag_array_struct](std::size_t tag_index) -> HEK::Pointer64 {
            return static_cast<HEK::Pointer64>(name_tag_data_pointer + *tag_array_struct.offset + tags[tag_index].path_offset);
        };

        auto &engine_target = this->engine_target;

        auto recursively_generate_data = [&structs, &tags, &pointers, &pointers_64_bit, &pointer_of_tag_path, &engine_target](std::vector<std::byte> &data, std::size_t struct_index, auto &recursively_generate_data) {
            auto &s = structs[struct_index];

            // Return the pointer thingy
            if(s.offset.has_value()) {
                return;
            }

            // Set the offset thingy
            std::size_t offset = data.size();
            s.offset = offset;
            data.insert(data.end(), s.data.begin(), s.data.end());

            // Get the pointers
            for(auto &pointer : s.pointers) {
                recursively_generate_data(data, pointer.struct_index, recursively_generate_data);
                PointerInternal pointer_internal(pointer.offset + offset, pointer.struct_index);
                if(engine_target != HEK::CacheFileEngine::CACHE_FILE_NATIVE || pointer.limit_to_32_bits) {
                    pointers.emplace_back(pointer_internal);
                }
                else {
                    pointers_64_bit.emplace_back(pointer_internal);
                }
            }

            // Get the pointers
            for(auto &dependency : s.dependencies) {
                auto tag_index = dependency.tag_index;
                std::uint32_t full_id = static_cast<std::uint32_t>(tag_index + 0xE741) << 16 | static_cast<std::uint16_t>(tag_index);
                HEK::TagID new_tag_id = { full_id };

                if(dependency.tag_id_only) {
                    *reinterpret_cast<HEK::LittleEndian<HEK::TagID> *>(data.data() + offset + dependency.offset) = new_tag_id;
                }
                else {
                    auto &dependency_struct = *reinterpret_cast<HEK::TagDependency<HEK::LittleEndian> *>(data.data() + offset + dependency.offset);
                    dependency_struct.tag_class_int = tags[tag_index].tag_class_int;
                    dependency_struct.tag_id = new_tag_id;
                    if(engine_target != HEK::CacheFileEngine::CACHE_FILE_NATIVE) {
                        dependency_struct.path_pointer = pointer_of_tag_path(tag_index);
                    }
                }
            }

            // Append stuff
            data.insert(data.end(), REQUIRED_PADDING_32_BIT(data.size()), std::byte());
        };

        // Build the tag data for the main tag data
        auto &tag_data_struct = this->map_data_structs.emplace_back();
        recursively_generate_data(tag_data_struct, 0, recursively_generate_data);
        auto *tag_data_b = tag_data_struct.data();

        // Adjust the pointers
        for(auto &p : pointers) {
            *reinterpret_cast<HEK::LittleEndian<HEK::Pointer> *>(tag_data_b + p.first) = static_cast<HEK::Pointer>(name_tag_data_pointer + *this->structs[p.second].offset);
        }

        for(auto &p : pointers_64_bit) {
            *reinterpret_cast<HEK::LittleEndian<HEK::Pointer64> *>(tag_data_b + p.first) = static_cast<HEK::Pointer64>(name_tag_data_pointer + *this->structs[p.second].offset);
        }

        // Get the tag path pointers working
        auto *tag_array = reinterpret_cast<HEK::CacheFileTagDataTag *>(tag_data_struct.data() + *TAG_ARRAY_STRUCT.offset);
        for(std::size_t t = 0; t < tag_count; t++) {
            tag_array[t].tag_path = pointer_of_tag_path(t);
        }

        // Get this set
        std::size_t bsp_end = sizeof(HEK::CacheFileHeader);

        // Get the scenario tag (if we're not on a native map)
        if(this->engine_target != HEK::CacheFileEngine::CACHE_FILE_NATIVE) {
            auto &scenario_tag = tags[this->scenario_index];
            auto &scenario_tag_data = *reinterpret_cast<const Parser::Scenario::struct_little *>(structs[*scenario_tag.base_struct].data.data());
            std::size_t bsp_count = scenario_tag_data.structure_bsps.count.read();
            std::size_t max_bsp_size = this->tag_data_size;

            if(bsp_count != this->bsp_count) {
                REPORT_ERROR_PRINTF(*this, ERROR_TYPE_FATAL_ERROR, this->scenario_index, "BSP count in scenario tag is wrong (%zu expected, %zu gotten)", bsp_count, this->bsp_count);
                throw InvalidTagDataException();
            }
            else if(bsp_count) {
                auto scenario_bsps_struct_index = *structs[*scenario_tag.base_struct].resolve_pointer(reinterpret_cast<const std::byte *>(&scenario_tag_data.structure_bsps.pointer) - reinterpret_cast<const std::byte *>(&scenario_tag_data));
                auto *scenario_bsps_struct_data = reinterpret_cast<Parser::ScenarioBSP::struct_little *>(map_data_structs[0].data() + *structs[scenario_bsps_struct_index].offset);

                // Go through each BSP tag
                for(std::size_t i = 0; i < tag_count; i++) {
                    auto &t = tags[i];
                    if(t.tag_class_int != TagClassInt::TAG_CLASS_SCENARIO_STRUCTURE_BSP && this->engine_target != HEK::CacheFileEngine::CACHE_FILE_NATIVE) {
                        continue;
                    }

                    // Do it!
                    auto &base_struct = *t.base_struct;

                    // Build the tag data for the BSP data now
                    pointers.clear();
                    pointers_64_bit.clear();
                    auto &bsp_data_struct = this->map_data_structs.emplace_back();
                    recursively_generate_data(bsp_data_struct, base_struct, recursively_generate_data);
                    std::size_t bsp_size = bsp_data_struct.size();

                    if(bsp_size > max_bsp_size) {
                        REPORT_ERROR_PRINTF(*this, ERROR_TYPE_FATAL_ERROR, i, "BSP size exceeds the maximum size for this engine (%zu > %zu)\n", bsp_size, max_bsp_size);
                        throw InvalidTagDataException();
                    }

                    HEK::Pointer64 tag_data_base;
                    if(engine_target != HEK::CacheFileEngine::CACHE_FILE_NATIVE) {
                        tag_data_base = this->tag_data_address + this->tag_data_size - bsp_size;
                    }
                    else {
                        tag_data_base = 0;
                    }
                    tag_data_b = bsp_data_struct.data();

                    // Chu
                    for(auto &p : pointers) {
                        auto &struct_pointed_to = this->structs[p.second];
                        auto base = struct_pointed_to.bsp.has_value() ? tag_data_base : name_tag_data_pointer;
                        *reinterpret_cast<HEK::LittleEndian<HEK::Pointer> *>(tag_data_b + p.first) = static_cast<HEK::Pointer>(base + *struct_pointed_to.offset);
                    }
                    for(auto &p : pointers_64_bit) {
                        auto &struct_pointed_to = this->structs[p.second];
                        auto base = struct_pointed_to.bsp.has_value() ? tag_data_base : name_tag_data_pointer;
                        *reinterpret_cast<HEK::LittleEndian<HEK::Pointer64> *>(tag_data_b + p.first) = static_cast<HEK::Pointer64>(base + *struct_pointed_to.offset);
                    }

                    // Find the BSP in the scenario array thingy
                    bool found = false;
                    for(std::size_t b = 0; b < bsp_count; b++) {
                        if(scenario_bsps_struct_data[b].structure_bsp.tag_id.read().index == i) {
                            scenario_bsps_struct_data[b].bsp_address = tag_data_base;
                            scenario_bsps_struct_data[b].bsp_size = bsp_size;
                            scenario_bsps_struct_data[b].bsp_start = bsp_end;
                            found = true;
                        }
                    }

                    // Add up the size
                    bsp_end += bsp_size;

                    if(!found) {
                        REPORT_ERROR_PRINTF(*this, ERROR_TYPE_ERROR, this->scenario_index, "Scenario structure BSP array is missing %s.%s", File::halo_path_to_preferred_path(t.path).c_str(), HEK::tag_class_to_extension(t.tag_class_int));
                    }
                }
            }
        }
        return bsp_end;
    }

    void BuildWorkload::generate_bitmap_sound_data(std::size_t file_offset) {
        // Prepare for the worst
        std::size_t total_raw_data_size = 0;
        for(auto &r : this->raw_data) {
            total_raw_data_size += r.size();
        }
        auto &all_raw_data = this->all_raw_data;
        all_raw_data.reserve(total_raw_data_size);

        // Offset followed by size
        std::vector<std::pair<std::size_t, std::size_t>> all_assets;

        auto add_or_dedupe_asset = [&all_assets, &all_raw_data](const std::vector<std::byte> &raw_data, std::size_t &counter) -> std::uint32_t {
            std::size_t raw_data_size = raw_data.size();
            for(auto &a : all_assets) {
                if(a.second == raw_data_size && std::memcmp(raw_data.data(), all_raw_data.data() + a.first, raw_data_size) == 0) {
                    return static_cast<std::uint32_t>(&a - all_assets.data());
                }
            }

            auto &new_asset = all_assets.emplace_back();
            new_asset.first = all_raw_data.size();
            new_asset.second = raw_data_size;
            counter += raw_data_size;
            all_raw_data.insert(all_raw_data.end(), raw_data.begin(), raw_data.end());
            return static_cast<std::uint32_t>(all_assets.size() - 1);
        };

        // Go through each tag
        for(auto &t : this->tags) {
            auto asset_count = t.asset_data.size();
            if(t.resource_index.has_value() || asset_count == 0 || t.external_asset_data) {
                continue;
            }
            std::size_t resource_index = 0;
            if(t.tag_class_int == TagClassInt::TAG_CLASS_BITMAP) {
                auto &bitmap_struct = this->structs[*t.base_struct];
                auto &bitmap_header = *reinterpret_cast<Parser::Bitmap::struct_little *>(bitmap_struct.data.data());
                std::size_t bitmap_data_count = bitmap_header.bitmap_data.count.read();
                auto bitmap_data_array = reinterpret_cast<Parser::BitmapData::struct_little *>(this->map_data_structs[0].data() + *this->structs[*bitmap_struct.resolve_pointer(&bitmap_header.bitmap_data.pointer)].offset);
                for(std::size_t b = 0; b < bitmap_data_count; b++) {
                    auto &bitmap_data = bitmap_data_array[b];
                    auto &index = t.asset_data[resource_index++];
                    if(index == static_cast<std::size_t>(~0)) {
                        continue;
                    }

                    // Put it in its place
                    auto resource_index = add_or_dedupe_asset(this->raw_data[index], this->raw_bitmap_size);
                    if(this->engine_target == HEK::CacheFileEngine::CACHE_FILE_NATIVE) {
                        bitmap_data.pixel_data_offset = resource_index;
                    }
                    else {
                        bitmap_data.pixel_data_offset = all_assets[resource_index].first + file_offset;
                    }
                }
            }
            else if(t.tag_class_int == TagClassInt::TAG_CLASS_SOUND) {
                auto &sound_struct = this->structs[*t.base_struct];
                auto &sound_header = *reinterpret_cast<Parser::Sound::struct_little *>(sound_struct.data.data());
                std::size_t pitch_range_count = sound_header.pitch_ranges.count.read();
                auto &pitch_range_struct = this->structs[*sound_struct.resolve_pointer(&sound_header.pitch_ranges.pointer)];
                auto *pitch_range_array = reinterpret_cast<Parser::SoundPitchRange::struct_little *>(pitch_range_struct.data.data());
                for(std::size_t pr = 0; pr < pitch_range_count; pr++) {
                    auto &pitch_range = pitch_range_array[pr];
                    std::size_t permutation_count = pitch_range.permutations.count.read();
                    auto *permutation_array = reinterpret_cast<Parser::SoundPermutation::struct_little *>(this->map_data_structs[0].data() + *this->structs[*pitch_range_struct.resolve_pointer(&pitch_range.permutations.pointer)].offset);
                    for(std::size_t pe = 0; pe < permutation_count; pe++) {
                        auto &permutation = permutation_array[pe];
                        auto &index = t.asset_data[resource_index++];
                        if(index == static_cast<std::size_t>(~0)) {
                            continue;
                        }

                        // Put it in its place
                        auto resource_index = add_or_dedupe_asset(this->raw_data[index], this->raw_sound_size);
                        if(this->engine_target == HEK::CacheFileEngine::CACHE_FILE_NATIVE) {
                            permutation.samples.file_offset = resource_index;
                        }
                        else {
                            permutation.samples.file_offset = all_assets[resource_index].first + file_offset;
                        }
                    }
                }
            }
        }

        // Put the offsets in an array
        if(this->engine_target == HEK::CacheFileEngine::CACHE_FILE_NATIVE) {
            std::vector<LittleEndian<std::uint64_t>> offsets;
            for(auto &i : all_assets) {
                offsets.emplace_back(i.first + file_offset);
            }
            this->raw_data_indices_offset = this->all_raw_data.size() + file_offset;
            this->all_raw_data.insert(this->all_raw_data.end(), reinterpret_cast<const std::byte *>(offsets.data()), reinterpret_cast<const std::byte *>(offsets.data() + offsets.size()));
        }
    }

    void BuildWorkload::set_scenario_name(const char *name) {
        const char *last_slash = name;
        for(const char *c = name; *c; c++) {
            if(*c == '\\') {
                last_slash = c + 1;
            }
        }
        if(*last_slash == 0) {
            throw InvalidScenarioNameException();
        }
        if(static_cast<std::size_t>(std::snprintf(this->scenario_name.string, sizeof(this->scenario_name.string), "%s", last_slash)) >= sizeof(this->scenario_name.string)) {
            throw InvalidScenarioNameException();
        }
    }

    void BuildWorkload::externalize_tags() noexcept {
        bool always_index_tags = this->raw_data_handling == RawDataHandling::RAW_DATA_HANDLING_ALWAYS_INDEX;

        switch(this->engine_target) {
            case HEK::CacheFileEngine::CACHE_FILE_CUSTOM_EDITION:
                for(auto &t : this->tags) {
                    // Find the tag
                    auto find_tag_index = [](const std::string &path, const std::vector<Resource> &resources, bool every_other) -> std::optional<std::size_t> {
                        std::size_t count = resources.size();
                        std::size_t iterate_count, iterate_start;
                        if(every_other) {
                            iterate_count = 2;
                            iterate_start = 1;
                        }
                        else {
                            iterate_count = 1;
                            iterate_start = 0;
                        }
                        for(std::size_t i = iterate_start; i < count; i += iterate_count) {
                            if(resources[i].path == path) {
                                return i;
                            }
                        }
                        return std::nullopt;
                    };

                    switch(t.tag_class_int) {
                        case TagClassInt::TAG_CLASS_BITMAP: {
                            auto index = find_tag_index(t.path, this->bitmaps, true);
                            if(index.has_value()) {
                                if((*index % 2) == 0) {
                                    REPORT_ERROR_PRINTF(*this, ERROR_TYPE_ERROR, std::nullopt, "%s in bitmaps.map appears to be corrupt (tag is on an even index)", File::halo_path_to_preferred_path(t.path).c_str());
                                    break;
                                }

                                bool match = true;
                                if(!always_index_tags) {
                                    const auto &bitmap_tag_struct = this->structs[*t.base_struct];
                                    const auto &bitmap_tag = *reinterpret_cast<const Parser::Bitmap::struct_little *>(bitmap_tag_struct.data.data());
                                    const auto &bitmap_tag_struct_other = this->bitmaps[*index];
                                    const auto *bitmap_tag_struct_other_data = bitmap_tag_struct_other.data.data();
                                    std::size_t bitmap_tag_struct_other_size = bitmap_tag_struct_other.data.size();

                                    if(bitmap_tag_struct_other_size < sizeof(bitmap_tag)) {
                                        REPORT_ERROR_PRINTF(*this, ERROR_TYPE_ERROR, std::nullopt, "%s in bitmaps.map appears to be corrupt (bitmap main struct goes out of bounds)", File::halo_path_to_preferred_path(t.path).c_str());
                                        match = false;
                                        break;
                                    }

                                    const auto &bitmap_tag_struct_other_raw = this->bitmaps[*index - 1];
                                    const auto *bitmap_tag_struct_other_raw_data = bitmap_tag_struct_other_raw.data.data();
                                    std::size_t bitmap_tag_struct_other_raw_data_size = bitmap_tag_struct_other_raw.data.size();

                                    std::size_t bitmap_tag_struct_raw_data_translation = bitmap_tag_struct_other_raw.data_offset;
                                    const auto &bitmap_tag_other = *reinterpret_cast<const Parser::Bitmap::struct_little *>(bitmap_tag_struct_other_data);
                                    std::size_t bitmap_data_count = bitmap_tag.bitmap_data.count;
                                    std::size_t bitmap_data_other_count = bitmap_tag_other.bitmap_data.count;
                                    std::size_t sequence_count = bitmap_tag.bitmap_group_sequence.count;
                                    std::size_t sequence_other_count = bitmap_tag_other.bitmap_group_sequence.count;
                                    
                                    // Make sure the bitmap type matches, first off
                                    if(bitmap_tag_other.type == bitmap_tag.type) {
                                        // Make sure our sequences match
                                        if(match && sequence_count > 0 && sequence_count == sequence_other_count) {
                                            // Make sure it's not out-of-bounds
                                            const auto *all_sequence_other = reinterpret_cast<const Parser::BitmapGroupSequence::struct_little *>(bitmap_tag_struct_other_data + bitmap_tag_other.bitmap_group_sequence.pointer);
                                            if(static_cast<std::size_t>(reinterpret_cast<const std::byte *>(all_sequence_other + sequence_other_count) - bitmap_tag_struct_other_data) > bitmap_tag_struct_other_size) {
                                                REPORT_ERROR_PRINTF(*this, ERROR_TYPE_ERROR, std::nullopt, "%s in bitmaps.map appears to be corrupt (sequence data goes out of bounds)", File::halo_path_to_preferred_path(t.path).c_str());
                                                match = false;
                                                break;
                                            }
                                            
                                            // Also get the bitmap sequences we have
                                            auto &sequence_struct = this->structs[*bitmap_tag_struct.resolve_pointer(&bitmap_tag.bitmap_group_sequence.pointer)];
                                            const auto *all_sequence = reinterpret_cast<const Parser::BitmapGroupSequence::struct_little *>(sequence_struct.data.data());
                                            
                                            if(bitmap_tag_other.type == BitmapType::BITMAP_TYPE_SPRITES) {
                                                for(std::size_t s = 0; s < sequence_count && match; s++) {
                                                    // Get sequence data
                                                    const auto &sequence = all_sequence[s];
                                                    const auto &sequence_other = all_sequence_other[s];
                                                    
                                                    // Make sure the sprites match
                                                    std::size_t sprite_count = sequence.sprites.count;
                                                    std::size_t sprite_count_other = sequence_other.sprites.count;
                                                    
                                                    if(sprite_count > 0 && sprite_count == sprite_count_other) {
                                                        // Make sure it's not out-of-bounds
                                                        const auto *all_sprites_other = reinterpret_cast<const Parser::BitmapGroupSprite::struct_little *>(bitmap_tag_struct_other_data + sequence_other.sprites.pointer);
                                                        if(static_cast<std::size_t>(reinterpret_cast<const std::byte *>(all_sprites_other + sprite_count_other) - bitmap_tag_struct_other_data) > bitmap_tag_struct_other_size) {
                                                            REPORT_ERROR_PRINTF(*this, ERROR_TYPE_ERROR, std::nullopt, "%s in bitmaps.map appears to be corrupt (sequence sprites goes out of bounds)", File::halo_path_to_preferred_path(t.path).c_str());
                                                            match = false;
                                                            break;
                                                        }
                                                        
                                                        // And get our sprites, too
                                                        auto &sprites_struct = this->structs[*sequence_struct.resolve_pointer(&sequence.sprites.pointer)];
                                                        const auto *all_sprites = reinterpret_cast<const Parser::BitmapGroupSprite::struct_little *>(sprites_struct.data.data());
                                                        
                                                        // Check individual sprites to make sure they match
                                                        for(std::size_t sp = 0; sp < sprite_count && match; sp++) {
                                                            const auto &sprite = all_sprites[sp];
                                                            const auto &sprite_other = all_sprites_other[sp];
                                                            match = match && sprite.bitmap_index == sprite_other.bitmap_index;
                                                            match = match && sprite.bottom == sprite_other.bottom;
                                                            match = match && sprite.left == sprite_other.left;
                                                            match = match && sprite.top == sprite_other.top;
                                                            match = match && sprite.right == sprite_other.right;
                                                            match = match && sprite.registration_point == sprite_other.registration_point;
                                                        }
                                                    }
                                                    else {
                                                        match = match && sprite_count == sprite_count_other;
                                                    }
                                                }
                                            }
                                            else {
                                                for(std::size_t s = 0; s < sequence_count && match; s++) {
                                                    // Get sequence data
                                                    const auto &sequence = all_sequence[s];
                                                    const auto &sequence_other = all_sequence_other[s];
                                                    
                                                    // Make sure the bitmap range matches
                                                    match = match && sequence.bitmap_count == sequence_other.bitmap_count;
                                                    match = match && sequence.first_bitmap_index == sequence_other.first_bitmap_index;
                                                }
                                            }
                                        }
                                        else {
                                            match = match && sequence_count == sequence_other_count;
                                        }

                                        // Make sure we have the same number of bitmap data
                                        if(match && bitmap_data_count > 0 && bitmap_data_count == bitmap_data_other_count) {
                                            // Make sure it's not out-of-bounds
                                            const auto *all_bitmap_data_other = reinterpret_cast<const Parser::BitmapData::struct_little *>(bitmap_tag_struct_other_data + bitmap_tag_other.bitmap_data.pointer);
                                            if(static_cast<std::size_t>(reinterpret_cast<const std::byte *>(all_bitmap_data_other + bitmap_data_other_count) - bitmap_tag_struct_other_data) > bitmap_tag_struct_other_size) {
                                                REPORT_ERROR_PRINTF(*this, ERROR_TYPE_ERROR, std::nullopt, "%s in bitmaps.map appears to be corrupt (bitmap data goes out of bounds)", File::halo_path_to_preferred_path(t.path).c_str());
                                                match = false;
                                                break;
                                            }
                                            
                                            // Also get the bitmap data we have
                                            auto &bitmap_data_struct = this->structs[*bitmap_tag_struct.resolve_pointer(&bitmap_tag.bitmap_data.pointer)];
                                            const auto *all_bitmap_data = reinterpret_cast<const Parser::BitmapData::struct_little *>(bitmap_data_struct.data.data());

                                            // Make sure we get match to equal the bitmap data count
                                            for(std::size_t b = 0; b < bitmap_data_count && match; b++) {
                                                // Get the bitmap data
                                                const auto &bitmap_data = all_bitmap_data[b];
                                                const auto &bitmap_data_other = all_bitmap_data_other[b];

                                                // Get the raw data and make sure the sizes match
                                                std::size_t raw_data_index = t.asset_data[b];
                                                auto &asset_raw_data = this->raw_data[raw_data_index];
                                                std::size_t raw_data_size = asset_raw_data.size();
                                                std::size_t raw_data_other_size = bitmap_data_other.pixel_data_size;
                                                if(raw_data_other_size != raw_data_size) {
                                                    match = false;
                                                    break;
                                                }
                                                
                                                // Make sure the formats and dimensions match too
                                                match = match && bitmap_data_other.width == bitmap_data.width;
                                                match = match && bitmap_data_other.height == bitmap_data.height;
                                                match = match && bitmap_data_other.depth == bitmap_data.depth;
                                                match = match && bitmap_data_other.mipmap_count == bitmap_data.mipmap_count;
                                                match = match && bitmap_data_other.format == bitmap_data.format;
                                                if(!match) {
                                                    break;
                                                }
                                                
                                                auto *raw_data_data = asset_raw_data.data();
                                                auto *raw_data_other_data = bitmap_tag_struct_other_raw_data + bitmap_data_other.pixel_data_offset - bitmap_tag_struct_raw_data_translation;

                                                // Make sure it's not bullshit
                                                if(raw_data_other_data < bitmap_tag_struct_other_raw_data || raw_data_other_data > (bitmap_tag_struct_other_raw_data + bitmap_tag_struct_other_raw_data_size)) {
                                                    REPORT_ERROR_PRINTF(*this, ERROR_TYPE_ERROR, std::nullopt, "%s in bitmaps.map appears to be corrupt (pixel data goes out of bounds)", File::halo_path_to_preferred_path(t.path).c_str());
                                                    match = false;
                                                    break;
                                                }

                                                // Check the data
                                                match = std::memcmp(raw_data_other_data, raw_data_data, raw_data_size) == 0;
                                            }
                                        }
                                        else {
                                            match = match && bitmap_data_count == bitmap_data_other_count;
                                        }
                                    }
                                    else {
                                        match = false;
                                    }
                                }

                                if(match) {
                                    t.resource_index = index;
                                    this->indexed_data_amount += this->bitmaps[*index].data.size();
                                    t.base_struct = std::nullopt;
                                    break;
                                }
                                else {
                                    REPORT_ERROR_PRINTF(*this, ERROR_TYPE_WARNING_PEDANTIC, &t - this->tags.data(), "%s.%s does not match the one found in bitmaps.map, so it will NOT be indexed out", File::halo_path_to_preferred_path(t.path).c_str(), HEK::tag_class_to_extension(t.tag_class_int));
                                }
                            }
                            break;
                        }
                        case TagClassInt::TAG_CLASS_SOUND: {
                            auto index = find_tag_index(t.path, this->sounds, true);
                            if(index.has_value()) {
                                if((*index % 2) == 0) {
                                    REPORT_ERROR_PRINTF(*this, ERROR_TYPE_ERROR, std::nullopt, "%s in sounds.map appears to be corrupt (tag is on an even index)", File::halo_path_to_preferred_path(t.path).c_str());
                                    break;
                                }

                                bool match = true;
                                if(!always_index_tags) {
                                    const auto &sound_tag_struct = this->structs[*t.base_struct];
                                    const auto &sound_tag = *reinterpret_cast<const Parser::Sound::struct_little *>(sound_tag_struct.data.data());
                                    const auto &sound_tag_struct_other = this->sounds[*index];
                                    const auto *sound_tag_struct_other_data = sound_tag_struct_other.data.data();
                                    std::size_t sound_tag_struct_other_size = sound_tag_struct_other.data.size();

                                    if(sound_tag_struct_other_size < sizeof(sound_tag)) {
                                        REPORT_ERROR_PRINTF(*this, ERROR_TYPE_ERROR, std::nullopt, "%s in sounds.map appears to be corrupt (sound main struct goes out of bounds)", File::halo_path_to_preferred_path(t.path).c_str());
                                        match = false;
                                        break;
                                    }

                                    const auto &sound_tag_struct_other_raw = this->sounds[*index - 1];
                                    const auto *sound_tag_struct_other_raw_data = sound_tag_struct_other_raw.data.data();
                                    std::size_t sound_tag_struct_raw_data_size = sound_tag_struct_other_raw.data.size();
                                    std::size_t sound_tag_struct_raw_data_translation = sound_tag_struct_other_raw.data_offset;
                                    const auto &sound_tag_other = *reinterpret_cast<const Parser::Sound::struct_little *>(sound_tag_struct_other_data);
                                    std::size_t pitch_range_count = sound_tag.pitch_ranges.count;
                                    std::size_t pitch_range_other_count = sound_tag_other.pitch_ranges.count;
                                    
                                    match = sound_tag.format == sound_tag_other.format; // unlike bitmap tags, this matters because the engine checks this
                                    match = sound_tag.channel_count == sound_tag_other.channel_count;
                                    match = sound_tag.sample_rate == sound_tag_other.sample_rate;

                                    // Make sure we have the same number of stuff
                                    if(match && pitch_range_count > 0 && pitch_range_count == pitch_range_other_count) {
                                        // Make sure it's not out-of-bounds
                                        const auto *sound_data_ref = sound_tag_struct_other_data + sizeof(sound_tag);
                                        const auto &pitch_range_struct = this->structs[*sound_tag_struct.resolve_pointer(&sound_tag.pitch_ranges.pointer)];
                                        const auto *all_pitch_ranges = reinterpret_cast<const Parser::SoundPitchRange::struct_little *>(pitch_range_struct.data.data());
                                        const auto *all_pitch_ranges_other = reinterpret_cast<const Parser::SoundPitchRange::struct_little *>(sound_data_ref);
                                        if(static_cast<std::size_t>(reinterpret_cast<const std::byte *>(all_pitch_ranges_other + pitch_range_other_count) - sound_tag_struct_other_data) > sound_tag_struct_other_size) {
                                            REPORT_ERROR_PRINTF(*this, ERROR_TYPE_ERROR, std::nullopt, "%s in sounds.map appears to be corrupt (pitch ranges go out of bounds)", File::halo_path_to_preferred_path(t.path).c_str());
                                            match = false;
                                            break;
                                        }

                                        // Make sure we get match to equal the bitmap data count
                                        std::size_t raw_data_index_index = 0;
                                        for(std::size_t pr = 0; pr < pitch_range_count && match; pr++) {
                                            // Get the bitmap data
                                            const auto &pitch_range = all_pitch_ranges[pr];
                                            const auto &pitch_range_other = all_pitch_ranges_other[pr];
                                        
                                            // Make sure these match
                                            match = match && pitch_range.bend_bounds == pitch_range_other.bend_bounds;
                                            match = match && pitch_range.actual_permutation_count == pitch_range_other.actual_permutation_count;
                                            match = match && pitch_range.natural_pitch == pitch_range_other.natural_pitch; // we could check the value derived from this, but I don't want to deal with floating point precision memes, so I'll just assume that the sounds.map isn't *total* bullshit
                                            if(!match) {
                                                match = false;
                                                break;
                                            }

                                            std::size_t permutation_count = pitch_range.permutations.count;
                                            std::size_t permutation_other_count = pitch_range_other.permutations.count;

                                            if(permutation_count != permutation_other_count) {
                                                match = false;
                                                break;
                                            }

                                            if(permutation_count == 0) {
                                                continue;
                                            }
                                            if(permutation_count > 0) {
                                                // Bounds check
                                                const auto *all_permutations_other = reinterpret_cast<const Parser::SoundPermutation::struct_little *>(sound_data_ref + pitch_range_other.permutations.pointer);
                                                if(static_cast<std::size_t>(reinterpret_cast<const std::byte *>(all_permutations_other + permutation_count) - sound_tag_struct_other_data) > sound_tag_struct_other_size) {
                                                    REPORT_ERROR_PRINTF(*this, ERROR_TYPE_ERROR, std::nullopt, "%s in sounds.map appears to be corrupt (permutations go out of bounds)", File::halo_path_to_preferred_path(t.path).c_str());
                                                    match = false;
                                                    break;
                                                }
                                            
                                                const auto &permutation_struct = this->structs[*pitch_range_struct.resolve_pointer(&pitch_range.permutations.pointer)];
                                                const auto *permutations = reinterpret_cast<const Parser::SoundPermutation::struct_little *>(permutation_struct.data.data());

                                                for(std::size_t p = 0; p < permutation_count && match; p++) {
                                                    const auto &permutation = permutations[p];
                                                    const auto &permutation_other = all_permutations_other[p];
                                                    
                                                    // Make sure these match
                                                    match = match && permutation.format == permutation_other.format;
                                                    match = match && permutation.gain == permutation_other.gain;
                                                    match = match && permutation.next_permutation_index == permutation_other.next_permutation_index;
                                                    match = match && permutation.skip_fraction == permutation_other.skip_fraction;
                                                    
                                                    if(!match) {
                                                        break;
                                                    }

                                                    std::size_t raw_data_index = t.asset_data[raw_data_index_index++];
                                                    const auto &raw_data = this->raw_data[raw_data_index];

                                                    const auto *raw_data_data = raw_data.data();
                                                    std::size_t raw_data_size = raw_data.size();

                                                    const auto *raw_data_other_data = sound_tag_struct_other_raw_data + permutation_other.samples.file_offset - sound_tag_struct_raw_data_translation;
                                                    std::size_t raw_data_other_size = permutation_other.samples.size;

                                                    // Make sure it's not bullshit
                                                    if(raw_data_other_data < sound_tag_struct_other_raw_data || raw_data_other_data > (sound_tag_struct_other_raw_data + sound_tag_struct_raw_data_size)) {
                                                        REPORT_ERROR_PRINTF(*this, ERROR_TYPE_ERROR, std::nullopt, "%s in sounds.map appears to be corrupt (sample data goes out of bounds)", File::halo_path_to_preferred_path(t.path).c_str());
                                                        match = false;
                                                        break;
                                                    }

                                                    // Make sure the sizes match
                                                    if(raw_data_other_size != raw_data_size) {
                                                        match = false;
                                                        break;
                                                    }

                                                    // Check the data
                                                    match = match && std::memcmp(raw_data_other_data, raw_data_data, raw_data_size) == 0;
                                                }
                                            }
                                        }
                                    }
                                    else {
                                        match = match && pitch_range_count == pitch_range_other_count;
                                    }
                                }

                                if(match) {
                                    // Index it. Unlike other indexed tags, the header remains (probably for the promotion sound dependencies?)
                                    t.resource_index = index;
                                    this->indexed_data_amount += this->sounds[*index].data.size() - sizeof(Sound<LittleEndian>);
                                    
                                    // Strip these values since they'll be replaced on load anyway
                                    auto &sound_tag_struct = this->structs[*t.base_struct];
                                    auto &sound_tag = *reinterpret_cast<Parser::Sound::struct_little *>(sound_tag_struct.data.data());
                                    sound_tag.channel_count = {};
                                    sound_tag.sample_rate = {};
                                    sound_tag.format = {};
                                    sound_tag.longest_permutation_length = {};
                                    
                                    // Clear the pointers so that way we only include this stuff
                                    sound_tag_struct.pointers.clear();
                                    
                                    break;
                                }
                                else {
                                    REPORT_ERROR_PRINTF(*this, ERROR_TYPE_WARNING_PEDANTIC, &t - this->tags.data(), "%s.%s does not match the one found in sounds.map, so it will NOT be indexed out", File::halo_path_to_preferred_path(t.path).c_str(), HEK::tag_class_to_extension(t.tag_class_int));
                                }
                            }
                            break;
                        }
                        case TagClassInt::TAG_CLASS_FONT:
                        case TagClassInt::TAG_CLASS_UNICODE_STRING_LIST:
                        case TagClassInt::TAG_CLASS_HUD_MESSAGE_TEXT: {
                            auto index = find_tag_index(t.path, this->loc, false);
                            if(index.has_value()) {
                                bool match = true;

                                const auto &loc_tag_struct_other = this->loc[*index];
                                const auto *loc_tag_struct_other_data = loc_tag_struct_other.data.data();
                                std::size_t loc_tag_struct_other_size = loc_tag_struct_other.data.size();

                                const auto &loc_tag_struct = this->structs[*t.base_struct];

                                switch(t.tag_class_int) {
                                    case TagClassInt::TAG_CLASS_FONT: {
                                        const auto &font_tag = *reinterpret_cast<const Parser::Font::struct_little *>(loc_tag_struct.data.data());
                                        if(loc_tag_struct_other_size < sizeof(font_tag)) {
                                            REPORT_ERROR_PRINTF(*this, ERROR_TYPE_ERROR, std::nullopt, "%s in loc.map appears to be corrupt (font main struct goes out of bounds)", File::halo_path_to_preferred_path(t.path).c_str());
                                            match = false;
                                            break;
                                        }
                                        const auto &font_tag_other = *reinterpret_cast<const Parser::Font::struct_little *>(loc_tag_struct_other_data);

                                        // First, get the character counts of both
                                        std::size_t character_count = font_tag.characters.count;
                                        std::size_t character_count_other = font_tag_other.characters.count;
                                        std::size_t pixel_data_size = font_tag.pixels.size;
                                        std::size_t pixel_data_size_other = font_tag_other.pixels.size;

                                        // Next, compare the data
                                        match = font_tag_other.ascending_height == font_tag.ascending_height &&
                                                font_tag_other.descending_height == font_tag.descending_height &&
                                                std::memcmp(font_tag_other.flags.value, font_tag.flags.value, sizeof(font_tag.flags.value)) == 0 &&
                                                font_tag_other.leading_height == font_tag.leading_height &&
                                                font_tag_other.leading_width == font_tag.leading_width &&
                                                character_count == character_count_other &&
                                                pixel_data_size == pixel_data_size_other;

                                        // No match? Break.
                                        if(!match) {
                                            break;
                                        }

                                        // Now, compare the characters
                                        if(character_count > 0) {
                                            const auto *character_data = reinterpret_cast<const Parser::FontCharacter::struct_little *>(this->structs[*loc_tag_struct.resolve_pointer(&font_tag.characters.pointer)].data.data());
                                            const auto *character_data_other = reinterpret_cast<const Parser::FontCharacter::struct_little *>(loc_tag_struct_other_data + font_tag_other.characters.pointer);

                                            if(static_cast<std::size_t>(reinterpret_cast<const std::byte *>(character_data_other + character_count) - loc_tag_struct_other_data) > loc_tag_struct_other_size) {
                                                REPORT_ERROR_PRINTF(*this, ERROR_TYPE_WARNING, std::nullopt, "%s in loc.map appears to be corrupt (character data goes out of bounds)", File::halo_path_to_preferred_path(t.path).c_str());
                                                match = false;
                                                break;
                                            }

                                            for(std::size_t c = 0; c < character_count; c++) {
                                                const auto &character = character_data[c];
                                                const auto &character_other = character_data_other[c];

                                                match = character.bitmap_height == character_other.bitmap_height &&
                                                        character.bitmap_origin_x == character_other.bitmap_origin_x &&
                                                        character.bitmap_origin_y == character_other.bitmap_origin_y &&
                                                        character.bitmap_width == character_other.bitmap_width &&
                                                        character.character == character_other.character &&
                                                        character.pixels_offset == character_other.pixels_offset;

                                                // No match? Break.
                                                if(!match) {
                                                    break;
                                                }
                                            }

                                            // If we didn't get a match, stop
                                            if(!match) {
                                                break;
                                            }
                                        }

                                        // Lastly, check pixel data
                                        if(pixel_data_size > 0) {
                                            const auto *pixel_data = this->structs[*loc_tag_struct.resolve_pointer(&font_tag.pixels.pointer)].data.data();
                                            const auto *pixel_data_other = loc_tag_struct_other_data + font_tag_other.pixels.pointer;

                                            if(static_cast<std::size_t>(pixel_data_other + pixel_data_size - loc_tag_struct_other_data) > loc_tag_struct_other_size) {
                                                REPORT_ERROR_PRINTF(*this, ERROR_TYPE_ERROR, std::nullopt, "%s in loc.map appears to be corrupt (pixel data goes out of bounds)", File::halo_path_to_preferred_path(t.path).c_str());
                                                match = false;
                                                break;
                                            }

                                            match = std::memcmp(pixel_data, pixel_data_other, pixel_data_size) == 0;
                                        }
                                        break;
                                    }
                                    case TagClassInt::TAG_CLASS_UNICODE_STRING_LIST: {
                                        const auto &ustr_tag = *reinterpret_cast<const Parser::UnicodeStringList::struct_little *>(loc_tag_struct.data.data());
                                        if(loc_tag_struct_other_size < sizeof(ustr_tag)) {
                                            REPORT_ERROR_PRINTF(*this, ERROR_TYPE_ERROR, std::nullopt, "%s in loc.map appears to be corrupt (unicode string list main struct goes out of bounds)", File::halo_path_to_preferred_path(t.path).c_str());
                                            match = false;
                                            break;
                                        }
                                        const auto &ustr_tag_other = *reinterpret_cast<const Parser::UnicodeStringList::struct_little *>(loc_tag_struct_other_data);
                                        std::size_t string_count = ustr_tag.strings.count;
                                        std::size_t string_count_other = ustr_tag_other.strings.count;
                                        if(string_count != string_count_other) {
                                            match = false;
                                            break;
                                        }
                                        if(string_count > 0) {
                                            const auto &string_list_struct = this->structs[*loc_tag_struct.resolve_pointer(&ustr_tag.strings.pointer)];
                                            const auto *string_list = reinterpret_cast<const Parser::UnicodeStringListString::struct_little *>(string_list_struct.data.data());
                                            const auto *string_list_other = reinterpret_cast<const Parser::UnicodeStringListString::struct_little *>(loc_tag_struct_other_data + ustr_tag_other.strings.pointer);

                                            if(static_cast<std::size_t>(reinterpret_cast<const std::byte *>(string_list_other + string_count) - loc_tag_struct_other_data) > loc_tag_struct_other_size) {
                                                REPORT_ERROR_PRINTF(*this, ERROR_TYPE_ERROR, std::nullopt, "%s in loc.map appears to be corrupt (strings go out of bounds)", File::halo_path_to_preferred_path(t.path).c_str());
                                                match = false;
                                                break;
                                            }

                                            // Check each string
                                            for(std::size_t s = 0; s < string_count && match; s++) {
                                                const auto &string = string_list[s];
                                                const auto &string_other = string_list_other[s];
                                                std::size_t string_data_size = string.string.size;
                                                std::size_t string_data_size_other = string_other.string.size;

                                                // Make sure the sizes match
                                                if(string_data_size != string_data_size_other) {
                                                    match = false;
                                                    break;
                                                }

                                                if(string_data_size > 0) {
                                                    const auto *string_data = this->structs[*string_list_struct.resolve_pointer(&string.string.pointer)].data.data();
                                                    const auto *string_data_other = loc_tag_struct_other_data + string_other.string.pointer;

                                                    if(static_cast<std::size_t>(string_data_other + string_data_size - loc_tag_struct_other_data) > loc_tag_struct_other_size) {
                                                        REPORT_ERROR_PRINTF(*this, ERROR_TYPE_ERROR, std::nullopt, "%s in loc.map appears to be corrupt (string goes out of bounds)", File::halo_path_to_preferred_path(t.path).c_str());
                                                        match = false;
                                                        break;
                                                    }

                                                    match = std::memcmp(string_data, string_data_other, string_data_size) == 0;
                                                    if(!match) {
                                                        break;
                                                    }
                                                }
                                            }
                                        }
                                        break;
                                    }
                                    case TagClassInt::TAG_CLASS_HUD_MESSAGE_TEXT: {
                                        const auto &hud_message_tag = *reinterpret_cast<const Parser::HUDMessageText::struct_little *>(loc_tag_struct.data.data());
                                        if(loc_tag_struct_other_size < sizeof(hud_message_tag)) {
                                            REPORT_ERROR_PRINTF(*this, ERROR_TYPE_ERROR, std::nullopt, "%s in loc.map appears to be corrupt (unicode string list main struct goes out of bounds)", t.path.c_str());
                                            match = false;
                                            break;
                                        }
                                        const auto &hud_message_tag_other = *reinterpret_cast<const Parser::HUDMessageText::struct_little *>(loc_tag_struct_other_data);

                                        std::size_t message_count = hud_message_tag.messages.count;
                                        std::size_t message_count_other = hud_message_tag_other.messages.count;

                                        std::size_t message_element_count = hud_message_tag.message_elements.count;
                                        std::size_t message_element_count_other = hud_message_tag_other.message_elements.count;

                                        std::size_t text_data_size = hud_message_tag.text_data.size;
                                        std::size_t text_data_size_other = hud_message_tag_other.text_data.size;

                                        match = message_count == message_count_other && message_element_count == message_element_count_other && text_data_size == text_data_size_other;

                                        // Give up if needed
                                        if(!match) {
                                            break;
                                        }

                                        // Make sure the messages are the same
                                        if(message_count > 0) {
                                            const auto *messages = reinterpret_cast<const Parser::HUDMessageTextMessage::struct_little *>(this->structs[*loc_tag_struct.resolve_pointer(&hud_message_tag.messages.pointer)].data.data());
                                            const auto *messages_other = reinterpret_cast<const Parser::HUDMessageTextMessage::struct_little *>(loc_tag_struct_other_data + hud_message_tag_other.messages.pointer);

                                            if(static_cast<std::size_t>(reinterpret_cast<const std::byte *>(messages_other + message_count) - loc_tag_struct_other_data) > loc_tag_struct_other_size) {
                                                REPORT_ERROR_PRINTF(*this, ERROR_TYPE_ERROR, std::nullopt, "%s in loc.map appears to be corrupt (messages go out of bounds)", File::halo_path_to_preferred_path(t.path).c_str());
                                                match = false;
                                                break;
                                            }

                                            for(std::size_t i = 0; i < message_count; i++) {
                                                auto &message = messages[i];
                                                auto &message_other = messages_other[i];

                                                // If they are different, bail
                                                match = message.name == message_other.name && message.panel_count == message_other.panel_count && message.start_index_into_text_blob == message_other.start_index_into_text_blob && message.start_index_of_message_block == message_other.start_index_of_message_block;
                                                if(!match) {
                                                    break;
                                                }
                                            }
                                        }

                                        // Make sure the message elements are the same
                                        if(message_element_count > 0) {
                                            const auto *message_elements = reinterpret_cast<const Parser::HUDMessageTextElement::struct_little *>(this->structs[*loc_tag_struct.resolve_pointer(&hud_message_tag.message_elements.pointer)].data.data());
                                            const auto *message_elements_other = reinterpret_cast<const Parser::HUDMessageTextElement::struct_little *>(loc_tag_struct_other_data + hud_message_tag_other.message_elements.pointer);

                                            if(static_cast<std::size_t>(reinterpret_cast<const std::byte *>(message_elements_other + message_element_count) - loc_tag_struct_other_data) > loc_tag_struct_other_size) {
                                                REPORT_ERROR_PRINTF(*this, ERROR_TYPE_ERROR, std::nullopt, "%s in loc.map appears to be corrupt (message elements go out of bounds)", File::halo_path_to_preferred_path(t.path).c_str());
                                                match = false;
                                                break;
                                            }

                                            for(std::size_t i = 0; i < message_element_count && match; i++) {
                                                auto &element = message_elements[i];
                                                auto &element_other = message_elements_other[i];

                                                // If they are different, bail
                                                match = element.data == element_other.data && element.type == element_other.type;
                                                if(!match) {
                                                    break;
                                                }
                                            }
                                        }

                                        // Make sure the text data is the same
                                        if(text_data_size > 0) {
                                            const auto *text_data = this->structs[*loc_tag_struct.resolve_pointer(&hud_message_tag.text_data.pointer)].data.data();
                                            const auto *text_data_other = loc_tag_struct_other_data + hud_message_tag_other.text_data.pointer;

                                            if(static_cast<std::size_t>(text_data_other + text_data_size - loc_tag_struct_other_data) > loc_tag_struct_other_size) {
                                                REPORT_ERROR_PRINTF(*this, ERROR_TYPE_ERROR, std::nullopt, "%s in loc.map appears to be corrupt (text data goes out of bounds)", File::halo_path_to_preferred_path(t.path).c_str());
                                                match = false;
                                                break;
                                            }

                                            match = std::memcmp(text_data, text_data_other, text_data_size) == 0;
                                        }
                                        break;
                                    }
                                    default:
                                        // There is no way we can get here
                                        std::terminate();
                                }
                                if(match) {
                                    t.resource_index = index;
                                    this->indexed_data_amount += this->loc[*index].data.size();
                                    t.base_struct = std::nullopt;
                                    break;
                                }
                                else {
                                    REPORT_ERROR_PRINTF(*this, ERROR_TYPE_WARNING_PEDANTIC, &t - this->tags.data(), "%s.%s does not match the one found in loc.map, so it will NOT be indexed out", File::halo_path_to_preferred_path(t.path).c_str(), HEK::tag_class_to_extension(t.tag_class_int));
                                }
                            }
                            break;
                        }
                        default:
                            break;
                    }

                    // Clear off stuff
                    if(t.resource_index.has_value()) {
                        for(auto &a : t.asset_data) {
                            this->delete_raw_data(a);
                        }
                    }
                }
                break;
            case HEK::CacheFileEngine::CACHE_FILE_RETAIL:
            case HEK::CacheFileEngine::CACHE_FILE_DEMO:
                for(auto &t : this->tags) {
                    switch(t.tag_class_int) {
                        // Iterate through each permutation in each pitch range to find the bitmap
                        case TagClassInt::TAG_CLASS_BITMAP: {
                            auto &bitmap_tag_struct = this->structs[*t.base_struct];
                            auto &bitmap_tag = *reinterpret_cast<Parser::Bitmap::struct_little *>(bitmap_tag_struct.data.data());
                            std::size_t bitmap_data_count = bitmap_tag.bitmap_data.count;
                            if(bitmap_data_count) {
                                auto *all_bitmap_data = reinterpret_cast<Parser::BitmapData::struct_little *>(this->structs[*bitmap_tag_struct.resolve_pointer(&bitmap_tag.bitmap_data.pointer)].data.data());
                                for(std::size_t b = 0; b < bitmap_data_count; b++) {
                                    auto &bitmap_data = all_bitmap_data[b];
                                    std::size_t raw_data_index = t.asset_data[b];
                                    auto &raw_data = this->raw_data[raw_data_index];
                                    auto *raw_data_data = raw_data.data();
                                    std::size_t raw_data_size = raw_data.size();

                                    // Find bitmaps
                                    for(auto &ab : this->bitmaps) {
                                        if(ab.data.size() >= raw_data_size && std::memcmp(ab.data.data(), raw_data_data, raw_data_size) == 0) {
                                            this->delete_raw_data(raw_data_index);
                                            bitmap_data.pixel_data_offset = static_cast<std::uint32_t>(ab.data_offset);
                                            auto flags = bitmap_data.flags.read();
                                            flags |= HEK::BitmapDataFlagsFlag::BITMAP_DATA_FLAGS_FLAG_EXTERNAL;
                                            bitmap_data.flags = flags;
                                            break;
                                        }
                                    }
                                }
                            }
                            break;
                        }

                        // Iterate through each permutation in each pitch range to find the sound
                        case TagClassInt::TAG_CLASS_SOUND: {
                            auto &sound_tag_struct = this->structs[*t.base_struct];
                            auto &sound_tag = *reinterpret_cast<Parser::Sound::struct_little *>(sound_tag_struct.data.data());
                            std::size_t sound_pitch_range_count = sound_tag.pitch_ranges.count;
                            std::size_t resource_index = 0;
                            if(sound_pitch_range_count) {
                                auto &pitch_range_struct = this->structs[*sound_tag_struct.resolve_pointer(&sound_tag.pitch_ranges.pointer)];
                                auto *all_pitch_ranges = reinterpret_cast<Parser::SoundPitchRange::struct_little *>(pitch_range_struct.data.data());
                                for(std::size_t pr = 0; pr < sound_pitch_range_count; pr++) {
                                    auto &pitch_range = all_pitch_ranges[pr];
                                    std::size_t permutation_count = pitch_range.permutations.count;
                                    if(permutation_count) {
                                        auto *all_permutations = reinterpret_cast<Parser::SoundPermutation::struct_little *>(this->structs[*pitch_range_struct.resolve_pointer(&pitch_range.permutations.pointer)].data.data());
                                        for(std::size_t p = 0; p < permutation_count; p++) {
                                            auto &permutation = all_permutations[p];
                                            std::size_t raw_data_index = t.asset_data[resource_index++];
                                            auto &raw_data = this->raw_data[raw_data_index];
                                            auto *raw_data_data = raw_data.data();
                                            std::size_t raw_data_size = raw_data.size();

                                            // Find sounds
                                            for(auto &ab : this->sounds) {
                                                if(ab.data.size() >= raw_data_size && std::memcmp(ab.data.data(), raw_data_data, raw_data_size) == 0) {
                                                    this->delete_raw_data(raw_data_index);
                                                    permutation.samples.file_offset = static_cast<std::uint32_t>(ab.data_offset);
                                                    permutation.samples.external = 1;
                                                    break;
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                            break;
                        }
                        default:
                            break;
                    }
                }
                break;
            default:
                break;
        }
    }

    void BuildWorkload::delete_raw_data(std::size_t index) {
        for(auto &t : this->tags) {
            for(auto &r : t.asset_data) {
                if(r == index) {
                    r = static_cast<std::size_t>(~0);
                }
                else if(r > index && r != static_cast<std::size_t>(~0)) {
                    r--;
                }
            }
        }
        this->raw_data.erase(this->raw_data.begin() + index);
    }
}
