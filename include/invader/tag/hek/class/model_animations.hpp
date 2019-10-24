// SPDX-License-Identifier: GPL-3.0-only

#ifndef INVADER__TAG__HEK__CLASS__MODEL_ANIMATIONS_HPP
#define INVADER__TAG__HEK__CLASS__MODEL_ANIMATIONS_HPP

#include "../../compiled_tag.hpp"
#include "../../../hek/data_type.hpp"
#include "../header.hpp"

namespace Invader::HEK {
    enum AnimationFunction : TagEnum {
        ANIMATION_FUNCTION_A_OUT,
        ANIMATION_FUNCTION_B_OUT,
        ANIMATION_FUNCTION_C_OUT,
        ANIMATION_FUNCTION_D_OUT
    };

    enum AnimationFunctionControls : TagEnum {
        ANIMATION_FUNCTION_CONTROLS_FRAME,
        ANIMATION_FUNCTION_CONTROLS_SCALE
    };

    enum AnimationType : TagEnum {
        ANIMATION_TYPE_BASE,
        ANIMATION_TYPE_OVERLAY,
        ANIMATION_TYPE_REPLACEMENT
    };

    enum AnimationFrameInfoType : TagEnum {
        ANIMATION_FRAME_INFO_TYPE_NONE,
        ANIMATION_FRAME_INFO_TYPE_DX_DY,
        ANIMATION_FRAME_INFO_TYPE_DX_DY_DYAW,
        ANIMATION_FRAME_INFO_TYPE_DX_DY_DZ_DYAW
    };

    ENDIAN_TEMPLATE(EndianType) struct ModelAnimationRotation {
        EndianType<std::int16_t> rotation[4];

        ENDIAN_TEMPLATE(NewType) operator ModelAnimationRotation<NewType>() const noexcept {
            ModelAnimationRotation<NewType> copy;
            COPY_THIS_ARRAY(rotation);
            return copy;
        }
    };
    static_assert(sizeof(ModelAnimationRotation<BigEndian>) == 0x8);

    ENDIAN_TEMPLATE(EndianType) struct ModelAnimationTransform {
        Point3D<EndianType> transform;

        ENDIAN_TEMPLATE(NewType) operator ModelAnimationTransform<NewType>() const noexcept {
            ModelAnimationTransform<NewType> copy;
            COPY_THIS(transform);
            return copy;
        }
    };
    static_assert(sizeof(ModelAnimationTransform<BigEndian>) == 0xC);

    ENDIAN_TEMPLATE(EndianType) struct ModelAnimationScale {
        EndianType<Fraction> scale;

        ENDIAN_TEMPLATE(NewType) operator ModelAnimationScale<NewType>() const noexcept {
            ModelAnimationScale<NewType> copy;
            COPY_THIS(scale);
            return copy;
        }
    };
    static_assert(sizeof(ModelAnimationScale<BigEndian>) == 0x4);

    ENDIAN_TEMPLATE(EndianType) struct ModelAnimationFrameInfoDxDy {
        EndianType<std::uint32_t> ints[2];

        ENDIAN_TEMPLATE(NewType) operator ModelAnimationFrameInfoDxDy<NewType>() const noexcept {
            ModelAnimationFrameInfoDxDy<NewType> copy;
            COPY_THIS_ARRAY(ints);
            return copy;
        }
    };
    static_assert(sizeof(ModelAnimationFrameInfoDxDy<BigEndian>) == 0x8);

    ENDIAN_TEMPLATE(EndianType) struct ModelAnimationFrameInfoDxDyDyaw {
        EndianType<std::uint32_t> ints[3];

        ENDIAN_TEMPLATE(NewType) operator ModelAnimationFrameInfoDxDyDyaw<NewType>() const noexcept {
            ModelAnimationFrameInfoDxDyDyaw<NewType> copy;
            COPY_THIS_ARRAY(ints);
            return copy;
        }
    };
    static_assert(sizeof(ModelAnimationFrameInfoDxDyDyaw<BigEndian>) == 0xC);

