// SPDX-License-Identifier: GPL-3.0-only

#ifndef INVADER__TAG__PARSER__COMPILE__SHADER_HPP
#define INVADER__TAG__PARSER__COMPILE__SHADER_HPP

namespace Invader::Parser {
    struct ShaderTransparentChicago;
    struct ShaderTransparentChicagoExtended;

    /**
     * Convert a shader_transparent_chicago_extended tag into a shader_transparent_chicago tag.
     * @param  shader shader tag to convert
     * @return        converted tag
     */
    ShaderTransparentChicago convert_shader_transparent_chicago_extended_to_shader_transparent_chicago(const ShaderTransparentChicagoExtended &shader);
}

#endif
