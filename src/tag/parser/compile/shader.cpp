// SPDX-License-Identifier: GPL-3.0-only

#include <invader/build/build_workload.hpp>
#include <invader/tag/parser/parser.hpp>
#include <invader/tag/parser/compile/bitmap.hpp>

namespace Invader::Parser {
    static std::uint16_t convert_shader_type(const BuildWorkload &workload, HEK::ShaderType pc_input) noexcept {
        if(workload.get_build_parameters()->details.build_cache_file_engine == HEK::CacheFileEngine::CACHE_FILE_XBOX && pc_input >= HEK::SHADER_TYPE_TRANSPARENT_CHICAGO_EXTENDED) { // xbox version doesn't have this
            return pc_input - 1;
        }
        else {
            return pc_input;
        }
    }

    static void verify_bitmap_is_type(BuildWorkload &workload, HEK::BitmapType expected_type, std::size_t shader_tag_index, HEK::TagDependency<HEK::LittleEndian> &dependency, const char *what) {
        if(workload.disable_recursion) {
            return;
        }

        auto tag_id = dependency.tag_id.read();
        if(tag_id.is_null()) {
            return;
        }

        auto &bitmap = workload.tags[tag_id.index];
        if(!bitmap.base_struct.has_value()) {
            return;
        }
        auto bitmap_type = reinterpret_cast<Bitmap::struct_little *>(workload.structs[*bitmap.base_struct].data.data())->type;
        if(bitmap_type != expected_type) {
            REPORT_ERROR_PRINTF(workload, ERROR_TYPE_FATAL_ERROR, shader_tag_index, "%s references %s bitmap '%s.bitmap' where a %s was expected", what, HEK::BitmapType_to_string(bitmap_type), File::halo_path_to_preferred_path(bitmap.path).c_str(), HEK::BitmapType_to_string(expected_type));
            throw InvalidTagDataException();
        }
    }

    template<typename T> static void default_maps(T &maps) {
        for(auto &i : maps) {
            if(i.map_u_scale == 0.0F && i.map_v_scale == 0.0F) {
                i.map_u_scale = 1.0F;
                i.map_v_scale = 1.0F;
            }
            else if(i.map_u_scale == 0.0F) {
                i.map_u_scale = i.map_v_scale;
            }
            else if(i.map_v_scale == 0.0F) {
                i.map_v_scale = i.map_u_scale;
            }
        }
    }

    void ShaderEnvironment::pre_compile(BuildWorkload &workload, std::size_t, std::size_t, std::size_t) {
        this->shader_type = convert_shader_type(workload, HEK::ShaderType::SHADER_TYPE_ENVIRONMENT);
        this->bump_map_scale_xy.x = this->bump_map_scale;
        this->bump_map_scale_xy.y = this->bump_map_scale;
        if(this->material_color.red == 0.0F && this->material_color.green == 0.0F && this->material_color.blue == 0.0F) {
            this->material_color.red = 1.0F;
            this->material_color.green = 1.0F;
            this->material_color.blue = 1.0F;
        }
    }

    #define GET_SHADER_STRUCT auto &s = workload.structs[struct_index]; auto &s_data = *reinterpret_cast<struct_little *>(s.data.data());
    #define VERIFY_BITMAP_IS_TYPE(what, type) verify_bitmap_is_type(workload, HEK::BitmapType::type, tag_index, s_data.what, # what);

