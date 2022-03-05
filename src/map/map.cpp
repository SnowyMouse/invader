// SPDX-License-Identifier: GPL-3.0-only

#include "../util/assert.hpp"

#include <invader/hek/map.hpp>
#include <invader/tag/hek/definition.hpp>
#include <invader/resource/hek/resource_map.hpp>
#include <invader/compress/compression.hpp>
#include <invader/resource/resource_map.hpp>
#include <invader/map/map.hpp>
#include <invader/file/file.hpp>
#include <invader/crc/hek/crc.hpp>

namespace Invader {
    Map Map::map_with_copy(const std::byte *data, std::size_t data_size,
                           const std::byte *bitmaps_data, std::size_t bitmaps_data_size,
                           const std::byte *loc_data, std::size_t loc_data_size,
                           const std::byte *sounds_data, std::size_t sounds_data_size) {
        return Map::map_with_move(std::vector<std::byte>(data, data + data_size), 
                                  std::vector<std::byte>(bitmaps_data, bitmaps_data + bitmaps_data_size),
                                  std::vector<std::byte>(loc_data, loc_data + loc_data_size),
                                  std::vector<std::byte>(sounds_data, sounds_data + sounds_data_size));
    }

    Map Map::map_with_move(std::vector<std::byte> &&data,
                           std::vector<std::byte> &&bitmaps_data,
                           std::vector<std::byte> &&loc_data,
                           std::vector<std::byte> &&sounds_data) {
        if(data.size() < sizeof(HEK::CacheFileHeader)) {
            throw InvalidMapException(); // no
        }
        
        Map map;
        try {
            if(map.decompress_if_needed(data.data(), data.size())) {
                data.clear();
            }
            else {
                map.data = std::move(data);
            }
            map.bitmap_data = std::move(bitmaps_data);
            map.sound_data = std::move(sounds_data);
            map.loc_data = std::move(loc_data);
            map.load_map();
        }
        catch(Exception &) {
            throw InvalidMapException();
        }
        return map;
    }

    bool Map::decompress_if_needed(const std::byte *data, std::size_t data_size) {
        using namespace Invader::HEK;
        
        const auto *potential_header = reinterpret_cast<const CacheFileHeader *>(data);
        CompressionType compression_type = CompressionType::COMPRESSION_TYPE_NONE;
        
        auto do_decompress_if_needed = [&compression_type, &data, &data_size](Map *map, auto *header) {
            switch(header->engine) {
                case CacheFileEngine::CACHE_FILE_XBOX:
                    compression_type = CompressionType::COMPRESSION_TYPE_DEFLATE;
                    break;
                default:
                    break;
            }
            
            // If so, decompress it
            if(compression_type) {
                // Uh... deja vu?
                if(map->get_compression_algorithm()) {
                    eprintf_error("map was double-compressed");
                    throw InvalidMapException();
                }
                
                // Okay we're good
                map->data = Compression::decompress_map_data(data, data_size);
                map->compressed = compression_type;
            }
        };
        
        if(potential_header->valid()) {
            do_decompress_if_needed(this, potential_header);
        }
        
        return compression_type;
    }

    std::byte *Map::get_data_at_offset(std::size_t offset, std::size_t minimum_size, DataMapType map_type) {
        std::size_t max_length = this->get_data_length(map_type);
        std::byte *data_ptr = this->get_data(map_type);

        if(offset >= max_length || offset + minimum_size > max_length) {
            throw OutOfBoundsException();
        }
        else {
            return data_ptr + offset;
        }
    }

    std::byte *Map::get_data(DataMapType map_type) {
        if(map_type != DATA_MAP_CACHE && this->get_data_length(map_type) == 0) {
            throw ResourceMapRequiredException();
        }
        switch(map_type) {
            case DATA_MAP_CACHE:
                return this->data.data();
            case DATA_MAP_BITMAP:
                return this->bitmap_data.data();
            case DATA_MAP_SOUND:
                return this->sound_data.data();
            case DATA_MAP_LOC:
                return this->loc_data.data();
            default:
                std::terminate();
        }
    }

