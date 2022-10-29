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
        if(workload.disable_recursion) {
            return;
        }

        if(*workload.cache_file_type == HEK::CacheFileType::SCENARIO_TYPE_USER_INTERFACE) {
            this->unit.path.clear();
            this->unit.tag_id = HEK::TagID::null_tag_id();
            this->unit.tag_fourcc = TagFourCC::TAG_FOURCC_NONE;
        }
        else if(this->unit.path.size() == 0 && !workload.disable_error_checking) {
            workload.report_error(BuildWorkload::ErrorType::ERROR_TYPE_FATAL_ERROR, "Globals tag player information is missing a player unit which is required for the map type", tag_index);
            throw InvalidTagDataException();
        }
    }
    void Globals::pre_compile(BuildWorkload &workload, std::size_t tag_index, std::size_t, std::size_t) {
        if(workload.disable_recursion) {
            return;
        }

        if(*workload.cache_file_type != HEK::CacheFileType::SCENARIO_TYPE_MULTIPLAYER) {
            this->multiplayer_information.clear();
            this->cheat_powerups.clear();
            this->weapon_list.clear();
        }
        else if(!workload.disable_error_checking) {
            if(this->multiplayer_information.size() != 1) {
                workload.report_error(BuildWorkload::ErrorType::ERROR_TYPE_FATAL_ERROR, "Globals tag does not have exactly 1 multiplayer information block which is required for the map type", tag_index);
                throw InvalidTagDataException();
            }

            // Check for this stuff
            auto engine = workload.get_build_parameters()->details.build_cache_file_engine;
            if(engine != HEK::CacheFileEngine::CACHE_FILE_NATIVE) {
                std::size_t required_weapons = workload.get_build_parameters()->details.build_cache_file_engine == HEK::CacheFileEngine::CACHE_FILE_XBOX ? 14 : 16;
                if(this->weapon_list.size() < required_weapons) {
                    REPORT_ERROR_PRINTF(workload, ERROR_TYPE_FATAL_ERROR, tag_index, "Globals tag does not have at least %zu weapons blocks which is required for the map type", required_weapons);
                    throw InvalidTagDataException();
                }
            }
        }

        if(*workload.cache_file_type == HEK::CacheFileType::SCENARIO_TYPE_USER_INTERFACE) {
            this->falling_damage.clear();
            this->materials.clear();
        }
        else if(!workload.disable_error_checking) {
            if(this->falling_damage.size() != 1) {
                workload.report_error(BuildWorkload::ErrorType::ERROR_TYPE_FATAL_ERROR, "Globals tag does not have exactly 1 falling damage block which is required for the map type", tag_index);
                throw InvalidTagDataException();
            }
            if(this->materials.size() < 32) {
                workload.report_error(BuildWorkload::ErrorType::ERROR_TYPE_FATAL_ERROR, "Globals tag does not have at least 32 material blocks which is required for the map type", tag_index);
                throw InvalidTagDataException();
            }
        }
    }
    void GlobalsGrenade::pre_compile(BuildWorkload &workload, std::size_t tag_index, std::size_t, std::size_t offset) {
        // Account for a four-bit signed integer underflow glitch in the Gearbox netcode that occurs when using more than 7 (0b0111) grenades.
        // Basically, a 4-bit signed integer's range is -8 to 7. So, if you have more than 7, you get underflowed to -8, and you can't throw grenades if you have 0 or fewer grenades.
        // I have no idea why they thought that their netcode needed this to be optimized down to four bits SIGNED (so effectively three bits can actually be used) for grenades, but it's a massive meme.
        static constexpr const std::size_t max_grenades_mp_gbx = 7;
        auto engine_target = workload.get_build_parameters()->details.build_cache_file_engine;

        if(
            workload.cache_file_type == HEK::CacheFileType::SCENARIO_TYPE_MULTIPLAYER &&
            (engine_target == HEK::CacheFileEngine::CACHE_FILE_CUSTOM_EDITION ||
            engine_target == HEK::CacheFileEngine::CACHE_FILE_RETAIL ||
            engine_target == HEK::CacheFileEngine::CACHE_FILE_DEMO) &&
            !workload.disable_error_checking
        ) {
            #define CHECK_NADE_ON_MP_COUNT(what) if(static_cast<std::size_t>(this->what) > max_grenades_mp_gbx) { \
                REPORT_ERROR_PRINTF(workload, ERROR_TYPE_FATAL_ERROR, tag_index, # what " for grenade #%zu exceeds the maximum allowed for multiplayer for the target engine (%zu > %zu)", offset / sizeof(struct_little), static_cast<std::size_t>(this->what), max_grenades_mp_gbx); \
                throw InvalidTagDataException(); \
            }
            CHECK_NADE_ON_MP_COUNT(maximum_count);
            CHECK_NADE_ON_MP_COUNT(mp_spawn_default);
        }
    }
    void GlobalsMultiplayerInformation::post_compile(BuildWorkload &workload, std::size_t tag_index, std::size_t struct_index, std::size_t offset) {
        if(workload.disable_recursion) {
            return;
        }

        const auto &globals_multiplayer_information_struct = workload.structs[struct_index];
        const auto &globals_multiplayer_information_data = *reinterpret_cast<const struct_little *>(globals_multiplayer_information_struct.data.data() + offset);

        auto target_engine = workload.get_build_parameters()->details.build_cache_file_engine;
        const std::size_t sound_count = globals_multiplayer_information_data.sounds.count;
        std::size_t required_sounds;

        // Xbox doesn't have ting (required_sounds is exclusive)
        if(target_engine == HEK::CacheFileEngine::CACHE_FILE_XBOX) {
            required_sounds = HEK::MultiplayerInformationSound::MULTIPLAYER_INFORMATION_SOUND_TING;
        }
        else {
            required_sounds = HEK::MultiplayerInformationSound::MULTIPLAYER_INFORMATION_SOUND_ENUM_COUNT;

            // See if we have the ting sound. If so, make it louder on custom edition.
            if(sound_count > HEK::MultiplayerInformationSound::MULTIPLAYER_INFORMATION_SOUND_TING) {
                const float TING_VOLUME = target_engine == HEK::CacheFileEngine::CACHE_FILE_CUSTOM_EDITION ? 1.0F : 0.2F;
                const auto sound_id = reinterpret_cast<const GlobalsSound::struct_little *>(workload.structs[*globals_multiplayer_information_struct.resolve_pointer(&globals_multiplayer_information_data.sounds.pointer)].data.data())[HEK::MultiplayerInformationSound::MULTIPLAYER_INFORMATION_SOUND_TING].sound.tag_id.read();
                if(!sound_id.is_null()) {
                    auto &random_gain_modifier = reinterpret_cast<Sound::struct_little *>(workload.structs[*workload.tags[sound_id.index].base_struct].data.data())->random_gain_modifier;

                    if(random_gain_modifier.read() != TING_VOLUME) {
                        REPORT_ERROR_PRINTF(workload, ERROR_TYPE_WARNING_PEDANTIC, tag_index, "Random gain modifier was set to %.01f for the target engine", TING_VOLUME);
                        random_gain_modifier = TING_VOLUME;
                    }
                }
            }
        }

        // See if we have all sounds, too
        if(sound_count < required_sounds && workload.get_build_parameters()->verbosity > BuildWorkload::BuildParameters::BuildVerbosity::BUILD_VERBOSITY_HIDE_PEDANTIC) {
            REPORT_ERROR_PRINTF(workload, ERROR_TYPE_WARNING_PEDANTIC, tag_index, "Some sounds are missing from multiplayer information #%zu (%zu / %zu sounds present)", offset / sizeof(struct_little), sound_count, static_cast<std::size_t>(HEK::MultiplayerInformationSound::MULTIPLAYER_INFORMATION_SOUND_ENUM_COUNT));
        }
    }
}
