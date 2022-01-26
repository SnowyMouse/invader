// SPDX-License-Identifier: GPL-3.0-only

#include <invader/hek/map.hpp>
#include <invader/version.hpp>
#include <variant>
#include <optional>
#include <array>

namespace Invader::HEK {
    static constexpr const Pointer64 GEARBOX_TAG_SPACE_LENGTH = (23 * 1024 * 1024);
    static constexpr const Pointer64 GEARBOX_MAX_FILE_SIZE = (384 * 1024 * 1024);
    static constexpr const Pointer64 GEARBOX_BASE_MEMORY_ADDRESS = 0x40440000;
    static constexpr const Pointer64 XBOX_BASE_MEMORY_ADDRESS = 0x803A6000;
    static constexpr const Pointer64 XBOX_TAG_SPACE_LENGTH = (22 * 1024 * 1024);
    
    template<Pointer64 singleplayer_size, Pointer64 multiplayer_size, Pointer64 user_interface_size> static constexpr Pointer64 max_file_size_fn(CacheFileType type) {
        switch(type) {
            case CacheFileType::SCENARIO_TYPE_SINGLEPLAYER:
                return singleplayer_size;
            case CacheFileType::SCENARIO_TYPE_MULTIPLAYER:
                return multiplayer_size;
            case CacheFileType::SCENARIO_TYPE_USER_INTERFACE:
                return user_interface_size;
            case CacheFileType::SCENARIO_TYPE_ENUM_COUNT:
                break;
        }
        std::terminate();
    }
    
    #define xbox_max_file_size max_file_size_fn<static_cast<Pointer64>(278 * 1024 * 1024), static_cast<Pointer64>(47 * 1024 * 1024), static_cast<Pointer64>(35 * 1024 * 1024)>
    #define xbox_demo_max_file_size max_file_size_fn<static_cast<Pointer64>(215 * 1024 * 1024), static_cast<Pointer64>(INT32_MAX), static_cast<Pointer64>(23 * 1024 * 1024)>
    
    #define TAG(path,fourcc) GameEngineInfo::RequiredTags::TagPairPtr {path, HEK::TagFourCC::fourcc}
    
    #define GLOBALS_TAG TAG("globals\\globals", TAG_FOURCC_GLOBALS)
    
    static constexpr std::array INVADER_REQUIRED_TAGS = {GLOBALS_TAG};
    
    #define XBOX_REQUIRED_TAGS_ALL GLOBALS_TAG, TAG("ui\\shell\\bitmaps\\white", TAG_FOURCC_BITMAP), TAG("ui\\multiplayer_game_text", TAG_FOURCC_UNICODE_STRING_LIST)
    
    static constexpr std::array XBOX_REQUIRED_TAGS_ALL_2276 = { XBOX_REQUIRED_TAGS_ALL };
    static constexpr std::array XBOX_REQUIRED_TAGS_ALL_EVERYTHING_ELSE = { XBOX_REQUIRED_TAGS_ALL, TAG("ui\\shell\\strings\\temp_strings", TAG_FOURCC_UNICODE_STRING_LIST) };
    
