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

    bool CacheFileHeader::valid() const noexcept {
        // Ensure the name and build don't overflow
        if(this->name.overflows() && !this->build.overflows()) {
            return false;
        }

        // Make sure the head/foot things are valid
        if(this->engine == CacheFileEngine::CACHE_FILE_DEMO) {
            if(this->head_literal != CacheFileLiteral::CACHE_FILE_HEAD_DEMO || this->foot_literal != CacheFileLiteral::CACHE_FILE_FOOT_DEMO) {
                return false;
            }
        }
        else {
            if(this->head_literal != CacheFileLiteral::CACHE_FILE_HEAD || this->foot_literal != CacheFileLiteral::CACHE_FILE_FOOT) {
                return false;
            }
        }

        return true;
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

    bool CacheFileDemoHeader::valid() const noexcept {
        return static_cast<CacheFileHeader>(*this).valid();
    }
}
