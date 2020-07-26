// SPDX-License-Identifier: GPL-3.0-only

#include <invader/hek/map.hpp>
#include <invader/tag/hek/definition.hpp>
#include <invader/resource/hek/resource_map.hpp>
#include <invader/compress/compression.hpp>
#include <invader/resource/resource_map.hpp>
#include <invader/map/map.hpp>
#include <invader/file/file.hpp>

namespace Invader {
    Map Map::map_with_copy(const std::byte *data, std::size_t data_size,
                           const std::byte *bitmaps_data, std::size_t bitmaps_data_size,
                           const std::byte *loc_data, std::size_t loc_data_size,
                           const std::byte *sounds_data, std::size_t sounds_data_size) {
        Map map;
        if(!map.decompress_if_needed(data, data_size)) {
            map.data_m.insert(map.data_m.end(), data, data + data_size);
        }
        map.data = map.data_m.data();
        map.data_length = map.data_m.size();
        map.bitmap_data_m.insert(map.bitmap_data_m.end(), bitmaps_data, bitmaps_data + bitmaps_data_size);
        map.bitmap_data = map.bitmap_data_m.data();
        map.bitmap_data_length = bitmaps_data_size;
        map.sound_data_m.insert(map.sound_data_m.end(), sounds_data, sounds_data + sounds_data_size);
        map.sound_data = map.sound_data_m.data();
        map.sound_data_length = sounds_data_size;
        map.loc_data_m.insert(map.loc_data_m.end(), loc_data, loc_data + loc_data_size);
        map.loc_data = map.loc_data_m.data();
        map.loc_data_length = loc_data_size;
        map.load_map();
        return map;
    }

    Map Map::map_with_move(std::vector<std::byte> &&data,
                           std::vector<std::byte> &&bitmaps_data,
                           std::vector<std::byte> &&loc_data,
                           std::vector<std::byte> &&sounds_data) {
        Map map;
        if(map.decompress_if_needed(data.data(), data.size())) {
            data.clear();
        }
        else {
            map.data_m = data;
        }
        map.data = map.data_m.data();
        map.data_length = map.data_m.size();

        map.bitmap_data_m = bitmaps_data;
        map.bitmap_data = map.bitmap_data_m.data();
        map.bitmap_data_length = map.bitmap_data_m.size();

        map.sound_data_m = sounds_data;
        map.sound_data = map.sound_data_m.data();
        map.sound_data_length = map.sound_data_m.size();

        map.loc_data_m = loc_data;
        map.loc_data = map.loc_data_m.data();
        map.loc_data_length = map.loc_data_m.size();
        
        map.load_map();
        return map;
    }

    Map Map::map_with_pointer(std::byte *data, std::size_t data_size,
                              std::byte *bitmaps_data, std::size_t bitmaps_data_size,
                              std::byte *loc_data, std::size_t loc_data_size,
                              std::byte *sounds_data, std::size_t sounds_data_size) {
        Map map;
        map.data = data;
        map.data_length = data_size;
        map.bitmap_data = bitmaps_data;
        map.bitmap_data_length = bitmaps_data_size;
        map.loc_data = loc_data;
        map.loc_data_length = loc_data_size;
        map.sound_data = sounds_data;
        map.sound_data_length = sounds_data_size;
        map.load_map();
        return map;
    }

