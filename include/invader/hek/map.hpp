// SPDX-License-Identifier: GPL-3.0-only

#ifndef INVADER__HEK__MAP_HPP
#define INVADER__HEK__MAP_HPP

#include <cstdint>
#include <variant>
#include <list>
#include "data_type.hpp"
#include "../tag/hek/definition.hpp"

namespace Invader::HEK {
    enum CacheFileEngine : std::uint32_t {
        CACHE_FILE_XBOX = 5,
        CACHE_FILE_DEMO = 6,
        CACHE_FILE_RETAIL = 7,
        CACHE_FILE_CUSTOM_EDITION = 609,
        CACHE_FILE_MCC_CEA = 13,
        
        CACHE_FILE_NATIVE = 0x1A86
    };
    
    using CacheFileType = ScenarioType;
    const char *type_name(CacheFileType type) noexcept;
    
    enum GameEngine {
        GAME_ENGINE_NATIVE,
        GAME_ENGINE_XBOX_GENERIC,
        GAME_ENGINE_XBOX_DEMO,
        GAME_ENGINE_XBOX_NTSC_US,
        GAME_ENGINE_XBOX_NTSC_JP,
        GAME_ENGINE_XBOX_NTSC_TW,
        GAME_ENGINE_XBOX_PAL,
        GAME_ENGINE_GEARBOX_DEMO,
        GAME_ENGINE_GEARBOX_RETAIL,
        GAME_ENGINE_GEARBOX_CUSTOM_EDITION,
        GAME_ENGINE_MCC_COMBAT_EVOLVED_ANNIVERSARY,
        
        GAME_ENGINE_ENUM_COUNT
    };
    
    struct GameEngineInfo {
        using maximum_file_size_t = std::variant<Pointer64, Pointer64 (*)(CacheFileType)>;
        
        /** Human-readable identifier */
        const char *name;
        
        /** Human-readable shorthand identifier */
        const char *shorthand = nullptr;
        
        /** Engine enumerator. Note that this should NOT be stored in a file as this can and will change! */
        GameEngine engine;
        
        /** Cache version */
        CacheFileEngine cache_version;
        
        /** Build string, or a function to get the build string - use get_build_string() to retrieve it */
        std::variant<const char *, const char *(*)()> build_string;
        
        /** The build string must be exact */
        bool build_string_is_enforced;
        
        /** Base memory address for tag space */
        Pointer64 base_memory_address;
        
        /** When reading a cache file, infer the base memory address by the tag array address rather than a hardcoded address */
        bool base_memory_address_is_inferred = false;
        
        /** Tag space length */
        Pointer64 tag_space_length;
        
        /** Maximum file size, or a function to get the maximum file size - use get_maximum_file_size() to retrieve it */
        maximum_file_size_t maximum_file_size;
        
        /** Maximum script nodes allowed for the target engine */
        std::uint16_t maximum_scenario_script_nodes = UINT16_MAX;
        
        /** Compile target for scenario scripts (cast to RIATCompileTarget) */
        int scenario_script_compile_target = 0;
        
        bool scenario_name_and_file_name_must_be_equal = true;
        bool bsps_occupy_tag_space = true;
        bool supports_external_bitmaps_map = false;
        bool supports_external_sounds_map = false;
        bool supports_external_loc_map = false;
        bool uses_indexing = false;
        bool uses_compression = false;
        
        struct RequiredTags {
            struct TagPairPtr {
                const char *path;
                HEK::TagFourCC fourcc;
            };
            
            struct TagPairPtrArray {
                const TagPairPtr *ptr;
                std::size_t count;
            };
            
            /** An array of required tags for all types */
            TagPairPtrArray all = {};
            
            /** An array of required tags for singleplayer */
            TagPairPtrArray singleplayer = {};
            TagPairPtrArray singleplayer_demo = {};
            TagPairPtrArray singleplayer_full = {};
            
            /** An array of required tags for multiplayer */
            TagPairPtrArray multiplayer = {};
            TagPairPtrArray multiplayer_demo = {};
            TagPairPtrArray multiplayer_full = {};
            
