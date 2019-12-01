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
        if(*workload.cache_file_type == HEK::CacheFileType::CACHE_FILE_USER_INTERFACE) {
            this->unit.path.clear();
            this->unit.tag_id = HEK::TagID::null_tag_id();
        }
        else if(this->unit.path.size() == 0) {
            workload.report_error(BuildWorkload::ErrorType::ERROR_TYPE_ERROR, "Globals tag player information is missing a player unit which is required for the map type", tag_index);
        }
    }
    void Globals::pre_compile(BuildWorkload &workload, std::size_t tag_index, std::size_t, std::size_t) {
        if(*workload.cache_file_type != HEK::CacheFileType::CACHE_FILE_MULTIPLAYER) {
            this->multiplayer_information.clear();
            this->cheat_powerups.clear();
        }
        else {
            if(this->multiplayer_information.size() != 1) {
                workload.report_error(BuildWorkload::ErrorType::ERROR_TYPE_ERROR, "Globals tag does not have exactly 1 multiplayer information block which is required for the map type", tag_index);
            }
            if(this->weapon_list.size() < 16) {
                workload.report_error(BuildWorkload::ErrorType::ERROR_TYPE_ERROR, "Globals tag does not have at least 16 weapons blocks which is required for the map type", tag_index);
            }
        }

        if(*workload.cache_file_type == HEK::CacheFileType::CACHE_FILE_USER_INTERFACE) {
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
}