    bool Map::decompress_if_needed(const std::byte *data, std::size_t data_size) {
        using namespace Invader::HEK;
        const auto *potential_header = reinterpret_cast<const CacheFileHeader *>(data);
        bool needs_decompressed = false;
        if(data_size > sizeof(*potential_header)) {
            // Check if it needs decompressed based on header
            if(potential_header->valid()) {
                switch(potential_header->engine.read()) {
                    case CacheFileEngine::CACHE_FILE_NATIVE: {
                        auto *header = reinterpret_cast<const NativeCacheFileHeader *>(&potential_header);
                        needs_decompressed = header->compression_type != NativeCacheFileHeader::NativeCacheFileCompressionType::NATIVE_CACHE_FILE_COMPRESSION_UNCOMPRESSED;
                        
                        if(!needs_decompressed && header->decompressed_file_size.read() != data_size) {
                            eprintf_error("decompressed file size in the header is wrong");
                            throw InvalidMapException();
                        }
                        
                        break;
                    }
                    case CacheFileEngine::CACHE_FILE_XBOX:
                        if(potential_header->decompressed_file_size != 0) {
                            needs_decompressed = true;
                        }
                        break;
                    case CacheFileEngine::CACHE_FILE_RETAIL_COMPRESSED:
                    case CacheFileEngine::CACHE_FILE_CUSTOM_EDITION_COMPRESSED:
                    case CacheFileEngine::CACHE_FILE_DEMO_COMPRESSED:
                        needs_decompressed = true;
                    default:
                        break;
                }
            }

            // If so, decompress it
            if(needs_decompressed) {
                this->data_m = Compression::decompress_map_data(data, data_size);
                this->compressed = true;
            }
        }

        return needs_decompressed;
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
                return this->data;
            case DATA_MAP_BITMAP:
                return this->bitmap_data;
            case DATA_MAP_SOUND:
                return this->sound_data;
            case DATA_MAP_LOC:
                return this->loc_data;
        }
        std::terminate();
    }

    const std::byte *Map::get_data(DataMapType map_type) const {
        return const_cast<Map *>(this)->get_data(map_type);
    }

    std::size_t Map::get_data_length(DataMapType map_type) const noexcept {
        switch(map_type) {
            case DATA_MAP_CACHE:
                return this->data_length;
            case DATA_MAP_BITMAP:
                return this->bitmap_data_length;
            case DATA_MAP_SOUND:
                return this->sound_data_length;
            case DATA_MAP_LOC:
                return this->loc_data_length;
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
        auto &data_length = this->data_length;

        auto continue_loading_map = [&data_length](Map &map, auto &header) {
            // Set the engine and type
            map.engine = header.engine;
            map.type = header.map_type;

            // If we don't know the type of engine, bail
            switch(map.engine) {
                case CacheFileEngine::CACHE_FILE_DEMO:
                case CacheFileEngine::CACHE_FILE_RETAIL:
                case CacheFileEngine::CACHE_FILE_CUSTOM_EDITION:
                    break;
                case CacheFileEngine::CACHE_FILE_NATIVE: {
                    auto *native_header = reinterpret_cast<const NativeCacheFileHeader *>(&header);
                    if(native_header->compression_type != NativeCacheFileHeader::NativeCacheFileCompressionType::NATIVE_CACHE_FILE_COMPRESSION_UNCOMPRESSED) {
                        oprintf("RAI! ;-;\n");
                        throw MapNeedsDecompressedException();
                    }
                    break;
                }
                case CacheFileEngine::CACHE_FILE_XBOX:
                    if(header.decompressed_file_size != 0) {
                        throw MapNeedsDecompressedException();
                    }
                    break;
                case CacheFileEngine::CACHE_FILE_RETAIL_COMPRESSED:
                case CacheFileEngine::CACHE_FILE_CUSTOM_EDITION_COMPRESSED:
                case CacheFileEngine::CACHE_FILE_DEMO_COMPRESSED:
                    throw MapNeedsDecompressedException();
                default:
                    throw UnsupportedMapEngineException();
            }

            // Check if any overflowing occurs
            if(header.decompressed_file_size > data_length || header.build.overflows() || header.name.overflows()) {
                throw InvalidMapException();
            }

            // Get tag data
            switch(map.engine) {
                case CacheFileEngine::CACHE_FILE_NATIVE:
                    map.base_memory_address = HEK::CACHE_FILE_NATIVE_BASE_MEMORY_ADDRESS;
                    break;
                case CacheFileEngine::CACHE_FILE_DEMO:
                    map.base_memory_address = HEK::CACHE_FILE_DEMO_BASE_MEMORY_ADDRESS;
                    break;
                case CacheFileEngine::CACHE_FILE_XBOX:
                    map.base_memory_address = HEK::CACHE_FILE_XBOX_BASE_MEMORY_ADDRESS;
                    break;
                default:
                    map.base_memory_address = HEK::CACHE_FILE_PC_BASE_MEMORY_ADDRESS;
                    break;
            }

            map.tag_data_length = header.tag_data_size;
            map.tag_data = map.get_data_at_offset(header.tag_data_offset, map.tag_data_length);
            map.scenario_name = header.name;
            map.build = header.build;
            map.crc32 = header.crc32;
        };

        // Check if the literals are invalid
        if(header_maybe->head_literal != CACHE_FILE_HEAD || header_maybe->foot_literal != CACHE_FILE_FOOT) {
            // Maybe it's a demo map?
            auto *demo_header_maybe = reinterpret_cast<const CacheFileDemoHeader *>(this->get_data_at_offset(0, sizeof(CacheFileDemoHeader)));
            if(demo_header_maybe->head_literal != CACHE_FILE_HEAD_DEMO || demo_header_maybe->foot_literal != CACHE_FILE_FOOT_DEMO) {
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

    void Map::populate_tag_array() {
        using namespace Invader::HEK;

        auto &map = *this;

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
        if(this->get_engine() == HEK::CacheFileEngine::CACHE_FILE_NATIVE) {
            auto &native_header = *reinterpret_cast<const NativeCacheFileTagDataHeader *>(this->get_tag_data_at_offset(0, sizeof(NativeCacheFileTagDataHeader)));
            set_model_stuff(native_header);
            this->asset_indices_offset = native_header.raw_data_indices;
        }
        else if(this->get_engine() != HEK::CacheFileEngine::CACHE_FILE_XBOX) {
            set_model_stuff(*reinterpret_cast<const CacheFileTagDataHeaderPC *>(this->get_tag_data_at_offset(0, sizeof(CacheFileTagDataHeaderPC))));
        }

        auto do_populate_the_array = [&map, &tag_count](auto *tags) {
            // Have a pointer for the end of the tag data so we can check to make sure things aren't null terminated
            const char *tag_data_end = reinterpret_cast<const char *>(map.tag_data) + map.tag_data_length;

            for(std::size_t i = 0; i < tag_count; i++) {
                map.tags.push_back(Tag(map));
                auto &tag = map.tags[i];
                tag.tag_class_int = tags[i].primary_class;
                tag.tag_data_index_offset = reinterpret_cast<const std::byte *>(tags + i) - map.tag_data;
                tag.tag_index = i;

                try {
                    auto *path = reinterpret_cast<const char *>(map.resolve_tag_data_pointer(tags[i].tag_path));

                    // Make sure the path is null-terminated and it doesn't contain whitespace that isn't an ASCII space (0x20) or forward slash characters
                    bool null_terminated = false;
                    for(auto *path_test = path; path < tag_data_end; path_test++) {
                        if(*path_test == 0) {
                            null_terminated = true;
                            break;
                        }
                        else if(*path_test < ' ') {
                            throw InvalidTagPathException();
                        }
                        else if(*path_test == '/') {
                            throw InvalidTagPathException();
                        }
                    }

                    // If it was null terminated and it does NOT start with a dot, use it. Otherwise, don't.
                    if(null_terminated && *path != '.') {
                        tag.path = Invader::File::remove_duplicate_slashes(path);
                    }
                    else {
                        throw InvalidTagPathException();
                    }
                }
                catch (std::exception &) {
                    tag.path = "";
                }

                if(tag.tag_class_int == TagClassInt::TAG_CLASS_SCENARIO_STRUCTURE_BSP && map.engine != HEK::CacheFileEngine::CACHE_FILE_NATIVE) {
                    continue;
                }
                else if(sizeof(tags->tag_data) == sizeof(HEK::Pointer) && reinterpret_cast<const CacheFileTagDataTag *>(tags)[i].indexed) {
                    tag.indexed = true;

                    // Indexed sound tags still use tag data (until you use reflexives)
                    if(tag.tag_class_int == TagClassInt::TAG_CLASS_SOUND) {
                        tag.base_struct_pointer = tags[i].tag_data;
                    }
                    else {
                        tag.base_struct_pointer = 0;
                    }

                    // Find where it's located
                    DataMapType type;
                    switch(tag.tag_class_int) {
                        case TagClassInt::TAG_CLASS_BITMAP:
                            type = DataMapType::DATA_MAP_BITMAP;
                            break;
                        case TagClassInt::TAG_CLASS_SOUND:
                            type = DataMapType::DATA_MAP_SOUND;
                            break;
                        default:
                            type = DataMapType::DATA_MAP_LOC;
                            break;
                    }

                    // Next, check if we have that
                    if(type == DataMapType::DATA_MAP_BITMAP && map.bitmap_data_length == 0) {
                        continue;
                    }
                    else if(type == DataMapType::DATA_MAP_SOUND && map.sound_data_length == 0) {
                        continue;
                    }
                    else if(type == DataMapType::DATA_MAP_LOC && map.loc_data_length == 0) {
                        continue;
                    }

                    // Let's begin.
                    auto &header = *reinterpret_cast<ResourceMapHeader *>(map.get_data_at_offset(0, sizeof(ResourceMapHeader), type));
                    auto count = header.resource_count.read();
                    auto *indices = reinterpret_cast<ResourceMapResource *>(map.get_data_at_offset(header.resources, count * sizeof(ResourceMapResource), type));
                    std::uint32_t resource_index = static_cast<std::uint32_t>(~0);

                    // Find that index
                    if(type == DataMapType::DATA_MAP_SOUND) {
                        auto *paths = reinterpret_cast<const char *>(map.get_data_at_offset(header.paths, 0, type));
                        for(std::uint32_t i = 1; i < count; i+=2) {
                            auto *path = paths + indices[i].path_offset;
                            if(tag.path == path) {
                                resource_index = i;
                                break;
                            }
                        }
                    }
                    else {
                        resource_index = tags[i].tag_data;
                    }

                    // Make sure it's valid
                    if(resource_index >= count) {
                        throw OutOfBoundsException();
                    }

                    // Set it all
                    auto &index = indices[resource_index];
                    tag.tag_data_size = index.size;
                    if(tag.tag_class_int == TagClassInt::TAG_CLASS_SOUND) {
                        tag.base_struct_offset = index.data_offset + sizeof(HEK::Sound<HEK::LittleEndian>);
                    }
                    else {
                        tag.base_struct_offset = index.data_offset;
                    }
                }
                else {
                    tag.base_struct_pointer = tags[i].tag_data;
                }
            }
        };

        if(this->engine == HEK::CacheFileEngine::CACHE_FILE_NATIVE) {
            do_populate_the_array(reinterpret_cast<const NativeCacheFileTagDataTag *>(this->resolve_tag_data_pointer(header.tag_array_address, sizeof(CacheFileTagDataTag) * tag_count)));
        }
        else {
            do_populate_the_array(reinterpret_cast<const CacheFileTagDataTag *>(this->resolve_tag_data_pointer(header.tag_array_address, sizeof(CacheFileTagDataTag) * tag_count)));
            this->get_bsps();
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

    bool Map::is_compressed() const noexcept {
        return this->compressed;
    }

    bool Map::is_protected() const noexcept {
        using namespace HEK;

        // We can get this right off the bat
        if(this->get_tag(this->get_scenario_tag_id()).get_tag_class_int() != TagClassInt::TAG_CLASS_SCENARIO) {
            return true;
        }

        // Go through each tag
        auto tag_count = this->get_tag_count();
        for(std::size_t t = 0; t < tag_count; t++) {
            auto &tag = this->get_tag(t);
            auto tag_class = tag.get_tag_class_int();
            auto &tag_path = tag.get_path();

            // If the tag has no data, but it's not because it's indexed, keep going
            if(!tag.data_is_available() && !tag.is_indexed()) {
                continue;
            }

            // If the extension is invalid, return true
            if(tag_class == TagClassInt::TAG_CLASS_NULL || tag_class == TagClassInt::TAG_CLASS_NONE || extension_to_tag_class(tag_class_to_extension(tag_class)) != tag_class) {
                return true;
            }

            // Empty path? Probably protected
            if(tag_path == "") {
                return true;
            }

            // Go through each tag and see if we have any duplicates
            for(std::size_t t2 = t + 1; t2 < tag_count; t2++) {
                auto &tag2 = this->get_tag(t2);
                if(tag_class != tag2.get_tag_class_int()) {
                    continue;
                }
                if(tag2.get_path() == tag_path) {
                    return true;
                }
            }
        }
        return false;
    }

    std::optional<std::size_t> Map::find_tag(const char *tag_path, TagClassInt tag_class_int) const noexcept {
        for(auto &tag : tags) {
            if(tag.get_tag_class_int() == tag_class_int && tag.get_path() == tag_path) {
                return &tag - tags.data();
            }
        }
        return std::nullopt;
    }

    Map::Map(Map &&move) {
        this->data_m = std::move(move.data_m);
        this->data = move.data;
        this->data_length = move.data_length;
        this->bitmap_data_m = std::move(move.bitmap_data_m);
        this->bitmap_data = move.bitmap_data;
        this->bitmap_data_length = move.bitmap_data_length;
        this->loc_data_m = std::move(move.loc_data_m);
        this->loc_data = move.loc_data;
        this->loc_data_length = move.loc_data_length;
        this->sound_data_m = std::move(move.sound_data_m);
        this->sound_data = move.sound_data;
        this->sound_data_length = move.sound_data_length;
        this->engine = move.engine;
        this->type = move.type;
        this->model_data_offset = move.model_data_offset;
        this->model_index_offset = move.model_index_offset;
        this->model_data_size = move.model_data_size;
        this->asset_indices_offset = move.asset_indices_offset;

        if(this->data_m.size()) {
            this->data = this->data_m.data();
        }
        if(this->loc_data_m.size()) {
            this->loc_data = this->loc_data_m.data();
        }
        if(this->sound_data_m.size()) {
            this->sound_data = this->sound_data_m.data();
        }
        if(this->bitmap_data_m.size()) {
            this->bitmap_data = this->bitmap_data_m.data();
        }

        move.tags.clear();

        this->load_map();
        this->compressed = move.compressed;
    }

    std::byte *Map::get_internal_asset(std::size_t offset, std::size_t minimum_size) {
        if(this->engine == HEK::CacheFileEngine::CACHE_FILE_NATIVE) {
            auto *data = reinterpret_cast<HEK::LittleEndian<std::uint64_t> *>(this->get_data_at_offset(this->asset_indices_offset, (1 + offset) * sizeof(HEK::LittleEndian<std::uint64_t>))) + offset;
            offset = *data;
        }

        return this->get_data_at_offset(offset, minimum_size);
    }
}