    void ShaderEnvironment::post_compile(BuildWorkload &workload, std::size_t tag_index, std::size_t struct_index, std::size_t) {
        if(!this->base_map.tag_id.is_null()) {
            set_bitmap_data_environment_flag(workload, this->base_map.tag_id.index);
        }

        GET_SHADER_STRUCT
        VERIFY_BITMAP_IS_TYPE(map, BITMAP_TYPE_2D_TEXTURES);
        VERIFY_BITMAP_IS_TYPE(base_map, BITMAP_TYPE_2D_TEXTURES);
        VERIFY_BITMAP_IS_TYPE(primary_detail_map, BITMAP_TYPE_2D_TEXTURES);
        VERIFY_BITMAP_IS_TYPE(secondary_detail_map, BITMAP_TYPE_2D_TEXTURES);
        VERIFY_BITMAP_IS_TYPE(bump_map, BITMAP_TYPE_2D_TEXTURES);
        VERIFY_BITMAP_IS_TYPE(micro_detail_map, BITMAP_TYPE_2D_TEXTURES);
        VERIFY_BITMAP_IS_TYPE(reflection_cube_map, BITMAP_TYPE_CUBE_MAPS);
    }

    void ShaderModel::pre_compile(BuildWorkload &workload, std::size_t tag_index, std::size_t, std::size_t) {
        this->shader_type = convert_shader_type(workload, HEK::ShaderType::SHADER_TYPE_MODEL);
        this->unknown = 1.0F;

        if(this->map_u_scale == 0.0F && this->map_v_scale == 0.0F) {
            this->map_u_scale = 1.0F;
            this->map_v_scale = 1.0F;
        }
        else if(this->map_u_scale == 0.0F) {
            this->map_u_scale = this->map_v_scale;
        }
        else if(this->map_v_scale == 0.0F) {
            this->map_v_scale = this->map_u_scale;
        }

        if(this->reflection_falloff_distance >= this->reflection_cutoff_distance && (this->reflection_cutoff_distance != 0.0F && this->reflection_falloff_distance != 0.0F)) {
            REPORT_ERROR_PRINTF(workload, ERROR_TYPE_WARNING_PEDANTIC, tag_index, "Reflection falloff is greater than or equal to cutoff, so both of these values were set to 0 (%f >= %f)", this->reflection_falloff_distance, this->reflection_cutoff_distance);
            this->reflection_cutoff_distance = 0.0F;
            this->reflection_falloff_distance = 0.0F;
        }
    }

    void ShaderModel::post_compile(BuildWorkload &workload, std::size_t tag_index, std::size_t struct_index, std::size_t) {
        GET_SHADER_STRUCT
        VERIFY_BITMAP_IS_TYPE(base_map, BITMAP_TYPE_2D_TEXTURES);
        VERIFY_BITMAP_IS_TYPE(multipurpose_map, BITMAP_TYPE_2D_TEXTURES);
        VERIFY_BITMAP_IS_TYPE(detail_map, BITMAP_TYPE_2D_TEXTURES);
        VERIFY_BITMAP_IS_TYPE(reflection_cube_map, BITMAP_TYPE_CUBE_MAPS);
    }

    void ShaderTransparentChicago::pre_compile(BuildWorkload &workload, std::size_t, std::size_t, std::size_t) {
        this->shader_type = convert_shader_type(workload, HEK::ShaderType::SHADER_TYPE_TRANSPARENT_CHICAGO);
        default_maps(this->maps);
    }

    template <typename Array> static void check_generic_transparent_shader(BuildWorkload &workload, std::size_t tag_index, Invader::BuildWorkload::BuildWorkloadStruct &base_struct, const Array &array, const HEK::ShaderFirstMapType first_map_type) {
        std::size_t count = array.count;
        if(count > 0) {
            auto &s = workload.structs[*base_struct.resolve_pointer(&array.pointer)];
            auto *data = reinterpret_cast<typename Array::struct_type_little *>(s.data.data());
            for(std::size_t c = 0; c < count; c++) {
                auto &s_data = data[c];
                verify_bitmap_is_type(workload,
                                      (c == 0 && first_map_type != HEK::ShaderFirstMapType::SHADER_FIRST_MAP_TYPE_2D_MAP) ? HEK::BitmapType::BITMAP_TYPE_CUBE_MAPS
                                                                                                                          : HEK::BitmapType::BITMAP_TYPE_2D_TEXTURES,
                                      tag_index,
                                      s_data.map,
                                      "map");
            }
        }
    };

