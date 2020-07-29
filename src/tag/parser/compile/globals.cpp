// SPDX-License-Identifier: GPL-3.0-only

#include <invader/tag/parser/parser.hpp>
#include <invader/build/build_workload.hpp>

namespace Invader::Parser {
    void GlobalsFallingDamage::pre_compile(BuildWorkload &, std::size_t, std::size_t, std::size_t) {
        this->maximum_falling_velocity = static_cast<float>(std::sqrt(this->maximum_falling_distance * GRAVITY * 2.0f));
        this->harmful_falling_velocity.from = static_cast<float>(std::sqrt(this->harmful_falling_distance.from * GRAVITY * 2.0f));
        this->harmful_falling_velocity.to = static_cast<float>(std::sqrt(this->harmful_falling_distance.to * GRAVITY * 2.0f));
    }
    void GlobalsPlayerInformation::pre_compile(BuildWorkload &workload, std::size_t tag_index, std::size_t, std::size_t) {
        if(*workload.cache_file_type == HEK::CacheFileType::SCENARIO_TYPE_USER_INTERFACE) {
            this->unit.path.clear();
            this->unit.tag_id = HEK::TagID::null_tag_id();
            this->unit.tag_class_int = TagClassInt::TAG_CLASS_NONE;
        }
        else if(this->unit.path.size() == 0) {
            workload.report_error(BuildWorkload::ErrorType::ERROR_TYPE_ERROR, "Globals tag player information is missing a player unit which is required for the map type", tag_index);
        }
    }
    void Globals::pre_compile(BuildWorkload &workload, std::size_t tag_index, std::size_t, std::size_t) {
        if(*workload.cache_file_type != HEK::CacheFileType::SCENARIO_TYPE_MULTIPLAYER) {
            this->multiplayer_information.clear();
            this->cheat_powerups.clear();
            this->weapon_list.clear();
        }
        else {
            if(this->multiplayer_information.size() != 1) {
                workload.report_error(BuildWorkload::ErrorType::ERROR_TYPE_ERROR, "Globals tag does not have exactly 1 multiplayer information block which is required for the map type", tag_index);
            }
            if(this->weapon_list.size() < 16) {
                workload.report_error(BuildWorkload::ErrorType::ERROR_TYPE_ERROR, "Globals tag does not have at least 16 weapons blocks which is required for the map type", tag_index);
            }
        }

        if(*workload.cache_file_type == HEK::CacheFileType::SCENARIO_TYPE_USER_INTERFACE) {
            this->falling_damage.clear();
            this->materials.clear();
        }
        else {
            if(this->falling_damage.size() != 1) {
                workload.report_error(BuildWorkload::ErrorType::ERROR_TYPE_ERROR, "Globals tag does not have exactly 1 falling damage block which is required for the map type", tag_index);
            }
            if(this->materials.size() != 32) {
                workload.report_error(BuildWorkload::ErrorType::ERROR_TYPE_ERROR, "Globals tag does not have exactly 32 material blocks which is required for the map type", tag_index);
            }
        }
    }
    void GlobalsGrenade::pre_compile(BuildWorkload &workload, std::size_t tag_index, std::size_t, std::size_t offset) {
        // Account for a four-bit signed integer underflow glitch in the Gearbox netcode that occurs when using more than 7 (0b0111) grenades.
        // Basically, a 4-bit signed integer's range is -8 to 7. So, if you have more than 7, you get underflowed to -8, and you can't throw grenades if you have 0 or fewer grenades.
        // I have no idea why they thought that their netcode needed this to be optimized down to four bits SIGNED (so effectively three bits can actually be used) for grenades, but it's a massive meme.
        static constexpr const std::size_t max_grenades_mp_gbx = 7;
        if(
            workload.cache_file_type == HEK::CacheFileType::SCENARIO_TYPE_MULTIPLAYER &&
            (workload.engine_target == HEK::CacheFileEngine::CACHE_FILE_CUSTOM_EDITION || 
            workload.engine_target == HEK::CacheFileEngine::CACHE_FILE_RETAIL ||
            workload.engine_target == HEK::CacheFileEngine::CACHE_FILE_DEMO)
        ) {
            #define CHECK_NADE_ON_MP_COUNT(what) if(this->what > max_grenades_mp_gbx) { \
                REPORT_ERROR_PRINTF(workload, ERROR_TYPE_ERROR, tag_index, # what " for grenade #%zu exceeds the maximum allowed for multiplayer for the target engine (%zu > %zu)", offset / sizeof(struct_little), static_cast<std::size_t>(this->what), max_grenades_mp_gbx); \
            }
            CHECK_NADE_ON_MP_COUNT(maximum_count);
            CHECK_NADE_ON_MP_COUNT(mp_spawn_default);
        }
    }
    void GlobalsMultiplayerInformation::post_compile(BuildWorkload &workload, std::size_t tag_index, std::size_t struct_index, std::size_t offset) {
        const auto &globals_multiplayer_information_struct = workload.structs[struct_index];
        const auto &globals_multiplayer_information_data = *reinterpret_cast<const struct_little *>(globals_multiplayer_information_struct.data.data() + offset);

        // See if we have the thing sound
        const std::size_t SOUND_COUNT = globals_multiplayer_information_data.sounds.count;
        if(SOUND_COUNT > HEK::MultiplayerInformationSound::MULTIPLAYER_INFORMATION_SOUND_TING) {
            const float TING_VOLUME = workload.engine_target == HEK::CacheFileEngine::CACHE_FILE_CUSTOM_EDITION ? 1.0F : 0.2F;
            const auto sound_id = reinterpret_cast<const GlobalsSound::struct_little *>(workload.structs[*globals_multiplayer_information_struct.resolve_pointer(&globals_multiplayer_information_data.sounds.pointer)].data.data())[HEK::MultiplayerInformationSound::MULTIPLAYER_INFORMATION_SOUND_TING].sound.tag_id.read();
            if(!sound_id.is_null()) {
                reinterpret_cast<Sound::struct_little *>(workload.structs[*workload.tags[sound_id.index].base_struct].data.data())->random_gain_modifier = TING_VOLUME;
            }
        }

        // See if we have all sounds, too
        if(SOUND_COUNT < HEK::MultiplayerInformationSound::MULTIPLAYER_INFORMATION_SOUND_ENUM_COUNT && !workload.hide_pedantic_warnings) {
            REPORT_ERROR_PRINTF(workload, ERROR_TYPE_WARNING_PEDANTIC, tag_index, "Some sounds are missing from multiplayer information #%zu (%zu / %zu sounds present)", SOUND_COUNT, static_cast<std::size_t>(HEK::MultiplayerInformationSound::MULTIPLAYER_INFORMATION_SOUND_ENUM_COUNT), offset / sizeof(struct_little));
        }
    }
}
