/*
 * Invader (c) 2018 Kavawuvi
 *
 * This program is free software under the GNU General Public License v3.0 or later. See LICENSE for more information.
 */

#pragma once

#include "../../compiled_tag.hpp"
#include "../../../hek/data_type.hpp"
#include "../header.hpp"

namespace Invader::HEK {
    enum ObjectAttachmentPrimaryScale : TagEnum {
        OBJECT_ATTACHMENT_PRIMARY_SCALE_NONE,
        OBJECT_ATTACHMENT_PRIMARY_SCALE_A_OUT,
        OBJECT_ATTACHMENT_PRIMARY_SCALE_B_OUT,
        OBJECT_ATTACHMENT_PRIMARY_SCALE_C_OUT,
        OBJECT_ATTACHMENT_PRIMARY_SCALE_D_OUT
    };
    enum ObjectAttachmentChangeColor : TagEnum {
        OBJECT_ATTACHMENT_CHANGE_COLOR_NONE,
        OBJECT_ATTACHMENT_CHANGE_COLOR_A,
        OBJECT_ATTACHMENT_CHANGE_COLOR_B,
        OBJECT_ATTACHMENT_CHANGE_COLOR_C,
        OBJECT_ATTACHMENT_CHANGE_COLOR_D
    };

    ENDIAN_TEMPLATE(EndianType) struct ObjectAttachment {
        TagDependency<EndianType> type; // light, light_volume, contrail, particle_system, effect, sound_looping
        TagString marker;
        EndianType<ObjectAttachmentPrimaryScale> primary_scale;
        EndianType<ObjectAttachmentPrimaryScale> secondary_scale;
        EndianType<ObjectAttachmentChangeColor> change_color;
        PAD(0x2);
        PAD(0x10);

        ENDIAN_TEMPLATE(NewEndian) operator ObjectAttachment<NewEndian>() const noexcept {
            ObjectAttachment<NewEndian> copy = {};
            COPY_THIS(type);
            COPY_THIS(marker);
            COPY_THIS(primary_scale);
            COPY_THIS(secondary_scale);
            COPY_THIS(change_color);
            return copy;
        }
    };
    static_assert(sizeof(ObjectAttachment<BigEndian>) == 0x48);

    SINGLE_DEPENDENCY_PADDED_STRUCT(ObjectWidget, reference, 0x10);

    struct ObjectFunctionFlags {
        std::uint32_t invert : 1;
        std::uint32_t additive : 1;
        std::uint32_t always_active : 1;
    };

    ENDIAN_TEMPLATE(EndianType) struct ObjectFunction {
        EndianType<ObjectFunctionFlags> flags;
        EndianType<float> period;
        EndianType<FunctionScaleBy> scale_period_by;
        EndianType<FunctionType2> function;
        EndianType<FunctionScaleBy> scale_function_by;
        EndianType<FunctionType2> wobble_function;
        EndianType<float> wobble_period;
        EndianType<float> wobble_magnitude;
        EndianType<Fraction> square_wave_threshold;
        EndianType<std::int16_t> step_count;
        EndianType<FunctionType> map_to;
        EndianType<std::int16_t> sawtooth_count;
        EndianType<FunctionScaleBy> add;
        EndianType<FunctionScaleBy> scale_result_by;
        EndianType<FunctionBoundsMode> bounds_mode;
        Bounds<EndianType<Fraction>> bounds;
        PAD(0x4);
        PAD(0x2);
        EndianType<std::int16_t> turn_off_with;
        EndianType<float> scale_by;
        PAD(0xFC);
        LittleEndian<float> inverse_bounds;
        LittleEndian<float> unknown_float_2;
        LittleEndian<float> inverse_step;
        LittleEndian<float> inverse_period;
        TagString usage;

        ENDIAN_TEMPLATE(NewEndian) operator ObjectFunction<NewEndian>() const noexcept {
            ObjectFunction<NewEndian> copy = {};
            COPY_THIS(flags);
            COPY_THIS(period);
            COPY_THIS(scale_period_by);
            COPY_THIS(function);
            COPY_THIS(scale_function_by);
            COPY_THIS(wobble_function);
            COPY_THIS(wobble_period);
            COPY_THIS(wobble_magnitude);
            COPY_THIS(square_wave_threshold);
            COPY_THIS(step_count);
            COPY_THIS(map_to);
            COPY_THIS(sawtooth_count);
            COPY_THIS(add);
            COPY_THIS(scale_result_by);
            COPY_THIS(bounds_mode);
            COPY_THIS(bounds);
            COPY_THIS(turn_off_with);
            COPY_THIS(scale_by);
            COPY_THIS(inverse_bounds);
            COPY_THIS(unknown_float_2);
            COPY_THIS(inverse_step);
            COPY_THIS(inverse_period);
            COPY_THIS(usage);
            return copy;
        }
    };
    static_assert(sizeof(ObjectFunction<BigEndian>) == 0x168);

