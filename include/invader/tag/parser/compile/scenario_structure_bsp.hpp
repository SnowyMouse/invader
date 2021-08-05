// SPDX-License-Identifier: GPL-3.0-only

#ifndef INVADER__TAG__PARSER__COMPILE__SCENARIO_STRUCTURE_BSP_HPP
#define INVADER__TAG__PARSER__COMPILE__SCENARIO_STRUCTURE_BSP_HPP

#include "../definition/scenario_structure_bsp.hpp"

namespace Invader {
    class BuildWorkload;
}

namespace Invader::Parser {
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

    ScenarioStructureBSPMaterialCompressedRenderedVertex::C<NativeEndian> compress_sbsp_rendered_vertex(const ScenarioStructureBSPMaterialUncompressedRenderedVertex::C<NativeEndian> &vertex) noexcept;
    ScenarioStructureBSPMaterialUncompressedRenderedVertex::C<NativeEndian> decompress_sbsp_rendered_vertex(const ScenarioStructureBSPMaterialCompressedRenderedVertex::C<NativeEndian> &vertex) noexcept;
    
    ScenarioStructureBSPMaterialCompressedLightmapVertex::C<NativeEndian> compress_sbsp_lightmap_vertex(const ScenarioStructureBSPMaterialUncompressedLightmapVertex::C<NativeEndian> &vertex) noexcept;
    ScenarioStructureBSPMaterialUncompressedLightmapVertex::C<NativeEndian> decompress_sbsp_lightmap_vertex(const ScenarioStructureBSPMaterialCompressedLightmapVertex::C<NativeEndian> &vertex) noexcept;
}

#endif