    ENDIAN_TEMPLATE(EndianType) struct ModelAnimationFrameInfoDxDyDzDyaw {
        EndianType<std::uint32_t> ints[4];

        ENDIAN_TEMPLATE(NewType) operator ModelAnimationFrameInfoDxDyDzDyaw<NewType>() const noexcept {
            ModelAnimationFrameInfoDxDyDzDyaw<NewType> copy;
            COPY_THIS_ARRAY(ints);
            return copy;
        }
    };
    static_assert(sizeof(ModelAnimationFrameInfoDxDyDzDyaw<BigEndian>) == 0x10);

    ENDIAN_TEMPLATE(EndianType) struct ModelAnimationAnimationGraphObjectOverlay {
        EndianType<Index> animation;
        EndianType<AnimationFunction> function;
        EndianType<AnimationFunctionControls> function_controls;
        PAD(0x2);
        PAD(0xC);

        ENDIAN_TEMPLATE(NewType) operator ModelAnimationAnimationGraphObjectOverlay<NewType>() const noexcept {
            ModelAnimationAnimationGraphObjectOverlay<NewType> copy = {};
            COPY_THIS(animation);
            COPY_THIS(function);
            COPY_THIS(function_controls);
            return copy;
        }
    };
    static_assert(sizeof(ModelAnimationAnimationGraphObjectOverlay<BigEndian>) == 0x14);

    ENDIAN_TEMPLATE(EndianType) struct ModelAnimationUnitSeatAnimation {
        EndianType<Index> animation;

        ENDIAN_TEMPLATE(NewType) operator ModelAnimationUnitSeatAnimation<NewType>() const noexcept {
            ModelAnimationUnitSeatAnimation<NewType> copy = {};
            COPY_THIS(animation);
            return copy;
        }
    };
    static_assert(sizeof(ModelAnimationUnitSeatAnimation<BigEndian>) == 0x2);

    ENDIAN_TEMPLATE(EndianType) struct ModelAnimationAnimationGraphUnitSeatikPoint {
        TagString marker;
        TagString attach_to_marker;

        ENDIAN_TEMPLATE(NewType) operator ModelAnimationAnimationGraphUnitSeatikPoint<NewType>() const noexcept {
            ModelAnimationAnimationGraphUnitSeatikPoint<NewType> copy = {};
            COPY_THIS(marker);
            COPY_THIS(attach_to_marker);
            return copy;
        }
    };
    static_assert(sizeof(ModelAnimationAnimationGraphUnitSeatikPoint<BigEndian>) == 0x40);

    ENDIAN_TEMPLATE(EndianType) struct ModelAnimationAnimationWeaponClassAnimation {
        EndianType<Index> animation;

        ENDIAN_TEMPLATE(NewType) operator ModelAnimationAnimationWeaponClassAnimation<NewType>() const noexcept {
            ModelAnimationAnimationWeaponClassAnimation<NewType> copy = {};
            COPY_THIS(animation);
            return copy;
        }
    };
    static_assert(sizeof(ModelAnimationAnimationWeaponClassAnimation<BigEndian>) == 0x2);

    ENDIAN_TEMPLATE(EndianType) struct ModelAnimationAnimationWeaponTypeAnimation {
        EndianType<Index> animation;

        ENDIAN_TEMPLATE(NewType) operator ModelAnimationAnimationWeaponTypeAnimation<NewType>() const noexcept {
            ModelAnimationAnimationWeaponTypeAnimation<NewType> copy = {};
            COPY_THIS(animation);
            return copy;
        }
    };
    static_assert(sizeof(ModelAnimationAnimationWeaponTypeAnimation<BigEndian>) == 0x2);

