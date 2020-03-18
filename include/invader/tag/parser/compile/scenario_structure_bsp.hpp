// SPDX-License-Identifier: GPL-3.0-only

#ifndef INVADER__TAG__PARSER__COMPILE__SCENARIO_STRUCTURE_BSP_HPP
#define INVADER__TAG__PARSER__COMPILE__SCENARIO_STRUCTURE_BSP_HPP

namespace Invader::Parser {
    struct ScenarioStructureBSP;
    struct ScenarioStructureBSPMaterial;

    /**
     * Regenerate missing vertices from the BSP
     * @param  bsp BSP to regenerate missing vertices
     * @return     true if any vertices were regenerated
     */
    bool regenerate_missing_bsp_vertices(ScenarioStructureBSP &bsp, bool fix);

    /**
     * Regenerate missing vertices from the BSP material
     * @param  material BSP material to regenerate missing vertices
     * @return          true if any vertices were regenerated
     */
    bool regenerate_missing_bsp_vertices(ScenarioStructureBSPMaterial &material, bool fix);
}

#endif