            /** An array of required tags for user interface */
            TagPairPtrArray user_interface = {};
            TagPairPtrArray user_interface_demo = {};
            TagPairPtrArray user_interface_full = {};
        } required_tags = {};
        
        /**
         * Retrieve the build string
         * 
         * @return build string
         */
        const char *get_build_string() const noexcept;
        
        /**
         * Retrieve the maximum file size
         * 
         * @param type cache file type
         * @return     maximum file size
         */
        Pointer64 get_maximum_file_size(CacheFileType type) const noexcept;
        
        /**
         * Get whether or not external resource maps are supported
         * 
         * @return true if external resource maps are supported, false if not
         */
        bool supports_external_resource_maps() const noexcept { return this->supports_external_bitmaps_map || this->supports_external_sounds_map || this->supports_external_loc_map; }
    
        /**
         * Get the game engine info given a game engine enumerator
         * 
         * @param engine engine enumerator to look for
         * @return       reference to game engine
         */
        static const GameEngineInfo &get_game_engine_info(GameEngine engine) noexcept;
    
        /**
         * Get the game engine info given a game engine shorthand
         * 
         * @param shorthand shorthand to look for
         * @return          reference to game engine if found, or nullptr if not found
         */
        static const GameEngineInfo *get_game_engine_info(const char *shorthand) noexcept;
        
        /**
         * Get the game engine info given a cache version and a build string
         * 
         * @param cache_version cache version
         * @param build_string  build string version
         * @return              reference to game engine info if found, or nullptr if not found
         */
        static const GameEngineInfo *get_game_engine_info(CacheFileEngine cache_version, const char *build_string) noexcept;
        
        /**
         * Get all shorthands
         * @return shorthands
         */
        static std::list<const char *> get_all_shorthands();
    };

    enum CacheFileLiteral : std::uint32_t {
        CACHE_FILE_HEAD = 0x68656164,
        CACHE_FILE_FOOT = 0x666F6F74,
        CACHE_FILE_TAGS = 0x74616773,
        CACHE_FILE_HEAD_DEMO = 0x45686564,
        CACHE_FILE_FOOT_DEMO = 0x47666F74
    };

    // TODO: remove many of these constants since they've been refactored to GameEngineInfo
    enum CacheFileTagDataBaseMemoryAddress : Pointer64 {
        CACHE_FILE_PC_BASE_MEMORY_ADDRESS = 0x40440000,
        CACHE_FILE_DEMO_BASE_MEMORY_ADDRESS = 0x4BF10000,
        CACHE_FILE_NATIVE_BASE_MEMORY_ADDRESS = 0x00000000,
        CACHE_FILE_XBOX_BASE_MEMORY_ADDRESS = 0x803A6000,
        CACHE_FILE_MCC_CEA_BASE_MEMORY_ADDRESS = 0x50000000,

        CACHE_FILE_STUB_MEMORY_ADDRESS = UINT32_MAX,
        CACHE_FILE_STUB_MEMORY_ADDRESS_NATIVE = UINT64_MAX
    };
    
    enum CacheFileXboxConstants : std::uint32_t {
        CACHE_FILE_XBOX_SECTOR_SIZE = 512,
        CACHE_FILE_XBOX_BITMAP_SIZE_GRANULARITY = 128
    };

    enum CacheFileLimits : Pointer64 {
        CACHE_FILE_MEMORY_LENGTH_PC = 0x1700000,
        CACHE_FILE_MEMORY_LENGTH_XBOX = 0x1600000,
        CACHE_FILE_MEMORY_LENGTH_MCC_CEA = 0x4000000,
        
        CACHE_FILE_MAXIMUM_FILE_LENGTH_MCC_CEA = UINT32_MAX / 2,
        CACHE_FILE_MAXIMUM_FILE_LENGTH_PC = 0x18000000,
        CACHE_FILE_MAXIMUM_FILE_LENGTH_XBOX_USER_INTERFACE = 0x2300000,
        CACHE_FILE_MAXIMUM_FILE_LENGTH_XBOX_SINGLEPLAYER = 0x11600000,
        CACHE_FILE_MAXIMUM_FILE_LENGTH_XBOX_MULTIPLAYER = 0x2F00000,
        CACHE_FILE_MAXIMUM_FILE_LENGTH_NATIVE = UINT64_MAX,

