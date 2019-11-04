// SPDX-License-Identifier: GPL-3.0-only

#include <invader/tag/hek/compile.hpp>
#include <invader/tag/hek/class/effect.hpp>

namespace Invader::HEK {
    void compile_effect_tag(CompiledTag &compiled, const std::byte *data, std::size_t size) {
        BEGIN_COMPILE(Effect)
        ADD_REFLEXIVE(tag.locations);

        bool has_damage_effect = false;

        ADD_REFLEXIVE_START(tag.events) {
            ADD_REFLEXIVE_START(reflexive.parts) {
                if(reflexive.type.path_size != 0) {
                    auto r = reflexive.type.tag_class_int.read();
                    if(IS_OBJECT_TAG(r)) {
                        reflexive.type_type = TagClassInt::TAG_CLASS_OBJECT;
                    }
                    else {
                        reflexive.type_type = r;

                        if(r == TagClassInt::TAG_CLASS_DAMAGE_EFFECT) {
                            has_damage_effect = true;
                        }
                    }
                }
                ADD_DEPENDENCY_ADJUST_SIZES(reflexive.type);
            } ADD_REFLEXIVE_END;
            ADD_REFLEXIVE_START(reflexive.particles) {
                ADD_DEPENDENCY_ADJUST_SIZES(reflexive.particle_type);
                reflexive.relative_direction_vector = euler2d_to_vector(reflexive.relative_direction);
            } ADD_REFLEXIVE_END
        } ADD_REFLEXIVE_END

        if(has_damage_effect) {
            auto flags = tag.flags.read();
            flags.use_in_dedicated_servers = 1;
            tag.flags = flags;
        }

        FINISH_COMPILE
    }
}