    ENDIAN_TEMPLATE(EndianType) struct ObjectChangeColorsPermutation {
        EndianType<float> weight;
        ColorRGB<EndianType> color_lower_bound;
        ColorRGB<EndianType> color_upper_bound;

        ENDIAN_TEMPLATE(NewEndian) operator ObjectChangeColorsPermutation<NewEndian>() const noexcept {
            ObjectChangeColorsPermutation<NewEndian> copy;
            COPY_THIS(weight);
            COPY_THIS(color_lower_bound);
            COPY_THIS(color_upper_bound);
            return copy;
        }
    };
    static_assert(sizeof(ObjectChangeColorsPermutation<BigEndian>) == 0x1C);

    struct ObjectChangeColorsScaleFlags {
        std::uint32_t blend_in_hsv : 1;
        std::uint32_t _more_colors : 1;
    };

    ENDIAN_TEMPLATE(EndianType) struct ObjectChangeColors {
        EndianType<FunctionScaleBy> darken_by;
        EndianType<FunctionScaleBy> scale_by;
        EndianType<ObjectChangeColorsScaleFlags> flags;
        ColorRGB<EndianType> color_lower_bound;
        ColorRGB<EndianType> color_upper_bound;
        TagReflexive<EndianType, ObjectChangeColorsPermutation> permutations;

        ENDIAN_TEMPLATE(NewEndian) operator ObjectChangeColors<NewEndian>() const noexcept {
            ObjectChangeColors<NewEndian> copy;
            COPY_THIS(darken_by);
            COPY_THIS(scale_by);
            COPY_THIS(flags);
            COPY_THIS(color_lower_bound);
            COPY_THIS(color_upper_bound);
            COPY_THIS(permutations);
            return copy;
        }
    };
    static_assert(sizeof(ObjectChangeColors<BigEndian>) == 0x2C);

    struct ObjectFlags {
        std::uint16_t does_not_cast_shadow : 1;
        std::uint16_t transparent_self_occlusion : 1;
        std::uint16_t brighter_than_it_should_be : 1;
        std::uint16_t not_a_pathfinding_obstacle : 1;
        std::uint16_t cast_shadow_by_default : 1;
    };

    enum ObjectFunctionIn : TagEnum {
        OBJECT_FUNCTION_IN_NONE,
        OBJECT_FUNCTION_IN_BODY_VITALITY,
        OBJECT_FUNCTION_IN_SHIELD_VITALITY,
        OBJECT_FUNCTION_IN_RECENT_BODY_DAMAGE,
        OBJECT_FUNCTION_IN_RECENT_SHIELD_DAMAGE,
        OBJECT_FUNCTION_IN_RANDOM_CONSTANT,
        OBJECT_FUNCTION_IN_UMBRELLA_SHIELD_VITALITY,
        OBJECT_FUNCTION_IN_SHIELD_STUN,
        OBJECT_FUNCTION_IN_RECENT_UMBRELLA_SHIELD_VITALITY,
        OBJECT_FUNCTION_IN_UMBRELLA_SHIELD_STUN,
        OBJECT_FUNCTION_IN_REGION,
        OBJECT_FUNCTION_IN_REGION_1,
        OBJECT_FUNCTION_IN_REGION_2,
        OBJECT_FUNCTION_IN_REGION_3,
        OBJECT_FUNCTION_IN_REGION_4,
        OBJECT_FUNCTION_IN_REGION_5,
        OBJECT_FUNCTION_IN_REGION_6,
        OBJECT_FUNCTION_IN_REGION_7,
        OBJECT_FUNCTION_IN_ALIVE,
        OBJECT_FUNCTION_IN_COMPASS
    };

    #define COPY_OBJECT_DATA COPY_THIS(object_type); \
                             COPY_THIS(flags); \
                             COPY_THIS(bounding_radius); \
                             COPY_THIS(bounding_offset); \
                             COPY_THIS(origin_offset); \
                             COPY_THIS(acceleration_scale); \
                             COPY_THIS(has_change_colors); \
                             COPY_THIS(model); \
                             COPY_THIS(animation_graph); \
                             COPY_THIS(collision_model); \
                             COPY_THIS(physics); \
                             COPY_THIS(modifier_shader); \
                             COPY_THIS(creation_effect); \
                             COPY_THIS(render_bounding_radius); \
                             COPY_THIS(a_in); \
                             COPY_THIS(b_in); \
                             COPY_THIS(c_in); \
                             COPY_THIS(d_in); \
                             COPY_THIS(hud_text_message_index); \
                             COPY_THIS(forced_shader_permutation_index); \
                             COPY_THIS(attachments); \
                             COPY_THIS(widgets); \
                             COPY_THIS(functions); \
                             COPY_THIS(change_colors); \
                             COPY_THIS(predicted_resources);