        CACHE_FILE_MAX_TAG_COUNT = 65535,
        
        CACHE_FILE_MEMORY_LENGTH_NATIVE = UINT64_MAX
    };

    struct CacheFileDemoHeader;

    struct CacheFileHeader {
        constexpr const static bool IS_DEMO = false;
        
        LittleEndian<CacheFileLiteral> head_literal;
        LittleEndian<CacheFileEngine> engine;
        LittleEndian<std::uint32_t> decompressed_file_size;
        LittleEndian<std::uint32_t> compressed_padding;
        LittleEndian<std::uint32_t> tag_data_offset;
        LittleEndian<std::uint32_t> tag_data_size;
        PAD(0x8);
        TagString name;
        TagString build;
        LittleEndian<CacheFileType> map_type;
        PAD(0x2);
        LittleEndian<std::uint32_t> crc32;
        PAD(0x794);
        LittleEndian<CacheFileLiteral> foot_literal;

        CacheFileHeader() = default;
        CacheFileHeader(const CacheFileHeader &copy);
        CacheFileHeader(const CacheFileDemoHeader &copy);
        CacheFileHeader &operator =(const CacheFileHeader &copy);

        bool valid() const noexcept;
    };
    static_assert(sizeof(CacheFileHeader) == 0x800);
    
    enum CacheFileHeaderCEAFlags {
        CACHE_FILE_HEADER_CEA_FLAGS_USE_BITMAPS_CACHE = 0b1,
        CACHE_FILE_HEADER_CEA_FLAGS_USE_SOUNDS_CACHE = 0b10,
        CACHE_FILE_HEADER_CEA_FLAGS_CLASSIC_ONLY = 0b100
    };

    struct CacheFileHeaderCEA {
        LittleEndian<CacheFileLiteral> head_literal;
        LittleEndian<CacheFileEngine> engine;
        LittleEndian<std::uint32_t> decompressed_file_size;
        PAD(0x4);
        LittleEndian<std::uint32_t> tag_data_offset;
        LittleEndian<std::uint32_t> tag_data_size;
        PAD(0x8);
        TagString name;
        TagString build;
        LittleEndian<CacheFileType> map_type;
        PAD(0x2);
        LittleEndian<std::uint32_t> crc32;
        LittleEndian<std::uint16_t> flags;
        PAD(0x792);
        LittleEndian<CacheFileLiteral> foot_literal;

        bool valid() const noexcept {
            return reinterpret_cast<const CacheFileHeader *>(this)->valid();
        };
    };
    static_assert(sizeof(CacheFileHeaderCEA) == sizeof(CacheFileHeader));

    struct NativeCacheFileHeader {
        constexpr const static bool IS_DEMO = false;
        
        LittleEndian<CacheFileLiteral> head_literal;
        LittleEndian<CacheFileEngine> engine;
        LittleEndian<std::uint64_t> decompressed_file_size;
        LittleEndian<std::uint64_t> tag_data_offset;
        LittleEndian<std::uint64_t> tag_data_size;
        TagString name;
        TagString build;
        LittleEndian<CacheFileType> map_type;
        PAD(0x2);
        LittleEndian<std::uint32_t> crc32;
        PAD(0x18);
        TagString timestamp;
        PAD(0x75C);
        LittleEndian<CacheFileLiteral> foot_literal;

        bool valid() const noexcept;
    };
    static_assert(sizeof(NativeCacheFileHeader) == sizeof(CacheFileHeader));

    struct CacheFileDemoHeader {
        constexpr const static bool IS_DEMO = true;
        
        PAD(0x2);
        LittleEndian<CacheFileType> map_type;
        PAD(0x2BC);
        LittleEndian<CacheFileLiteral> head_literal;
        LittleEndian<std::uint32_t> tag_data_size;
        TagString build;
        PAD(0x2A0);
        LittleEndian<CacheFileEngine> engine;
        TagString name;
        PAD(0x4);
        LittleEndian<std::uint32_t> crc32;
        PAD(0x34);
        LittleEndian<std::uint32_t> decompressed_file_size;
        LittleEndian<std::uint32_t> tag_data_offset;
        LittleEndian<CacheFileLiteral> foot_literal;
        PAD(0x20C);

