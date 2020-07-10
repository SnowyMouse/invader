// SPDX-License-Identifier: GPL-3.0-only

#include <invader/build/build_workload.hpp>
#include <invader/tag/parser/parser.hpp>

namespace Invader::Parser {
    void ShaderEnvironment::pre_compile(BuildWorkload &, std::size_t, std::size_t, std::size_t) {
        this->shader_type = HEK::ShaderType::SHADER_TYPE_SHADER_ENVIRONMENT;
        this->bump_map_scale_xy.x = this->bump_map_scale;
        this->bump_map_scale_xy.y = this->bump_map_scale;
        if(this->material_color.red == 0.0F && this->material_color.green == 0.0F && this->material_color.blue == 0.0F) {
            this->material_color.red = 1.0F;
            this->material_color.green = 1.0F;
            this->material_color.blue = 1.0F;
        }
    }
    void ShaderModel::pre_compile(BuildWorkload &workload, std::size_t tag_index, std::size_t, std::size_t) {
        this->shader_type = HEK::ShaderType::SHADER_TYPE_SHADER_MODEL;
        this->unknown = 1.0F;
        
        // Do we have a multipurpose? If so, what type of flag?
        if(this->multipurpose_map.path.size()) {
            bool uses_xbox_multi_order = this->shader_model_flags & HEK::ShaderModelFlagsFlag::SHADER_MODEL_FLAGS_FLAG_USE_XBOX_MULTIPURPOSE_CHANNEL_ORDER;
            bool clear_flag = false;
            
            switch(workload.engine_target) {
                case HEK::CacheFileEngine::CACHE_FILE_XBOX:
                    if(!uses_xbox_multi_order) {
                        workload.report_error(BuildWorkload::ErrorType::ERROR_TYPE_WARNING, "The target engine requires Xbox multipurpose channel order; the resulting shader may not appear as intended", tag_index);
                    }
                    clear_flag = true;
                    break;
                case HEK::CacheFileEngine::CACHE_FILE_RETAIL:
                case HEK::CacheFileEngine::CACHE_FILE_DEMO:
                case HEK::CacheFileEngine::CACHE_FILE_CUSTOM_EDITION:
                    if(uses_xbox_multi_order) {
                        workload.report_error(BuildWorkload::ErrorType::ERROR_TYPE_WARNING, "The target engine does not support Xbox multipurpose channel order; the resulting shader may not appear as intended", tag_index);
                    }
                    clear_flag = true;
                    break;
                default:
                    break;
            }
        
            // Clear the flag when putting it in a map if we need to
            if(clear_flag) {
                this->shader_model_flags &= ~HEK::ShaderModelFlagsFlag::SHADER_MODEL_FLAGS_FLAG_USE_XBOX_MULTIPURPOSE_CHANNEL_ORDER;
            }
        }
    }
    void ShaderModel::post_cache_parse(const Invader::Tag &tag, std::optional<HEK::Pointer>) {
        // If we have a multipurpose and we're Xbox, set the flag. Otherwise, negate it.
        if(tag.get_map().get_engine() == HEK::CacheFileEngine::CACHE_FILE_XBOX && this->multipurpose_map.path.size() > 0) {
            this->shader_model_flags |= HEK::ShaderModelFlagsFlag::SHADER_MODEL_FLAGS_FLAG_USE_XBOX_MULTIPURPOSE_CHANNEL_ORDER;
        }
        else {
            this->shader_model_flags &= ~HEK::ShaderModelFlagsFlag::SHADER_MODEL_FLAGS_FLAG_USE_XBOX_MULTIPURPOSE_CHANNEL_ORDER;
        }
    }
    
    void ShaderTransparentChicago::pre_compile(BuildWorkload &, std::size_t, std::size_t, std::size_t) {
        this->shader_type = HEK::ShaderType::SHADER_TYPE_SHADER_TRANSPARENT_CHICAGO;
    }
    void ShaderTransparentChicagoExtended::pre_compile(BuildWorkload &workload, std::size_t tag_index, std::size_t, std::size_t) {
        // Error if the target engine can't use it
        if(workload.engine_target == HEK::CacheFileEngine::CACHE_FILE_XBOX) {
            workload.report_error(BuildWorkload::ErrorType::ERROR_TYPE_ERROR, "shader_transparent_chicago_extended tags do not exist on the target engine", tag_index);
        }
        
        this->shader_type = HEK::ShaderType::SHADER_TYPE_SHADER_TRANSPARENT_CHICAGO_EXTENDED;
    }
    void ShaderTransparentWater::pre_compile(BuildWorkload &, std::size_t, std::size_t, std::size_t) {
        this->shader_type = HEK::ShaderType::SHADER_TYPE_SHADER_TRANSPARENT_WATER;
    }
    void ShaderTransparentGlass::pre_compile(BuildWorkload &, std::size_t, std::size_t, std::size_t) {
        this->shader_type = HEK::ShaderType::SHADER_TYPE_SHADER_TRANSPARENT_GLASS;
    }
    void ShaderTransparentMeter::pre_compile(BuildWorkload &, std::size_t, std::size_t, std::size_t) {
        this->shader_type = HEK::ShaderType::SHADER_TYPE_SHADER_TRANSPARENT_METER;
    }
    void ShaderTransparentPlasma::pre_compile(BuildWorkload &, std::size_t, std::size_t, std::size_t) {
        this->shader_type = HEK::ShaderType::SHADER_TYPE_SHADER_TRANSPARENT_PLASMA;
    }
    void ShaderTransparentGeneric::pre_compile(BuildWorkload &workload, std::size_t tag_index, std::size_t, std::size_t) {
        // Warn if the target engine can't render it
        switch(workload.engine_target) {
            case HEK::CacheFileEngine::CACHE_FILE_DEMO:
            case HEK::CacheFileEngine::CACHE_FILE_RETAIL:
            case HEK::CacheFileEngine::CACHE_FILE_CUSTOM_EDITION:
                workload.report_error(BuildWorkload::ErrorType::ERROR_TYPE_WARNING, "shader_transparent_generic tags will not render on the target engine", tag_index);
                break;
            default: break;
        }
        
        this->shader_type = HEK::ShaderType::SHADER_TYPE_SHADER_TRANSPARENT_GENERIC;
    }
}
