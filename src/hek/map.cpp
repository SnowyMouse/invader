// SPDX-License-Identifier: GPL-3.0-only

#include <invader/hek/map.hpp>
#include <invader/version.hpp>
#include <variant>
#include <optional>

namespace Invader::HEK {
    static constexpr const Pointer64 GEARBOX_TAG_SPACE_LENGTH = (23 * 1024 * 1024);
    static constexpr const Pointer64 GEARBOX_MAX_FILE_SIZE = (384 * 1024 * 1024);
    static constexpr const Pointer64 GEARBOX_BASE_MEMORY_ADDRESS = 0x40440000;
    static constexpr const Pointer64 XBOX_BASE_MEMORY_ADDRESS = 0x803A6000;
    static constexpr const Pointer64 XBOX_TAG_SPACE_LENGTH = (22 * 1024 * 1024);
    
    static constexpr Pointer64 xbox_max_file_size(CacheFileType type) {
        switch(type) {
            case CacheFileType::SCENARIO_TYPE_SINGLEPLAYER:
                return 278 * 1024 * 1024;
            case CacheFileType::SCENARIO_TYPE_MULTIPLAYER:
                return 47 * 1024 * 1024;
            case CacheFileType::SCENARIO_TYPE_USER_INTERFACE:
                return 35 * 1024 * 1024;
            case CacheFileType::SCENARIO_TYPE_ENUM_COUNT:
                break;
        }
        std::terminate();
    }
    
    static constexpr const GameEngineInfo engine_infos[] = {
        { 
            .name = "Invader",
            .engine = GameEngine::GAME_ENGINE_NATIVE,
            .cache_version = CacheFileEngine::CACHE_FILE_NATIVE,
            .build_string = full_version,
            .build_string_is_enforced = true,
            .base_memory_address = 0,
            .tag_space_length = UINT64_MAX,
            .maximum_file_size = UINT64_MAX
        },
        { 
            .name = "Halo: Combat Evolved Anniversary (MCC)",
            .engine = GameEngine::GAME_ENGINE_MCC_COMBAT_EVOLVED_ANNIVERSARY,
            .cache_version = CacheFileEngine::CACHE_FILE_MCC_CEA,
            .build_string = "01.03.43.0000",
            .build_string_is_enforced = false,
            .base_memory_address = 0x50000000,
            .tag_space_length = 64 * 1024 * 1024,
            .maximum_file_size = static_cast<Pointer64>(INT32_MAX),
            .base_memory_address_is_inferred = true
        },
        {
            .name = "Halo Demo / Trial (Gearbox)",
            .engine = GameEngine::GAME_ENGINE_GEARBOX_DEMO,
            .cache_version = CacheFileEngine::CACHE_FILE_DEMO,
            .build_string = "01.00.00.0576",
            .build_string_is_enforced = false,
            .base_memory_address = 0x4BF10000,
            .tag_space_length = GEARBOX_TAG_SPACE_LENGTH,
            .maximum_file_size = GEARBOX_MAX_FILE_SIZE
        },
        {
            .name = "Halo Custom Edition (Gearbox)",
            .engine = GameEngine::GAME_ENGINE_GEARBOX_CUSTOM_EDITION,
            .cache_version = CacheFileEngine::CACHE_FILE_CUSTOM_EDITION,
            .build_string = "01.00.00.0609",
            .build_string_is_enforced = false,
            .base_memory_address = GEARBOX_BASE_MEMORY_ADDRESS,
            .tag_space_length = GEARBOX_TAG_SPACE_LENGTH,
            .maximum_file_size = GEARBOX_MAX_FILE_SIZE
        },
        {
            .name = "Halo: Combat Evolved (Gearbox)",
            .engine = GameEngine::GAME_ENGINE_GEARBOX_RETAIL,
            .cache_version = CacheFileEngine::CACHE_FILE_RETAIL,
            .build_string = "01.00.00.0564",
            .build_string_is_enforced = false,
            .base_memory_address = GEARBOX_BASE_MEMORY_ADDRESS,
            .tag_space_length = GEARBOX_TAG_SPACE_LENGTH,
            .maximum_file_size = GEARBOX_MAX_FILE_SIZE
        },
        {
            .name = "Halo: Combat Evolved (Xbox NTSC-US)",
            .engine = GameEngine::GAME_ENGINE_XBOX_NTSC_US,
            .cache_version = CacheFileEngine::CACHE_FILE_XBOX,
            .build_string = "01.10.12.2276",
            .build_string_is_enforced = true,
            .base_memory_address = XBOX_BASE_MEMORY_ADDRESS,
            .tag_space_length = XBOX_TAG_SPACE_LENGTH,
            .maximum_file_size = xbox_max_file_size
        },
        {
            .name = "Halo: Combat Evolved (Xbox NTSC-JP)",
            .engine = GameEngine::GAME_ENGINE_XBOX_NTSC_JP,
            .cache_version = CacheFileEngine::CACHE_FILE_XBOX,
            .build_string = "01.03.14.0009",
            .build_string_is_enforced = true,
            .base_memory_address = XBOX_BASE_MEMORY_ADDRESS,
            .tag_space_length = static_cast<Pointer64>(XBOX_TAG_SPACE_LENGTH + 288 * 1024),
            .maximum_file_size = xbox_max_file_size
        },
        {
            .name = "Halo: Combat Evolved (Xbox NTSC-TW)",
            .engine = GameEngine::GAME_ENGINE_XBOX_NTSC_TW,
            .cache_version = CacheFileEngine::CACHE_FILE_XBOX,
            .build_string = "01.12.09.0135",
            .build_string_is_enforced = true,
            .base_memory_address = XBOX_BASE_MEMORY_ADDRESS,
            .tag_space_length = static_cast<Pointer64>(XBOX_TAG_SPACE_LENGTH + 500 * 1024),
            .maximum_file_size = xbox_max_file_size
        },
        {
            .name = "Halo: Combat Evolved (Xbox PAL)",
            .engine = GameEngine::GAME_ENGINE_XBOX_PAL,
            .cache_version = CacheFileEngine::CACHE_FILE_XBOX,
            .build_string = "01.01.14.2342",
            .build_string_is_enforced = true,
            .base_memory_address = XBOX_BASE_MEMORY_ADDRESS,
            .tag_space_length = XBOX_TAG_SPACE_LENGTH,
            .maximum_file_size = xbox_max_file_size,
            .tick_rate = 25.0
        }
    };
    
