// SPDX-License-Identifier: GPL-3.0-only

#include <invader/hek/map.hpp>

namespace Invader::HEK {
    const char *engine_name(CacheFileEngine engine) noexcept {
        switch(engine) {
            case CacheFileEngine::CACHE_FILE_NATIVE:
                return "Invader (native)";
            case CacheFileEngine::CACHE_FILE_CUSTOM_EDITION:
                return "Halo Custom Edition";
            case CacheFileEngine::CACHE_FILE_RETAIL:
                return "Halo: Combat Evolved (PC)";
            case CacheFileEngine::CACHE_FILE_XBOX:
                return "Halo: Combat Evolved (Xbox)";
            case CacheFileEngine::CACHE_FILE_DEMO:
                return "Halo Demo / Trial";
            case CacheFileEngine::CACHE_FILE_MCC_CEA:
                return "Halo: Combat Evolved Anniversary (MCC)";
            default:
                return "Unknown";
        }
    }

    const char *type_name(CacheFileType type) noexcept {
        switch(type) {
            case CacheFileType::SCENARIO_TYPE_MULTIPLAYER:
                return "Multiplayer";
            case CacheFileType::SCENARIO_TYPE_SINGLEPLAYER:
                return "Singleplayer";
            case CacheFileType::SCENARIO_TYPE_USER_INTERFACE:
                return "User interface";
            default:
                return "Unknown";
        }
    }

    #define PERFORM_COPY std::fill(reinterpret_cast<std::byte *>(this), reinterpret_cast<std::byte *>(this + 1), static_cast<std::byte>(0)); \
                         this->head_literal = copy.head_literal; \
                         this->engine = copy.engine; \
                         this->decompressed_file_size = copy.decompressed_file_size; \
                         this->tag_data_offset = copy.tag_data_offset; \
                         this->tag_data_size = copy.tag_data_size; \
                         this->name = copy.name; \
                         this->build = copy.build; \
                         this->map_type = copy.map_type; \
                         this->crc32 = copy.crc32; \
                         this->foot_literal = copy.foot_literal;

    CacheFileHeader::CacheFileHeader(const CacheFileHeader &copy) {
        PERFORM_COPY
    }

    CacheFileHeader::CacheFileHeader(const CacheFileDemoHeader &copy) {
        PERFORM_COPY
    }

    CacheFileHeader &CacheFileHeader::operator =(const CacheFileHeader &copy) {
        PERFORM_COPY
        return *this;
    }

    bool CacheFileHeader::valid() const noexcept {
        // Ensure the name and build don't overflow
        if(this->name.overflows() || this->build.overflows()) {
            return false;
        }
        
        // Make sure the head/foot things are valid
        if(this->engine == CacheFileEngine::CACHE_FILE_DEMO) {
            return this->head_literal == CacheFileLiteral::CACHE_FILE_HEAD_DEMO && this->foot_literal == CacheFileLiteral::CACHE_FILE_FOOT_DEMO;
        }
        else {
            return this->head_literal == CacheFileLiteral::CACHE_FILE_HEAD && this->foot_literal == CacheFileLiteral::CACHE_FILE_FOOT;
        }
    }

    bool NativeCacheFileHeader::valid() const noexcept {
        // Ensure the name and build don't overflow
        if(this->name.overflows() || this->build.overflows()) {
            return false;
        }

        // Make sure head/foot things are valid
        if(this->head_literal != CacheFileLiteral::CACHE_FILE_HEAD || this->foot_literal != CacheFileLiteral::CACHE_FILE_FOOT) {
            return false;
        }

        return true;
    }

    CacheFileDemoHeader::CacheFileDemoHeader(const CacheFileHeader &copy) {
        PERFORM_COPY
    }

    CacheFileDemoHeader::CacheFileDemoHeader(const CacheFileDemoHeader &copy) {
        PERFORM_COPY
    }

    CacheFileDemoHeader &CacheFileDemoHeader::operator =(const CacheFileDemoHeader &copy) {
        PERFORM_COPY
        return *this;
    }

    bool CacheFileDemoHeader::valid() const noexcept {
        try {
            return static_cast<CacheFileHeader>(*this).valid();
        }
        catch(std::exception &) {
            return false;
        }
    }
}
