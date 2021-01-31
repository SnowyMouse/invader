// SPDX-License-Identifier: GPL-3.0-only

#ifndef INVADER__MAP__MAP_HPP
#define INVADER__MAP__MAP_HPP

#include <string>
#include <vector>
#include <cstddef>
#include <memory>
#include <optional>

#include "../resource/resource_map.hpp"
#include "../hek/map.hpp"
#include "tag.hpp"

namespace Invader {
    /**
     * Class for handling compiled maps
     */
    class Map {
    friend class Tag;
    public:
        enum DataMapType {
            DATA_MAP_CACHE,
            DATA_MAP_BITMAP,
            DATA_MAP_SOUND,
            DATA_MAP_LOC
        };
    
        enum CompressionType {
            COMPRESSION_TYPE_NONE,
            COMPRESSION_TYPE_ZSTANDARD,
            COMPRESSION_TYPE_DEFLATE,
            COMPRESSION_TYPE_MCC_DEFLATE
        };

        /**
         * Get the internal bitmap or sound asset
         * @param  offset       offset or index
         * @param  minimum_size size of the asset
         * @return              pointer to asset
         */
        std::byte *get_internal_asset(std::size_t offset, std::size_t minimum_size);

        /**
         * Get the internal bitmap or sound asset
         * @param  offset       offset or index
         * @param  minimum_size size of the asset
         * @return              pointer to asset
         */
        const std::byte *get_internal_asset(std::size_t offset, std::size_t minimum_size) const {
            return const_cast<Map *>(this)->get_internal_asset(offset, minimum_size);
        }

        /**
         * Get the engine
         * @return engine
         */
        HEK::CacheFileEngine get_engine() const noexcept {
            return this->engine;
        }

        /**
         * Get the map type
         * @return map type
         */
        HEK::CacheFileType get_type() const noexcept {
            return this->type;
        }

        /**
         * Get the model data offset
         * @return offset
         */
        std::size_t get_model_data_offset() const noexcept {
            return this->model_data_offset;
        }

        /**
         * Get the model data size
         * @return size
         */
        std::size_t get_model_data_size() const noexcept {
            return this->model_data_size;
        }

        /**
         * Get the model index offset
         * @return offset
         */
        std::size_t get_model_index_offset() const noexcept {
            return this->model_index_offset;
        }

        /**
         * Get the scenario name
         * @return scenario name
         */
        const char *get_scenario_name() const noexcept {
            return this->scenario_name.string;
        }

        /**
         * Get the build
         * @return build
         */
        const char *get_build() const noexcept {
            return this->build.string;
        }

        /**
         * Get the header's precomputed file size. This may not be indicative of the actual file size.
         * @return header's precomputed file size
         */
        std::uint64_t get_header_decompressed_file_size() const noexcept {
            return this->header_decompressed_file_size;
        }

        /**
         * Get the header's precomputed CRC32. This may not be indicative of the actual CRC32.
         * @return header's precomputed CRC32
         */
        std::uint32_t get_header_crc32() const noexcept {
            return this->header_crc32;
        }

        /**
         * Get the header's precomputed cache file type. This may not be indicative of the actual cache file type.
         * @return header's precomputed cache file type
         */
        HEK::CacheFileType get_header_type() const noexcept {
            return this->header_type;
        }
        
        /**
         * Calculate the map's CRC32
         * @return crc32
         */
        std::uint32_t get_crc32() const noexcept;

        /**
         * Get the tag data length
         * @return tag data length
         */
        std::size_t get_tag_data_length() const noexcept {
            return this->tag_data_length;
        }

        /**
         * Create a Map by copying the given data, bitmaps, loc, and sound data. Compressed maps can be loaded this
         * way.
         *
         * @param data              pointer to map data
         * @param data_size         length of map data
         * @param bitmaps_data      pointer to bitmaps data
         * @param bitmaps_data_size length of bitmaps data
         * @param loc_data          pointer to loc data
         * @param loc_data_size     length of loc data
         * @param sounds_data       pointer to sounds data
         * @param sounds_data_size  length of sounds data
         * @return                  map
         */
        static Map map_with_copy(const std::byte *data, std::size_t data_size,
                                 const std::byte *bitmaps_data = nullptr, std::size_t bitmaps_data_size = 0,
                                 const std::byte *loc_data = nullptr, std::size_t loc_data_size = 0,
                                 const std::byte *sounds_data = nullptr, std::size_t sounds_data_size = 0);

        /**
         * Create a Map by moving the given data, bitmaps, loc, and sound data. Compressed maps can be loaded this way.
         * @param  data         map data vector
         * @param  bitmaps_data bitmap data vector
         * @param  loc_data     loc data vector
         * @param  sounds_data  sound data vector
         * @return              map
         */
        static Map map_with_move(std::vector<std::byte> &&data,
                                 std::vector<std::byte> &&bitmaps_data = std::vector<std::byte>(),
                                 std::vector<std::byte> &&loc_data = std::vector<std::byte>(),
                                 std::vector<std::byte> &&sounds_data = std::vector<std::byte>());

        /**
         * Get the data at the specified offset
         * @param  offset       offset
         * @param  minimum_size minimum number of bytes to guarantee
         * @return              pointer to the data
         * @throws              OutOfBoundsException if data is out of bounds
         */
        std::byte *get_data_at_offset(std::size_t offset, std::size_t minimum_size = 0, DataMapType map_type = DATA_MAP_CACHE);

        /**
         * Get the data at the specified offset
         * @param  offset       offset
         * @param  minimum_size minimum number of bytes to guarantee
         * @return              pointer to the data
         * @throws              OutOfBoundsException if data is out of bounds
         */
        const std::byte *get_data_at_offset(std::size_t offset, std::size_t minimum_size = 0, DataMapType map_type = DATA_MAP_CACHE) const;