    static constexpr std::array XBOX_REQUIRED_TAGS_SP_FULL = { TAG("ui\\shell\\solo", TAG_FOURCC_UI_WIDGET_COLLECTION) };
    static constexpr std::array XBOX_REQUIRED_TAGS_SP_DEMO = { TAG("ui\\shell\\solo_demo", TAG_FOURCC_UI_WIDGET_COLLECTION) };
    static constexpr std::array XBOX_REQUIRED_TAGS_MP = { TAG("ui\\shell\\multiplayer", TAG_FOURCC_UI_WIDGET_COLLECTION) };
    static constexpr std::array XBOX_REQUIRED_TAGS_UI = {
        TAG("ui\\default_multiplayer_game_setting_names", TAG_FOURCC_UNICODE_STRING_LIST),
        TAG("ui\\saved_game_file_strings", TAG_FOURCC_UNICODE_STRING_LIST),
        TAG("ui\\multiplayer_scenarios", TAG_FOURCC_MULTIPLAYER_SCENARIO_DESCRIPTION),
        TAG("ui\\random_player_names", TAG_FOURCC_UNICODE_STRING_LIST),
        TAG("ui\\english", TAG_FOURCC_VIRTUAL_KEYBOARD),
        TAG("ui\\shell\\strings\\default_player_profile_names", TAG_FOURCC_UNICODE_STRING_LIST),
        TAG("ui\\shell\\strings\\game_variant_descriptions", TAG_FOURCC_UNICODE_STRING_LIST),
        TAG("ui\\shell\\main_menu\\player_profiles_select\\joystick_set_short_descriptions", TAG_FOURCC_UNICODE_STRING_LIST),
        TAG("ui\\shell\\main_menu\\player_profiles_select\\button_set_short_descriptions", TAG_FOURCC_UNICODE_STRING_LIST),
        TAG("ui\\shell\\main_menu\\player_profiles_select\\button_set_long_descriptions", TAG_FOURCC_UNICODE_STRING_LIST),
        TAG("sound\\music\\title1\\title1", TAG_FOURCC_SOUND_LOOPING),
        TAG("sound\\sfx\\ui\\flag_failure", TAG_FOURCC_SOUND),
        TAG("sound\\sfx\\ui\\cursor", TAG_FOURCC_SOUND),
    };
    static constexpr std::array XBOX_REQUIRED_TAGS_UI_DEMO = { TAG("ui\\shell\\main_menu_demo", TAG_FOURCC_UI_WIDGET_COLLECTION) };
    static constexpr std::array XBOX_REQUIRED_TAGS_UI_FULL = { TAG("ui\\shell\\main_menu", TAG_FOURCC_UI_WIDGET_COLLECTION) };
    
    #define USE_TAG_ARRAY(tag_array) { tag_array.data(), tag_array.size() }
    #define XBOX_BOILERPLATE(REQUIRED_TAGS) \
        .required_tags = { \
            .all = USE_TAG_ARRAY(REQUIRED_TAGS), \
            .singleplayer_demo = USE_TAG_ARRAY(XBOX_REQUIRED_TAGS_SP_DEMO), \
            .singleplayer_full = USE_TAG_ARRAY(XBOX_REQUIRED_TAGS_SP_FULL), \
            .multiplayer = USE_TAG_ARRAY(XBOX_REQUIRED_TAGS_MP), \
            .user_interface = USE_TAG_ARRAY(XBOX_REQUIRED_TAGS_UI), \
            .user_interface_demo = USE_TAG_ARRAY(XBOX_REQUIRED_TAGS_UI_DEMO), \
            .user_interface_full = USE_TAG_ARRAY(XBOX_REQUIRED_TAGS_UI_FULL) \
        }
    
    static constexpr std::array PC_REQUIRED_TAGS = {
        GLOBALS_TAG,
        TAG("ui\\ui_tags_loaded_all_scenario_types", TAG_FOURCC_TAG_COLLECTION),
        TAG("sound\\sfx\\ui\\cursor", TAG_FOURCC_SOUND),
        TAG("sound\\sfx\\ui\\back", TAG_FOURCC_SOUND),
        TAG("sound\\sfx\\ui\\flag_failure", TAG_FOURCC_SOUND),
        // WHY ARE THESE IN SINGLEPLAYER?
        TAG("ui\\shell\\main_menu\\mp_map_list", TAG_FOURCC_UNICODE_STRING_LIST),
        TAG("ui\\shell\\strings\\loading", TAG_FOURCC_UNICODE_STRING_LIST),
        TAG("ui\\shell\\bitmaps\\trouble_brewing", TAG_FOURCC_BITMAP),
        TAG("ui\\shell\\bitmaps\\background", TAG_FOURCC_BITMAP)
    };
    static constexpr std::array PC_REQUIRED_TAGS_SP = { TAG("ui\\ui_tags_loaded_solo_scenario_type", TAG_FOURCC_TAG_COLLECTION) };
    static constexpr std::array PC_REQUIRED_TAGS_MP = { TAG("ui\\ui_tags_loaded_multiplayer_scenario_type", TAG_FOURCC_TAG_COLLECTION) };
    static constexpr std::array PC_REQUIRED_TAGS_UI = { TAG("ui\\ui_tags_loaded_mainmenu_scenario_type", TAG_FOURCC_TAG_COLLECTION) };
    
