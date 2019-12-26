// SPDX-License-Identifier: GPL-3.0-only

#include <invader/tag/parser/parser.hpp>
#include <invader/build/build_workload.hpp>

namespace Invader::Parser {
    void WeaponHUDInterface::pre_compile(BuildWorkload &workload, std::size_t tag_index, std::size_t, std::size_t) {
        union {
            std::uint32_t inty;
            HEK::WeaponHUDInterfaceCrosshairTypeFlags flaggy;
        } crosshair_types = {};
        for(auto &c : this->crosshairs) {
            crosshair_types.inty |= (1 << c.crosshair_type);
        }
        this->crosshair_types = crosshair_types.flaggy;

        if(workload.engine_target != HEK::CacheFileEngine::CACHE_FILE_DARK_CIRCLET && this->crosshair_types.zoom == 0) {
            std::size_t zooms = 0;
            for(auto &c : this->crosshairs) {
                for(auto &o : c.crosshair_overlays) {
                    if(o.flags.dont_show_when_zoomed || o.flags.show_only_when_zoomed) {
                        zooms++;
                    }
                }
            }
            if(zooms) {
                REPORT_ERROR_PRINTF(workload, ERROR_TYPE_WARNING, tag_index, "%zu overlay%s set to change on zoom, but no zoom crosshairs exist.", zooms, zooms == 1 ? " is" : "s are");
            }
        }
    }
}