    ENDIAN_TEMPLATE(EndianType) struct ModelAnimationAnimationGraphWeaponType {
        TagString label;
        PAD(0x10);
        TagReflexive<EndianType, ModelAnimationAnimationWeaponTypeAnimation> animations;

        ENDIAN_TEMPLATE(NewType) operator ModelAnimationAnimationGraphWeaponType<NewType>() const noexcept {
            ModelAnimationAnimationGraphWeaponType<NewType> copy = {};
            COPY_THIS(label);
            COPY_THIS(animations);
            return copy;
        }
    };
    static_assert(sizeof(ModelAnimationAnimationGraphWeaponType<BigEndian>) == 0x3C);

    ENDIAN_TEMPLATE(EndianType) struct ModelAnimationAnimationGraphWeapon {
        TagString name;
        TagString grip_marker;
        TagString hand_marker;
        EndianType<Angle> right_yaw_per_frame;
        EndianType<Angle> left_yaw_per_frame;
        EndianType<std::int16_t> right_frame_count;
        EndianType<std::int16_t> left_frame_count;
        EndianType<Angle> down_pitch_per_frame;
        EndianType<Angle> up_pitch_per_frame;
        EndianType<std::int16_t> down_pitch_frame_count;
        EndianType<std::int16_t> up_pitch_frame_count;
        PAD(0x20);
        TagReflexive<EndianType, ModelAnimationAnimationWeaponClassAnimation> animations;
        TagReflexive<EndianType, ModelAnimationAnimationGraphUnitSeatikPoint> ik_point;
        TagReflexive<EndianType, ModelAnimationAnimationGraphWeaponType> weapon_types;

        ENDIAN_TEMPLATE(NewType) operator ModelAnimationAnimationGraphWeapon<NewType>() const noexcept {
            ModelAnimationAnimationGraphWeapon<NewType> copy = {};
            COPY_THIS(name);
            COPY_THIS(grip_marker);
            COPY_THIS(hand_marker);
            COPY_THIS(right_yaw_per_frame);
            COPY_THIS(left_yaw_per_frame);
            COPY_THIS(right_frame_count);
            COPY_THIS(left_frame_count);
            COPY_THIS(down_pitch_per_frame);
            COPY_THIS(up_pitch_per_frame);
            COPY_THIS(down_pitch_frame_count);
            COPY_THIS(up_pitch_frame_count);
            COPY_THIS(animations);
            COPY_THIS(ik_point);
            COPY_THIS(weapon_types);
            return copy;
        }
    };
    static_assert(sizeof(ModelAnimationAnimationGraphWeapon<BigEndian>) == 0xBC);

    ENDIAN_TEMPLATE(EndianType) struct ModelAnimationAnimationGraphUnitSeat {
        TagString label;
        EndianType<Angle> right_yaw_per_frame;
        EndianType<Angle> left_yaw_per_frame;
        EndianType<std::int16_t> right_frame_count;
        EndianType<std::int16_t> left_frame_count;
        EndianType<Angle> down_pitch_per_frame;
        EndianType<Angle> up_pitch_per_frame;
        EndianType<std::int16_t> down_pitch_frame_count;
        EndianType<std::int16_t> up_pitch_frame_count;
        PAD(0x8);
        TagReflexive<EndianType, ModelAnimationAnimationWeaponClassAnimation> animations;
        TagReflexive<EndianType, ModelAnimationAnimationGraphUnitSeatikPoint> ik_points;
        TagReflexive<EndianType, ModelAnimationAnimationGraphWeapon> weapons;

        ENDIAN_TEMPLATE(NewType) operator ModelAnimationAnimationGraphUnitSeat<NewType>() const noexcept {
            ModelAnimationAnimationGraphUnitSeat<NewType> copy = {};
            COPY_THIS(label);
            COPY_THIS(right_yaw_per_frame);
            COPY_THIS(left_yaw_per_frame);
            COPY_THIS(right_frame_count);
            COPY_THIS(left_frame_count);
            COPY_THIS(down_pitch_per_frame);
            COPY_THIS(up_pitch_per_frame);
            COPY_THIS(down_pitch_frame_count);
            COPY_THIS(up_pitch_frame_count);
            COPY_THIS(animations);
            COPY_THIS(ik_points);
            COPY_THIS(weapons);
            return copy;
        }
    };
    static_assert(sizeof(ModelAnimationAnimationGraphUnitSeat<BigEndian>) == 0x64);

