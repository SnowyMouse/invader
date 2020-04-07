// SPDX-License-Identifier: GPL-3.0-only

#include <invader/tag/parser/parser.hpp>

namespace Invader::Parser {
    void EffectParticle::pre_compile(BuildWorkload &, std::size_t, std::size_t, std::size_t) {
        this->relative_direction_vector = HEK::euler2d_to_vector(this->relative_direction);
    }

    void Effect::pre_compile(BuildWorkload &, std::size_t, std::size_t, std::size_t) {
        bool do_not_cull = false;

        for(auto &e : this->events) {
            for(auto &p : e.parts) {
                if(p.type.path.size() != 0) {
                    auto r = p.type.tag_class_int;
                    if(IS_OBJECT_TAG(r)) {
                        p.type_class = TagClassInt::TAG_CLASS_OBJECT;
                    }
                    else {
                        p.type_class = r;
                        if(r == TagClassInt::TAG_CLASS_DAMAGE_EFFECT || r == TagClassInt::TAG_CLASS_LIGHT) {
                            do_not_cull = true;
                        }
                    }
                }
            }
        }

        if(do_not_cull) {
            this->flags |= HEK::EffectFlagsFlag::EFFECT_FLAGS_FLAG_DO_NOT_CULL;
        }
        else {
            this->flags &= ~HEK::EffectFlagsFlag::EFFECT_FLAGS_FLAG_DO_NOT_CULL;
        }
    }

    void Invader::Parser::Effect::post_cache_deformat() {
        this->flags &= ~HEK::EffectFlagsFlag::EFFECT_FLAGS_FLAG_DO_NOT_CULL;
    }
}