        CacheFileDemoHeader() = default;
        CacheFileDemoHeader(const CacheFileDemoHeader &copy);
        CacheFileDemoHeader(const CacheFileHeader &copy);
        CacheFileDemoHeader &operator =(const CacheFileDemoHeader &copy);

        bool valid() const noexcept;
    };
    static_assert(sizeof(CacheFileDemoHeader) == sizeof(CacheFileHeader));

    struct CacheFileTagDataHeader {
        LittleEndian<Pointer> tag_array_address;
        LittleEndian<TagID> scenario_tag;
        LittleEndian<std::uint32_t> tag_file_checksums;
        LittleEndian<std::uint32_t> tag_count;
        LittleEndian<std::uint32_t> model_part_count;
    };
    static_assert(sizeof(CacheFileTagDataHeader) == 0x14);

    struct CacheFileTagDataHeaderPC : CacheFileTagDataHeader {
        LittleEndian<std::uint32_t> model_data_file_offset;
        LittleEndian<std::uint32_t> model_part_count_again;
        LittleEndian<std::uint32_t> vertex_size;
        LittleEndian<std::uint32_t> model_data_size;
        LittleEndian<CacheFileLiteral> tags_literal;
    };
    static_assert(sizeof(CacheFileTagDataHeaderPC) == 0x28);
    
    // The Xbox version accesses vertices indirectly for some reason. Not 100% sure what for yet.
    struct CacheFileModelPartVerticesXbox {
        LittleEndian<std::uint32_t> unknown1;
        LittleEndian<Pointer> vertices;
        LittleEndian<std::uint32_t> unknown2;
    };
    static_assert(sizeof(CacheFileModelPartVerticesXbox) == 0xC);
    
    // I don't know what this is used for. It uses indices directly unlike with vertices.
    struct CacheFileModelPartIndicesXbox {
        LittleEndian<std::uint32_t> unknown1;
        LittleEndian<Pointer> indices;
        LittleEndian<std::uint32_t> unknown2;
    };
    static_assert(sizeof(CacheFileModelPartIndicesXbox) == 0xC);

    struct CacheFileTagDataHeaderXbox : CacheFileTagDataHeader {
        LittleEndian<Pointer> model_part_vertices_address;
        LittleEndian<std::uint32_t> model_part_count_again;
        LittleEndian<Pointer> model_part_indices_address;
        LittleEndian<CacheFileLiteral> tags_literal;
    };
    static_assert(sizeof(CacheFileTagDataHeaderXbox) == 0x24);

    struct NativeCacheFileTagDataHeader : CacheFileTagDataHeader {
        LittleEndian<std::uint64_t> model_data_file_offset;
        LittleEndian<std::uint64_t> vertex_size;
        LittleEndian<std::uint64_t> model_data_size;
        LittleEndian<std::uint64_t> raw_data_indices;
        LittleEndian<CacheFileLiteral> tags_literal;
    };
    static_assert(sizeof(NativeCacheFileTagDataHeader) == 0x38);

    struct CacheFileTagDataTag {
        LittleEndian<TagFourCC> primary_class;
        LittleEndian<TagFourCC> secondary_class;
        LittleEndian<TagFourCC> tertiary_class;
        LittleEndian<TagID> tag_id;
        LittleEndian<Pointer> tag_path;
        LittleEndian<Pointer> tag_data;
        LittleEndian<std::uint32_t> indexed;
        PAD(0x4);
    };
    static_assert(sizeof(CacheFileTagDataTag) == 0x20);

    struct NativeCacheFileTagDataTag {
        LittleEndian<TagFourCC> primary_class;
        LittleEndian<TagFourCC> secondary_class;
        LittleEndian<TagFourCC> tertiary_class;
        LittleEndian<TagID> tag_id;
        LittleEndian<Pointer64> tag_path;
        LittleEndian<Pointer64> tag_data;
    };
    static_assert(sizeof(NativeCacheFileTagDataTag) == 0x20);
}
#endif