    ENDIAN_TEMPLATE(EndianType) struct ModelAnimationWeaponAnimation {
        EndianType<Index> animation;

        ENDIAN_TEMPLATE(NewType) operator ModelAnimationWeaponAnimation<NewType>() const noexcept {
            ModelAnimationWeaponAnimation<NewType> copy = {};
            COPY_THIS(animation);
            return copy;
        }
    };
    static_assert(sizeof(ModelAnimationWeaponAnimation<BigEndian>) == 0x2);

    ENDIAN_TEMPLATE(EndianType) struct ModelAnimationAnimationGraphWeaponAnimations {
        PAD(0x10);
        TagReflexive<EndianType, ModelAnimationWeaponAnimation> animations;

        ENDIAN_TEMPLATE(NewType) operator ModelAnimationAnimationGraphWeaponAnimations<NewType>() const noexcept {
            ModelAnimationAnimationGraphWeaponAnimations<NewType> copy = {};
            COPY_THIS(animations);
            return copy;
        }
    };
    static_assert(sizeof(ModelAnimationAnimationGraphWeaponAnimations<BigEndian>) == 0x1C);

    ENDIAN_TEMPLATE(EndianType) struct ModelAnimationVehicleAnimation {
        EndianType<Index> animation;

        ENDIAN_TEMPLATE(NewType) operator ModelAnimationVehicleAnimation<NewType>() const noexcept {
            ModelAnimationVehicleAnimation<NewType> copy = {};
            COPY_THIS(animation);
            return copy;
        }
    };
    static_assert(sizeof(ModelAnimationVehicleAnimation<BigEndian>) == 0x2);

    ENDIAN_TEMPLATE(EndianType) struct ModelAnimationSuspensionAnimation {
        EndianType<Index> mass_point_index;
        EndianType<Index> animation;
        EndianType<float> full_extension_ground_depth;
        EndianType<float> full_compression_ground_depth;
        PAD(0x8);

        ENDIAN_TEMPLATE(NewType) operator ModelAnimationSuspensionAnimation<NewType>() const noexcept {
            ModelAnimationSuspensionAnimation<NewType> copy = {};
            COPY_THIS(mass_point_index);
            COPY_THIS(animation);
            COPY_THIS(full_extension_ground_depth);
            COPY_THIS(full_compression_ground_depth);
            return copy;
        }
    };
    static_assert(sizeof(ModelAnimationSuspensionAnimation<BigEndian>) == 0x14);

    ENDIAN_TEMPLATE(EndianType) struct ModelAnimationAnimationGraphVehicleAnimations {
        EndianType<Angle> right_yaw_per_frame;
        EndianType<Angle> left_yaw_per_frame;
        EndianType<std::int16_t> right_frame_count;
        EndianType<std::int16_t> left_frame_count;
        EndianType<Angle> down_pitch_per_frame;
        EndianType<Angle> up_pitch_per_frame;
        EndianType<std::int16_t> down_pitch_frame_count;
        EndianType<std::int16_t> up_pitch_frame_count;
        PAD(0x44);
        TagReflexive<EndianType, ModelAnimationVehicleAnimation> animations;
        TagReflexive<EndianType, ModelAnimationSuspensionAnimation> suspension_animations;

        ENDIAN_TEMPLATE(NewType) operator ModelAnimationAnimationGraphVehicleAnimations<NewType>() const noexcept {
            ModelAnimationAnimationGraphVehicleAnimations<NewType> copy = {};
            COPY_THIS(right_yaw_per_frame);
            COPY_THIS(left_yaw_per_frame);
            COPY_THIS(right_frame_count);
            COPY_THIS(left_frame_count);
            COPY_THIS(down_pitch_per_frame);
            COPY_THIS(up_pitch_per_frame);
            COPY_THIS(down_pitch_frame_count);
            COPY_THIS(up_pitch_frame_count);
            COPY_THIS(animations);
            COPY_THIS(suspension_animations);
            return copy;
        }
    };
    static_assert(sizeof(ModelAnimationAnimationGraphVehicleAnimations<BigEndian>) == 0x74);

