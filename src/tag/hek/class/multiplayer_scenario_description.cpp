/*
 * Invader (c) 2018 Kavawuvi
 *
 * This program is free software under the GNU General Public License v3.0. See LICENSE for more information.
 */

#include "../compile.hpp"
#include "multiplayer_scenario_description.hpp"

namespace Invader::HEK {
    void compile_multiplayer_scenario_description_tag(CompiledTag &compiled, const std::byte *data, std::size_t size) {
        BEGIN_COMPILE(MultiplayerScenarioDescription);

        ADD_REFLEXIVE_START(tag.multiplayer_scenarios) {
            ADD_DEPENDENCY_ADJUST_SIZES(reflexive.descriptive_bitmap);
            ADD_DEPENDENCY_ADJUST_SIZES(reflexive.displayed_map_name);
        } ADD_REFLEXIVE_END

        FINISH_COMPILE
    }
}