    static constexpr bool engine_info_error_check() {
        return true;
    }
    static_assert(engine_info_error_check());
    
    const GameEngineInfo &GameEngineInfo::get_game_engine_info(GameEngine engine) noexcept {
        for(auto &e : engine_infos) {
            if(e.engine == engine) {
                return e;
            }
        }
        std::terminate();
    }
    
    const GameEngineInfo *GameEngineInfo::get_game_engine_info(CacheFileEngine cache_version, const char *build_string) noexcept {
        for(auto &e : engine_infos) {
            if(e.cache_version == cache_version && (!e.build_string_is_enforced || std::strcmp(e.get_build_string(), build_string) == 0)) {
                return &e;
            }
        }
        return nullptr;
    }
    
    const char *GameEngineInfo::get_build_string() const noexcept {
        // If it's a string pointer...
        if(const char * const *build_string_maybe = std::get_if<const char *>(&this->build_string)) {
            return *build_string_maybe;
        }
        
        // Or maybe it's a function that gets a string pointer?
        else if(const char *(*const *build_string_maybe)() = std::get_if<const char *(*)()>(&this->build_string)) {
            return (*build_string_maybe)();
        }
        
        else {
            std::terminate();
        }
    }
    
    Pointer64 GameEngineInfo::get_maximum_file_size(CacheFileType type) const noexcept {
        // If it's simple size
        if(const Pointer64 *max_file_size_maybe = std::get_if<Pointer64>(&this->maximum_file_size)) {
            return *max_file_size_maybe;
        }
        
        // Or maybe it's a function that takes a file type?
        else if(Pointer64 (*const *max_file_size_maybe)(CacheFileType) = std::get_if<Pointer64 (*)(CacheFileType)>(&this->maximum_file_size)) {
            return (*max_file_size_maybe)(type);
        }
        
        else {
            std::terminate();
        }
    }
    
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