    ENDIAN_TEMPLATE(EndianType) struct ModelAnimationDeviceAnimation {
        EndianType<Index> animation;

        ENDIAN_TEMPLATE(NewType) operator ModelAnimationDeviceAnimation<NewType>() const noexcept {
            ModelAnimationDeviceAnimation<NewType> copy = {};
            COPY_THIS(animation);
            return copy;
        }
    };
    static_assert(sizeof(ModelAnimationDeviceAnimation<BigEndian>) == 0x2);

    ENDIAN_TEMPLATE(EndianType) struct ModelAnimationDeviceAnimations {
        PAD(0x54);
        TagReflexive<EndianType, ModelAnimationDeviceAnimation> animations;

        ENDIAN_TEMPLATE(NewType) operator ModelAnimationDeviceAnimations<NewType>() const noexcept {
            ModelAnimationDeviceAnimations<NewType> copy = {};
            COPY_THIS(animations);
            return copy;
        }
    };
    static_assert(sizeof(ModelAnimationDeviceAnimations<BigEndian>) == 0x60);

    ENDIAN_TEMPLATE(EndianType) struct ModelAnimationUnitDamageAnimations {
        EndianType<Index> animation;

        ENDIAN_TEMPLATE(NewType) operator ModelAnimationUnitDamageAnimations<NewType>() const noexcept {
            ModelAnimationUnitDamageAnimations<NewType> copy = {};
            COPY_THIS(animation);
            return copy;
        }
    };
    static_assert(sizeof(ModelAnimationUnitDamageAnimations<BigEndian>) == 0x2);

    ENDIAN_TEMPLATE(EndianType) struct ModelAnimationFirstPersonWeapon {
        EndianType<Index> animation;

        ENDIAN_TEMPLATE(NewType) operator ModelAnimationFirstPersonWeapon<NewType>() const noexcept {
            ModelAnimationFirstPersonWeapon<NewType> copy = {};
            COPY_THIS(animation);
            return copy;
        }
    };
    static_assert(sizeof(ModelAnimationFirstPersonWeapon<BigEndian>) == 0x2);

    ENDIAN_TEMPLATE(EndianType) struct ModelAnimationAnimationGraphFirstPersonWeaponAnimations {
        PAD(0x10);
        TagReflexive<EndianType, ModelAnimationFirstPersonWeapon> animations;

        ENDIAN_TEMPLATE(NewType) operator ModelAnimationAnimationGraphFirstPersonWeaponAnimations<NewType>() const noexcept {
            ModelAnimationAnimationGraphFirstPersonWeaponAnimations<NewType> copy = {};
            COPY_THIS(animations);
            return copy;
        }
    };
    static_assert(sizeof(ModelAnimationAnimationGraphFirstPersonWeaponAnimations<BigEndian>) == 0x1C);

    SINGLE_DEPENDENCY_PADDED_STRUCT(ModelAnimationAnimationGraphSoundReference, sound, 0x4);

    struct ModelAnimationAnimationGraphNodeFlags {
        std::uint32_t ball_socket : 1;
        std::uint32_t hinge : 1;
        std::uint32_t no_movement : 1;
    };

