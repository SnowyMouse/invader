// SPDX-License-Identifier: GPL-3.0-only

#include <invader/map/map.hpp>
#include <invader/map/tag.hpp>
#include <invader/tag/parser/parser.hpp>

namespace Invader::Parser {
    void ShaderEnvironment::pre_compile(BuildWorkload2 &, std::size_t, std::size_t, std::size_t) {
        this->shader_type = HEK::ShaderType::SHADER_TYPE_SHADER_ENVIRONMENT;
        this->bump_map_scale_xy.x = this->bump_map_scale;
        this->bump_map_scale_xy.y = this->bump_map_scale;
    }
    void ShaderModel::pre_compile(BuildWorkload2 &, std::size_t, std::size_t, std::size_t) {
        this->shader_type = HEK::ShaderType::SHADER_TYPE_SHADER_MODEL;
        this->unknown = 1.0F;
    }
    void ShaderTransparentChicago::pre_compile(BuildWorkload2 &, std::size_t, std::size_t, std::size_t) {
        this->shader_type = HEK::ShaderType::SHADER_TYPE_SHADER_TRANSPARENT_CHICAGO;
    }
    void ShaderTransparentChicagoExtended::pre_compile(BuildWorkload2 &, std::size_t, std::size_t, std::size_t) {
        this->shader_type = HEK::ShaderType::SHADER_TYPE_SHADER_TRANSPARENT_CHICAGO_EXTENDED;
    }
    void ShaderTransparentWater::pre_compile(BuildWorkload2 &, std::size_t, std::size_t, std::size_t) {
        this->shader_type = HEK::ShaderType::SHADER_TYPE_SHADER_TRANSPARENT_WATER;
    }
    void ShaderTransparentGlass::pre_compile(BuildWorkload2 &, std::size_t, std::size_t, std::size_t) {
        this->shader_type = HEK::ShaderType::SHADER_TYPE_SHADER_TRANSPARENT_GLASS;
    }
    void ShaderTransparentMeter::pre_compile(BuildWorkload2 &, std::size_t, std::size_t, std::size_t) {
        this->shader_type = HEK::ShaderType::SHADER_TYPE_SHADER_TRANSPARENT_METER;
    }
    void ShaderTransparentPlasma::pre_compile(BuildWorkload2 &, std::size_t, std::size_t, std::size_t) {
        this->shader_type = HEK::ShaderType::SHADER_TYPE_SHADER_TRANSPARENT_PLASMA;
    }
    void ShaderTransparentGeneric::pre_compile(BuildWorkload2 &, std::size_t, std::size_t, std::size_t) {
        this->shader_type = HEK::ShaderType::SHADER_TYPE_SHADER_TRANSPARENT_GENERIC;
    }
}