    void ShaderTransparentChicago::post_compile(BuildWorkload &workload, std::size_t tag_index, std::size_t struct_index, std::size_t) {
        auto &s = workload.structs[struct_index];
        auto &s_data = *reinterpret_cast<struct_little *>(s.data.data());
        check_generic_transparent_shader(workload, tag_index, s, s_data.maps, s_data.first_map_type);
    }

    void ShaderTransparentChicagoExtended::pre_compile(BuildWorkload &workload, std::size_t tag_index, std::size_t, std::size_t) {
        // Error if the target engine can't use it
        if(workload.get_build_parameters()->details.build_cache_file_engine == HEK::CacheFileEngine::CACHE_FILE_XBOX) {
            workload.report_error(BuildWorkload::ErrorType::ERROR_TYPE_FATAL_ERROR, "shader_transparent_chicago_extended tags do not exist on the target engine. Use shader_transparent_chicago, instead.", tag_index);
            throw InvalidTagDataException();
        }

        this->shader_type = HEK::ShaderType::SHADER_TYPE_TRANSPARENT_CHICAGO_EXTENDED;
        default_maps(this->maps_4_stage);
        default_maps(this->maps_2_stage);
    }

    void ShaderTransparentChicagoExtended::post_compile(BuildWorkload &workload, std::size_t tag_index, std::size_t struct_index, std::size_t) {
        GET_SHADER_STRUCT
        check_generic_transparent_shader(workload, tag_index, s, s_data.maps_2_stage, s_data.first_map_type);
        check_generic_transparent_shader(workload, tag_index, s, s_data.maps_4_stage, s_data.first_map_type);
    }

    void ShaderTransparentGeneric::pre_compile(BuildWorkload &workload, std::size_t tag_index, std::size_t, std::size_t) {
        // Warn if the target engine can't render it
        auto engine = workload.get_build_parameters()->details.build_cache_file_engine;
        if(engine == HEK::CacheFileEngine::CACHE_FILE_DEMO || engine == HEK::CacheFileEngine::CACHE_FILE_RETAIL) {
            workload.report_error(BuildWorkload::ErrorType::ERROR_TYPE_WARNING, "shader_transparent_generic tags will not render on the target engine.", tag_index);
        }

        if(this->maps.size() == 0 && this->stages.size() == 0) {
            workload.report_error(BuildWorkload::ErrorType::ERROR_TYPE_FATAL_ERROR, "shader_transparent_generic tags must contain at least either one map or one stage", tag_index);
            throw InvalidTagDataException();
        }

        default_maps(this->maps);

        this->shader_type = convert_shader_type(workload, HEK::ShaderType::SHADER_TYPE_TRANSPARENT_GENERIC);
    }

    void ShaderTransparentGeneric::post_compile(BuildWorkload &workload, std::size_t tag_index, std::size_t struct_index, std::size_t) {
        GET_SHADER_STRUCT
        check_generic_transparent_shader(workload, tag_index, s, s_data.maps, s_data.first_map_type);
    }

    void ShaderTransparentWater::pre_compile(BuildWorkload &workload, std::size_t, std::size_t, std::size_t) {
        this->shader_type = convert_shader_type(workload, HEK::ShaderType::SHADER_TYPE_TRANSPARENT_WATER);
    }

    void ShaderTransparentWater::post_compile(BuildWorkload &workload, std::size_t tag_index, std::size_t struct_index, std::size_t) {
        GET_SHADER_STRUCT
        VERIFY_BITMAP_IS_TYPE(base_map, BITMAP_TYPE_2D_TEXTURES);
        VERIFY_BITMAP_IS_TYPE(reflection_map, BITMAP_TYPE_CUBE_MAPS);
        VERIFY_BITMAP_IS_TYPE(ripple_maps, BITMAP_TYPE_2D_TEXTURES);
    }

