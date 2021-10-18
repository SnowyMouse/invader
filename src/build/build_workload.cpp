// SPDX-License-Identifier: GPL-3.0-only

#include <ctime>
#include <cstdio>

#include <invader/build/build_workload.hpp>
#include <invader/hek/map.hpp>
#include <invader/file/file.hpp>
#include <invader/tag/hek/header.hpp>
#include <invader/version.hpp>
#include <invader/crc/hek/crc.hpp>
#include <invader/compress/compression.hpp>
#include <invader/tag/index/index.hpp>
#include <invader/tag/parser/compile/scenario_structure_bsp.hpp>
#include <invader/resource/list/resource_list.hpp>
#include "../crc/crc32.h"

namespace Invader {
    using namespace HEK;
    
    BuildWorkload::BuildParameters::BuildParametersDetails::BuildParametersDetails(const HEK::GameEngineInfo &engine_info) noexcept {
        this->build_maximum_tag_space = engine_info.tag_space_length;
        this->build_tag_data_address = engine_info.base_memory_address;
        this->build_compress = engine_info.uses_compression;
        this->build_version = engine_info.build_string_is_enforced ? engine_info.get_build_string() : full_version();
        this->build_bsps_occupy_tag_space = engine_info.bsps_occupy_tag_space;
        
        if(engine_info.supports_external_resource_maps()) {
            this->build_raw_data_handling = RawDataHandling::RAW_DATA_HANDLING_RETAIN_AUTOMATICALLY;
        }
        else {
            this->build_raw_data_handling = RawDataHandling::RAW_DATA_HANDLING_RETAIN_ALL;
        }
        
        this->build_cache_file_engine = engine_info.cache_version;
        this->build_game_engine = engine_info.engine;
        this->build_maximum_cache_file_size = engine_info.maximum_file_size;
        this->build_required_tags = engine_info.required_tags;
    }
    
    BuildWorkload::BuildParameters::BuildParameters::BuildParameters(HEK::GameEngine engine) noexcept : details(engine) {}
    
    BuildWorkload::BuildParameters::BuildParameters(const std::string &scenario, const std::vector<std::filesystem::path> &tags_directories, HEK::GameEngine engine) : 
        scenario(scenario),
        tags_directories(tags_directories),
        details(engine) {}

    #define TAG_DATA_HEADER_STRUCT (structs[0])
    #define TAG_ARRAY_STRUCT (structs[1])

    BuildWorkload::BuildWorkload() : ErrorHandler() {}

    std::vector<std::byte> BuildWorkload::compile_map(const BuildParameters &parameters) {
        BuildWorkload workload;
        workload.parameters = &parameters;

        // Start benchmark
        workload.start = std::chrono::steady_clock::now();

        // Hide these?
        switch(parameters.verbosity) {
            case BuildParameters::BuildVerbosity::BUILD_VERBOSITY_SHOW_ALL:
                break;
            case BuildParameters::BuildVerbosity::BUILD_VERBOSITY_HIDE_PEDANTIC:
                workload.set_reporting_level(REPORTING_LEVEL_HIDE_ALL_PEDANTIC_WARNINGS);
                break;
            case BuildParameters::BuildVerbosity::BUILD_VERBOSITY_QUIET:
            case BuildParameters::BuildVerbosity::BUILD_VERBOSITY_HIDE_WARNINGS:
                workload.set_reporting_level(REPORTING_LEVEL_HIDE_ALL_WARNINGS);
                break;
            case BuildParameters::BuildVerbosity::BUILD_VERBOSITY_HIDE_ERRORS:
                workload.set_reporting_level(REPORTING_LEVEL_HIDE_EVERYTHING);
                break;
        }

        return workload.build_cache_file();
    }

    #define BYTES_TO_MiB(bytes) (bytes / 1024.0 / 1024.0)

