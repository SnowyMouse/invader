// SPDX-License-Identifier: GPL-3.0-only

#ifndef INVADER__TAG__PARSER__COMPILE__SCENARIO_STRUCTURE_BSP_HPP
#define INVADER__TAG__PARSER__COMPILE__SCENARIO_STRUCTURE_BSP_HPP

namespace Invader {
    class BuildWorkload;
}

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
    
    /**
     * Set up the tables for the BSP cache data
     * @param workload                workload to modify
     * @param bsp_header_struct_index index of the BSP's header
     * @param bsp_struct_index        index of the BSP tag
     */
    void set_up_xbox_cache_bsp_data(BuildWorkload &workload, std::size_t bsp_header_struct_index, std::size_t bsp_struct_index, std::size_t bsp);
}

#endif
