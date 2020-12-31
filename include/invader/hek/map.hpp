// SPDX-License-Identifier: GPL-3.0-only

#ifndef INVADER__HEK__MAP_HPP
#define INVADER__HEK__MAP_HPP

#include <cstdint>
#include "data_type.hpp"
#include "../tag/hek/definition.hpp"

namespace Invader::HEK {
    enum CacheFileEngine : std::uint32_t {
        CACHE_FILE_XBOX = 0x5,
        CACHE_FILE_DEMO = 0x6,
        CACHE_FILE_RETAIL = 0x7,
        CACHE_FILE_CUSTOM_EDITION = 0x261,
        CACHE_FILE_NATIVE = 0x1A86,

        CACHE_FILE_DEMO_COMPRESSED = 0x861A0006,
        CACHE_FILE_RETAIL_COMPRESSED = 0x861A0007,
        CACHE_FILE_CUSTOM_EDITION_COMPRESSED = 0x861A0261
    };

    const char *engine_name(CacheFileEngine engine) noexcept;

    using CacheFileType = ScenarioType;
    const char *type_name(CacheFileType type) noexcept;

    enum CacheFileLiteral : std::uint32_t {
        CACHE_FILE_HEAD = 0x68656164,
        CACHE_FILE_FOOT = 0x666F6F74,
        CACHE_FILE_TAGS = 0x74616773,
        CACHE_FILE_HEAD_DEMO = 0x45686564,
        CACHE_FILE_FOOT_DEMO = 0x47666F74
    };

    enum CacheFileTagDataBaseMemoryAddress : Pointer64 {
        CACHE_FILE_PC_BASE_MEMORY_ADDRESS = 0x40440000,
        CACHE_FILE_DEMO_BASE_MEMORY_ADDRESS = 0x4BF10000,
        CACHE_FILE_NATIVE_BASE_MEMORY_ADDRESS = 0x00000000,
        CACHE_FILE_XBOX_BASE_MEMORY_ADDRESS = 0x803A6000,

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
        CACHE_FILE_MEMORY_LENGTH_NATIVE = UINT64_MAX,
        
        CACHE_FILE_MAXIMUM_FILE_LENGTH_PC = 0x18000000,
        CACHE_FILE_MAXIMUM_FILE_LENGTH_XBOX_USER_INTERFACE = 0x2300000,
        CACHE_FILE_MAXIMUM_FILE_LENGTH_XBOX_SINGLEPLAYER = 0x11600000,
        CACHE_FILE_MAXIMUM_FILE_LENGTH_XBOX_MULTIPLAYER = 0x2F00000,
        CACHE_FILE_MAXIMUM_FILE_LENGTH_NATIVE = UINT64_MAX,

        CACHE_FILE_MAX_TAG_COUNT = 65535
    };

    struct CacheFileDemoHeader;

    struct CacheFileHeader {
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

    struct NativeCacheFileHeader {
        enum NativeCacheFileCompressionType : TagEnum {
            NATIVE_CACHE_FILE_COMPRESSION_UNCOMPRESSED = 0,
            NATIVE_CACHE_FILE_COMPRESSION_ZSTD = 1
        };
        
        LittleEndian<CacheFileLiteral> head_literal;
        LittleEndian<CacheFileEngine> engine;
        LittleEndian<std::uint64_t> decompressed_file_size;
        LittleEndian<std::uint64_t> tag_data_offset;
        LittleEndian<std::uint64_t> tag_data_size;
        TagString name;
        TagString build;
        LittleEndian<CacheFileType> map_type;
        LittleEndian<NativeCacheFileCompressionType> compression_type;
        LittleEndian<std::uint32_t> crc32;
        PAD(0x18);
        TagString timestamp;
        PAD(0x75C);
        LittleEndian<CacheFileLiteral> foot_literal;

        bool valid() const noexcept;
    };
    static_assert(sizeof(NativeCacheFileHeader) == 0x800);

    struct CacheFileDemoHeader {
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
    static_assert(sizeof(CacheFileDemoHeader) == 0x800);

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
        LittleEndian<TagClassInt> primary_class;
        LittleEndian<TagClassInt> secondary_class;
        LittleEndian<TagClassInt> tertiary_class;
        LittleEndian<TagID> tag_id;
        LittleEndian<Pointer> tag_path;
        LittleEndian<Pointer> tag_data;
        LittleEndian<std::uint32_t> indexed;
        PAD(0x4);
    };
    static_assert(sizeof(CacheFileTagDataTag) == 0x20);

    struct NativeCacheFileTagDataTag {
        LittleEndian<TagClassInt> primary_class;
        LittleEndian<TagClassInt> secondary_class;
        LittleEndian<TagClassInt> tertiary_class;
        LittleEndian<TagID> tag_id;
        LittleEndian<Pointer64> tag_path;
        LittleEndian<Pointer64> tag_data;
    };
    static_assert(sizeof(NativeCacheFileTagDataTag) == 0x20);
}
#endif
