// SPDX-License-Identifier: GPL-3.0-only

#include <invader/tag/parser/parser.hpp>
#include <invader/build/build_workload.hpp>

namespace Invader::Parser {
    void Decal::postprocess_hek_data() {
        if(this->layer == HEK::DecalLayer::DECAL_LAYER_WATER) {
            flags |= HEK::DecalFlagsFlag::DECAL_FLAGS_FLAG_WATER_EFFECT;
        }
        else {
            flags &= ~HEK::DecalFlagsFlag::DECAL_FLAGS_FLAG_WATER_EFFECT;
        }

        this->radius.to = std::max(this->radius.from, this->radius.to);
        this->maximum_sprite_extent = 16.0F;

        // If RGB values are 0 for both lower and upper, default to 1
        if(this->color_lower_bounds.red == 0 && this->color_lower_bounds.green == 0 && this->color_lower_bounds.blue == 0 && this->color_upper_bounds.red == 0 && this->color_upper_bounds.green == 0 && this->color_upper_bounds.blue == 0) {
            this->color_lower_bounds.red = 1.0F;
            this->color_lower_bounds.green = 1.0F;
            this->color_lower_bounds.blue = 1.0F;

            this->color_upper_bounds.red = 1.0F;
            this->color_upper_bounds.green = 1.0F;
            this->color_upper_bounds.blue = 1.0F;
        }

        this->lifetime.to = std::max(this->lifetime.from, this->lifetime.to);
    }

    void Decal::pre_compile(BuildWorkload &workload, std::size_t tag_index, std::size_t, std::size_t) {
        // This will be available in the ringhopper implementation instead.
        if(flags & HEK::DecalFlagsFlag::DECAL_FLAGS_FLAG_SPRITE_SCALE_BUG_FIX) {
            workload.report_error(BuildWorkload::ErrorType::ERROR_TYPE_FATAL_ERROR, "The \"sprite scale bug fix\" option is not supported by this version of invader", tag_index);
            throw InvalidTagDataException();
        }
    }

}
