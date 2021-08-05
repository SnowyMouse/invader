// SPDX-License-Identifier: GPL-3.0-only

#include <vector>
#include "../crc32.h"
#include "../crc_spoof.h"
#include <invader/tag/parser/definition/scenario.hpp>
#include <invader/tag/parser/definition/scenario_structure_bsp.hpp>
#include <invader/crc/hek/crc.hpp>
#include <invader/map/map.hpp>

using namespace Invader::Parser;

namespace Invader {
    std::uint32_t calculate_map_crc(const Invader::Map &map, const std::uint32_t *new_crc, std::uint32_t *new_random, bool *check_dirty) {
        // Reassign variables if needed
        auto *data = map.get_data();
        auto size = map.get_data_length();
        
        std::vector<std::byte> data_crc;
        std::uint32_t crc = 0;

        if(new_crc && !new_random) {
            std::terminate();
        }

        if(new_crc) {
            data_crc.reserve(size);
        }
        
        auto engine = map.get_engine();
        if(engine == CacheFileEngine::CACHE_FILE_XBOX) {
            return 0;
        }

        #define CRC_DATA(data_start, data_end) \
            if(new_crc) { \
                data_crc.insert(data_crc.end(), data + data_start, data + data_end); \
            } \
            else { \
                crc = crc32(crc, data + data_start, data_end - data_start); \
            }

        auto &scenario_tag = map.get_tag(map.get_scenario_tag_id());
        auto &scenario = scenario_tag.get_base_struct<Parser::Scenario::C>();

        if(engine != CacheFileEngine::CACHE_FILE_NATIVE) {
            std::size_t bsp_count = scenario.structure_bsps.count.read();
            auto *bsps = scenario_tag.resolve_reflexive(scenario.structure_bsps);

            // Iterate through all BSPs
            for(std::size_t b = 0; b < bsp_count; b++) {
                std::size_t start = bsps[b].bsp_start.read();
                std::size_t end = start + bsps[b].bsp_size.read();

                if(start >= size || end > size) {
                    throw OutOfBoundsException();
                }
                
                // If it's MCC, CRC32 the vertex data
                if(engine == CacheFileEngine::CACHE_FILE_MCC_CEA) {
                    const auto *header = reinterpret_cast<const Parser::ScenarioStructureBSPCompiledHeaderCEA::C<LittleEndian> *>(map.get_data() + start);
                    if(start >= size || start + sizeof(header) > size) {
                        throw OutOfBoundsException();
                    }
                    
                    if(header->lightmap_vertex_size.read() > 0) {
                        CRC_DATA(header->lightmap_vertices, header->lightmap_vertices + header->lightmap_vertex_size);
                    }
                }
                
                // Add it
                CRC_DATA(start, end);
            }
        }

        // Now copy model data
        std::size_t model_start = map.get_model_data_offset();
        std::size_t model_end = model_start + map.get_model_data_size();
        if(model_start >= size || model_end > size) {
            throw OutOfBoundsException();
        }
        CRC_DATA(model_start, model_end);

        // Lastly, do tag data
        auto *tag_data = map.get_tag_data_at_offset(0);
        std::size_t tag_data_start = map.get_tag_data_at_offset(0) - map.get_data_at_offset(0);
        std::size_t tag_data_end = tag_data_start + map.get_tag_data_length();
        if(tag_data_start >= size || tag_data_end > size) {
            throw OutOfBoundsException();
        }

        // Find out where we're going to be doing CRC32 stuff
        auto *tag_file_checksums = &reinterpret_cast<const CacheFileTagDataHeader *>(map.get_tag_data_at_offset(0, sizeof(CacheFileTagDataHeader)))->tag_file_checksums;
        const std::byte *tag_file_checksums_ptr = reinterpret_cast<const std::byte *>(tag_file_checksums);
        std::size_t tag_file_checksums_offset_in_memory = tag_file_checksums_ptr - tag_data + data_crc.size();
        CRC_DATA(tag_data_start, tag_data_end);

        // Overwrite with new CRC32
        if(new_crc) {
            FakeFileHandle handle = { reinterpret_cast<std::uint8_t *>(data_crc.data()), data_crc.size(), 0 };
            std::uint32_t newcrc = ~crc_spoof_reverse_bits(*new_crc);
            crc_spoof_modify_file_crc32(&handle, tag_file_checksums_offset_in_memory, newcrc, false);
            *new_random = *reinterpret_cast<std::uint32_t *>(data_crc.data() + tag_file_checksums_offset_in_memory);

            // We have no way of knowing if the map was dirty or not because we just forged the CRC
            if(check_dirty) {
                *check_dirty = false;
            }

            return ~crc32(0, data_crc.data(), data_crc.size());
        }
        else {
            std::uint32_t crc_value = ~crc;
            if(check_dirty) {
                *check_dirty = crc_value != map.get_header_crc32();
            }
            return crc_value;
        }
    }
    
    std::uint32_t calculate_map_crc(const std::byte *data, std::size_t size, const std::uint32_t *new_crc, std::uint32_t *new_random, bool *check_dirty) {
        return calculate_map_crc(Map::map_with_copy(data, size), new_crc, new_random, check_dirty);
    }
}
