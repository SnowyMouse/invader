// SPDX-License-Identifier: GPL-3.0-only

#include <invader/build/build_workload.hpp>
#include <invader/tag/parser/parser.hpp>

namespace Invader::Parser {
    void LensFlare::pre_compile(BuildWorkload &workload, std::size_t tag_index, std::size_t, std::size_t) {
        this->cos_falloff_angle = std::cos(this->falloff_angle);
        this->cos_cutoff_angle = std::cos(this->cutoff_angle);
        
        if(this->rotation_function_scale == 360.0) {
            REPORT_ERROR_PRINTF(workload, ERROR_TYPE_WARNING_PEDANTIC, tag_index, "Function scale is set to 20626.480625° (360 radians). This is a errorneous default value set by older tools.");
        }
    }
}
