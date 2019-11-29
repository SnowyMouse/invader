// SPDX-License-Identifier: GPL-3.0-only

#include <invader/tag/parser/parser.hpp>

namespace Invader::Parser {
    void Actor::pre_compile(BuildWorkload2 &, std::size_t, std::size_t, std::size_t) {
        #define SET_INVERTED_VALUE(normal,inverted) { if(normal > 0.0F) { inverted = 1.0F / (TICK_RATE * normal); } else { inverted = 0.0F; } }

        SET_INVERTED_VALUE(this->combat_perception_time, this->inverse_combat_perception_time);
        SET_INVERTED_VALUE(this->guard_perception_time, this->inverse_guard_perception_time);
        SET_INVERTED_VALUE(this->non_combat_perception_time, this->inverse_non_combat_perception_time);

        this->cosine_maximum_aiming_deviation.i = std::cos(this->maximum_aiming_deviation.i);
        this->cosine_maximum_aiming_deviation.j = std::cos(this->maximum_aiming_deviation.j);
        this->cosine_maximum_looking_deviation.i = std::cos(this->maximum_looking_deviation.i);
        this->cosine_maximum_looking_deviation.j = std::cos(this->maximum_looking_deviation.j);
    }
}
