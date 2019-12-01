// SPDX-License-Identifier: GPL-3.0-only

#include <invader/tag/parser/parser.hpp>

namespace Invader::Parser {
    void Effect::pre_compile(BuildWorkload &, std::size_t, std::size_t, std::size_t) {
        bool has_damage_effect = false;

        for(auto &e : this->events) {
            for(auto &p : e.parts) {
                if(p.type.path.size() != 0) {
                    auto r = p.type.tag_class_int;
                    if(IS_OBJECT_TAG(r)) {
                        p.type_class = TagClassInt::TAG_CLASS_OBJECT;
                    }
                    else {
                        p.type_class = r;
                        if(r == TagClassInt::TAG_CLASS_DAMAGE_EFFECT) {
                            has_damage_effect = true;
                        }
                    }
                }
            }
        }

        if(has_damage_effect) {
            this->flags.use_in_dedicated_servers = 1;
        }
    }
}