    const std::byte *Map::get_data(DataMapType map_type) const {
        return const_cast<Map *>(this)->get_data(map_type);
    }

    std::size_t Map::get_data_length(DataMapType map_type) const noexcept {
        switch(map_type) {
            case DATA_MAP_CACHE:
                return this->data.size();
            case DATA_MAP_BITMAP:
                return this->bitmap_data.size();
            case DATA_MAP_SOUND:
                return this->sound_data.size();
            case DATA_MAP_LOC:
                return this->loc_data.size();
        }
        std::terminate();
    }

    const std::byte *Map::get_data_at_offset(std::size_t offset, std::size_t minimum_size, DataMapType map_type) const {
        return const_cast<Map *>(this)->get_data_at_offset(offset, minimum_size, map_type);
    }

    std::byte *Map::get_tag_data_at_offset(std::size_t offset, std::size_t minimum_size) {
        if(offset >= this->tag_data_length || offset + minimum_size > this->tag_data_length) {
            throw OutOfBoundsException();
        }
        else {
            return this->tag_data + offset;
        }
    }

    const std::byte *Map::get_tag_data_at_offset(std::size_t offset, std::size_t minimum_size) const {
        return const_cast<Map *>(this)->get_tag_data_at_offset(offset, minimum_size);
    }

    std::byte *Map::resolve_tag_data_pointer(HEK::Pointer pointer, std::size_t minimum_size) {
        return this->get_tag_data_at_offset(pointer - this->base_memory_address, minimum_size);
    }

    const std::byte *Map::resolve_tag_data_pointer(HEK::Pointer pointer, std::size_t minimum_size) const {
        return const_cast<Map *>(this)->resolve_tag_data_pointer(pointer, minimum_size);
    }

    std::size_t Map::get_tag_count() const noexcept {
        return this->tags.size();
    }

    Tag &Map::get_tag(std::size_t index) {
        if(index >= this->get_tag_count()) {
            throw OutOfBoundsException();
        }
        else {
            return this->tags[index];
        }
    }

    const Tag &Map::get_tag(std::size_t index) const {
        return const_cast<Map *>(this)->get_tag(index);
    }

    std::size_t Map::get_scenario_tag_id() const noexcept {
        return this->scenario_tag_id;
    }

    void Map::load_map() {
        using namespace Invader::HEK;
        
        // Get header
        auto *header_maybe = reinterpret_cast<const CacheFileHeader *>(this->get_data_at_offset(0, sizeof(CacheFileHeader)));
        auto data_length = this->data.size();

        auto continue_loading_map = [&data_length](Map &map, auto &header) {
            // Set the cache_version and type
            map.cache_version = header.engine;
            map.header_type = header.map_type;
            
            // Check if it's valid
            const auto *game_engine_info_maybe = GameEngineInfo::get_game_engine_info(map.cache_version, header.build.string);
            if(game_engine_info_maybe == nullptr) {
                throw UnsupportedMapEngineException();
            }
            map.game_engine = game_engine_info_maybe->engine;

            // If we don't know the type of engine, bail
            switch(map.cache_version) {
                case CacheFileEngine::CACHE_FILE_DEMO:
                case CacheFileEngine::CACHE_FILE_RETAIL:
                case CacheFileEngine::CACHE_FILE_CUSTOM_EDITION:
                case CacheFileEngine::CACHE_FILE_NATIVE:
                case CacheFileEngine::CACHE_FILE_MCC_CEA:
                case CacheFileEngine::CACHE_FILE_XBOX:
                    break;
                default:
                    throw UnsupportedMapEngineException();
            }

            // Check if any overflowing occurs
            if(header.decompressed_file_size > data_length || header.build.overflows() || header.name.overflows()) {
                throw InvalidMapException();
            }
            
            map.tag_data_length = header.tag_data_size;
            map.tag_data = map.get_data_at_offset(header.tag_data_offset, map.tag_data_length);
            map.base_memory_address = game_engine_info_maybe->base_memory_address;
            
            // Eschaton the base memory address (base memory address is inferred with the tag data address)
            if(game_engine_info_maybe->base_memory_address_is_inferred) {
                auto *tag_data_header = reinterpret_cast<HEK::CacheFileTagDataHeader *>(map.tag_data);
                map.base_memory_address = tag_data_header->tag_array_address - (map.cache_version == CacheFileEngine::CACHE_FILE_XBOX ? sizeof(HEK::CacheFileTagDataHeaderXbox) : sizeof(HEK::CacheFileTagDataHeaderPC));
            }

            map.scenario_name = header.name;
            map.build = header.build;
            map.header_decompressed_file_size = header.decompressed_file_size;
            map.header_crc32 = header.crc32;
        };

        // Check if the literals are invalid
        if(!header_maybe->valid()) {
            // Maybe it's a demo map?
            auto *demo_header_maybe = reinterpret_cast<const CacheFileDemoHeader *>(header_maybe);
            if(!demo_header_maybe->valid()) {
                throw InvalidMapException();
            }
            continue_loading_map(*this, *demo_header_maybe);
        }
        else if(header_maybe->engine == CacheFileEngine::CACHE_FILE_NATIVE) {
            continue_loading_map(*this, *reinterpret_cast<const NativeCacheFileHeader *>(header_maybe));
        }
        else {
            continue_loading_map(*this, *header_maybe);
        }

        this->populate_tag_array();
    }
    
