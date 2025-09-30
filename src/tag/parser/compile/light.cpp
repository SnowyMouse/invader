// SPDX-License-Identifier: GPL-3.0-only

#include <invader/tag/parser/parser.hpp>
#include <invader/build/build_workload.hpp>

#include "hud_interface.hpp"

namespace Invader::Parser {
    void Light::pre_compile(BuildWorkload &, std::size_t, std::size_t, std::size_t) {
        // Clamp to 180 degrees
        this->cutoff_angle = std::min(static_cast<double>(this->cutoff_angle), HALO_PI);
        this->falloff_angle = std::min(static_cast<double>(this->falloff_angle), HALO_PI);

        // If cutoff angle is less than FLOAT_EPSILON, set both to 180 degrees
        if(this->cutoff_angle < FLOAT_EPSILON) {
            this->cutoff_angle = HALO_PI;
            this->falloff_angle = HALO_PI;
        }

        // Set falloff angle to zero if greater than cutoff angle
        if(this->falloff_angle > this->cutoff_angle) {
            this->falloff_angle = 0.0F;
        }

        // Tool does it this way
        if(this->falloff_angle < HALO_PI) {
            this->cos_cutoff_angle = std::cos(this->cutoff_angle);
            this->cos_falloff_angle = std::cos(this->falloff_angle);
            this->sin_cutoff_angle = std::sin(this->cutoff_angle);
        }
        else {
            this->cos_cutoff_angle = -1.0F;
            this->cos_falloff_angle = -1.0F;
            this->sin_cutoff_angle = 0.0F;
        }

        this->specular_radius_multiplier = 2.0F;
        this->duration *= TICK_RATE;

        // If RGB values are 0 for both lower and upper, default to 1 (alpha isn't read or modified here)
        if(this->color_lower_bound.red == 0 && this->color_lower_bound.green == 0 && this->color_lower_bound.blue == 0 && this->color_upper_bound.red == 0 && this->color_upper_bound.green == 0 && this->color_upper_bound.blue == 0) {
            this->color_lower_bound.red = 1.0F;
            this->color_lower_bound.green = 1.0F;
            this->color_lower_bound.blue = 1.0F;

            this->color_upper_bound.red = 1.0F;
            this->color_upper_bound.green = 1.0F;
            this->color_upper_bound.blue = 1.0F;
        }
    }

    void LightVolume::postprocess_hek_data() {
        if(this->perpendicular_brightness_scale == 0.0F && this->parallel_brightness_scale == 0.0F) {
            this->parallel_brightness_scale = 1.0F;
            this->perpendicular_brightness_scale = 1.0F;
        }
    }

    void LightVolumeFrame::postprocess_hek_data() {
        if(this->offset_exponent <= 0.0F) {
            this->offset_exponent = 1.0F;
        }
        if(this->radius_exponent <= 0.0F) {
            this->radius_exponent = 1.0F;
        }
    }

    void LightVolume::post_compile(BuildWorkload &workload, std::size_t tag_index, std::size_t, std::size_t) {
        CHECK_BITMAP_SEQUENCE(workload, this->map, this->sequence_index, "light volume");
    }
}
