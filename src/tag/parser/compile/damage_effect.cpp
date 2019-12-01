// SPDX-License-Identifier: GPL-3.0-only

#include <invader/tag/parser/parser.hpp>
#include <invader/build/build_workload.hpp>

namespace Invader::Parser {
    void DamageEffect::pre_compile(BuildWorkload &workload, std::size_t tag_index, std::size_t, std::size_t) {
        if(*workload.cache_file_type == HEK::CacheFileType::CACHE_FILE_SINGLEPLAYER && workload.tags[tag_index].path == "weapons\\pistol\\bullet") {
            this->elite_energy_shield = 0.8F;
        }
    }
}
