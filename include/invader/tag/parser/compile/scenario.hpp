// SPDX-License-Identifier: GPL-3.0-only

#ifndef INVADER__TAG__PARSER__COMPILE__SCENARIO_HPP
#define INVADER__TAG__PARSER__COMPILE__SCENARIO_HPP

#include "../definition/scenario.hpp"

namespace Invader::Parser {
    /**
     * Fix scenario script source data being missing by decompiling scripts
     * @param  scenario scenario to fix
     * @param  fix      actually apply changes
     * @return          true if the flag was wrong, false if not
     */
    bool fix_missing_script_source_data(Scenario &scenario, bool fix);
    
    /**
     * Fix excessive script nodes being present
     * @param  scenario scenario to fix
     * @param  fix      actually apply changes
     * @return          true if there were too many nodes and it was fixable, false if not
     */
    bool fix_excessive_script_nodes(Scenario &scenario, bool fix);
}

#endif
