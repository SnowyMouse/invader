// SPDX-License-Identifier: GPL-3.0-only

#include <invader/tag/parser/parser.hpp>

namespace Invader::Parser {
    void Decal::postprocess_hek_data() {
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
}
