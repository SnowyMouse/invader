// SPDX-License-Identifier: GPL-3.0-only

#include <invader/tag/parser/parser.hpp>
#include <invader/build/build_workload.hpp>

namespace Invader::Parser {
    void Actor::post_compile(BuildWorkload &workload, std::size_t, std::size_t struct_index, std::size_t struct_offset) {
        auto *actor = reinterpret_cast<struct_little *>(workload.structs[struct_index].data.data() + struct_offset);
        
        #define SET_INVERTED_VALUE(normal,inverted) { if(normal.read() > 0.0F) { inverted = 1.0F / (TICK_RATE * normal.read()); } else { inverted = 0.0F; } }

        SET_INVERTED_VALUE(actor->combat_perception_time, actor->inverse_combat_perception_time);
        SET_INVERTED_VALUE(actor->guard_perception_time, actor->inverse_guard_perception_time);
        SET_INVERTED_VALUE(actor->non_combat_perception_time, actor->inverse_non_combat_perception_time);

        actor->cosine_begin_moving_angle = actor->begin_moving_angle != 0.0F ? std::cos(actor->begin_moving_angle) : 0.0F;
        actor->cosine_maximum_aiming_deviation.i = std::cos(actor->maximum_aiming_deviation.i);
        actor->cosine_maximum_aiming_deviation.j = std::cos(actor->maximum_aiming_deviation.j);
        actor->cosine_maximum_looking_deviation.i = std::cos(actor->maximum_looking_deviation.i);
        actor->cosine_maximum_looking_deviation.j = std::cos(actor->maximum_looking_deviation.j);
    }
    
    void ActorVariant::post_compile(BuildWorkload &workload, std::size_t, std::size_t struct_index, std::size_t struct_offset) {
        auto *actor_variant = reinterpret_cast<struct_little *>(workload.structs[struct_index].data.data() + struct_offset);
        actor_variant->grenade_velocity = actor_variant->grenade_velocity.read() / TICK_RATE;
    }
}