        /**
         * Get a pointer to the uncompressed map data
         * @param map_type map to get data from
         * @return         data
         */
        std::byte *get_data(DataMapType map_type = DATA_MAP_CACHE);

        /**
         * Get a pointer to the uncompressed map data
         * @param map_type map to get data from
         * @return         data
         */
        const std::byte *get_data(DataMapType map_type = DATA_MAP_CACHE) const;

        /**
         * Get the data length in bytes
         * @param map_type map to get data from
         * @return         data length in bytes
         */
        std::size_t get_data_length(DataMapType map_type = DATA_MAP_CACHE) const noexcept;

        /**
         * Get the tag data at the specified offset
         * @param  offset       offset
         * @param  minimum_size minimum number of bytes to guarantee
         * @return              pointer to the data
         * @throws              OutOfBoundsException if data is out of bounds
         */
        std::byte *get_tag_data_at_offset(std::size_t offset, std::size_t minimum_size = 0);

        /**
         * Get the tag data at the specified offset
         * @param  offset       offset
         * @param  minimum_size minimum number of bytes to guarantee
         * @return              pointer to the data
         * @throws              OutOfBoundsException if data is out of bounds
         */
        const std::byte *get_tag_data_at_offset(std::size_t offset, std::size_t minimum_size = 0) const;

        /**
         * Resolve the tag data pointer
         * @param  pointer      pointer
         * @param  minimum_size minimum number of bytes to guarantee
         * @return              pointer to the data
         * @throws              OutOfBoundsException if data is out of bounds
         */
        std::byte *resolve_tag_data_pointer(std::uint32_t offset, std::size_t minimum_size = 0);

        /**
         * Resolve the tag data pointer
         * @param  pointer      pointer
         * @param  minimum_size minimum number of bytes to guarantee
         * @return              pointer to the data
         * @throws              OutOfBoundsException if data is out of bounds
         */
        const std::byte *resolve_tag_data_pointer(std::uint32_t offset, std::size_t minimum_size = 0) const;

        /**
         * Get the tag count
         * @return the tag count
         */
        std::size_t get_tag_count() const noexcept;

        /**
         * Get the tag at the specified index
         * @param index the tag index
         * @return      the tag
         * @throws      OutOfBoundsException if index is invalid
         */
        Tag &get_tag(std::size_t index);

        /**
         * Get the tag at the specified index
         * @param index the tag index
         * @return      the tag
         * @throws      OutOfBoundsException if index is invalid
         */
        const Tag &get_tag(std::size_t index) const;

        /**
         * Find the tag with the given path and class
         * @param tag_path      tag path to find
         * @param tag_class_int tag class to find
         * @return              the index of the first tag found or std::nullopt if not found
         */
        std::optional<std::size_t> find_tag(const char *tag_path, TagClassInt tag_class_int) const noexcept;

        /**
         * Get the scenario tag ID
         * @return The scenario tag ID
         */
        std::size_t get_scenario_tag_id() const noexcept;

        /**
         * Get whether the map was originally compressed and, if so, the compression algorithm
         * @return compression algorithm
         */
        CompressionType get_compression_algorithm() const noexcept;

        /**
         * Get whether the map is obviously protected
         * @return true if the map is obviously protected
         */
        bool is_protected() const noexcept;
        
        /**
         * Do a basic check to ensure the map hasn't been improperly modified or corrupted
         * @return true if the map is clean
         */
        bool is_clean() const noexcept;

        Map(Map &&);
    private:
        /** Map data if managed */
        std::vector<std::byte> data;


        /** Bitmaps data if managed */
        std::vector<std::byte> bitmap_data;


        /** Loc data if managed */
        std::vector<std::byte> loc_data;


        /** Sounds data if managed */
        std::vector<std::byte> sound_data;
        

        /** Model data offset */
        std::size_t model_data_offset;

        /** Model index offset */
        std::size_t model_index_offset;

        /** Model data size too! */
        std::size_t model_data_size;


        /** Tag array */
        std::vector<Tag> tags;

        /** Scenario tag ID */
        std::size_t scenario_tag_id = 0;

        /** Tag data */
        std::byte *tag_data = nullptr;

        /** Tag data length */
        std::size_t tag_data_length = 0;

        /** Base memory address */
        std::uint32_t base_memory_address = HEK::CACHE_FILE_PC_BASE_MEMORY_ADDRESS;
        
        /** Invalid paths? */
        bool invalid_paths_detected = false;

        /** Map is compressed */
        CompressionType compressed = CompressionType::COMPRESSION_TYPE_NONE;

        /** Engine */
        HEK::CacheFileEngine engine;

        /** Type */
        HEK::CacheFileType type;

        /** Name */
        HEK::TagString scenario_name;

        /** Build */
        HEK::TagString build;

        /** CRC32 */
        std::optional<std::uint32_t> crc32;
        
        /** CRC32 in header */
        std::uint32_t header_crc32;

        /** Asset indices offset */
        std::uint64_t asset_indices_offset;
        
        /** Header file size */
        std::uint64_t header_decompressed_file_size;
        
        /** Header type */
        HEK::CacheFileType header_type;
        

        /** Load the map now */
        void load_map();

        /** Populate tag array */
        void populate_tag_array();

        /** Get BSPs */
        void get_bsps();

        /**
         * Decompress if we are compressed
         * @param data      pointer to data
         * @param data_size size of data buffer
         * @return true if it was decompressed
         */
        bool decompress_if_needed(const std::byte *data, std::size_t data_size);

        Map() = default;
    };
}
#endif
