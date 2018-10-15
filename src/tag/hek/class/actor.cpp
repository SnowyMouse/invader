/*
 * Invader (c) 2018 Kavawuvi
 *
 * This program is free software under the GNU General Public License v3.0 or later. See LICENSE for more information.
 */

#include <iostream>

#include "../compile.hpp"

#include "actor.hpp"

namespace Invader::HEK {
    void compile_actor_tag(CompiledTag &compiled, const std::byte *data, std::size_t size) {
        BEGIN_COMPILE(Actor)
        ADD_DEPENDENCY_ADJUST_SIZES(tag.do_not_use)
        ADD_DEPENDENCY_ADJUST_SIZES(tag.do_not_use_1)
        ADD_DEPENDENCY_ADJUST_SIZES(tag.do_not_use_2)
        FINISH_COMPILE
    }
}
