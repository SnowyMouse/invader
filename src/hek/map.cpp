// SPDX-License-Identifier: GPL-3.0-only

#include "invader/hek/map.hpp"

namespace Invader::HEK {
    CacheFileHeader::CacheFileHeader(const CacheFileDemoHeader &copy) {
        std::fill(reinterpret_cast<std::byte *>(this), reinterpret_cast<std::byte *>(this + 1), static_cast<std::byte>(0));
        this->head_literal = copy.head_literal;
        this->engine = copy.engine;
        this->decompressed_file_size = copy.decompressed_file_size;
        this->tag_data_offset = copy.tag_data_offset;
        this->tag_data_size = copy.tag_data_size;
        this->name = copy.name;
        this->build = copy.build;
        this->map_type = copy.map_type;
        this->crc32 = copy.crc32;
        this->foot_literal = copy.foot_literal;
    }

    CacheFileDemoHeader::CacheFileDemoHeader(const CacheFileHeader &copy) {
        std::fill(reinterpret_cast<std::byte *>(this), reinterpret_cast<std::byte *>(this + 1), static_cast<std::byte>(0));
        this->head_literal = copy.head_literal;
        this->engine = copy.engine;
        this->decompressed_file_size = copy.decompressed_file_size;
        this->tag_data_offset = copy.tag_data_offset;
        this->tag_data_size = copy.tag_data_size;
        this->name = copy.name;
        this->build = copy.build;
        this->map_type = copy.map_type;
        this->crc32 = copy.crc32;
        this->foot_literal = copy.foot_literal;
    }
}