    void ShaderTransparentGlass::pre_compile(BuildWorkload &workload, std::size_t, std::size_t, std::size_t) {
        this->shader_type = convert_shader_type(workload, HEK::ShaderType::SHADER_TYPE_TRANSPARENT_GLASS);
    }

    void ShaderTransparentGlass::post_compile(BuildWorkload &workload, std::size_t tag_index, std::size_t struct_index, std::size_t) {
        GET_SHADER_STRUCT
        VERIFY_BITMAP_IS_TYPE(bump_map, BITMAP_TYPE_2D_TEXTURES);
        VERIFY_BITMAP_IS_TYPE(diffuse_map, BITMAP_TYPE_2D_TEXTURES);
        VERIFY_BITMAP_IS_TYPE(diffuse_detail_map, BITMAP_TYPE_2D_TEXTURES);
        VERIFY_BITMAP_IS_TYPE(specular_detail_map, BITMAP_TYPE_2D_TEXTURES);
        VERIFY_BITMAP_IS_TYPE(specular_map, BITMAP_TYPE_CUBE_MAPS);
        VERIFY_BITMAP_IS_TYPE(reflection_map, BITMAP_TYPE_CUBE_MAPS);
    }

    void ShaderTransparentMeter::pre_compile(BuildWorkload &workload, std::size_t, std::size_t, std::size_t) {
        this->shader_type = convert_shader_type(workload, HEK::ShaderType::SHADER_TYPE_TRANSPARENT_METER);
    }

    void ShaderTransparentMeter::post_compile(BuildWorkload &workload, std::size_t tag_index, std::size_t struct_index, std::size_t) {
        GET_SHADER_STRUCT
        VERIFY_BITMAP_IS_TYPE(map, BITMAP_TYPE_2D_TEXTURES);
    }

    void ShaderTransparentPlasma::pre_compile(BuildWorkload &workload, std::size_t, std::size_t, std::size_t) {
        this->shader_type = convert_shader_type(workload, HEK::ShaderType::SHADER_TYPE_TRANSPARENT_PLASMA);
    }

    void ShaderTransparentPlasma::post_compile(BuildWorkload &workload, std::size_t tag_index, std::size_t struct_index, std::size_t) {
        GET_SHADER_STRUCT
        VERIFY_BITMAP_IS_TYPE(primary_noise_map, BITMAP_TYPE_3D_TEXTURES);
        VERIFY_BITMAP_IS_TYPE(secondary_noise_map, BITMAP_TYPE_3D_TEXTURES);
    }

    ShaderTransparentChicago convert_shader_transparent_chicago_extended_to_shader_transparent_chicago(const ShaderTransparentChicagoExtended &shader) {
        ShaderTransparentChicago tag;
        tag.shader_flags = shader.shader_flags;
        tag.detail_level = shader.detail_level;
        tag.power = shader.power;
        tag.color_of_emitted_light = shader.color_of_emitted_light;
        tag.tint_color = shader.tint_color;
        tag.physics_flags = shader.physics_flags;
        tag.material_type = shader.material_type;
        tag.numeric_counter_limit = shader.numeric_counter_limit;
        tag.shader_transparent_chicago_flags = shader.shader_transparent_chicago_extended_flags;
        tag.first_map_type = shader.first_map_type;
        tag.framebuffer_blend_function = shader.framebuffer_blend_function;
        tag.framebuffer_fade_mode = shader.framebuffer_fade_mode;
        tag.framebuffer_fade_source = shader.framebuffer_fade_source;
        tag.lens_flare_spacing = shader.lens_flare_spacing;
        tag.lens_flare = shader.lens_flare;
        tag.extra_layers = shader.extra_layers;
        tag.maps = shader.maps_4_stage;
        tag.extra_flags = shader.extra_flags;
        return tag;
    }
}