    static constexpr std::uint16_t MCC_CEA_MAXIMUM_SCENARIO_SCRIPT_NODES = static_cast<std::uint16_t>(INT16_MAX);
    static constexpr std::uint16_t ORIGINAL_MAXIMUM_SCENARIO_SCRIPT_NODES = 19001;
    
    #define PC_BOILERPLATE \
        .required_tags = { \
            .all = USE_TAG_ARRAY(PC_REQUIRED_TAGS), \
            .singleplayer = USE_TAG_ARRAY(PC_REQUIRED_TAGS_SP), \
            .multiplayer = USE_TAG_ARRAY(PC_REQUIRED_TAGS_MP), \
            .user_interface = USE_TAG_ARRAY(PC_REQUIRED_TAGS_UI) \
        }
    
    static constexpr const GameEngineInfo engine_infos[] = {
        { 
            .name = "Invader (Native)",
            .shorthand = "native",
            .engine = GameEngine::GAME_ENGINE_NATIVE,
            .cache_version = CacheFileEngine::CACHE_FILE_NATIVE,
            .build_string = full_version,
            .build_string_is_enforced = true,
            .base_memory_address = 0,
            .tag_space_length = UINT64_MAX,
            .maximum_file_size = UINT64_MAX,
            .maximum_scenario_script_nodes = MCC_CEA_MAXIMUM_SCENARIO_SCRIPT_NODES,
            .bsps_occupy_tag_space = false,
            .required_tags = {
                .all = USE_TAG_ARRAY(INVADER_REQUIRED_TAGS)
            }
        },
        { 
            .name = "Halo: Combat Evolved Anniversary (MCC)",
            .shorthand = "mcc-cea",
            .engine = GameEngine::GAME_ENGINE_MCC_COMBAT_EVOLVED_ANNIVERSARY,
            .cache_version = CacheFileEngine::CACHE_FILE_MCC_CEA,
            .build_string = "01.03.43.0000",
            .build_string_is_enforced = false,
            .base_memory_address = 0x50000000,
            .base_memory_address_is_inferred = true,
            .tag_space_length = 64 * 1024 * 1024,
            .maximum_file_size = static_cast<Pointer64>(INT32_MAX),
            .maximum_scenario_script_nodes = MCC_CEA_MAXIMUM_SCENARIO_SCRIPT_NODES,
            .scenario_script_compile_target = RIAT_CompileTarget::RIAT_COMPILE_TARGET_MCC_CEA,
            .scenario_name_and_file_name_must_be_equal = false,
            .supports_external_bitmaps_map = true,
            PC_BOILERPLATE
        },
        {
            .name = "Halo Demo / Trial (Gearbox)",
            .shorthand = "gbx-demo",
            .engine = GameEngine::GAME_ENGINE_GEARBOX_DEMO,
            .cache_version = CacheFileEngine::CACHE_FILE_DEMO,
            .build_string = "01.00.00.0576",
            .build_string_is_enforced = false,
            .base_memory_address = 0x4BF10000,
            .tag_space_length = GEARBOX_TAG_SPACE_LENGTH,
            .maximum_file_size = GEARBOX_MAX_FILE_SIZE,
            .maximum_scenario_script_nodes = ORIGINAL_MAXIMUM_SCENARIO_SCRIPT_NODES,
            .scenario_script_compile_target = RIAT_CompileTarget::RIAT_COMPILE_TARGET_GEARBOX_DEMO,
            .supports_external_bitmaps_map = true,
            .supports_external_sounds_map = true,
            PC_BOILERPLATE
        },
        {
            .name = "Halo Custom Edition (Gearbox)",
            .shorthand = "gbx-custom",
            .engine = GameEngine::GAME_ENGINE_GEARBOX_CUSTOM_EDITION,
            .cache_version = CacheFileEngine::CACHE_FILE_CUSTOM_EDITION,
            .build_string = "01.00.00.0609",
            .build_string_is_enforced = false,
            .base_memory_address = GEARBOX_BASE_MEMORY_ADDRESS,
            .tag_space_length = GEARBOX_TAG_SPACE_LENGTH,
            .maximum_file_size = GEARBOX_MAX_FILE_SIZE,
            .maximum_scenario_script_nodes = ORIGINAL_MAXIMUM_SCENARIO_SCRIPT_NODES,
            .scenario_script_compile_target = RIAT_CompileTarget::RIAT_COMPILE_TARGET_GEARBOX_CUSTOM_EDITION,
            .supports_external_bitmaps_map = true,
            .supports_external_sounds_map = true,
            .supports_external_loc_map = true,
            .uses_indexing = true,
            PC_BOILERPLATE
        },
        {
            .name = "Halo: Combat Evolved (Gearbox)",
            .shorthand = "gbx-retail",
            .engine = GameEngine::GAME_ENGINE_GEARBOX_RETAIL,
            .cache_version = CacheFileEngine::CACHE_FILE_RETAIL,
            .build_string = "01.00.00.0564",
            .build_string_is_enforced = false,
            .base_memory_address = GEARBOX_BASE_MEMORY_ADDRESS,
            .tag_space_length = GEARBOX_TAG_SPACE_LENGTH,
            .maximum_file_size = GEARBOX_MAX_FILE_SIZE,
            .maximum_scenario_script_nodes = ORIGINAL_MAXIMUM_SCENARIO_SCRIPT_NODES,
            .scenario_script_compile_target = RIAT_CompileTarget::RIAT_COMPILE_TARGET_GEARBOX_RETAIL,
            .supports_external_bitmaps_map = true,
            .supports_external_sounds_map = true,
            PC_BOILERPLATE
        },
        {
            .name = "Halo: Combat Evolved (Xbox NTSC-US)",
            .shorthand = "xbox-ntsc",
            .engine = GameEngine::GAME_ENGINE_XBOX_NTSC_US,
            .cache_version = CacheFileEngine::CACHE_FILE_XBOX,
            .build_string = "01.10.12.2276",
            .build_string_is_enforced = true,
            .base_memory_address = XBOX_BASE_MEMORY_ADDRESS,
            .tag_space_length = XBOX_TAG_SPACE_LENGTH,
            .maximum_file_size = xbox_max_file_size,
            
            .maximum_scenario_script_nodes = ORIGINAL_MAXIMUM_SCENARIO_SCRIPT_NODES,
            .scenario_script_compile_target = RIAT_CompileTarget::RIAT_COMPILE_TARGET_XBOX,
            .uses_compression = true,
            
            XBOX_BOILERPLATE(XBOX_REQUIRED_TAGS_ALL_2276)
        },
        {
            .name = "Halo: Combat Evolved (Xbox NTSC-JP)",
            .shorthand = "xbox-ntsc-jp",
            .engine = GameEngine::GAME_ENGINE_XBOX_NTSC_JP,
            .cache_version = CacheFileEngine::CACHE_FILE_XBOX,
            .build_string = "01.03.14.0009",
            .build_string_is_enforced = true,
            .base_memory_address = XBOX_BASE_MEMORY_ADDRESS,
            .tag_space_length = static_cast<Pointer64>(XBOX_TAG_SPACE_LENGTH + 288 * 1024),
            .maximum_file_size = xbox_max_file_size,
            
            .maximum_scenario_script_nodes = ORIGINAL_MAXIMUM_SCENARIO_SCRIPT_NODES,
            .scenario_script_compile_target = RIAT_CompileTarget::RIAT_COMPILE_TARGET_XBOX,
            .uses_compression = true,
            
            XBOX_BOILERPLATE(XBOX_REQUIRED_TAGS_ALL_EVERYTHING_ELSE)
        },
        {
            .name = "Halo: Combat Evolved (Xbox NTSC-TW)",
            .shorthand = "xbox-ntsc-tw",
            .engine = GameEngine::GAME_ENGINE_XBOX_NTSC_TW,
            .cache_version = CacheFileEngine::CACHE_FILE_XBOX,
            .build_string = "01.12.09.0135",
            .build_string_is_enforced = true,
            .base_memory_address = XBOX_BASE_MEMORY_ADDRESS,
            .tag_space_length = static_cast<Pointer64>(XBOX_TAG_SPACE_LENGTH + 500 * 1024),
            .maximum_file_size = xbox_max_file_size,
            
            .maximum_scenario_script_nodes = ORIGINAL_MAXIMUM_SCENARIO_SCRIPT_NODES,
            .scenario_script_compile_target = RIAT_CompileTarget::RIAT_COMPILE_TARGET_XBOX,
            .uses_compression = true,
            
            XBOX_BOILERPLATE(XBOX_REQUIRED_TAGS_ALL_EVERYTHING_ELSE)
        },
        {
            .name = "Halo: Combat Evolved (Xbox PAL)",
            .shorthand = "xbox-pal",
            .engine = GameEngine::GAME_ENGINE_XBOX_PAL,
            .cache_version = CacheFileEngine::CACHE_FILE_XBOX,
            .build_string = "01.01.14.2342",
            .build_string_is_enforced = true,
            .base_memory_address = XBOX_BASE_MEMORY_ADDRESS,
            .tag_space_length = XBOX_TAG_SPACE_LENGTH,
            .maximum_file_size = xbox_max_file_size,
            
            .maximum_scenario_script_nodes = ORIGINAL_MAXIMUM_SCENARIO_SCRIPT_NODES,
            .scenario_script_compile_target = RIAT_CompileTarget::RIAT_COMPILE_TARGET_XBOX,
            .uses_compression = true,
            
            XBOX_BOILERPLATE(XBOX_REQUIRED_TAGS_ALL_EVERYTHING_ELSE)
        },
        {
            .name = "Untracked cache file (Xbox)",
            .engine = GameEngine::GAME_ENGINE_XBOX_GENERIC,
            .cache_version = CacheFileEngine::CACHE_FILE_XBOX,
            .build_string = "",
            .build_string_is_enforced = false,
            .base_memory_address = XBOX_BASE_MEMORY_ADDRESS,
            .base_memory_address_is_inferred = true,
            .tag_space_length = XBOX_TAG_SPACE_LENGTH,
            .maximum_file_size = static_cast<Pointer64>(INT32_MAX),
            
            .maximum_scenario_script_nodes = ORIGINAL_MAXIMUM_SCENARIO_SCRIPT_NODES,
            .scenario_script_compile_target = RIAT_CompileTarget::RIAT_COMPILE_TARGET_XBOX,
            .uses_compression = true
        },
        {
            .name = "Halo: Combat Evolved (Xbox OXM Demo)",
            .shorthand = "xbox-demo",
            .engine = GameEngine::GAME_ENGINE_XBOX_DEMO,
            .cache_version = CacheFileEngine::CACHE_FILE_XBOX,
            .build_string = "",
            .build_string_is_enforced = true,
            .base_memory_address = XBOX_BASE_MEMORY_ADDRESS,
            .tag_space_length = XBOX_TAG_SPACE_LENGTH,
            .maximum_file_size = xbox_demo_max_file_size,
            
            .maximum_scenario_script_nodes = ORIGINAL_MAXIMUM_SCENARIO_SCRIPT_NODES,
            .scenario_script_compile_target = RIAT_CompileTarget::RIAT_COMPILE_TARGET_XBOX,
            .uses_compression = true,
            
            XBOX_BOILERPLATE(XBOX_REQUIRED_TAGS_ALL_EVERYTHING_ELSE)
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
    
    const GameEngineInfo *GameEngineInfo::get_game_engine_info(const char *shorthand) noexcept {
        for(auto &e : engine_infos) {
            if(e.shorthand != nullptr && std::strcmp(e.shorthand, shorthand) == 0) {
                return &e;
            }
        }
        return nullptr;
    }
    
    const GameEngineInfo *GameEngineInfo::get_game_engine_info(CacheFileEngine cache_version, const char *build_string) noexcept {
        const GameEngineInfo *info = nullptr;
        
        for(auto &e : engine_infos) {
            if(e.cache_version == cache_version) {
                // Exact match - we can return this!
                if(std::strcmp(e.get_build_string(), build_string) == 0) {
                    return &e;
                }
                
                // Hold onto this for now (matching cache version, but not build string, however build string is not enforced)
                else if(!e.build_string_is_enforced) {
                    info = &e;
                }
            }
        }
        
        return info;
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
    
    std::list<const char *> GameEngineInfo::get_all_shorthands() {
        // Add everything to a list
        std::list<const char *> shorthands;
        for(auto &e : engine_infos) {
            if(e.shorthand != nullptr) {
                shorthands.emplace_back(e.shorthand);
            }
        }
        
        // Sort alphabetically
        shorthands.sort([](const char * const &a, const char * const &b) -> bool { return std::strcmp(a, b) < 0; });
        
        // Done
        return shorthands;
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
