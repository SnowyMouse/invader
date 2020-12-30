// SPDX-License-Identifier: GPL-3.0-only

#include <invader/build/build_workload.hpp>
#include <invader/tag/parser/parser.hpp>

namespace Invader::Parser {
    static std::uint16_t convert_shader_type(const BuildWorkload &workload, HEK::ShaderType pc_input) noexcept {
        if(workload.get_build_parameters()->details.build_cache_file_engine == HEK::CacheFileEngine::CACHE_FILE_XBOX && pc_input >= HEK::SHADER_TYPE_SHADER_TRANSPARENT_CHICAGO_EXTENDED) { // xbox version doesn't have this
            return pc_input - 1;
        }
        else {
            return pc_input;
        }
    }
    
    void ShaderEnvironment::pre_compile(BuildWorkload &workload, std::size_t, std::size_t, std::size_t) {
        this->shader_type = convert_shader_type(workload, HEK::ShaderType::SHADER_TYPE_SHADER_ENVIRONMENT);
        this->bump_map_scale_xy.x = this->bump_map_scale;
        this->bump_map_scale_xy.y = this->bump_map_scale;
        if(this->material_color.red == 0.0F && this->material_color.green == 0.0F && this->material_color.blue == 0.0F) {
            this->material_color.red = 1.0F;
            this->material_color.green = 1.0F;
            this->material_color.blue = 1.0F;
        }
    }
    void ShaderModel::pre_compile(BuildWorkload &workload, std::size_t tag_index, std::size_t, std::size_t) {
        this->shader_type = convert_shader_type(workload, HEK::ShaderType::SHADER_TYPE_SHADER_MODEL);
        this->unknown = 1.0F;
        
        if(this->reflection_falloff_distance >= this->reflection_cutoff_distance && (this->reflection_cutoff_distance != 0.0F && this->reflection_falloff_distance != 0.0F)) {
            REPORT_ERROR_PRINTF(workload, ERROR_TYPE_WARNING_PEDANTIC, tag_index, "Reflection falloff is greater than or equal to cutoff, so both of these values were set to 0 (%f >= %f)", this->reflection_falloff_distance, this->reflection_cutoff_distance);
            this->reflection_cutoff_distance = 0.0F;
            this->reflection_falloff_distance = 0.0F;
        }
    }
    
    void ShaderTransparentChicago::pre_compile(BuildWorkload &workload, std::size_t, std::size_t, std::size_t) {
        this->shader_type = convert_shader_type(workload, HEK::ShaderType::SHADER_TYPE_SHADER_TRANSPARENT_CHICAGO);
    }
    void ShaderTransparentChicagoExtended::pre_compile(BuildWorkload &workload, std::size_t tag_index, std::size_t, std::size_t) {
        // Error if the target engine can't use it
        if(workload.get_build_parameters()->details.build_cache_file_engine == HEK::CacheFileEngine::CACHE_FILE_XBOX) {
            workload.report_error(BuildWorkload::ErrorType::ERROR_TYPE_FATAL_ERROR, "shader_transparent_chicago_extended tags do not exist on the target engine. Use shader_transparent_chicago, instead.", tag_index);
            throw InvalidTagDataException();
        }
        
        // Why would you use shader_transparent_chicago_extended if you don't use two stage maps?
        if(this->maps_2_stage.size() == 0) {
            workload.report_error(BuildWorkload::ErrorType::ERROR_TYPE_FATAL_ERROR, "There are no 2-stage maps. If you do not want 2-stage maps, use shader_transparent_chicago, instead.", tag_index);
            throw InvalidTagDataException();
        }
        
        this->shader_type = convert_shader_type(workload, HEK::ShaderType::SHADER_TYPE_SHADER_TRANSPARENT_CHICAGO_EXTENDED);
    }
    
    void ShaderTransparentWater::pre_compile(BuildWorkload &workload, std::size_t, std::size_t, std::size_t) {
        this->shader_type = convert_shader_type(workload, HEK::ShaderType::SHADER_TYPE_SHADER_TRANSPARENT_WATER);
    }
    void ShaderTransparentGlass::pre_compile(BuildWorkload &workload, std::size_t, std::size_t, std::size_t) {
        this->shader_type = convert_shader_type(workload, HEK::ShaderType::SHADER_TYPE_SHADER_TRANSPARENT_GLASS);
    }
    void ShaderTransparentMeter::pre_compile(BuildWorkload &workload, std::size_t, std::size_t, std::size_t) {
        this->shader_type = convert_shader_type(workload, HEK::ShaderType::SHADER_TYPE_SHADER_TRANSPARENT_METER);
    }
    void ShaderTransparentPlasma::pre_compile(BuildWorkload &workload, std::size_t, std::size_t, std::size_t) {
        this->shader_type = convert_shader_type(workload, HEK::ShaderType::SHADER_TYPE_SHADER_TRANSPARENT_PLASMA);
    }
    void ShaderTransparentGeneric::pre_compile(BuildWorkload &workload, std::size_t tag_index, std::size_t, std::size_t) {
        // Warn if the target engine can't render it
        switch(workload.get_build_parameters()->details.build_cache_file_engine) {
            case HEK::CacheFileEngine::CACHE_FILE_DEMO:
            case HEK::CacheFileEngine::CACHE_FILE_RETAIL:
            case HEK::CacheFileEngine::CACHE_FILE_CUSTOM_EDITION:
                workload.report_error(BuildWorkload::ErrorType::ERROR_TYPE_WARNING, "shader_transparent_generic tags will not render on the target engine", tag_index);
                break;
            default: break;
        }
        
        this->shader_type = convert_shader_type(workload, HEK::ShaderType::SHADER_TYPE_SHADER_TRANSPARENT_GENERIC);
    }
}
