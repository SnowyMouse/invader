// SPDX-License-Identifier: GPL-3.0-only

#include <invader/tag/parser/definition/damage_effect.hpp>
#include <invader/tag/parser/definition/effect.hpp>
#include <invader/build/build_workload.hpp>

namespace Invader::Parser {
    void EffectParticle::post_compile(BuildWorkload &workload, std::size_t, std::size_t struct_index, std::size_t struct_offset) {
        reinterpret_cast<C<LittleEndian> *>(workload.structs[struct_index].data.data() + struct_offset)->relative_direction_vector = euler2d_to_vector(this->relative_direction);
    }

    void Effect::post_compile(BuildWorkload &workload, std::size_t, std::size_t struct_index, std::size_t struct_offset) {
        bool do_not_cull = false;
        auto &effect_struct = workload.structs[struct_index];
        auto &effect = *reinterpret_cast<C<LittleEndian> *>(effect_struct.data.data() + struct_offset);
        std::size_t event_count = effect.events.count;
        
        // Go through each part (requires going through each event)
        if(event_count) {
            auto &events_struct = workload.structs[*effect_struct.resolve_pointer(&effect.events.pointer)];
            auto *events = reinterpret_cast<EffectEvent::C<LittleEndian> *>(events_struct.data.data());
            
            for(std::size_t e = 0; e < event_count; e++) {
                auto &event = events[e];
                std::size_t part_count = event.parts.count;
                if(part_count) {
                    auto *parts = reinterpret_cast<EffectPart::C<LittleEndian> *>(workload.structs[*events_struct.resolve_pointer(&event.parts.pointer)].data.data());
                    
                    for(std::size_t p = 0; p < part_count; p++) {
                        auto &part = parts[p];
                        auto part_id = part.type.tag_id.read();
                        if(!part_id.is_null()) {
                            auto r = part.type.tag_fourcc.read();
                            if(IS_OBJECT_TAG(r)) {
                                part.type_class = TagFourCC::TAG_FOURCC_OBJECT;
                            }
                            else {
                                part.type_class = r;
                                if(r == TagFourCC::TAG_FOURCC_DAMAGE_EFFECT || r == TagFourCC::TAG_FOURCC_LIGHT) {
                                    do_not_cull = true;
                                }
                                
                                // Find the maximum radius
                                if(r == TagFourCC::TAG_FOURCC_DAMAGE_EFFECT) {
                                    float max_radius = reinterpret_cast<DamageEffect::C<LittleEndian> *>(workload.structs[*workload.tags[part_id.index].base_struct].data.data())->radius.to;
                                    if(max_radius > effect.maximum_damage_radius.read()) {
                                        effect.maximum_damage_radius = max_radius;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        
        auto flags = effect.flags.read();
        if(do_not_cull) {
            flags |= EffectFlagsFlag::EFFECT_FLAGS_FLAG_DO_NOT_CULL;
        }
        else {
            flags &= ~EffectFlagsFlag::EFFECT_FLAGS_FLAG_DO_NOT_CULL;
        }
        effect.flags = flags;
    }
}