    std::vector<std::byte> BuildWorkload::build_cache_file() {
        // Yay
        File::check_working_directory("./toolbeta.map");
        auto cache_version = this->parameters->details.build_cache_file_engine;
        auto engine_target = this->parameters->details.build_game_engine;
        
        // Update scenario name if needed
        auto scenario_name_fixed = File::preferred_path_to_halo_path(this->parameters->scenario);
        this->scenario = scenario_name_fixed.c_str();

        // Set the scenario name too
        if(this->parameters->rename_scenario.has_value()) {
            this->set_scenario_name((*this->parameters->rename_scenario).c_str());
        }
        else {
            this->set_scenario_name(scenario_name_fixed.c_str());
        }
        
        // Reserve indexed tags
        if(this->parameters->index.has_value()) {
            auto &index = *this->parameters->index;
            auto &tag_paths = this->get_tag_paths();
            this->tags.reserve(index.size());
            tag_paths.reserve(index.size());
            for(auto &i : index) {
                auto &tag = this->tags.emplace_back();
                tag_paths.emplace_back(i);
                tag.path = i.path;
                tag.tag_fourcc = i.fourcc;
                tag.stubbed = true;
            }
        }

        // First, make our tag data header and array
        this->structs.resize(2);
        TAG_DATA_HEADER_STRUCT.unsafe_to_dedupe = true;
        TAG_ARRAY_STRUCT.unsafe_to_dedupe = true;

        // Add all of the tags
        if(this->parameters->verbosity > BuildParameters::BuildVerbosity::BUILD_VERBOSITY_QUIET) {
            oprintf("Reading tags...\n");
        }
        this->add_tags();
        
        // Check this stuff
        this->check_hud_text_indices();

        // If we have resource maps to check, check them
        if(this->parameters->details.build_raw_data_handling != BuildParameters::BuildParametersDetails::RawDataHandling::RAW_DATA_HANDLING_RETAIN_ALL) {
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
        switch(cache_version) {
            case HEK::CacheFileEngine::CACHE_FILE_NATIVE:
                make_tag_data_header_struct(this->scenario_index, this->structs, sizeof(HEK::NativeCacheFileTagDataHeader));
                break;
            case HEK::CacheFileEngine::CACHE_FILE_XBOX:
                make_tag_data_header_struct(this->scenario_index, this->structs, sizeof(HEK::CacheFileTagDataHeaderXbox));
                break;
            default:
                make_tag_data_header_struct(this->scenario_index, this->structs, sizeof(HEK::CacheFileTagDataHeaderPC));
                break;
        }

        auto errors = this->get_errors();
        if(errors) {
            auto warnings = this->get_warnings();
            if(this->parameters->verbosity > BuildParameters::BuildVerbosity::BUILD_VERBOSITY_QUIET) {
                if(warnings) {
                    oprintf_fail("Build failed with %zu error%s and %zu warning%s", errors, errors == 1 ? "" : "s", warnings, warnings == 1 ? "" : "s");
                }
                else {
                    oprintf_fail("Build failed with %zu error%s", errors, errors == 1 ? "" : "s");
                }
            }
            throw InvalidTagDataException();
        }
        
        // Generate memes on Xbox
        if(cache_version == HEK::CacheFileEngine::CACHE_FILE_XBOX) {
            this->generate_compressed_model_tag_array();
        }

        // Dedupe structs
        if(this->parameters->optimize_space) {
            this->dedupe_structs();
        }

        // Get the tag data
        if(this->parameters->verbosity > BuildParameters::BuildVerbosity::BUILD_VERBOSITY_QUIET) {
            oprintf("Building tag data...");
            oflush();
        }
        std::size_t end_of_bsps = this->generate_tag_data();
        if(this->parameters->verbosity > BuildParameters::BuildVerbosity::BUILD_VERBOSITY_QUIET) {
            oprintf(" done\n");
        }

        // Find the largest BSP
        std::size_t bsp_size = 0;
        std::size_t largest_bsp_size = 0;
        std::size_t largest_bsp_count = 0;

        bool bsp_size_affects_tag_space = this->parameters->details.build_bsps_occupy_tag_space;

        // Calculate total BSP size (pointless on native maps)
        std::vector<std::size_t> bsp_sizes(this->bsp_count);
        if(cache_version != HEK::CacheFileEngine::CACHE_FILE_NATIVE) {
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
        if(this->parameters->verbosity > BuildParameters::BuildVerbosity::BUILD_VERBOSITY_QUIET) {
            oprintf("Building raw data...");
            oflush();
        }
        this->generate_bitmap_sound_data(end_of_bsps);
        if(this->parameters->verbosity > BuildParameters::BuildVerbosity::BUILD_VERBOSITY_QUIET) {
            oprintf(" done\n");
        }
        
        // Query the maximum file size
        auto &max_size_ref = this->parameters->details.build_maximum_cache_file_size;
        Pointer64 max_size = UINT64_MAX;
        if(auto *size = std::get_if<Pointer64>(&max_size_ref)) {
            max_size = *size;
        }
        else if(auto *size = std::get_if<Pointer64 (*)(CacheFileType)>(&max_size_ref)) {
            max_size = (*size)(*this->cache_file_type);
        }

        auto &workload = *this;
        auto generate_final_data = [&workload, &bsp_size_affects_tag_space, &bsp_size, &cache_version, &engine_target, &largest_bsp_size, &largest_bsp_count, &bsp_sizes, &max_size](auto &header) {
            std::vector<std::byte> final_data;
            std::strncpy(header.build.string, workload.parameters->details.build_version.c_str(), sizeof(header.build.string) - 1);
            header.engine = workload.parameters->details.build_cache_file_engine;
            header.map_type = *workload.cache_file_type;
            header.name = workload.scenario_name;

            if(workload.parameters->verbosity > BuildParameters::BuildVerbosity::BUILD_VERBOSITY_QUIET) {
                oprintf("Building cache file data...");
                oflush();
            }

            // Add header stuff
            final_data.resize(sizeof(HEK::CacheFileHeader));
            
            // Add each BSP data thing
            for(auto &b : workload.bsp_data) {
                final_data.insert(final_data.end(), b.begin(), b.end());
            }

            // Go through each BSP and add that stuff
            if(cache_version != HEK::CacheFileEngine::CACHE_FILE_NATIVE) {
                for(std::size_t b = 0; b < workload.bsp_count; b++) {
                    final_data.insert(final_data.end(), workload.map_data_structs[b + 1].begin(), workload.map_data_structs[b + 1].end());
                }
            }
            workload.map_data_structs.resize(1);

            // Now add all the raw data
            final_data.insert(final_data.end(), workload.all_raw_data.begin(), workload.all_raw_data.end());
            auto raw_data_size = workload.all_raw_data.size();
            workload.all_raw_data = std::vector<std::byte>();
            
            std::size_t model_data_size;
            std::size_t vertex_size;
            std::size_t model_offset;
            std::size_t tag_data_offset;
            
            // If we're not on Xbox, we put the model data here
            if(cache_version != HEK::CacheFileEngine::CACHE_FILE_XBOX) {
                // Let's get the model data there
                model_offset = final_data.size() + REQUIRED_PADDING_32_BIT(final_data.size());
                final_data.resize(model_offset, std::byte());
                final_data.insert(final_data.end(), reinterpret_cast<std::byte *>(workload.uncompressed_model_vertices.data()), reinterpret_cast<std::byte *>(workload.uncompressed_model_vertices.data() + workload.uncompressed_model_vertices.size()));

                // Now add model indices
                vertex_size = workload.uncompressed_model_vertices.size() * sizeof(*workload.uncompressed_model_vertices.data());
                final_data.insert(final_data.end(), reinterpret_cast<std::byte *>(workload.model_indices.data()), reinterpret_cast<std::byte *>(workload.model_indices.data() + workload.model_indices.size()));
                workload.uncompressed_model_vertices = decltype(workload.uncompressed_model_vertices)();
                workload.model_indices = decltype(workload.model_indices)();
                
                tag_data_offset = final_data.size() + REQUIRED_PADDING_32_BIT(final_data.size());
                model_data_size = tag_data_offset - model_offset;
            }
            
            // If we ARE on Xbox, then we go straight to the tag data
            else {
                model_data_size = 0;
                vertex_size = 0;
                model_offset = 0;
                tag_data_offset = final_data.size() + REQUIRED_PADDING_N_BYTES(final_data.size(), HEK::CacheFileXboxConstants::CACHE_FILE_XBOX_SECTOR_SIZE);
            }
            
            // We're almost there
            final_data.resize(tag_data_offset, std::byte());

            // Add tag data
            std::size_t tag_data_size = workload.map_data_structs[0].size();
            final_data.insert(final_data.end(), workload.map_data_structs[0].begin(), workload.map_data_structs[0].end());
            auto part_count = workload.model_parts.size();
            if(cache_version == HEK::CacheFileEngine::CACHE_FILE_NATIVE) {
                auto &tag_data_struct = *reinterpret_cast<HEK::NativeCacheFileTagDataHeader *>(final_data.data() + tag_data_offset);
                tag_data_struct.tag_count = static_cast<std::uint32_t>(workload.tags.size());
                tag_data_struct.tags_literal = CacheFileLiteral::CACHE_FILE_TAGS;
                tag_data_struct.model_part_count = static_cast<std::uint32_t>(part_count);
                tag_data_struct.model_data_file_offset = static_cast<std::uint32_t>(model_offset);
                tag_data_struct.vertex_size = static_cast<std::uint32_t>(vertex_size);
                tag_data_struct.model_data_size = static_cast<std::uint32_t>(model_data_size);
                tag_data_struct.raw_data_indices = workload.raw_data_indices_offset;
            }
            else if(cache_version == HEK::CacheFileEngine::CACHE_FILE_XBOX) {
                auto &tag_data_struct = *reinterpret_cast<HEK::CacheFileTagDataHeaderXbox *>(final_data.data() + tag_data_offset);
                tag_data_struct.tag_count = static_cast<std::uint32_t>(workload.tags.size());
                tag_data_struct.tags_literal = CacheFileLiteral::CACHE_FILE_TAGS;
                tag_data_struct.model_part_count = static_cast<std::uint32_t>(part_count);
                tag_data_struct.model_part_count_again = static_cast<std::uint32_t>(part_count);
            }
            else {
                auto &tag_data_struct = *reinterpret_cast<HEK::CacheFileTagDataHeaderPC *>(final_data.data() + tag_data_offset);
                tag_data_struct.tag_count = static_cast<std::uint32_t>(workload.tags.size());
                tag_data_struct.tags_literal = CacheFileLiteral::CACHE_FILE_TAGS;
                tag_data_struct.model_part_count = static_cast<std::uint32_t>(part_count);
                tag_data_struct.model_part_count_again = static_cast<std::uint32_t>(part_count);
                tag_data_struct.model_data_file_offset = static_cast<std::uint32_t>(model_offset);
                tag_data_struct.vertex_size = static_cast<std::uint32_t>(vertex_size);
                tag_data_struct.model_data_size = static_cast<std::uint32_t>(model_data_size);
            }

            // Lastly, do the header
            header.tag_data_size = static_cast<std::uint32_t>(tag_data_size);
            header.tag_data_offset = static_cast<std::uint32_t>(tag_data_offset);
            if(cache_version == HEK::CacheFileEngine::CACHE_FILE_DEMO) {
                header.head_literal = CacheFileLiteral::CACHE_FILE_HEAD_DEMO;
                header.foot_literal = CacheFileLiteral::CACHE_FILE_FOOT_DEMO;
                *reinterpret_cast<HEK::CacheFileDemoHeader *>(final_data.data()) = *reinterpret_cast<HEK::CacheFileHeader *>(&header);
            }
            else {
                header.head_literal = CacheFileLiteral::CACHE_FILE_HEAD;
                header.foot_literal = CacheFileLiteral::CACHE_FILE_FOOT;
                std::memcpy(final_data.data(), &header, sizeof(header));
            }

            if(workload.parameters->verbosity > BuildParameters::BuildVerbosity::BUILD_VERBOSITY_QUIET) {
                oprintf(" done\n");
            }
            
            // Resize to ye ol' sector
            if(cache_version == HEK::CacheFileEngine::CACHE_FILE_XBOX) {
                final_data.insert(final_data.end(), REQUIRED_PADDING_N_BYTES(final_data.size(), HEK::CacheFileXboxConstants::CACHE_FILE_XBOX_SECTOR_SIZE), std::byte());
            }

            // Check to make sure we aren't too big
            std::size_t uncompressed_size = final_data.size();
            if(static_cast<std::uint64_t>(uncompressed_size) > max_size) {
                REPORT_ERROR_PRINTF(workload, ERROR_TYPE_FATAL_ERROR, std::nullopt, "Map file exceeds maximum size for the target engine when uncompressed (%.04f MiB > %.04f MiB)", BYTES_TO_MiB(uncompressed_size), BYTES_TO_MiB(static_cast<std::size_t>(max_size)));
                throw MaximumFileSizeException();
            }

            // Make sure we don't go beyond the maximum tag space usage
            std::size_t tag_space_usage = workload.indexed_data_amount + tag_data_size;
            if(bsp_size_affects_tag_space) {
                tag_space_usage += largest_bsp_size;
            }
            if(tag_space_usage > workload.parameters->details.build_maximum_tag_space) {
                REPORT_ERROR_PRINTF(workload, ERROR_TYPE_FATAL_ERROR, std::nullopt, "Maximum tag space exceeded (%.04f MiB > %.04f MiB)", BYTES_TO_MiB(tag_space_usage), BYTES_TO_MiB(workload.parameters->details.build_maximum_tag_space));
                throw MaximumFileSizeException();
            }

            // Hold this here, of course
            auto &tag_file_checksums = reinterpret_cast<HEK::CacheFileTagDataHeader *>(final_data.data() + tag_data_offset)->tag_file_checksums;
            tag_file_checksums = workload.tag_file_checksums;
            
            // If we can calculate the CRC32, do it
            std::uint32_t new_crc = 0;
            bool can_calculate_crc = cache_version != CacheFileEngine::CACHE_FILE_XBOX;
            
            if(can_calculate_crc) {
                if(workload.parameters->verbosity > BuildParameters::BuildVerbosity::BUILD_VERBOSITY_QUIET) {
                    oprintf("Calculating CRC32...");
                    oflush();
                }
                
                // Calculate the CRC32 and/or forge one if we must
                if(workload.parameters->forge_crc.has_value()) {
                    std::uint32_t checksum_delta = 0;
                    new_crc = calculate_map_crc(final_data.data(), final_data.size(), &workload.parameters->forge_crc.value(), &checksum_delta);
                    tag_file_checksums = checksum_delta;
                }
                else {
                    new_crc = calculate_map_crc(final_data.data(), final_data.size());
                }
                
                header.crc32 = new_crc;
                if(workload.parameters->verbosity > BuildParameters::BuildVerbosity::BUILD_VERBOSITY_QUIET) {
                    oprintf(" done\n");
                }
            }
            else {
                header.crc32 = UINT32_MAX;
            }
            
            // Set the file size
            header.decompressed_file_size = final_data.size();

            // Copy it again, this time with the new CRC32
            if(cache_version == HEK::CacheFileEngine::CACHE_FILE_DEMO) {
                *reinterpret_cast<HEK::CacheFileDemoHeader *>(final_data.data()) = *reinterpret_cast<HEK::CacheFileHeader *>(&header);
            }
            else {
                std::memcpy(final_data.data(), &header, sizeof(header));
            }

            // Compress if needed
            if(workload.parameters->details.build_compress) {
                if(workload.parameters->verbosity > BuildParameters::BuildVerbosity::BUILD_VERBOSITY_QUIET) {
                    oprintf("Compressing...");
                    oflush();
                }
                final_data = Compression::compress_map_data(final_data.data(), final_data.size(), workload.parameters->details.build_compression_level.value_or(19));
                if(workload.parameters->verbosity > BuildParameters::BuildVerbosity::BUILD_VERBOSITY_QUIET) {
                    oprintf(" done\n");
                }
            }

            // Display the scenario name and information
            if(workload.parameters->verbosity > BuildParameters::BuildVerbosity::BUILD_VERBOSITY_QUIET) {
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
                oprintf("Engine:            %s\n", HEK::GameEngineInfo::get_game_engine_info(engine_target).name);
                oprintf("Map type:          %s\n", HEK::type_name(*workload.cache_file_type));
                oprintf("Tags:              %zu / %zu (%.02f MiB)", workload.tags.size(), static_cast<std::size_t>(UINT16_MAX), BYTES_TO_MiB(workload.map_data_structs[0].size()));
                if(workload.stubbed_tag_count) {
                    oprintf(", %zu stubbed", workload.stubbed_tag_count);
                }
                oprintf("\n");

                // Show the BSP count and/or size
                oprintf("BSPs:              %zu", workload.bsp_count);
                if(cache_version == HEK::CacheFileEngine::CACHE_FILE_NATIVE) {
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
                        if(workload.parameters->details.build_bsps_occupy_tag_space ) {
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
                    if(workload.parameters->details.build_bsps_occupy_tag_space && largest_bsp_count < workload.bsp_count) {
                        oprintf("                   * = Largest BSP%s%s\n", largest_bsp_count == 1 ? "" : "s", bsp_size_affects_tag_space ? " (affects final tag space usage)" : "");
                    }
                }

                // Show the total tag space (if applicable)
                if(cache_version != HEK::CacheFileEngine::CACHE_FILE_NATIVE) {
                    oprintf("Tag space:         %.02f / %.02f MiB (%.02f %%)\n", BYTES_TO_MiB(tag_space_usage), BYTES_TO_MiB(workload.parameters->details.build_maximum_tag_space), 100.0 * tag_space_usage / workload.parameters->details.build_maximum_tag_space);
                }

                // Show some other data that might be useful
                oprintf("Models:            %zu (%.02f MiB)\n", part_count, BYTES_TO_MiB(model_data_size));
                oprintf("Raw data:          %.02f MiB (%.02f MiB bitmaps, %.02f MiB sounds)\n", BYTES_TO_MiB(raw_data_size), BYTES_TO_MiB(workload.raw_bitmap_size), BYTES_TO_MiB(workload.raw_sound_size));

                // Show our CRC32
                if(can_calculate_crc) {
                    oprintf("CRC32 checksum:    0x%08X\n", new_crc);
                }

                // If we compressed it, how small did we get it?
                if(workload.parameters->details.build_compress) {
                    std::size_t compressed_size = final_data.size();
                    oprintf("Compressed size:   %.02f MiB (%.02f %%)\n", BYTES_TO_MiB(compressed_size), 100.0 * compressed_size / uncompressed_size);
                }

                // Show the original size
                oprintf("Uncompressed size: %.02f ", BYTES_TO_MiB(uncompressed_size));

                // If we have a limit, show it
                if(max_size < UINT64_MAX) {
                    oprintf("/ %.02f MiB (%.02f %%)\n", BYTES_TO_MiB(max_size), 100.0 * uncompressed_size / max_size);
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

        switch(this->parameters->details.build_cache_file_engine) {
            case HEK::CacheFileEngine::CACHE_FILE_NATIVE: {
                HEK::NativeCacheFileHeader header = {};
                
                // Store the timestamp
                std::time_t current_time = std::time(nullptr);
                auto *gmt = std::gmtime(&current_time);
                std::snprintf(header.timestamp.string, sizeof(header.timestamp.string), "%04u-%02u-%02uT%02u:%02u:%02uZ", gmt->tm_year + 1900, gmt->tm_mon + 1, gmt->tm_mday, gmt->tm_hour, gmt->tm_min, gmt->tm_sec);
                
                // Done
                return generate_final_data(header);
            }
            case HEK::CacheFileEngine::CACHE_FILE_MCC_CEA: {
                HEK::CacheFileHeaderCEA header = {};
                header.flags = this->parameters->details.build_flags_cea;
                return generate_final_data(header);
            }
            default: {
                HEK::CacheFileHeader header = {};
                return generate_final_data(header);
            }
        }
    }

    void BuildWorkload::compile_tag_data_recursively(const std::byte *tag_data, std::size_t tag_data_size, std::size_t tag_index, std::optional<TagFourCC> tag_fourcc) {
        #define COMPILE_TAG_CLASS(class_struct, fourcc) case TagFourCC::fourcc: { \
            do_compile_tag(std::move(Parser::class_struct::parse_hek_tag_file(tag_data, tag_data_size, true))); \
            break; \
        }

        auto *header = reinterpret_cast<const HEK::TagFileHeader *>(tag_data);

        if(!tag_fourcc.has_value()) {
            tag_fourcc = header->tag_fourcc;
        }

        // Set this in case it's not set yet
        this->tags[tag_index].tag_fourcc = *tag_fourcc;
        
        // Make sure the path isn't bullshit
        bool invalid_path = false;
        bool uppercase = false;
        bool forward_slash = false;
        bool control = false;
        for(auto &c : this->tags[tag_index].path) {
            if(c == '/') {
                invalid_path = true;
                forward_slash = true;
            }
            else if(c >= 'A' && c <= 'Z') {
                invalid_path = true;
                uppercase = true;
            }
            else {
                std::uint8_t latin1 = static_cast<std::uint8_t>(c);
                if(latin1 < ' ' || (latin1 >= 0x7F && latin1 < 0xA0)) {
                    invalid_path = true;
                    control = true;
                    c = '?';
                }
            }
        }
        
        if(invalid_path) {
            REPORT_ERROR_PRINTF(*this, ERROR_TYPE_FATAL_ERROR, tag_index, "Tag path contains invalid characters");
            
            if(uppercase) {
                eprintf_warn("Tags may not have uppercase characters");
            }
            if(forward_slash) {
                eprintf_warn("Tags may not have forward slashes internally");
            }
            if(control) {
                eprintf_warn("Tags may not have control characters");
            }
            
            throw InvalidTagPathException();
        }

        // Check header and CRC32
        HEK::TagFileHeader::validate_header(header, tag_data_size, tag_fourcc);
        HEK::BigEndian<std::uint32_t> expected_crc = ~crc32(0, header + 1, tag_data_size - sizeof(*header));
        
        // Make sure the header's CRC32 matches the calculated CRC32
        if(expected_crc != header->crc32) {
            REPORT_ERROR_PRINTF(*this, ERROR_TYPE_WARNING_PEDANTIC, tag_index, "%s.%s's CRC32 is incorrect. Tag may have been improperly modified.", File::halo_path_to_preferred_path(this->tags[tag_index].path).c_str(), tag_fourcc_to_extension(tag_fourcc.value()));
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

        switch(*tag_fourcc) {
            COMPILE_TAG_CLASS(Actor, TAG_FOURCC_ACTOR)
            COMPILE_TAG_CLASS(ActorVariant, TAG_FOURCC_ACTOR_VARIANT)
            COMPILE_TAG_CLASS(Antenna, TAG_FOURCC_ANTENNA)
            COMPILE_TAG_CLASS(ModelAnimations, TAG_FOURCC_MODEL_ANIMATIONS)
            COMPILE_TAG_CLASS(Biped, TAG_FOURCC_BIPED)
            COMPILE_TAG_CLASS(Bitmap, TAG_FOURCC_BITMAP)
            COMPILE_TAG_CLASS(ModelCollisionGeometry, TAG_FOURCC_MODEL_COLLISION_GEOMETRY)
            COMPILE_TAG_CLASS(ColorTable, TAG_FOURCC_COLOR_TABLE)
            COMPILE_TAG_CLASS(Contrail, TAG_FOURCC_CONTRAIL)
            COMPILE_TAG_CLASS(DeviceControl, TAG_FOURCC_DEVICE_CONTROL)
            COMPILE_TAG_CLASS(Decal, TAG_FOURCC_DECAL)
            COMPILE_TAG_CLASS(UIWidgetDefinition, TAG_FOURCC_UI_WIDGET_DEFINITION)
            COMPILE_TAG_CLASS(InputDeviceDefaults, TAG_FOURCC_INPUT_DEVICE_DEFAULTS)
            COMPILE_TAG_CLASS(DetailObjectCollection, TAG_FOURCC_DETAIL_OBJECT_COLLECTION)
            COMPILE_TAG_CLASS(Effect, TAG_FOURCC_EFFECT)
            COMPILE_TAG_CLASS(Equipment, TAG_FOURCC_EQUIPMENT)
            COMPILE_TAG_CLASS(Flag, TAG_FOURCC_FLAG)
            COMPILE_TAG_CLASS(Fog, TAG_FOURCC_FOG)
            COMPILE_TAG_CLASS(Font, TAG_FOURCC_FONT)
            COMPILE_TAG_CLASS(MaterialEffects, TAG_FOURCC_MATERIAL_EFFECTS)
            COMPILE_TAG_CLASS(Garbage, TAG_FOURCC_GARBAGE)
            COMPILE_TAG_CLASS(Glow, TAG_FOURCC_GLOW)
            COMPILE_TAG_CLASS(GrenadeHUDInterface, TAG_FOURCC_GRENADE_HUD_INTERFACE)
            COMPILE_TAG_CLASS(HUDMessageText, TAG_FOURCC_HUD_MESSAGE_TEXT)
            COMPILE_TAG_CLASS(HUDNumber, TAG_FOURCC_HUD_NUMBER)
            COMPILE_TAG_CLASS(HUDGlobals, TAG_FOURCC_HUD_GLOBALS)
            COMPILE_TAG_CLASS(ItemCollection, TAG_FOURCC_ITEM_COLLECTION)
            COMPILE_TAG_CLASS(DamageEffect, TAG_FOURCC_DAMAGE_EFFECT)
            COMPILE_TAG_CLASS(LensFlare, TAG_FOURCC_LENS_FLARE)
            COMPILE_TAG_CLASS(Lightning, TAG_FOURCC_LIGHTNING)
            COMPILE_TAG_CLASS(DeviceLightFixture, TAG_FOURCC_DEVICE_LIGHT_FIXTURE)
            COMPILE_TAG_CLASS(Light, TAG_FOURCC_LIGHT)
            COMPILE_TAG_CLASS(SoundLooping, TAG_FOURCC_SOUND_LOOPING)
            COMPILE_TAG_CLASS(DeviceMachine, TAG_FOURCC_DEVICE_MACHINE)
            COMPILE_TAG_CLASS(Globals, TAG_FOURCC_GLOBALS)
            COMPILE_TAG_CLASS(Meter, TAG_FOURCC_METER)
            COMPILE_TAG_CLASS(LightVolume, TAG_FOURCC_LIGHT_VOLUME)
            COMPILE_TAG_CLASS(GBXModel, TAG_FOURCC_GBXMODEL)
            COMPILE_TAG_CLASS(Model, TAG_FOURCC_MODEL)
            COMPILE_TAG_CLASS(MultiplayerScenarioDescription, TAG_FOURCC_MULTIPLAYER_SCENARIO_DESCRIPTION)
            COMPILE_TAG_CLASS(Particle, TAG_FOURCC_PARTICLE)
            COMPILE_TAG_CLASS(ParticleSystem, TAG_FOURCC_PARTICLE_SYSTEM)
            COMPILE_TAG_CLASS(Physics, TAG_FOURCC_PHYSICS)
            COMPILE_TAG_CLASS(Placeholder, TAG_FOURCC_PLACEHOLDER)
            COMPILE_TAG_CLASS(PointPhysics, TAG_FOURCC_POINT_PHYSICS)
            COMPILE_TAG_CLASS(Projectile, TAG_FOURCC_PROJECTILE)
            COMPILE_TAG_CLASS(WeatherParticleSystem, TAG_FOURCC_WEATHER_PARTICLE_SYSTEM)
            COMPILE_TAG_CLASS(Scenery, TAG_FOURCC_SCENERY)
            COMPILE_TAG_CLASS(ShaderTransparentChicagoExtended, TAG_FOURCC_SHADER_TRANSPARENT_CHICAGO_EXTENDED)
            COMPILE_TAG_CLASS(ShaderTransparentChicago, TAG_FOURCC_SHADER_TRANSPARENT_CHICAGO)
            COMPILE_TAG_CLASS(Scenario, TAG_FOURCC_SCENARIO)
            COMPILE_TAG_CLASS(ShaderEnvironment, TAG_FOURCC_SHADER_ENVIRONMENT)
            COMPILE_TAG_CLASS(ShaderTransparentGlass, TAG_FOURCC_SHADER_TRANSPARENT_GLASS)
            COMPILE_TAG_CLASS(Sky, TAG_FOURCC_SKY)
            COMPILE_TAG_CLASS(ShaderTransparentMeter, TAG_FOURCC_SHADER_TRANSPARENT_METER)
            COMPILE_TAG_CLASS(Sound, TAG_FOURCC_SOUND)
            COMPILE_TAG_CLASS(SoundEnvironment, TAG_FOURCC_SOUND_ENVIRONMENT)
            COMPILE_TAG_CLASS(ShaderModel, TAG_FOURCC_SHADER_MODEL)
            COMPILE_TAG_CLASS(ShaderTransparentGeneric, TAG_FOURCC_SHADER_TRANSPARENT_GENERIC)
            COMPILE_TAG_CLASS(TagCollection, TAG_FOURCC_UI_WIDGET_COLLECTION)
            COMPILE_TAG_CLASS(ShaderTransparentPlasma, TAG_FOURCC_SHADER_TRANSPARENT_PLASMA)
            COMPILE_TAG_CLASS(SoundScenery, TAG_FOURCC_SOUND_SCENERY)
            COMPILE_TAG_CLASS(StringList, TAG_FOURCC_STRING_LIST)
            COMPILE_TAG_CLASS(ShaderTransparentWater, TAG_FOURCC_SHADER_TRANSPARENT_WATER)
            COMPILE_TAG_CLASS(TagCollection, TAG_FOURCC_TAG_COLLECTION)
            COMPILE_TAG_CLASS(CameraTrack, TAG_FOURCC_CAMERA_TRACK)
            COMPILE_TAG_CLASS(Dialogue, TAG_FOURCC_DIALOGUE)
            COMPILE_TAG_CLASS(UnitHUDInterface, TAG_FOURCC_UNIT_HUD_INTERFACE)
            COMPILE_TAG_CLASS(UnicodeStringList, TAG_FOURCC_UNICODE_STRING_LIST)
            COMPILE_TAG_CLASS(VirtualKeyboard, TAG_FOURCC_VIRTUAL_KEYBOARD)
            COMPILE_TAG_CLASS(Vehicle, TAG_FOURCC_VEHICLE)
            COMPILE_TAG_CLASS(Weapon, TAG_FOURCC_WEAPON)
            COMPILE_TAG_CLASS(Wind, TAG_FOURCC_WIND)
            COMPILE_TAG_CLASS(WeaponHUDInterface, TAG_FOURCC_WEAPON_HUD_INTERFACE)
            
            case TagFourCC::TAG_FOURCC_OBJECT:
            case TagFourCC::TAG_FOURCC_UNIT:
            case TagFourCC::TAG_FOURCC_SHADER:
            case TagFourCC::TAG_FOURCC_ITEM:
            case TagFourCC::TAG_FOURCC_DEVICE:
                REPORT_ERROR_PRINTF(*this, ERROR_TYPE_FATAL_ERROR, std::nullopt, "%s tags are not real tags and are therefore unimplemented", tag_fourcc_to_extension(*tag_fourcc));
                throw UnimplementedTagClassException();

            // And, of course, BSP tags
            case TagFourCC::TAG_FOURCC_SCENARIO_STRUCTURE_BSP: {
                // First thing's first - parse the tag data
                auto tag_data_parsed = Parser::ScenarioStructureBSP::parse_hek_tag_file(tag_data, tag_data_size, true);
                std::size_t bsp = this->bsp_count++;
                
                auto cache_version = this->parameters->details.build_cache_file_engine;

                // Next, if we're making a native map, we need to only do this
                if(cache_version == HEK::CacheFileEngine::CACHE_FILE_NATIVE) {
                    do_compile_tag(std::move(tag_data_parsed));
                }

                // Otherwise, make the header struct
                else {
                    std::size_t bsp_header_struct_index = this->structs.size();
                    auto &new_bsp_header_struct = this->structs.emplace_back();
                    new_bsp_header_struct.bsp = bsp;
                    this->tags[tag_index].base_struct = bsp_header_struct_index;
                    Parser::ScenarioStructureBSPCompiledHeader::struct_little *bsp_data;
                    new_bsp_header_struct.data.resize(sizeof(*bsp_data), std::byte());
                    bsp_data = reinterpret_cast<decltype(bsp_data)>(new_bsp_header_struct.data.data());
                    auto &new_ptr = new_bsp_header_struct.pointers.emplace_back();
                    new_ptr.limit_to_32_bits = true;
                    new_ptr.offset = reinterpret_cast<std::byte *>(&bsp_data->pointer) - reinterpret_cast<std::byte *>(bsp_data);
                    bsp_data->signature = TagFourCC::TAG_FOURCC_SCENARIO_STRUCTURE_BSP;

                    // Make the new BSP struct thingy and make the header point to it
                    std::size_t new_bsp_struct_index = this->structs.size();
                    auto &new_bsp_struct = this->structs.emplace_back();
                    new_ptr.struct_index = new_bsp_struct_index;
                    new_bsp_struct.data.resize(sizeof(Parser::ScenarioStructureBSP::struct_little), std::byte());
                    tag_data_parsed.compile(*this, tag_index, new_ptr.struct_index, bsp);
                    
                    // Populate the vertices/indices index thingy
                    if(cache_version == HEK::CacheFileEngine::CACHE_FILE_XBOX) {
                        Parser::set_up_xbox_cache_bsp_data(*this, bsp_header_struct_index, new_bsp_struct_index, bsp);
                    }
                    
                    // If it uses CEA memery to store BSP data, then append it
                    if(cache_version == HEK::CacheFileEngine::CACHE_FILE_MCC_CEA) {
                        auto *bsp_data_cea = reinterpret_cast<Parser::ScenarioStructureBSPCompiledHeaderCEA::struct_little *>(this->structs[bsp_header_struct_index].data.data());
                        auto bsp_data_size = this->bsp_data[bsp].size();
                        bsp_data_cea->lightmap_vertices = this->bsp_offset;
                        bsp_data_cea->lightmap_vertex_size = bsp_data_size;
                        this->bsp_offset += bsp_data_size;
                    }
                }
                break;
            }

            // We don't have any way of handling these tags
            case TagFourCC::TAG_FOURCC_PREFERENCES_NETWORK_GAME:
            case TagFourCC::TAG_FOURCC_SPHEROID:
            case TagFourCC::TAG_FOURCC_CONTINUOUS_DAMAGE_EFFECT:
                REPORT_ERROR_PRINTF(*this, ERROR_TYPE_FATAL_ERROR, std::nullopt, "%s tags are unimplemented at this current time", tag_fourcc_to_extension(*tag_fourcc));
                throw UnimplementedTagClassException();
                
            // And this, of course, we don't know
            default:
                throw UnknownTagClassException();
        }
    }

    std::size_t BuildWorkload::compile_tag_recursively(const char *tag_path, TagFourCC tag_fourcc) {
        // Remove duplicate slashes
        auto fixed_path = Invader::File::remove_duplicate_slashes(tag_path);
        tag_path = fixed_path.c_str();

        // Search for the tag
        std::size_t return_value = this->tags.size();
        bool found = false;
        for(std::size_t i = 0; i < return_value; i++) {
            auto &tag = this->tags[i];
            if((tag.tag_fourcc == tag_fourcc || tag.alias == tag_fourcc) && tag.path == tag_path) {
                if(tag.base_struct.has_value()) {
                    return i;
                }
                return_value = i;
                found = true;
                tag.stubbed = false;
                break;
            }
        }
        
        auto &tags_directories = this->parameters->tags_directories;

        // Find it
        char formatted_path[512];
        std::optional<std::filesystem::path> new_path;
        if(tag_fourcc != TagFourCC::TAG_FOURCC_OBJECT) {
            std::snprintf(formatted_path, sizeof(formatted_path), "%s.%s", tag_path, tag_fourcc_to_extension(tag_fourcc));
            Invader::File::halo_path_to_preferred_path_chars(formatted_path);
            
            // Only set the new path if it exists
            if((new_path = Invader::File::tag_path_to_file_path(formatted_path, tags_directories)).has_value() && !std::filesystem::exists(*new_path)) {
                new_path = std::nullopt;
            }
        }
        else {
            #define TRY_THIS(new_int) if(!new_path.has_value()) { \
                std::snprintf(formatted_path, sizeof(formatted_path), "%s.%s", tag_path, tag_fourcc_to_extension(new_int)); \
                Invader::File::halo_path_to_preferred_path_chars(formatted_path); \
                new_path = Invader::File::tag_path_to_file_path(formatted_path, tags_directories); \
                if((new_path = Invader::File::tag_path_to_file_path(formatted_path, tags_directories)).has_value() && !std::filesystem::exists(*new_path)) { /* only set the new path if it exists */ \
                    new_path = std::nullopt; \
                } \
                tag_fourcc = new_int; \
            }
            TRY_THIS(TagFourCC::TAG_FOURCC_BIPED);
            TRY_THIS(TagFourCC::TAG_FOURCC_VEHICLE);
            TRY_THIS(TagFourCC::TAG_FOURCC_WEAPON);
            TRY_THIS(TagFourCC::TAG_FOURCC_EQUIPMENT);
            TRY_THIS(TagFourCC::TAG_FOURCC_GARBAGE);
            TRY_THIS(TagFourCC::TAG_FOURCC_SCENERY);
            TRY_THIS(TagFourCC::TAG_FOURCC_PLACEHOLDER);
            TRY_THIS(TagFourCC::TAG_FOURCC_SOUND_SCENERY);
            TRY_THIS(TagFourCC::TAG_FOURCC_DEVICE_CONTROL);
            TRY_THIS(TagFourCC::TAG_FOURCC_DEVICE_MACHINE);
            TRY_THIS(TagFourCC::TAG_FOURCC_DEVICE_LIGHT_FIXTURE);
            #undef TRY_THIS
            if(!new_path.has_value()) {
                tag_fourcc = TagFourCC::TAG_FOURCC_OBJECT;
                std::snprintf(formatted_path, sizeof(formatted_path), "%s.%s", tag_path, tag_fourcc_to_extension(tag_fourcc));
            }
            else {
                // Look for it again
                for(std::size_t i = 0; i < return_value; i++) {
                    auto &tag = this->tags[i];
                    if(tag.tag_fourcc == tag_fourcc && tag.path == tag_path) {
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
            tag.tag_fourcc = tag_fourcc;
            this->get_tag_paths().emplace_back(tag_path, tag_fourcc);
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
        auto tag_file = Invader::File::open_file(*new_path);
        if(!tag_file.has_value()) {
            eprintf_error("Failed to open %s\n", formatted_path);
            throw FailedToOpenFileException();
        }
        auto &tag_file_data = *tag_file;

        try {
            this->compile_tag_data_recursively(tag_file_data.data(), tag_file_data.size(), return_value, tag_fourcc);
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

        this->scenario_index = this->compile_tag_recursively(this->scenario, TagFourCC::TAG_FOURCC_SCENARIO);
        std::string full_scenario_path = this->tags[this->scenario_index].path;
        const char *first_char = full_scenario_path.c_str();
        const char *last_slash = first_char;
        for(const char *i = first_char; *i; i++) {
            if(*i == '\\') {
                last_slash = i + 1;
            }
        }
        this->tags[this->scenario_index].path = std::string(first_char, last_slash - first_char) + this->scenario_name.string;
        
        const auto &required_tags = this->parameters->details.build_required_tags;
        auto &workload = *this;
        
        auto import_all = [&workload](auto &what) {
            std::size_t count = what.count;
            auto *arr = what.ptr;
            for(std::size_t c = 0; c < count; c++) {
                auto &tag = arr[c];
                workload.compile_tag_recursively(tag.path, tag.fourcc);
            }
        };
        
        // Check if we have a demo UI if one was requested
        if(this->demo_ui && !this->disable_error_checking) {
            switch(*this->cache_file_type) {
                case ScenarioType::SCENARIO_TYPE_SINGLEPLAYER:
                    if(required_tags.singleplayer_demo.count == 0) {
                        REPORT_ERROR_PRINTF(*this, ERROR_TYPE_FATAL_ERROR, this->scenario_index, "No demo UI exists for the target engine for singleplayer scenarios");
                        throw InvalidTagDataException();
                    }
                    break;
                case ScenarioType::SCENARIO_TYPE_MULTIPLAYER:
                    if(required_tags.multiplayer_demo.count == 0) {
                        REPORT_ERROR_PRINTF(*this, ERROR_TYPE_FATAL_ERROR, this->scenario_index, "No demo UI exists for the target engine for multiplayer scenarios");
                        throw InvalidTagDataException();
                    }
                    break;
                case ScenarioType::SCENARIO_TYPE_USER_INTERFACE:
                    if(required_tags.user_interface_demo.count == 0) {
                        REPORT_ERROR_PRINTF(*this, ERROR_TYPE_FATAL_ERROR, this->scenario_index, "No demo UI exists for the target engine for user interface scenarios");
                        throw InvalidTagDataException();
                    }
                    break;
                case ScenarioType::SCENARIO_TYPE_ENUM_COUNT:
                    std::terminate();
            }
        }
        
        // Import all required tags
        import_all(required_tags.all);
        switch(*this->cache_file_type) {
            case ScenarioType::SCENARIO_TYPE_SINGLEPLAYER:
                import_all(required_tags.singleplayer);
                if(this->demo_ui) {
                    import_all(required_tags.singleplayer_demo);
                }
                else {
                    import_all(required_tags.singleplayer_full);
                }
                break;
            case ScenarioType::SCENARIO_TYPE_MULTIPLAYER:
                import_all(required_tags.multiplayer);
                if(this->demo_ui) {
                    import_all(required_tags.multiplayer_demo);
                }
                else {
                    import_all(required_tags.multiplayer_full);
                }
                break;
            case ScenarioType::SCENARIO_TYPE_USER_INTERFACE:
                import_all(required_tags.user_interface);
                if(this->demo_ui) {
                    import_all(required_tags.user_interface_demo);
                }
                else {
                    import_all(required_tags.user_interface_full);
                }
                break;
            case ScenarioType::SCENARIO_TYPE_ENUM_COUNT:
                std::terminate();
        };

        // Mark stubs
        std::size_t warned = 0;
        for(auto &tag : this->tags) {
            if(tag.stubbed) {
                // Object tags and damage effects are referenced directly over the netcode
                if(*this->cache_file_type == HEK::CacheFileType::SCENARIO_TYPE_MULTIPLAYER && (IS_OBJECT_TAG(tag.tag_fourcc) || tag.tag_fourcc == TagFourCC::TAG_FOURCC_DAMAGE_EFFECT)) {
                    REPORT_ERROR_PRINTF(*this, ERROR_TYPE_WARNING, &tag - this->tags.data(), "%s.%s was stubbed out due to not being referenced.", File::halo_path_to_preferred_path(tag.path).c_str(), tag_fourcc_to_extension(tag.tag_fourcc));
                    warned++;
                }

                tag.path = "MISSINGNO.";
                tag.tag_fourcc = TagFourCC::TAG_FOURCC_NONE;
                this->stubbed_tag_count++;
            }
        }

        // If we stubbed, explain why
        if(warned) {
            eprintf_warn("An exception error will occur if %s used by the server.", warned == 1 ? "this tag is" : "these tags are");
            eprintf_warn("You can fix this by referencing %s in some already referenced tag in the map.", warned == 1 ? "it" : "them");
        }
    }

    BuildWorkload BuildWorkload::compile_single_tag(const std::byte *tag_data, std::size_t tag_data_size, const std::vector<std::filesystem::path> &tags_directories, bool recursion, bool error_checking) {
        BuildWorkload workload = {};
        workload.set_reporting_level(ErrorHandler::ReportingLevel::REPORTING_LEVEL_HIDE_EVERYTHING);
        workload.disable_recursion = !recursion;
        workload.disable_error_checking = !error_checking;
        workload.cache_file_type = HEK::CacheFileType::SCENARIO_TYPE_MULTIPLAYER;
        
        BuildParameters parameters;
        parameters.tags_directories = tags_directories;
        workload.parameters = &parameters;
        
        auto &tag = workload.tags.emplace_back();
        tag.path = "unknown";
        workload.compile_tag_data_recursively(tag_data, tag_data_size, 0);
        return workload;
    }

    BuildWorkload BuildWorkload::compile_single_tag(const char *tag, TagFourCC tag_fourcc, const std::vector<std::filesystem::path> &tags_directories, bool recursion, bool error_checking) {
        BuildWorkload workload = {};
        
        BuildParameters parameters;
        parameters.tags_directories = tags_directories;
        workload.parameters = &parameters;
        
        workload.set_reporting_level(ErrorHandler::ReportingLevel::REPORTING_LEVEL_HIDE_EVERYTHING);
        workload.disable_recursion = !recursion;
        workload.disable_error_checking = !error_checking;
        workload.cache_file_type = HEK::CacheFileType::SCENARIO_TYPE_MULTIPLAYER;
        workload.compile_tag_recursively(tag, tag_fourcc);
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
            auto primary_class = tag.tag_fourcc;
            if(!native) {
                reinterpret_cast<HEK::CacheFileTagDataTag *>(&tag_index)->indexed = tag.resource_index.has_value();
            }

            if(tag.stubbed) {
                tag_index.tag_data = stub_address;
            }
            else if(tag.resource_index.has_value() && !tag.base_struct.has_value()) {
                tag_index.tag_data = *tag.resource_index;
            }
            else if(primary_class != TagFourCC::TAG_FOURCC_SCENARIO_STRUCTURE_BSP || native) {
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
            tag_index.primary_class = tag.tag_fourcc;
            tag_index.secondary_class = TagFourCC::TAG_FOURCC_NONE;
            tag_index.tertiary_class = TagFourCC::TAG_FOURCC_NONE;
            switch(tag.tag_fourcc) {
                case TagFourCC::TAG_FOURCC_BIPED:
                case TagFourCC::TAG_FOURCC_VEHICLE:
                    tag_index.secondary_class = TagFourCC::TAG_FOURCC_UNIT;
                    tag_index.tertiary_class = TagFourCC::TAG_FOURCC_OBJECT;
                    break;
                case TagFourCC::TAG_FOURCC_WEAPON:
                case TagFourCC::TAG_FOURCC_GARBAGE:
                case TagFourCC::TAG_FOURCC_EQUIPMENT:
                    tag_index.secondary_class = TagFourCC::TAG_FOURCC_ITEM;
                    tag_index.tertiary_class = TagFourCC::TAG_FOURCC_OBJECT;
                    break;
                case TagFourCC::TAG_FOURCC_DEVICE_CONTROL:
                case TagFourCC::TAG_FOURCC_DEVICE_LIGHT_FIXTURE:
                case TagFourCC::TAG_FOURCC_DEVICE_MACHINE:
                    tag_index.secondary_class = TagFourCC::TAG_FOURCC_DEVICE;
                    tag_index.tertiary_class = TagFourCC::TAG_FOURCC_OBJECT;
                    break;
                case TagFourCC::TAG_FOURCC_SCENERY:
                case TagFourCC::TAG_FOURCC_SOUND_SCENERY:
                case TagFourCC::TAG_FOURCC_PLACEHOLDER:
                case TagFourCC::TAG_FOURCC_PROJECTILE:
                    tag_index.secondary_class = TagFourCC::TAG_FOURCC_OBJECT;
                    break;
                case TagFourCC::TAG_FOURCC_SHADER_ENVIRONMENT:
                case TagFourCC::TAG_FOURCC_SHADER_MODEL:
                case TagFourCC::TAG_FOURCC_SHADER_TRANSPARENT_GENERIC:
                case TagFourCC::TAG_FOURCC_SHADER_TRANSPARENT_CHICAGO:
                case TagFourCC::TAG_FOURCC_SHADER_TRANSPARENT_CHICAGO_EXTENDED:
                case TagFourCC::TAG_FOURCC_SHADER_TRANSPARENT_WATER:
                case TagFourCC::TAG_FOURCC_SHADER_TRANSPARENT_PLASMA:
                case TagFourCC::TAG_FOURCC_SHADER_TRANSPARENT_METER:
                case TagFourCC::TAG_FOURCC_SHADER_TRANSPARENT_GLASS:
                    tag_index.secondary_class = TagFourCC::TAG_FOURCC_SHADER;
                    break;
                default:
                    break;
            }
        }
    }

    void BuildWorkload::generate_tag_array() {
        if(this->parameters->details.build_cache_file_engine == HEK::CacheFileEngine::CACHE_FILE_NATIVE) {
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
        struct PointerInternal {
            std::size_t from_offset;
            std::size_t to_struct;
            std::size_t to_offset;
        };
        
        std::vector<PointerInternal> pointers;
        std::vector<PointerInternal> pointers_64_bit;
        auto name_tag_data_pointer = this->parameters->details.build_tag_data_address;
        auto &tag_array_struct = TAG_ARRAY_STRUCT;

        auto pointer_of_tag_path = [&tags, &name_tag_data_pointer, &tag_array_struct](std::size_t tag_index) -> HEK::Pointer64 {
            return static_cast<HEK::Pointer64>(name_tag_data_pointer + *tag_array_struct.offset + tags[tag_index].path_offset);
        };

        auto &cache_version = this->parameters->details.build_cache_file_engine;

        auto recursively_generate_data = [&structs, &tags, &pointers, &pointers_64_bit, &pointer_of_tag_path, &cache_version](std::vector<std::byte> &data, std::size_t struct_index, auto &recursively_generate_data) {
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
                PointerInternal pointer_internal { pointer.offset + offset, pointer.struct_index, pointer.struct_data_offset };
                if(cache_version != HEK::CacheFileEngine::CACHE_FILE_NATIVE || pointer.limit_to_32_bits) {
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
                    dependency_struct.tag_fourcc = tags[tag_index].tag_fourcc;
                    dependency_struct.tag_id = new_tag_id;
                    if(cache_version != HEK::CacheFileEngine::CACHE_FILE_NATIVE) {
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
            *reinterpret_cast<HEK::LittleEndian<HEK::Pointer> *>(tag_data_b + p.from_offset) = static_cast<HEK::Pointer>(name_tag_data_pointer + *this->structs[p.to_struct].offset + p.to_offset);
        }

        for(auto &p : pointers_64_bit) {
            *reinterpret_cast<HEK::LittleEndian<HEK::Pointer64> *>(tag_data_b + p.from_offset) = static_cast<HEK::Pointer64>(name_tag_data_pointer + *this->structs[p.to_struct].offset + p.to_offset);
        }

        // Get the tag path pointers working
        auto *tag_array = reinterpret_cast<HEK::CacheFileTagDataTag *>(tag_data_struct.data() + *TAG_ARRAY_STRUCT.offset);
        for(std::size_t t = 0; t < tag_count; t++) {
            tag_array[t].tag_path = pointer_of_tag_path(t);
        }

        // Get this set
        std::size_t bsp_end = this->bsp_offset;

        // Get the scenario tag (if we're not on a native map)
        if(cache_version != HEK::CacheFileEngine::CACHE_FILE_NATIVE) {
            auto &scenario_tag = tags[this->scenario_index];
            auto &scenario_tag_data = *reinterpret_cast<const Parser::Scenario::struct_little *>(structs[*scenario_tag.base_struct].data.data());
            std::size_t bsp_count = scenario_tag_data.structure_bsps.count.read();
            std::size_t max_bsp_size = this->parameters->details.build_maximum_tag_space;

            if(bsp_count != this->bsp_count) {
                oprintf("\n");
                REPORT_ERROR_PRINTF(*this, ERROR_TYPE_FATAL_ERROR, this->scenario_index, "BSP count in scenario tag is wrong (%zu expected, %zu gotten)", bsp_count, this->bsp_count);
                throw InvalidTagDataException();
            }
            else if(bsp_count) {
                auto scenario_bsps_struct_index = *structs[*scenario_tag.base_struct].resolve_pointer(reinterpret_cast<const std::byte *>(&scenario_tag_data.structure_bsps.pointer) - reinterpret_cast<const std::byte *>(&scenario_tag_data));
                auto *scenario_bsps_struct_data = reinterpret_cast<Parser::ScenarioBSP::struct_little *>(map_data_structs[0].data() + *structs[scenario_bsps_struct_index].offset);

                // Go through each BSP tag
                for(std::size_t i = 0; i < tag_count; i++) {
                    auto &t = tags[i];
                    if(t.tag_fourcc != TagFourCC::TAG_FOURCC_SCENARIO_STRUCTURE_BSP) {
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
                    
                    // Resize the BSP to 2048 bytes alignment if on Xbox
                    if(cache_version == HEK::CacheFileEngine::CACHE_FILE_XBOX) {
                        bsp_size = bsp_size + REQUIRED_PADDING_N_BYTES(bsp_size, HEK::CacheFileXboxConstants::CACHE_FILE_XBOX_SECTOR_SIZE * 4);
                        bsp_data_struct.resize(bsp_size);
                    }

                    if(bsp_size > max_bsp_size) {
                        oprintf("\n");
                        REPORT_ERROR_PRINTF(*this, ERROR_TYPE_FATAL_ERROR, i, "BSP size exceeds the maximum size for this engine (%zu > %zu)", bsp_size, max_bsp_size);
                        throw InvalidTagDataException();
                    }

                    HEK::Pointer64 tag_data_base;
                    if(cache_version != HEK::CacheFileEngine::CACHE_FILE_NATIVE) {
                        tag_data_base = this->parameters->details.build_tag_data_address + this->parameters->details.build_maximum_tag_space - bsp_size;
                        if(static_cast<std::uint32_t>(tag_data_base + bsp_size) != static_cast<std::uint64_t>(tag_data_base + bsp_size)) {
                            oprintf("\n");
                            REPORT_ERROR_PRINTF(*this, ERROR_TYPE_FATAL_ERROR, i, "Tag space overflows BSP past 0x00000000");
                            throw InvalidTagDataException();
                        }
                    }
                    else {
                        tag_data_base = 0;
                    }
                    tag_data_b = bsp_data_struct.data();

                    // Chu
                    for(auto &p : pointers) {
                        auto &struct_pointed_to = this->structs[p.to_struct];
                        auto base = struct_pointed_to.bsp.has_value() ? tag_data_base : name_tag_data_pointer;
                        *reinterpret_cast<HEK::LittleEndian<HEK::Pointer> *>(tag_data_b + p.from_offset) = static_cast<HEK::Pointer>(base + *struct_pointed_to.offset + p.to_offset);
                    }
                    for(auto &p : pointers_64_bit) {
                        auto &struct_pointed_to = this->structs[p.to_struct];
                        auto base = struct_pointed_to.bsp.has_value() ? tag_data_base : name_tag_data_pointer;
                        *reinterpret_cast<HEK::LittleEndian<HEK::Pointer64> *>(tag_data_b + p.from_offset) = static_cast<HEK::Pointer64>(base + *struct_pointed_to.offset + p.to_offset);
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
                        oprintf("\n");
                        REPORT_ERROR_PRINTF(*this, ERROR_TYPE_ERROR, this->scenario_index, "Scenario structure BSP array is missing %s.%s", File::halo_path_to_preferred_path(t.path).c_str(), HEK::tag_fourcc_to_extension(t.tag_fourcc));
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
        auto cache_version = this->parameters->details.build_cache_file_engine;

        // Offset followed by size
        std::vector<std::pair<std::size_t, std::size_t>> all_assets;

        auto add_or_dedupe_asset = [&all_assets, &all_raw_data, &cache_version](const std::vector<std::byte> &raw_data, std::size_t &counter) -> std::uint32_t {
            std::size_t raw_data_size = raw_data.size();
            for(auto &a : all_assets) {
                if(a.second == raw_data_size && std::memcmp(raw_data.data(), all_raw_data.data() + a.first, raw_data_size) == 0) {
                    return static_cast<std::uint32_t>(&a - all_assets.data());
                }
            }

            // Pad to 512 bytes if Xbox
            auto all_raw_data_offset = all_raw_data.size();
            if(cache_version == HEK::CacheFileEngine::CACHE_FILE_XBOX) {
                all_raw_data_offset += REQUIRED_PADDING_N_BYTES(all_raw_data_offset, HEK::CacheFileXboxConstants::CACHE_FILE_XBOX_SECTOR_SIZE);
                all_raw_data.resize(all_raw_data_offset);
            }
            
            // Add the new asset
            auto &new_asset = all_assets.emplace_back();
            new_asset.first = all_raw_data_offset;
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
            if(t.tag_fourcc == TagFourCC::TAG_FOURCC_BITMAP) {
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
                    if(cache_version == HEK::CacheFileEngine::CACHE_FILE_NATIVE) {
                        bitmap_data.pixel_data_offset = resource_index;
                    }
                    else {
                        bitmap_data.pixel_data_offset = all_assets[resource_index].first + file_offset;
                    }
                    
                    // Set this size to be correct
                    if(cache_version == HEK::CacheFileEngine::CACHE_FILE_XBOX) {
                        bitmap_data.pixel_data_size = bitmap_data.pixel_data_size + REQUIRED_PADDING_N_BYTES(bitmap_data.pixel_data_size, HEK::CacheFileXboxConstants::CACHE_FILE_XBOX_BITMAP_SIZE_GRANULARITY);
                    }
                }
            }
            else if(t.tag_fourcc == TagFourCC::TAG_FOURCC_SOUND) {
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
                        if(cache_version == HEK::CacheFileEngine::CACHE_FILE_NATIVE) {
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
        if(this->parameters->details.build_cache_file_engine == HEK::CacheFileEngine::CACHE_FILE_NATIVE) {
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
        bool always_index_tags = this->parameters->details.build_raw_data_handling == BuildParameters::BuildParametersDetails::RawDataHandling::RAW_DATA_HANDLING_ALWAYS_INDEX;
        
        auto &bitmaps = this->parameters->bitmap_data;
        auto &sounds = this->parameters->sound_data;
        auto &loc = this->parameters->loc_data;
        
        bool check_ce_bounds = this->parameters->details.build_check_custom_edition_resource_map_bounds;

        switch(this->parameters->details.build_cache_file_engine) {
            case HEK::CacheFileEngine::CACHE_FILE_CUSTOM_EDITION:
                for(auto &t : this->tags) {
                    // Find the tag
                    auto find_tag_index = [](const std::string &path, const std::optional<std::vector<Resource>> &resources, bool every_other) -> std::optional<std::size_t> {
                        if(!resources.has_value()) {
                            return std::nullopt;
                        }
                        
                        std::size_t count = resources->size();
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
                            if((*resources)[i].path == path) {
                                return i;
                            }
                        }
                        return std::nullopt;
                    };

                    switch(t.tag_fourcc) {
                        case TagFourCC::TAG_FOURCC_BITMAP: {
                            auto index = find_tag_index(t.path, bitmaps, true);
                            if(index.has_value()) {
                                if((*index % 2) == 0) {
                                    REPORT_ERROR_PRINTF(*this, ERROR_TYPE_ERROR, std::nullopt, "%s in bitmaps.map appears to be corrupt (tag is on an even index)", File::halo_path_to_preferred_path(t.path).c_str());
                                    break;
                                }
                                
                                if(check_ce_bounds && (*index / 2 > get_default_bitmap_resources_count() || File::split_tag_class_extension_chars(get_default_bitmap_resources()[*index / 2])->path != t.path)) {
                                    break;
                                }

                                bool match = true;
                                if(!always_index_tags) {
                                    const auto &bitmap_tag_struct = this->structs[*t.base_struct];
                                    const auto &bitmap_tag = *reinterpret_cast<const Parser::Bitmap::struct_little *>(bitmap_tag_struct.data.data());
                                    const auto &bitmap_tag_struct_other = (*bitmaps)[*index];
                                    const auto *bitmap_tag_struct_other_data = bitmap_tag_struct_other.data.data();
                                    std::size_t bitmap_tag_struct_other_size = bitmap_tag_struct_other.data.size();

                                    if(bitmap_tag_struct_other_size < sizeof(bitmap_tag)) {
                                        REPORT_ERROR_PRINTF(*this, ERROR_TYPE_ERROR, std::nullopt, "%s in bitmaps.map appears to be corrupt (bitmap main struct goes out of bounds)", File::halo_path_to_preferred_path(t.path).c_str());
                                        match = false;
                                        break;
                                    }

                                    const auto &bitmap_tag_struct_other_raw = (*bitmaps)[*index - 1];
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
                                    this->indexed_data_amount += (*bitmaps)[*index].data.size();
                                    t.base_struct = std::nullopt;
                                    break;
                                }
                                else {
                                    REPORT_ERROR_PRINTF(*this, ERROR_TYPE_WARNING_PEDANTIC, &t - this->tags.data(), "%s.%s does not match the one found in bitmaps.map, so it will NOT be indexed out", File::halo_path_to_preferred_path(t.path).c_str(), HEK::tag_fourcc_to_extension(t.tag_fourcc));
                                }
                            }
                            break;
                        }
                        case TagFourCC::TAG_FOURCC_SOUND: {
                            auto index = find_tag_index(t.path, sounds, true);
                            if(index.has_value()) {
                                if((*index % 2) == 0) {
                                    REPORT_ERROR_PRINTF(*this, ERROR_TYPE_ERROR, std::nullopt, "%s in sounds.map appears to be corrupt (tag is on an even index)", File::halo_path_to_preferred_path(t.path).c_str());
                                    break;
                                }
                                
                                if(check_ce_bounds && (*index / 2 > get_default_sound_resources_count() || File::split_tag_class_extension_chars(get_default_sound_resources()[*index / 2])->path != t.path)) {
                                    break;
                                }

                                bool match = true;
                                if(!always_index_tags) {
                                    const auto &sound_tag_struct = this->structs[*t.base_struct];
                                    const auto &sound_tag = *reinterpret_cast<const Parser::Sound::struct_little *>(sound_tag_struct.data.data());
                                    const auto &sound_tag_struct_other = (*sounds)[*index];
                                    const auto *sound_tag_struct_other_data = sound_tag_struct_other.data.data();
                                    std::size_t sound_tag_struct_other_size = sound_tag_struct_other.data.size();

                                    if(sound_tag_struct_other_size < sizeof(sound_tag)) {
                                        REPORT_ERROR_PRINTF(*this, ERROR_TYPE_ERROR, std::nullopt, "%s in sounds.map appears to be corrupt (sound main struct goes out of bounds)", File::halo_path_to_preferred_path(t.path).c_str());
                                        match = false;
                                        break;
                                    }

                                    const auto &sound_tag_struct_other_raw = (*sounds)[*index - 1];
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
                                    this->indexed_data_amount += (*sounds)[*index].data.size() - sizeof(Sound<LittleEndian>);
                                    
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
                                    REPORT_ERROR_PRINTF(*this, ERROR_TYPE_WARNING_PEDANTIC, &t - this->tags.data(), "%s.%s does not match the one found in sounds.map, so it will NOT be indexed out", File::halo_path_to_preferred_path(t.path).c_str(), HEK::tag_fourcc_to_extension(t.tag_fourcc));
                                }
                            }
                            break;
                        }
                        case TagFourCC::TAG_FOURCC_FONT:
                        case TagFourCC::TAG_FOURCC_UNICODE_STRING_LIST:
                        case TagFourCC::TAG_FOURCC_HUD_MESSAGE_TEXT: {
                            auto index = find_tag_index(t.path, loc, false);
                            if(index.has_value()) {
                                bool match = true;
                                
                                if(check_ce_bounds && (*index > get_default_loc_resources_count() || (t.path + "." + HEK::tag_fourcc_to_extension(t.tag_fourcc)) != get_default_loc_resources()[*index])) {
                                    break;
                                }

                                const auto &loc_tag_struct_other = (*loc)[*index];
                                const auto *loc_tag_struct_other_data = loc_tag_struct_other.data.data();
                                std::size_t loc_tag_struct_other_size = loc_tag_struct_other.data.size();

                                const auto &loc_tag_struct = this->structs[*t.base_struct];

                                switch(t.tag_fourcc) {
                                    case TagFourCC::TAG_FOURCC_FONT: {
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
                                                font_tag.bold.tag_id.read().is_null() &&
                                                font_tag.italic.tag_id.read().is_null() &&
                                                font_tag.underline.tag_id.read().is_null() &&
                                                font_tag.condense.tag_id.read().is_null() &&
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
                                    case TagFourCC::TAG_FOURCC_UNICODE_STRING_LIST: {
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
                                    case TagFourCC::TAG_FOURCC_HUD_MESSAGE_TEXT: {
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
                                    this->indexed_data_amount += (*loc)[*index].data.size();
                                    t.base_struct = std::nullopt;
                                    break;
                                }
                                else {
                                    REPORT_ERROR_PRINTF(*this, ERROR_TYPE_WARNING_PEDANTIC, &t - this->tags.data(), "%s.%s does not match the one found in loc.map, so it will NOT be indexed out", File::halo_path_to_preferred_path(t.path).c_str(), HEK::tag_fourcc_to_extension(t.tag_fourcc));
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
                    switch(t.tag_fourcc) {
                        // Iterate through each permutation in each pitch range to find the bitmap
                        case TagFourCC::TAG_FOURCC_BITMAP: {
                            if(bitmaps.has_value()) {
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
                                        for(auto &ab : *bitmaps) {
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
                            }
                            break;
                        }

                        // Iterate through each permutation in each pitch range to find the sound
                        case TagFourCC::TAG_FOURCC_SOUND: {
                            if(sounds.has_value()) {
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
                                                for(auto &ab : *sounds) {
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
    
    void BuildWorkload::check_hud_text_indices() {
        // This should effectively just get the tag
        auto &globals_tag_struct = this->structs[this->tags[this->compile_tag_recursively("globals\\globals", HEK::TagFourCC::TAG_FOURCC_GLOBALS)].base_struct.value()];
        auto &globals_tag_data = *reinterpret_cast<Parser::Globals::struct_little *>(globals_tag_struct.data.data());
        if(globals_tag_data.interface_bitmaps.count != 1) {
            return;
        }
        auto &interface_bitmaps_data = *reinterpret_cast<Parser::GlobalsInterfaceBitmaps::struct_little *>(this->structs[globals_tag_struct.resolve_pointer(&globals_tag_data.interface_bitmaps.pointer).value()].data.data());
        auto hud_globals_id = interface_bitmaps_data.hud_globals.tag_id.read();
        if(hud_globals_id.is_null()) {
            return;
        }
        auto &hud_globals_data = *reinterpret_cast<Parser::HUDGlobals::struct_little *>(this->structs[this->tags[hud_globals_id.index].base_struct.value()].data.data());
        auto item_strings_id = hud_globals_data.item_message_text.tag_id.read();
        auto icon_strings_id = hud_globals_data.alternate_icon_text.tag_id.read();
        
        // Get item/icon counts
        std::size_t item_strings;
        std::size_t icon_strings;
        
        if(!item_strings_id.is_null()) {
            item_strings = reinterpret_cast<Parser::UnicodeStringList::struct_little *>(this->structs[this->tags[item_strings_id.index].base_struct.value()].data.data())->strings.count;
        }
        else {
            item_strings = 0;
        }
        
        if(!icon_strings_id.is_null()) {
            icon_strings = reinterpret_cast<Parser::UnicodeStringList::struct_little *>(this->structs[this->tags[icon_strings_id.index].base_struct.value()].data.data())->strings.count;
        }
        else {
            icon_strings = 0;
        }
        
        std::size_t tag_count = this->tags.size();
        std::size_t error_count = 0;
        
        for(std::size_t i = 0; i < tag_count; i++) {
            auto &tag = this->tags[i];
            
            // Skip tags we don't have
            if(!tag.base_struct.has_value()) {
                continue;
            }
            
            switch(tag.tag_fourcc) {
                case HEK::TagFourCC::TAG_FOURCC_WEAPON:
                case HEK::TagFourCC::TAG_FOURCC_EQUIPMENT: {
                    auto index = static_cast<std::size_t>(reinterpret_cast<Parser::Item::struct_little *>(this->structs[*tag.base_struct].data.data())->pickup_text_index);
                    if(index >= item_strings && index != NULL_INDEX) {
                        REPORT_ERROR_PRINTF(*this, ERROR_TYPE_ERROR, i, "Pickup text index is not valid (%zu >= %zu)", index, item_strings);
                        error_count++;
                    }
                    break;
                }
                case HEK::TagFourCC::TAG_FOURCC_BIPED:
                case HEK::TagFourCC::TAG_FOURCC_VEHICLE: {
                    auto &unit_struct = this->structs[*tag.base_struct];
                    auto &unit_data = *reinterpret_cast<Parser::Unit::struct_little *>(unit_struct.data.data());
                    auto index = static_cast<std::size_t>(unit_data.hud_text_message_index);
                    if(index >= icon_strings && index != NULL_INDEX) {
                        REPORT_ERROR_PRINTF(*this, ERROR_TYPE_ERROR, i, "Message text index is not valid (%zu >= %zu)", index, icon_strings);
                        error_count++;
                    }
                    std::size_t seat_count = unit_data.seats.count;
                    if(seat_count > 0) {
                        auto *seat_data = reinterpret_cast<Parser::UnitSeat::struct_little *>(this->structs[unit_struct.resolve_pointer(&unit_data.seats.pointer).value()].data.data());
                        for(std::size_t s = 0; s < seat_count; s++) {
                            auto seat_index = static_cast<std::size_t>(seat_data[s].hud_text_message_index);
                            if(seat_index >= icon_strings && seat_index != NULL_INDEX) {
                                REPORT_ERROR_PRINTF(*this, ERROR_TYPE_ERROR, i, "Seat #%zu's message text index is not valid (%zu >= %zu)", s, seat_index, icon_strings);
                                error_count++;
                            }
                        }
                    }
                    break;
                }
                default:
                    break;
            }
        }
        
        if(error_count) {
            REPORT_ERROR_PRINTF(*this, ERROR_TYPE_FATAL_ERROR, std::nullopt, "%zu error%s found when checking message indices. %s should be set to NULL if %s to be unset.", error_count, error_count == 1 ? " was" : "s were", error_count == 1 ? "This index" : "These indices", error_count == 1 ? "it needs" : "they need");
            throw InvalidTagDataException();
        }
    }
    
    void BuildWorkload::generate_compressed_model_tag_array() {
        auto part_count = this->model_parts.size();
        auto struct_count = this->structs.size();
        
        // Add four structs; two for vertices and two for indices
        this->structs.resize(struct_count + 4);
        
        auto indices_array_struct_index = struct_count;
        auto indices_data_struct_index = struct_count + 1;
        auto vertices_array_struct_index = struct_count + 2;
        auto vertices_data_struct_index = struct_count + 3;
        
        auto &indices_array_struct = this->structs[indices_array_struct_index];
        auto &vertices_array_struct = this->structs[vertices_array_struct_index];
        auto &indices_data_struct = this->structs[indices_data_struct_index];
        auto &vertices_data_struct = this->structs[vertices_data_struct_index];
        
        // Add an entry for each part
        auto *indices_array_data = reinterpret_cast<HEK::CacheFileModelPartIndicesXbox *>((indices_array_struct.data = std::vector<std::byte>(part_count * sizeof(HEK::CacheFileModelPartIndicesXbox))).data());
        auto *vertices_array_data = reinterpret_cast<HEK::CacheFileModelPartVerticesXbox *>((vertices_array_struct.data = std::vector<std::byte>(part_count * sizeof(HEK::CacheFileModelPartVerticesXbox))).data());
        
        // Fill it up with the vertices/indices
        auto *indices_data = this->model_indices.data();
        indices_data_struct.data.insert(indices_data_struct.data.end(), reinterpret_cast<const std::byte *>(indices_data), reinterpret_cast<const std::byte *>(indices_data + this->model_indices.size()));
        auto *vertices_data = this->compressed_model_vertices.data();
        vertices_data_struct.data.insert(vertices_data_struct.data.end(), reinterpret_cast<const std::byte *>(vertices_data), reinterpret_cast<const std::byte *>(vertices_data + this->compressed_model_vertices.size()));
        
        auto *header = reinterpret_cast<HEK::CacheFileTagDataHeaderXbox *>(TAG_DATA_HEADER_STRUCT.data.data());
        
        auto &ptr_to_vertices = TAG_DATA_HEADER_STRUCT.pointers.emplace_back();
        ptr_to_vertices.offset = reinterpret_cast<std::byte *>(&header->model_part_vertices_address) - reinterpret_cast<std::byte *>(header);
        ptr_to_vertices.limit_to_32_bits = true;
        ptr_to_vertices.struct_index = vertices_array_struct_index;
        
        auto &ptr_to_indices = TAG_DATA_HEADER_STRUCT.pointers.emplace_back();
        ptr_to_indices.offset = reinterpret_cast<std::byte *>(&header->model_part_indices_address) - reinterpret_cast<std::byte *>(header);
        ptr_to_indices.limit_to_32_bits = true;
        ptr_to_indices.struct_index = indices_array_struct_index;
        
        // Set up pointers
        for(std::size_t p = 0; p < part_count; p++) {
            auto &indices = indices_array_data[p];
            auto &vertices = vertices_array_data[p];
            auto &part = this->model_parts[p];
            auto &part_struct = this->structs[part.struct_index];
            auto *part_struct_bytes = part_struct.data.data();
            auto &part_data = *reinterpret_cast<Parser::ModelGeometryPart::struct_little *>(part_struct_bytes + part.offset);
            
            // Add three pointers - one for vertices; two for indices
            auto &vertex_ptr = part_struct.pointers.emplace_back();
            vertex_ptr.offset = reinterpret_cast<const std::byte *>(&part_data.vertex_offset) - part_struct_bytes;
            vertex_ptr.struct_index = vertices_array_struct_index;
            vertex_ptr.struct_data_offset = p * sizeof(vertices);
            vertex_ptr.limit_to_32_bits = true;
            
            auto &index_ptr = part_struct.pointers.emplace_back();
            index_ptr.offset = reinterpret_cast<const std::byte *>(&part_data.triangle_offset) - part_struct_bytes;
            index_ptr.struct_index = indices_data_struct_index;
            index_ptr.struct_data_offset = part_data.triangle_offset;
            index_ptr.limit_to_32_bits = true;
            
            auto &index_ptr2 = part_struct.pointers.emplace_back();
            index_ptr2.offset = reinterpret_cast<const std::byte *>(&part_data.triangle_offset_2) - part_struct_bytes;
            index_ptr2.struct_index = indices_array_struct_index;
            index_ptr2.struct_data_offset = p * sizeof(indices);
            index_ptr2.limit_to_32_bits = true;
            
            // Add two more pointers - one for vertices and one for indices
            auto &part_vertex_ptr = vertices_array_struct.pointers.emplace_back();
            part_vertex_ptr.offset = reinterpret_cast<const std::byte *>(&vertices.vertices) - reinterpret_cast<const std::byte *>(vertices_array_data);
            part_vertex_ptr.struct_index = vertices_data_struct_index;
            part_vertex_ptr.struct_data_offset = part_data.vertex_offset;
            part_vertex_ptr.limit_to_32_bits = true;
            
            auto &part_index_ptr = indices_array_struct.pointers.emplace_back();
            part_index_ptr.offset = reinterpret_cast<const std::byte *>(&indices.indices) - reinterpret_cast<const std::byte *>(indices_array_data);
            part_index_ptr.struct_index = indices_data_struct_index;
            part_index_ptr.struct_data_offset = part_data.triangle_offset;
            part_index_ptr.limit_to_32_bits = true;
        }
    }
}