    ENDIAN_TEMPLATE(EndianType) struct ModelAnimationAnimationGraphNode {
        TagString name;
        EndianType<Index> next_sibling_node_index;
        EndianType<Index> first_child_node_index;
        EndianType<Index> parent_node_index;
        PAD(0x2);
        EndianType<ModelAnimationAnimationGraphNodeFlags> node_joint_flags;
        Vector3D<EndianType> base_vector;
        EndianType<float> vector_range;
        PAD(0x4);

        ENDIAN_TEMPLATE(NewType) operator ModelAnimationAnimationGraphNode<NewType>() const noexcept {
            ModelAnimationAnimationGraphNode<NewType> copy = {};
            COPY_THIS(name);
            COPY_THIS(next_sibling_node_index);
            COPY_THIS(first_child_node_index);
            COPY_THIS(parent_node_index);
            COPY_THIS(node_joint_flags);
            COPY_THIS(base_vector);
            COPY_THIS(vector_range);
            return copy;
        }
    };
    static_assert(sizeof(ModelAnimationAnimationGraphNode<BigEndian>) == 0x40);

    ENDIAN_TEMPLATE(EndianType) struct ModelAnimationArrayNodeTransformFlagData {
        EndianType<std::int32_t> no_name;

        ENDIAN_TEMPLATE(NewType) operator ModelAnimationArrayNodeTransformFlagData<NewType>() const noexcept {
            ModelAnimationArrayNodeTransformFlagData<NewType> copy = {};
            COPY_THIS(no_name);
            return copy;
        }
    };
    static_assert(sizeof(ModelAnimationArrayNodeTransformFlagData<BigEndian>) == 4);

    ENDIAN_TEMPLATE(EndianType) struct ModelAnimationArrayNodeRotationFlagData {
        EndianType<std::int32_t> no_name;

        ENDIAN_TEMPLATE(NewType) operator ModelAnimationArrayNodeRotationFlagData<NewType>() const noexcept {
            ModelAnimationArrayNodeRotationFlagData<NewType> copy = {};
            COPY_THIS(no_name);
            return copy;
        }
    };
    static_assert(sizeof(ModelAnimationArrayNodeRotationFlagData<BigEndian>) == 4);

    ENDIAN_TEMPLATE(EndianType) struct ModelAnimationArrayNodeScaleFlagData {
        EndianType<std::int32_t> no_name;

        ENDIAN_TEMPLATE(NewType) operator ModelAnimationArrayNodeScaleFlagData<NewType>() const noexcept {
            ModelAnimationArrayNodeScaleFlagData<NewType> copy = {};
            COPY_THIS(no_name);
            return copy;
        }
    };
    static_assert(sizeof(ModelAnimationArrayNodeScaleFlagData<BigEndian>) == 4);

    struct ModelAnimationAnimationFlags {
        std::uint16_t compressed_data : 1;
        std::uint16_t world_relative : 1;
        std::uint16_t _25hz_pal : 1;
    };

