/*
 * Invader (c) 2018 Kavawuvi
 *
 * This program is free software under the GNU General Public License v3.0 or later. See LICENSE for more information.
 */

#pragma once

#include "data_type.hpp"

namespace Invader::HEK {
    enum CacheFileEngine : std::uint32_t {
        CACHE_FILE_XBOX = 0x5,
        CACHE_FILE_RETAIL = 0x7,
        CACHE_FILE_CUSTOM_EDITION = 0x261
    };

    enum CacheFileType : TagEnum {
        CACHE_FILE_SINGLEPLAYER,
        CACHE_FILE_MULTIPLAYER,
        CACHE_FILE_USER_INTERFACE
    };

    enum CacheFileLiteral : std::uint32_t {
        CACHE_FILE_HEAD = 0x68656164,
        CACHE_FILE_FOOT = 0x666F6F74,
        CACHE_FILE_TAGS = 0x74616773
    };

    enum CacheFileLimits : std::uint64_t {
        CACHE_FILE_MEMORY_LENGTH = 0x1700000,
        CACHE_FILE_MAXIMUM_FILE_LENGTH = 0x100000000,
        CACHE_FILE_MAX_TAG_COUNT = 65535
    };

    enum CacheFileTagDataBaseMemoryAddress : std::uint32_t {
        CACHE_FILE_PC_BASE_MEMORY_ADDRESS = 0x40440000
    };

    struct CacheFileHeader {
        LittleEndian<CacheFileLiteral> head_literal;
        LittleEndian<CacheFileEngine> engine;
        LittleEndian<std::uint32_t> file_size;
        PAD(0x4);
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
    };
    static_assert(sizeof(CacheFileHeader) == 0x800);

    struct CacheFileTagDataHeader {
        LittleEndian<std::uint32_t> tag_array_address;
        LittleEndian<TagID> scenario_tag;
        LittleEndian<std::uint32_t> random_number;
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

    struct CacheFileTagDataTag {
        LittleEndian<TagClassInt> primary_class;
        LittleEndian<TagClassInt> secondary_class;
        LittleEndian<TagClassInt> tertiary_class;
        LittleEndian<TagID> tag_id;
        LittleEndian<std::uint32_t> tag_path;
        LittleEndian<std::uint32_t> tag_data;
        LittleEndian<std::uint32_t> indexed;
        PAD(0x4);
    };
    static_assert(sizeof(CacheFileTagDataTag) == 0x20);
}
