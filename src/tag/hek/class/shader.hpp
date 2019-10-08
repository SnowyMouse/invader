// SPDX-License-Identifier: GPL-3.0-only

#ifndef INVADER__TAG__HEK__CLASS__SHADER_HPP
#define INVADER__TAG__HEK__CLASS__SHADER_HPP

#include "../../compiled_tag.hpp"
#include "../../../hek/data_type.hpp"
#include "../header.hpp"
#include "enum.hpp"

namespace Invader::HEK {
    SINGLE_DEPENDENCY_STRUCT(ShaderTransparentExtraLayer, shader); //shader

    struct ShaderFlags {
        std::uint16_t simple_parameterization : 1;
        std::uint16_t ignore_normals : 1;
        std::uint16_t transparent_lit : 1;
    };

    struct ShaderPhysicsFlags {
        std::uint16_t unused : 1;
    };

    enum ShaderDetailLevel : TagEnum {
        SHADER_DETAIL_LEVEL_HIGH,
        SHADER_DETAIL_LEVEL_MEDIUM,
        SHADER_DETAIL_LEVEL_LOW,
        SHADER_DETAIL_LEVEL_TURD
    };

    enum ShaderColorFunctionType : TagEnum {
        SHADER_COLOR_FUNCTION_TYPE_CURRENT,
        SHADER_COLOR_FUNCTION_TYPE_NEXT_MAP,
        SHADER_COLOR_FUNCTION_TYPE_MULTIPLY,
        SHADER_COLOR_FUNCTION_TYPE_DOUBLE_MULTIPLY,
        SHADER_COLOR_FUNCTION_TYPE_ADD,
        SHADER_COLOR_FUNCTION_TYPE_ADD_SIGNED_CURRENT,
        SHADER_COLOR_FUNCTION_TYPE_ADD_SIGNED_NEXT_MAP,
        SHADER_COLOR_FUNCTION_TYPE_SUBTRACT_CURRENT,
        SHADER_COLOR_FUNCTION_TYPE_SUBTRACT_NEXT_MAP,
        SHADER_COLOR_FUNCTION_TYPE_BLEND_CURRENT_ALPHA,
        SHADER_COLOR_FUNCTION_TYPE_BLEND_CURRENT_ALPHA_INVERSE,
        SHADER_COLOR_FUNCTION_TYPE_BLEND_NEXT_MAP_ALPHA,
        SHADER_COLOR_FUNCTION_TYPE_BLEND_NEXT_MAP_ALPHA_INVERSE
    };

    enum ShaderAnimationSource : TagEnum {
        SHADER_ANIMATION_SOURCE_NONE,
        SHADER_ANIMATION_SOURCE_A_OUT,
        SHADER_ANIMATION_SOURCE_B_OUT,
        SHADER_ANIMATION_SOURCE_C_OUT,
        SHADER_ANIMATION_SOURCE_D_OUT
    };

    enum ShaderFirstMapType : TagEnum {
        SHADER_FIRST_MAP_TYPE_2D_MAP,
        SHADER_FIRST_MAP_TYPE_FIRST_MAP_IS_REFLECTION_CUBE_MAP,
        SHADER_FIRST_MAP_TYPE_FIRST_MAP_IS_OBJECT_CENTERED_CUBE_MAP,
        SHADER_FIRST_MAP_TYPE_FIRST_MAP_IS_VIEWER_CENTERED_CUBE_MAP
    };

    enum ShaderFramebufferFadeMode : TagEnum {
        SHADER_FRAMEBUFFER_FADE_MODE_NONE,
        SHADER_FRAMEBUFFER_FADE_MODE_FADE_WHEN_PERPENDICULAR,
        SHADER_FRAMEBUFFER_FADE_MODE_FADE_WHEN_PARALLEL
    };

    enum ShaderType : std::uint8_t {
        SHADER_UNKN0 = 0x0, //unknown
        SHADER_UNKN1 = 0x1, //unknown
        SHADER_UNKN2 = 0x2, //unknown
        SHADER_TYPE_SHADER_ENVIRONMENT = 0x3,
        SHADER_TYPE_SHADER_MODEL = 0x4,
        SHADER_TYPE_SHADER_TRANSPARENT_GENERIC = 0x5,
        SHADER_TYPE_SHADER_TRANSPARENT_CHICAGO = 0x6,
        SHADER_TYPE_SHADER_TRANSPARENT_CHICAGO_EXTENDED = 0x7,
        SHADER_TYPE_SHADER_TRANSPARENT_WATER = 0x8,
        SHADER_TYPE_SHADER_TRANSPARENT_GLASS = 0x9,
        SHADER_TYPE_SHADER_TRANSPARENT_METER = 0xA,
        SHADER_TYPE_SHADER_TRANSPARENT_PLASMA = 0xB
    };

    ENDIAN_TEMPLATE(EndianType) struct Shader {
        EndianType<ShaderFlags> shader_flags;
        EndianType<ShaderDetailLevel> detail_level;
        EndianType<float> power;
        ColorRGB<EndianType> color_of_emitted_light;
        ColorRGB<EndianType> tint_color;
        ShaderPhysicsFlags physics_flags;
        EndianType<MaterialType> material_type;
        std::uint8_t shader_type;
        PAD(0x3);
    };
    static_assert(sizeof(Shader<BigEndian>) == 0x28);

    #define MAKE_WHITE_IF_BLACK(what) if(what.red == 0.0f && what.green == 0.0f && what.blue == 0.0f) { what.red = 1.0f; what.green = 1.0f; what.blue = 1.0f; }
    #define MAKE_ONE_IF_ZERO(what) if(what == 0.0f) { what = 1.0f; }

    #define INITIALIZE_SHADER_ANIMATION(what) DEFAULT_VALUE(what .u_animation_period, 1.0f); \
                                              DEFAULT_VALUE(what .u_animation_scale, 1.0f); \
                                              DEFAULT_VALUE(what .v_animation_period, 1.0f); \
                                              DEFAULT_VALUE(what .v_animation_scale, 1.0f); \
                                              DEFAULT_VALUE(what .rotation_animation_period, 1.0f); \
                                              DEFAULT_VALUE(what .rotation_animation_scale, 360.0f);

    #define COPY_SHADER_DATA COPY_THIS(shader_flags); \
                             COPY_THIS(detail_level); \
                             COPY_THIS(power); \
                             COPY_THIS(color_of_emitted_light); \
                             COPY_THIS(tint_color); \
                             COPY_THIS(physics_flags); \
                             COPY_THIS(material_type); \
                             COPY_THIS(shader_type);

}
#endif
