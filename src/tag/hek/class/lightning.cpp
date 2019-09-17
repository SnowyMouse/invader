/*
 * Invader (c) 2019 Kavawuvi
 *
 * This program is free software under the GNU General Public License v3.0. See LICENSE for more information.
 */

#include "../compile.hpp"

#include "lightning.hpp"

namespace Invader::HEK {
    void compile_lightning_tag(CompiledTag &compiled, const std::byte *data, std::size_t size) {
        BEGIN_COMPILE(Lightning);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.bitmap);
        ADD_REFLEXIVE(tag.markers);
        ADD_REFLEXIVE_START(tag.shader) {
            reflexive.make_it_work = 1;
            reflexive.some_more_stuff_that_should_be_set_for_some_reason = 0xFFFFFFFF;
        } ADD_REFLEXIVE_END
        FINISH_COMPILE
    }
}