    std::uint32_t Map::get_crc32() const noexcept {
        return calculate_map_crc(*const_cast<Map *>(this));
    }

    void Map::populate_tag_array() {
        using namespace Invader::HEK;

        auto &map = *this;
        map.type = CacheFileType::SCENARIO_TYPE_SINGLEPLAYER;

        // Preallocate tags
        const auto &header = *reinterpret_cast<const CacheFileTagDataHeader *>(this->get_tag_data_at_offset(0, sizeof(CacheFileTagDataHeader)));
        std::size_t tag_count = header.tag_count;
        this->tags.reserve(tag_count);

        // Determine our scenario tag
        this->scenario_tag_id = header.scenario_tag.read().index;
        if(this->scenario_tag_id >= tag_count) {
            throw OutOfBoundsException();
        }

        // Set this stuff!
        auto set_model_stuff = [&map](auto &header) {
            map.model_data_offset = header.model_data_file_offset;
            map.model_data_size = header.model_data_size;
            map.model_index_offset = header.vertex_size;
        };
        if(this->get_cache_version() == HEK::CacheFileEngine::CACHE_FILE_NATIVE) {
            auto &native_header = *reinterpret_cast<const NativeCacheFileTagDataHeader *>(this->get_tag_data_at_offset(0, sizeof(NativeCacheFileTagDataHeader)));
            set_model_stuff(native_header);
            this->asset_indices_offset = native_header.raw_data_indices;
        }
        else if(this->get_cache_version() != HEK::CacheFileEngine::CACHE_FILE_XBOX) {
            set_model_stuff(*reinterpret_cast<const CacheFileTagDataHeaderPC *>(this->get_tag_data_at_offset(0, sizeof(CacheFileTagDataHeaderPC))));
        }

        auto do_populate_the_array = [&map, &tag_count](auto *tags) {
            // Have a pointer for the end of the tag data so we can check to make sure things aren't null terminated
            const char *tag_data_end = reinterpret_cast<const char *>(map.tag_data) + map.tag_data_length;

            for(std::size_t i = 0; i < tag_count; i++) {
                map.tags.push_back(Tag(map));
                auto &tag = map.tags[i];
                tag.tag_fourcc = tags[i].primary_class;
                tag.tag_data_index_offset = reinterpret_cast<const std::byte *>(tags + i) - map.tag_data;
                tag.tag_index = i;
                
                // Set the map type
                if(i == map.scenario_tag_id) {
                    map.type = reinterpret_cast<Scenario<LittleEndian> *>(map.resolve_tag_data_pointer(tags[i].tag_data, sizeof(Scenario<LittleEndian>)))->type;
                }

                try {
                    const auto *path = reinterpret_cast<const char *>(map.resolve_tag_data_pointer(tags[i].tag_path));

                    // Make sure the path is null-terminated and it doesn't contain whitespace that isn't an ASCII space (0x20) or forward slash characters
                    bool null_terminated = false;
                    for(auto *path_test = path; path < tag_data_end; path_test++) {
                        if(*path_test == 0) {
                            null_terminated = true;
                            
                            // Did we even start?
                            if(path_test == path) {
                                throw InvalidTagPathException();
                            }
                            
                            break;
                        }
                        else if(*path_test == '/') {
                            throw InvalidTagPathException();
                        }
                        else {
                            // Control characters?
                            auto latin1 = static_cast<std::uint8_t>(*path_test);
                            if(latin1 < 0x20 || (latin1 > 0x7E && latin1 < 0xA0)) {
                                throw InvalidTagPathException();
                            }
                        }
                    }

                    // If it was null terminated and it does NOT start with a dot, use it. Otherwise, don't.
                    if(null_terminated && *path != '.') {
                        tag.path = Invader::File::remove_duplicate_slashes(path);
                    }
                    else {
                        throw InvalidTagPathException();
                    }
                    
                    // Lowercase everything
                    for(char &c : tag.path) {
                        c = std::tolower(c);
                    }
                }
                catch (std::exception &) {
                    char new_path[64];
                    std::snprintf(new_path, sizeof(new_path), "corrupted\\tag_%zu", i);
                    map.invalid_paths_detected = true;
                    tag.path = new_path;
                }

                if(tag.tag_fourcc == TagFourCC::TAG_FOURCC_SCENARIO_STRUCTURE_BSP && map.cache_version != HEK::CacheFileEngine::CACHE_FILE_NATIVE) {
                    continue;
                }
                else if(sizeof(tags->tag_data) == sizeof(HEK::Pointer) && reinterpret_cast<const CacheFileTagDataTag *>(tags)[i].indexed) {
                    tag.indexed = true;

                    // Indexed sound tags still use tag data (until you use reflexives)
                    if(tag.tag_fourcc == TagFourCC::TAG_FOURCC_SOUND) {
                        tag.base_struct_pointer = tags[i].tag_data;
                    }
                    else {
                        tag.base_struct_pointer = 0;
                        tag.resource_index = tags[i].tag_data;
                    }

                    // Find where it's located
                    DataMapType type;
                    bool unavailable = false;
                    switch(tag.tag_fourcc) {
                        case TagFourCC::TAG_FOURCC_BITMAP:
                            type = DataMapType::DATA_MAP_BITMAP;
                            unavailable = map.bitmap_data.size() == 0;
                            break;
                        case TagFourCC::TAG_FOURCC_SOUND:
                            type = DataMapType::DATA_MAP_SOUND;
                            unavailable = map.sound_data.size() == 0;
                            break;
                        default:
                            type = DataMapType::DATA_MAP_LOC;
                            unavailable = map.loc_data.size() == 0;
                            break;
                    }
                    
                    // If we don't have the corresponding map, continue
                    if(unavailable) {
                        continue;
                    }

                    // Let's begin.
                    auto &header = *reinterpret_cast<ResourceMapHeader *>(map.get_data_at_offset(0, sizeof(ResourceMapHeader), type));
                    auto count = header.resource_count.read();
                    auto *indices = reinterpret_cast<ResourceMapResource *>(map.get_data_at_offset(header.resources, count * sizeof(ResourceMapResource), type));

                    // Find that index if we're a sounds.map file
                    if(!tag.resource_index.has_value()) {
                        auto *paths = reinterpret_cast<const char *>(map.get_data_at_offset(header.paths, 0, type));
                        for(std::uint32_t i = 1; i < count; i+=2) {
                            auto *path = paths + indices[i].path_offset;
                            if(tag.path == path) {
                                tag.resource_index = i;
                                break;
                            }
                        }
                    }
                    
                    // Do we even have an index?
                    if(!tag.resource_index.has_value()) {
                        eprintf_error("Tag %s.%s could not be found in the resource map file", File::halo_path_to_preferred_path(tag.path).c_str(), HEK::tag_fourcc_to_extension(tag.tag_fourcc));
                        throw OutOfBoundsException();
                    }

                    // Make sure it's valid
                    if(*tag.resource_index >= count) {
                        eprintf_error("Tag %s.%s is out-of-bounds for the resource map(s) provided (%zu >= %zu)", File::halo_path_to_preferred_path(tag.path).c_str(), HEK::tag_fourcc_to_extension(tag.tag_fourcc), *tag.resource_index, static_cast<std::size_t>(count));
                        throw OutOfBoundsException();
                    }

                    // Set it all
                    auto &index = indices[*tag.resource_index];
                    tag.tag_data_size = index.size;
                    if(tag.tag_fourcc == TagFourCC::TAG_FOURCC_SOUND) {
                        tag.base_struct_offset = index.data_offset + sizeof(HEK::Sound<HEK::LittleEndian>);
                    }
                    else {
                        tag.base_struct_offset = index.data_offset;
                    }
                }
                else {
                    tag.base_struct_pointer = tags[i].tag_data;
                    
                    // Check if there are external pointers
                    switch(tag.tag_fourcc) {
                        case TagFourCC::TAG_FOURCC_BITMAP: {
                            auto &base_struct = tag.get_base_struct<HEK::Bitmap>();
                            std::size_t bitmap_data_count = base_struct.bitmap_data.count;
                            if(bitmap_data_count) {
                                auto *bitmaps = tag.resolve_reflexive(base_struct.bitmap_data);
                                for(std::size_t b = 0; b < bitmap_data_count; b++) {
                                    if(bitmaps[b].flags & BitmapDataFlagsFlag::BITMAP_DATA_FLAGS_FLAG_EXTERNAL) {
                                        tag.external_pointers = true;
                                        break;
                                    }
                                }
                            }
                            break;
                        }
                        case TagFourCC::TAG_FOURCC_SOUND: {
                            auto &base_struct = tag.get_base_struct<HEK::Sound>();
                            std::size_t pitch_range_count = base_struct.pitch_ranges.count;
                            if(pitch_range_count) {
                                auto *pitch_ranges = tag.resolve_reflexive(base_struct.pitch_ranges);
                                for(std::size_t pr = 0; pr < pitch_range_count && !tag.external_pointers; pr++) {
                                    auto &pitch_range = pitch_ranges[pr];
                                    std::size_t permutation_count = pitch_range.permutations.count;
                                    if(permutation_count) {
                                        auto *permutations = tag.resolve_reflexive(pitch_range.permutations);
                                        for(std::size_t p = 0; p < permutation_count; p++) {
                                            if(permutations[p].samples.external & 1) {
                                                tag.external_pointers = true;
                                                break; // breaks out of outer loop too due to the check
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
            }
        };

        if(this->cache_version == HEK::CacheFileEngine::CACHE_FILE_NATIVE) {
            try {
                do_populate_the_array(reinterpret_cast<const NativeCacheFileTagDataTag *>(this->resolve_tag_data_pointer(header.tag_array_address, sizeof(CacheFileTagDataTag) * tag_count)));
            }
            catch(std::exception &) {
                eprintf_error("Failed to populate the tag array");
                throw;
            }
        }
        else {
            try {
                do_populate_the_array(reinterpret_cast<const CacheFileTagDataTag *>(this->resolve_tag_data_pointer(header.tag_array_address, sizeof(CacheFileTagDataTag) * tag_count)));
            }
            catch(std::exception &) {
                eprintf_error("Failed to populate the tag array");
                throw;
            }
            try {
                this->get_bsps();
            }
            catch(std::exception &) {
                eprintf_error("Failed to read BSPs");
                throw;
            }
        }
    }

    void Map::get_bsps() {
        using namespace Invader::HEK;

        auto &scenario_tag = this->tags[this->scenario_tag_id];
        auto &tag = scenario_tag.get_base_struct<Scenario>();
        std::size_t bsp_count = tag.structure_bsps.count;
        auto *bsps = scenario_tag.resolve_reflexive(tag.structure_bsps);

        for(std::size_t i = 0; i < bsp_count; i++) {
            auto &bsp = bsps[i];
            std::size_t bsp_id = bsp.structure_bsp.tag_id.read().index;
            if(bsp_id >= this->tags.size()) {
                throw OutOfBoundsException();
            }

            // Add the BSP stuff here
            auto &bsp_tag = this->tags[bsp_id];
            bsp_tag.tag_data_size = bsp.bsp_size;
            bsp_tag.base_struct_offset = bsp.bsp_start;
            bsp_tag.base_struct_pointer = bsp.bsp_address;
        }
    }

    Map::CompressionType Map::get_compression_algorithm() const noexcept {
        return this->compressed;
    }

    bool Map::is_protected() const noexcept {
        using namespace HEK;
        
        // Invalid paths?
        if(this->invalid_paths_detected) {
            return true;
        }

        // We can get this right off the bat
        if(this->get_tag(this->get_scenario_tag_id()).get_tag_fourcc() != TagFourCC::TAG_FOURCC_SCENARIO) {
            return true;
        }

        // Go through each tag
        auto tag_count = this->get_tag_count();
        for(std::size_t t = 0; t < tag_count; t++) {
            auto &tag = this->get_tag(t);
            auto tag_class = tag.get_tag_fourcc();
            auto &tag_path = tag.get_path();

            // If the tag has no data, but it's not because it's indexed, keep going
            if(!tag.data_is_available() && !tag.is_indexed()) {
                continue;
            }

            // If the extension is invalid, return true
            if(tag_class == TagFourCC::TAG_FOURCC_NULL || tag_class == TagFourCC::TAG_FOURCC_NONE || tag_extension_to_fourcc(tag_fourcc_to_extension(tag_class)) != tag_class) {
                return true;
            }

            // Empty path? Probably protected
            if(tag_path == "") {
                return true;
            }

            // Go through each tag and see if we have any duplicates
            for(std::size_t t2 = t + 1; t2 < tag_count; t2++) {
                auto &tag2 = this->get_tag(t2);
                if(tag_class != tag2.get_tag_fourcc()) {
                    continue;
                }
                if(tag2.get_path() == tag_path) {
                    return true;
                }
            }
        }
        return false;
    }

    std::optional<std::size_t> Map::find_tag(const char *tag_path, TagFourCC tag_fourcc) const noexcept {
        for(auto &tag : tags) {
            if(tag.get_tag_fourcc() == tag_fourcc && tag.get_path() == tag_path) {
                return &tag - tags.data();
            }
        }
        return std::nullopt;
    }

    Map::Map(Map &&move) {
        this->data = std::move(move.data);
        this->bitmap_data = std::move(move.bitmap_data);
        this->loc_data = std::move(move.loc_data);
        this->sound_data = std::move(move.sound_data);
        this->cache_version = move.cache_version;
        this->load_map();
        this->compressed = move.compressed;
        
        // Clear tags from old version
        move.tags.clear();
    }

    std::byte *Map::get_internal_asset(std::size_t offset, std::size_t minimum_size) {
        if(this->cache_version == HEK::CacheFileEngine::CACHE_FILE_NATIVE) {
            auto *data = reinterpret_cast<HEK::LittleEndian<std::uint64_t> *>(this->get_data_at_offset(this->asset_indices_offset, (1 + offset) * sizeof(HEK::LittleEndian<std::uint64_t>))) + offset;
            offset = *data;
        }

        return this->get_data_at_offset(offset, minimum_size);
    }
    
    bool Map::is_clean() const noexcept {
        if(this->get_crc32() != this->get_header_crc32() || this->is_protected() || this->data.size() != this->get_header_decompressed_file_size() || this->get_type() != this->get_header_type()) {
            return false;
        }
        else if(this->get_cache_version() != HEK::CacheFileEngine::CACHE_FILE_NATIVE) {
            auto tag_count = this->get_tag_count();
            for(std::size_t i = 0; i < tag_count; i++) {
                auto &tag = this->get_tag(i);
                auto &index = tag.get_tag_data_index();
                
                // BSP tags are NOT supposed to have this set
                if(tag.get_tag_fourcc() == HEK::TagFourCC::TAG_FOURCC_SCENARIO_STRUCTURE_BSP && index.tag_data != 0) {
                    return false;
                }
            }
        }
        return true;
    }
    
    HEK::GameEngine Map::get_game_engine() const noexcept {
        return this->game_engine;
    }
}