    enum ObjectType : TagEnum {
        OBJECT_TYPE_BIPED = 0x0,
        OBJECT_TYPE_VEHICLE = 0x1,
        OBJECT_TYPE_WEAPON = 0x2,
        OBJECT_TYPE_EQUIPMENT = 0x3,
        OBJECT_TYPE_GARBAGE = 0x4,
        OBJECT_TYPE_PROJECTILE = 0x5,
        OBJECT_TYPE_SCENERY = 0x6,
        OBJECT_TYPE_DEVICE_MACHINE = 0x7,
        OBJECT_TYPE_DEVICE_CONTROL = 0x8,
        OBJECT_TYPE_DEVICE_LIGHT_FIXTURE = 0x9,
        OBJECT_TYPE_PLACEHOLDER = 0xA,
        OBJECT_TYPE_SOUND_SCENERY = 0xB,
    };

    #define COMPILE_OBJECT_DATA ADD_DEPENDENCY_ADJUST_SIZES(tag.model); \
                                ADD_DEPENDENCY_ADJUST_SIZES(tag.animation_graph); \
                                ADD_DEPENDENCY_ADJUST_SIZES(tag.collision_model); \
                                ADD_DEPENDENCY_ADJUST_SIZES(tag.physics); \
                                ADD_DEPENDENCY_ADJUST_SIZES(tag.modifier_shader); \
                                ADD_DEPENDENCY_ADJUST_SIZES(tag.creation_effect); \
                                ADD_BASIC_DEPENDENCY_REFLEXIVE(tag.attachments, type); \
                                ADD_BASIC_DEPENDENCY_REFLEXIVE(tag.widgets, reference); \
                                DEFAULT_VALUE(tag.render_bounding_radius, tag.bounding_radius); \
                                ADD_REFLEXIVE_START(tag.functions) { \
                                    DEFAULT_VALUE(reflexive.period, 1.0f); \
                                    if(reflexive.bounds.from == 0.0f && reflexive.bounds.to == 0.0f) { \
                                        reflexive.bounds.to = 1.0f; \
                                    } \
                                    reflexive.inverse_bounds = 1.0f / (reflexive.bounds.to - reflexive.bounds.from); \
                                    if(reflexive.step_count > 1) { \
                                        reflexive.inverse_step = 1.0f / (reflexive.step_count - 1); \
                                    } \
                                    reflexive.inverse_period = 1.0f / reflexive.period; \
                                } ADD_REFLEXIVE_END; \
                                ADD_REFLEXIVE_START(tag.change_colors) { \
                                    ADD_REFLEXIVE(reflexive.permutations); \
                                } ADD_REFLEXIVE_END; \
                                tag.has_change_colors = tag.change_colors.count != 0; \
                                ADD_REFLEXIVE(tag.predicted_resources);

    ENDIAN_TEMPLATE(EndianType) struct Object {
        LittleEndian<ObjectType> object_type;
        EndianType<ObjectFlags> flags;
        EndianType<float> bounding_radius;
        Point3D<EndianType> bounding_offset;
        Point3D<EndianType> origin_offset;
        EndianType<float> acceleration_scale;
        LittleEndian<std::uint32_t> has_change_colors;
        TagDependency<EndianType> model; // gbxmodel
        TagDependency<EndianType> animation_graph; // animation
        PAD(0x28);
        TagDependency<EndianType> collision_model; // model_collision_geometry
        TagDependency<EndianType> physics; // physics
        TagDependency<EndianType> modifier_shader; // shader
        TagDependency<EndianType> creation_effect; // effect
        PAD(0x54);
        EndianType<float> render_bounding_radius;
        EndianType<ObjectFunctionIn> a_in;
        EndianType<ObjectFunctionIn> b_in;
        EndianType<ObjectFunctionIn> c_in;
        EndianType<ObjectFunctionIn> d_in;
        PAD(0x2C);
        EndianType<std::int16_t> hud_text_message_index;
        EndianType<std::int16_t> forced_shader_permutation_index;
        TagReflexive<EndianType, ObjectAttachment> attachments;
        TagReflexive<EndianType, ObjectWidget> widgets;
        TagReflexive<EndianType, ObjectFunction> functions;
        TagReflexive<EndianType, ObjectChangeColors> change_colors;
        TagReflexive<EndianType, PredictedResource> predicted_resources;

        ENDIAN_TEMPLATE(NewType) operator Object<NewType>() const noexcept {
            Object<NewType> copy = {};
            COPY_OBJECT_DATA
            return copy;
        }
    };
    static_assert(sizeof(Object<BigEndian>) == 0x17C);

    ENDIAN_TEMPLATE(EndianType) struct BasicObject : Object<EndianType> {
        PAD(0x80);

        ENDIAN_TEMPLATE(NewType) operator BasicObject<NewType>() const noexcept {
            BasicObject<NewType> copy = {};
            COPY_OBJECT_DATA
            return copy;
        }
    };
    static_assert(sizeof(BasicObject<BigEndian>) == 0x1FC);

    void compile_object_tag(CompiledTag &compiled, const std::byte *data, std::size_t size, ObjectType type);
}