    ENDIAN_TEMPLATE(EndianType) struct ModelAnimationAnimation {
        TagString name;
        EndianType<AnimationType> type;
        EndianType<std::int16_t> frame_count;
        EndianType<std::int16_t> frame_size;
        EndianType<AnimationFrameInfoType> frame_info_type;
        EndianType<std::uint32_t> node_list_checksum;
        EndianType<std::int16_t> node_count;
        EndianType<Index> loop_frame_index;
        EndianType<Fraction> weight;
        EndianType<Index> key_frame_index;
        EndianType<Index> second_key_frame_index;
        EndianType<Index> next_animation;
        EndianType<ModelAnimationAnimationFlags> flags;
        EndianType<Index> sound;
        EndianType<Index> sound_frame_index;
        std::int8_t left_foot_frame_index;
        std::int8_t right_foot_frame_index;
        LittleEndian<std::uint16_t> main_animation_index;
        LittleEndian<float> relative_weight;
        TagDataOffset<EndianType> frame_info;
        ModelAnimationArrayNodeTransformFlagData<EndianType> node_transform_flag_data[2];
        PAD(0x8);
        ModelAnimationArrayNodeRotationFlagData<EndianType> node_rotation_flag_data[2];
        PAD(0x8);
        ModelAnimationArrayNodeScaleFlagData<EndianType> node_scale_flag_data[2];
        PAD(0x4);
        EndianType<std::uint32_t> offset_to_compressed_data;
        TagDataOffset<EndianType> default_data;
        TagDataOffset<EndianType> frame_data;

        ENDIAN_TEMPLATE(NewType) operator ModelAnimationAnimation<NewType>() const noexcept {
            ModelAnimationAnimation<NewType> copy = {};
            COPY_THIS(name);
            COPY_THIS(type);
            COPY_THIS(frame_count);
            COPY_THIS(frame_size);
            COPY_THIS(frame_info_type);
            COPY_THIS(node_list_checksum);
            COPY_THIS(node_count);
            COPY_THIS(loop_frame_index);
            COPY_THIS(weight);
            COPY_THIS(key_frame_index);
            COPY_THIS(second_key_frame_index);
            COPY_THIS(next_animation);
            COPY_THIS(flags);
            COPY_THIS(sound);
            COPY_THIS(sound_frame_index);
            COPY_THIS(left_foot_frame_index);
            COPY_THIS(right_foot_frame_index);
            COPY_THIS(main_animation_index);
            COPY_THIS(relative_weight);
            COPY_THIS(frame_info);
            COPY_THIS_ARRAY(node_transform_flag_data);
            COPY_THIS_ARRAY(node_rotation_flag_data);
            COPY_THIS_ARRAY(node_scale_flag_data);
            COPY_THIS(offset_to_compressed_data);
            COPY_THIS(default_data);
            COPY_THIS(frame_data);
            return copy;
        }
    };
    static_assert(sizeof(ModelAnimationAnimation<BigEndian>) == 0xB4);

    struct ModelAnimationsFlags {
        std::uint16_t compress_all_animations : 1;
        std::uint16_t force_idle_compression : 1;
    };

    ENDIAN_TEMPLATE(EndianType) struct ModelAnimations {
        TagReflexive<EndianType, ModelAnimationAnimationGraphObjectOverlay> objects;
        TagReflexive<EndianType, ModelAnimationAnimationGraphUnitSeat> units;
        TagReflexive<EndianType, ModelAnimationAnimationGraphWeaponAnimations> weapons;
        TagReflexive<EndianType, ModelAnimationAnimationGraphVehicleAnimations> vehicles;
        TagReflexive<EndianType, ModelAnimationDeviceAnimations> devices;
        TagReflexive<EndianType, ModelAnimationUnitDamageAnimations> unit_damage;
        TagReflexive<EndianType, ModelAnimationAnimationGraphFirstPersonWeaponAnimations> first_person_weapons;
        TagReflexive<EndianType, ModelAnimationAnimationGraphSoundReference> sound_references;
        EndianType<float> limp_body_node_radius;
        EndianType<ModelAnimationsFlags> flags;
        PAD(0x2);
        TagReflexive<EndianType, ModelAnimationAnimationGraphNode> nodes;
        TagReflexive<EndianType, ModelAnimationAnimation> animations;

        ENDIAN_TEMPLATE(NewType) operator ModelAnimations<NewType>() const noexcept {
            ModelAnimations<NewType> copy = {};
            COPY_THIS(objects);
            COPY_THIS(units);
            COPY_THIS(weapons);
            COPY_THIS(vehicles);
            COPY_THIS(devices);
            COPY_THIS(unit_damage);
            COPY_THIS(first_person_weapons);
            COPY_THIS(sound_references);
            COPY_THIS(limp_body_node_radius);
            COPY_THIS(flags);
            COPY_THIS(nodes);
            COPY_THIS(animations);
            return copy;
        }
    };
    static_assert(sizeof(ModelAnimations<BigEndian>) == 0x80);

    void compile_model_animations_tag(CompiledTag &compiled, const std::byte *data, std::size_t size);
}
#endif
