// SPDX-License-Identifier: GPL-3.0-only

#include <invader/map/map.hpp>
#include <invader/map/tag.hpp>
#include <invader/tag/parser/parser.hpp>
#include <invader/build2/build_workload.hpp>

namespace Invader::Parser {
    void DamageEffect::pre_compile(BuildWorkload2 &workload, std::size_t tag_index, std::size_t, std::size_t) {
        if(*workload.cache_file_type == HEK::CacheFileType::CACHE_FILE_SINGLEPLAYER && workload.tags[tag_index].path == "weapons\\pistol\\bullet") {
            this->elite_energy_shield = 0.8F;
        }
    }
}
