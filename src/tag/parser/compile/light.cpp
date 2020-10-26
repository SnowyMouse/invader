// SPDX-License-Identifier: GPL-3.0-only

#include <invader/tag/parser/parser.hpp>

namespace Invader::Parser {
    void Light::pre_compile(BuildWorkload &, std::size_t, std::size_t, std::size_t) {
        this->cos_cutoff_angle = std::cos(this->cutoff_angle);
        this->cos_falloff_angle = std::cos(this->falloff_angle);
        this->sin_cutoff_angle = std::sin(this->cutoff_angle);
        this->unknown_two = 2.0F;
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
}
