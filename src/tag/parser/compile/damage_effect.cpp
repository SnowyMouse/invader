// SPDX-License-Identifier: GPL-3.0-only

#include <invader/tag/parser/parser.hpp>
#include <invader/build/build_workload.hpp>

namespace Invader::Parser {
    void ContinuousDamageEffect::pre_compile(BuildWorkload &, std::size_t, std::size_t, std::size_t) {
        this->camera_shaking_wobble_period *= TICK_RATE;
    }

    void DamageEffect::pre_compile(BuildWorkload &workload, std::size_t tag_index, std::size_t, std::size_t) {
        this->camera_shaking_wobble_period *= TICK_RATE;

        if(*workload.cache_file_type == HEK::CacheFileType::SCENARIO_TYPE_SINGLEPLAYER && workload.tags[tag_index].path == "weapons\\pistol\\bullet") {
            this->elite_energy_shield = 0.8F;
        }

        if(workload.building_stock_map && (workload.tags[tag_index].path == "vehicles\\ghost\\ghost bolt" || workload.tags[tag_index].path == "vehicles\\banshee\\banshee bolt")) {
            bool custom_edition = workload.get_build_parameters()->details.build_cache_file_engine == HEK::CacheFileEngine::CACHE_FILE_CUSTOM_EDITION;

            float new_damage_stun = custom_edition ? 0.0F : 1.0F;
            float damage_maximum_stun = custom_edition ? 0.0F : 1.0F;
            float damage_stun_time = custom_edition ? 0.0F : 0.15F;

            if(new_damage_stun != this->damage_stun || damage_maximum_stun != this->damage_maximum_stun || damage_stun_time != this->damage_stun_time) {
                workload.report_error(BuildWorkload::ErrorType::ERROR_TYPE_WARNING_PEDANTIC, "Stun values were changed due to building a stock scenario", tag_index);

                this->damage_stun = new_damage_stun;
                this->damage_maximum_stun = damage_maximum_stun;
                this->damage_stun_time = damage_stun_time;
            }
        }
    }
}
