// SPDX-License-Identifier: GPL-3.0-only

#include <invader/tag/parser/parser.hpp>
#include <invader/build/build_workload.hpp>

namespace Invader::Parser {
    void EffectParticle::post_compile(BuildWorkload &workload, std::size_t, std::size_t struct_index, std::size_t struct_offset) {
        reinterpret_cast<struct_little *>(workload.structs[struct_index].data.data() + struct_offset)->relative_direction_vector = HEK::euler2d_to_vector(this->relative_direction);
    }

    void Effect::post_compile(BuildWorkload &workload, std::size_t tag_index, std::size_t struct_index, std::size_t struct_offset) {
        if(workload.disable_recursion) {
            return;
        }

        bool must_be_deterministic = false;
        auto &effect_struct = workload.structs[struct_index];
        auto &effect = *reinterpret_cast<struct_little *>(effect_struct.data.data() + struct_offset);
        std::size_t event_count = effect.events.count;

        // Go through each part (requires going through each event)
        if(event_count > 0) {
            auto &events_struct = workload.structs[*effect_struct.resolve_pointer(&effect.events.pointer)];
            auto *events = reinterpret_cast<EffectEvent::struct_little *>(events_struct.data.data());

            for(std::size_t e = 0; e < event_count; e++) {
                auto &event = events[e];
                std::size_t part_count = event.parts.count;
                std::size_t particle_count = event.particles.count;

                if(part_count > 0) {
                    auto *parts = reinterpret_cast<EffectPart::struct_little *>(workload.structs[*events_struct.resolve_pointer(&event.parts.pointer)].data.data());

                    for(std::size_t p = 0; p < part_count; p++) {
                        auto &part = parts[p];
                        auto part_id = part.type.tag_id.read();

                        auto r = part.type.tag_fourcc.read();
                        if(IS_OBJECT_TAG(r)) {
                            part.type_class = TagFourCC::TAG_FOURCC_OBJECT;
                            must_be_deterministic = true;
                        }
                        else {
                            part.type_class = r;
                            if(r == TagFourCC::TAG_FOURCC_DAMAGE_EFFECT || r == TagFourCC::TAG_FOURCC_LIGHT) {
                                must_be_deterministic = true;
                            }
                        }

                        // Find the maximum radius
                        if(!part_id.is_null()) {
                            if(r == TagFourCC::TAG_FOURCC_DAMAGE_EFFECT) {
                                float max_radius = reinterpret_cast<DamageEffect::struct_little *>(workload.structs[*workload.tags[part_id.index].base_struct].data.data())->radius.to;
                                if(max_radius > effect.maximum_damage_radius.read()) {
                                    effect.maximum_damage_radius = max_radius;
                                }
                            }
                        }

                        else if(r == TagFourCC::TAG_FOURCC_DAMAGE_EFFECT) {
                            REPORT_ERROR_PRINTF(workload, ERROR_TYPE_FATAL_ERROR, tag_index, "Part #%zu of event #%zu contains a null %s tag reference. This is invalid.", p, e, HEK::tag_fourcc_to_extension(r));
                            throw InvalidTagDataException();
                        }

                        else {
                            REPORT_ERROR_PRINTF(workload, ERROR_TYPE_WARNING_PEDANTIC, tag_index, "Part #%zu of event #%zu contains a null %s tag reference. The part will not do anything.", p, e, HEK::tag_fourcc_to_extension(r));
                        }
                    }
                }

                if(particle_count > 0) {
                    auto *particles = reinterpret_cast<EffectParticle::struct_little *>(workload.structs[*events_struct.resolve_pointer(&event.particles.pointer)].data.data());

                    for(std::size_t p = 0; p < particle_count; p++) {
                        if(particles[p].particle_type.tag_id.read().is_null()) {
                            REPORT_ERROR_PRINTF(workload, ERROR_TYPE_WARNING_PEDANTIC, tag_index, "Particle #%zu of event #%zu is null.", p, e);
                        }
                    }
                }
            }
        }

        auto flags = effect.flags.read();

        // Unset these if they're set
        flags &= ~(HEK::EffectFlagsFlag::EFFECT_FLAGS_FLAG_MUST_BE_DETERMINISTIC_PC | HEK::EffectFlagsFlag::EFFECT_FLAGS_FLAG_MUST_BE_DETERMINISTIC_XBOX);

        // Set the correct flag based on if it's Xbox or not
        if(must_be_deterministic) {
            flags |= workload.get_build_parameters()->details.build_cache_file_engine == HEK::CacheFileEngine::CACHE_FILE_XBOX ? HEK::EffectFlagsFlag::EFFECT_FLAGS_FLAG_MUST_BE_DETERMINISTIC_XBOX :
                                                                                                                                 HEK::EffectFlagsFlag::EFFECT_FLAGS_FLAG_MUST_BE_DETERMINISTIC_PC;
        }

        effect.flags = flags;
    }
}
