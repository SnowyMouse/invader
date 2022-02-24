// SPDX-License-Identifier: GPL-3.0-only

#ifndef INVADER__TAG__PARSER__COMPILE__SHADER_HPP
#define INVADER__TAG__PARSER__COMPILE__SHADER_HPP

namespace Invader {
    class BuildWorkload;
}

namespace Invader::Parser {
    struct ShaderTransparentChicago;
    struct ShaderTransparentChicagoExtended;

    /**
     * Convert a shader_transparent_chicago_extended tag into a shader_transparent_chicago tag.
     * @param  shader shader tag to convert
     * @return        converted tag
     */
    ShaderTransparentChicago convert_shader_transparent_chicago_extended_to_shader_transparent_chicago(const ShaderTransparentChicagoExtended &shader);
    
    /**
     * Recursively get all predicted resources from the struct
     * @param struct_index            struct index
     * @param resources               array of resources to fill with tag indices
     * @param ignore_shader_resources ignore immediate shader tags
     */
    void recursively_get_all_predicted_resources_from_struct(const BuildWorkload &workload, std::size_t struct_index, std::vector<std::size_t> &resources, bool ignore_shader_resources);
}

#endif
