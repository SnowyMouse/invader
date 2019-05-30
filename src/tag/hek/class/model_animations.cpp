/*
 * Invader (c) 2018 Kavawuvi
 *
 * This program is free software under the GNU General Public License v3.0 or later. See LICENSE for more information.
 */

#include "../compile.hpp"

#include "model_animations.hpp"

namespace Invader::HEK {
    struct DedupingAnimationData {
        std::vector<std::byte> data;
        std::vector<std::size_t> animations;
    };

    void compile_model_animations_tag(CompiledTag &compiled, const std::byte *data, std::size_t size) {
        BEGIN_COMPILE(ModelAnimations);
        ADD_REFLEXIVE(tag.objects);
        ADD_REFLEXIVE_START(tag.units) {
            ADD_REFLEXIVE(reflexive.animations);
            ADD_REFLEXIVE(reflexive.ik_points);
            ADD_REFLEXIVE_START(reflexive.weapons) {
                ADD_REFLEXIVE(reflexive.animations);
                ADD_REFLEXIVE(reflexive.ik_point);
                ADD_REFLEXIVE_START(reflexive.weapon_types) {
                    ADD_REFLEXIVE(reflexive.animations);
                } ADD_REFLEXIVE_END
            } ADD_REFLEXIVE_END
        } ADD_REFLEXIVE_END

        ADD_REFLEXIVE_START(tag.weapons) {
            ADD_REFLEXIVE(reflexive.animations);
        } ADD_REFLEXIVE_END

        ADD_REFLEXIVE_START(tag.vehicles) {
            ADD_REFLEXIVE(reflexive.animations);
            ADD_REFLEXIVE(reflexive.suspension_animations);
        } ADD_REFLEXIVE_END

        ADD_REFLEXIVE_START(tag.devices) {
            ADD_REFLEXIVE(reflexive.animations);
        } ADD_REFLEXIVE_END

        ADD_REFLEXIVE(tag.unit_damage);

        ADD_REFLEXIVE_START(tag.first_person_weapons) {
            ADD_REFLEXIVE(reflexive.animations);
        } ADD_REFLEXIVE_END

        ADD_BASIC_DEPENDENCY_REFLEXIVE(tag.sound_references, sound);

        ADD_REFLEXIVE(tag.nodes);

        bool no_sounds = tag.sound_references.count == 0;
        std::size_t animations_offset = compiled.data.size();

        ADD_REFLEXIVE_START(tag.animations) {
            // Get the required frame size for sanity checking
            auto frame_type = reflexive.frame_info_type.read();
            auto frame_count = static_cast<std::size_t>(reflexive.frame_count.read());

            reflexive.main_animation_index = 0;

            if(no_sounds) {
                reflexive.sound = -1;
            }

            std::size_t required_frame_info_size;
            switch(frame_type) {
                case AnimationFrameInfoType::ANIMATION_FRAME_INFO_TYPE_NONE:
                    required_frame_info_size = 0;
                    break;
                case AnimationFrameInfoType::ANIMATION_FRAME_INFO_TYPE_DX_DY:
                    required_frame_info_size = sizeof(ModelAnimationFrameInfoDxDy<LittleEndian>);
                    break;
                case AnimationFrameInfoType::ANIMATION_FRAME_INFO_TYPE_DX_DY_DYAW:
                    required_frame_info_size = sizeof(ModelAnimationFrameInfoDxDyDyaw<BigEndian>);
                    break;
                case AnimationFrameInfoType::ANIMATION_FRAME_INFO_TYPE_DX_DY_DZ_DYAW:
                    required_frame_info_size = sizeof(ModelAnimationFrameInfoDxDyDzDyaw<BigEndian>);
                    break;
                default:
                    throw std::exception();
            }

            // If things don't add up, stop
            if(required_frame_info_size * frame_count != static_cast<std::size_t>(reflexive.frame_info.size.read())) {
                throw OutOfBoundsException();
            }

            // Do frame info stuff
            if(reflexive.frame_info.size > 0) {
                const auto *frame_info_big = data;
                ASSERT_SIZE(reflexive.frame_info.size);
                ADD_POINTER_FROM_INT32(reflexive.frame_info.pointer, compiled.data.size());
                compiled.data.insert(compiled.data.end(), reflexive.frame_info.size, std::byte());
                std::byte *frame_info_little = compiled.data.data() + compiled.data.size() - reflexive.frame_info.size;
                INCREMENT_DATA_PTR(reflexive.frame_info.size);

                // Update frame_info data, updating everything to little endian endian
                switch(frame_type) {
                    case AnimationFrameInfoType::ANIMATION_FRAME_INFO_TYPE_NONE:
                        break;
                    case AnimationFrameInfoType::ANIMATION_FRAME_INFO_TYPE_DX_DY: {
                        const auto *big = reinterpret_cast<const ModelAnimationFrameInfoDxDy<BigEndian> *>(frame_info_big);
                        auto *little = reinterpret_cast<ModelAnimationFrameInfoDxDy<LittleEndian> *>(frame_info_little);
                        std::copy(big, big + frame_count, little);
                        break;
                    }
                    case AnimationFrameInfoType::ANIMATION_FRAME_INFO_TYPE_DX_DY_DYAW: {
                        const auto *big = reinterpret_cast<const ModelAnimationFrameInfoDxDyDyaw<BigEndian> *>(frame_info_big);
                        auto *little = reinterpret_cast<ModelAnimationFrameInfoDxDyDyaw<LittleEndian> *>(frame_info_little);
                        std::copy(big, big + frame_count, little);
                        break;
                    }
                    case AnimationFrameInfoType::ANIMATION_FRAME_INFO_TYPE_DX_DY_DZ_DYAW:
                        const auto *big = reinterpret_cast<const ModelAnimationFrameInfoDxDyDzDyaw<BigEndian> *>(frame_info_big);
                        auto *little = reinterpret_cast<ModelAnimationFrameInfoDxDyDzDyaw<LittleEndian> *>(frame_info_little);
                        std::copy(big, big + frame_count, little);
                        break;
                }
            }

            auto node_count = static_cast<std::size_t>(reflexive.node_count.read());
            if(node_count > 64) {
                throw OutOfBoundsException();
            }

            // Get rotation, scale, and transform count
            std::size_t rotation_count = 0;
            std::size_t scale_count = 0;
            std::size_t transform_count = 0;
            std::uint32_t rotation_flag_0 = reflexive.node_rotation_flag_data[0].no_name;
            std::uint32_t rotation_flag_1 = reflexive.node_rotation_flag_data[1].no_name;
            std::uint32_t scale_flag_0 = reflexive.node_scale_flag_data[0].no_name;
            std::uint32_t scale_flag_1 = reflexive.node_scale_flag_data[1].no_name;
            std::uint32_t transform_flag_0 = reflexive.node_transform_flag_data[0].no_name;
            std::uint32_t transform_flag_1 = reflexive.node_transform_flag_data[1].no_name;
            bool rotate[64];
            bool scale[64];
            bool transform[64];
            for(std::size_t i = 0; i < node_count; i++) {
                bool has_rotation = false;
                bool has_scale = false;
                bool has_transform = false;

                if(i > 31) {
                    has_rotation = (rotation_flag_1 >> (i - 32)) & 1;
                    has_scale = (scale_flag_1 >> (i - 32)) & 1;
                    has_transform = (transform_flag_1 >> (i - 32)) & 1;
                }
                else {
                    has_rotation = (rotation_flag_0 >> i) & 1;
                    has_scale = (scale_flag_0 >> i) & 1;
                    has_transform = (transform_flag_0 >> i) & 1;
                }

                rotation_count += has_rotation;
                scale_count += has_scale;
                transform_count += has_transform;

                rotate[i] = has_rotation;
                scale[i] = has_scale;
                transform[i] = has_transform;
            }

            // Make sure frame and default size is correct
            std::size_t total_frame_size = rotation_count * sizeof(ModelAnimationRotation<BigEndian>) + scale_count * sizeof(ModelAnimationScale<BigEndian>) + transform_count * sizeof(ModelAnimationTransform<BigEndian>);
            std::size_t max_frame_size = node_count * (sizeof(ModelAnimationRotation<BigEndian>) + sizeof(ModelAnimationScale<BigEndian>) + sizeof(ModelAnimationTransform<BigEndian>));
            if(static_cast<std::size_t>(reflexive.frame_size) != total_frame_size) {
                throw OutOfBoundsException();
            }

            if(static_cast<std::size_t>(reflexive.default_data.size) != (max_frame_size - total_frame_size)) {
                throw OutOfBoundsException();
            }
            ASSERT_SIZE(reflexive.default_data.size + reflexive.frame_data.size);

            // Get whether or not it's compressed
            bool compressed = reflexive.flags.read().compressed_data;
            std::size_t compressed_data_offset = reflexive.offset_to_compressed_data;
            reflexive.offset_to_compressed_data = 0;

            // Let's do default_data. Basically just add what isn't in frame_data, and only for one frame
            const auto *default_data_big = data;
            if(reflexive.default_data.size > 0) {
                std::size_t default_data_size = reflexive.default_data.size;
                std::vector<std::byte> default_data(default_data_size);
                auto *default_data_little = default_data.data();
                for(std::size_t node = 0; node < node_count; node++) {
                    if(!rotate[node]) {
                        const auto &rotation_big = *reinterpret_cast<const ModelAnimationRotation<BigEndian> *>(default_data_big);
                        auto &rotation_little = *reinterpret_cast<ModelAnimationRotation<LittleEndian> *>(default_data_little);
                        rotation_little = rotation_big;
                        default_data_big += sizeof(rotation_big);
                        default_data_little += sizeof(rotation_big);
                    }
                    if(!transform[node]) {
                        const auto &transform_big = *reinterpret_cast<const ModelAnimationTransform<BigEndian> *>(default_data_big);
                        auto &transform_little = *reinterpret_cast<ModelAnimationTransform<LittleEndian> *>(default_data_little);
                        transform_little = transform_big;
                        default_data_big += sizeof(transform_big);
                        default_data_little += sizeof(transform_big);
                    }
                    if(!scale[node]) {
                        const auto &scale_big = *reinterpret_cast<const ModelAnimationScale<BigEndian> *>(default_data_big);
                        auto &scale_little = *reinterpret_cast<ModelAnimationScale<LittleEndian> *>(default_data_little);
                        scale_little = scale_big;
                        default_data_big += sizeof(scale_big);
                        default_data_little += sizeof(scale_big);
                    }
                }
                INCREMENT_DATA_PTR(reflexive.default_data.size);

                // Add default data
                ADD_POINTER_FROM_INT32(reflexive.default_data.pointer, compiled.data.size());
                compiled.data.insert(compiled.data.end(), default_data.data(), default_data.data() + default_data.size());
            }
            else {
                INCREMENT_DATA_PTR(reflexive.default_data.size);
                std::memset(&reflexive.default_data, 0, sizeof(reflexive.default_data));
            }

            // Now let's do frame_data.
            if(compressed) {
                INCREMENT_DATA_PTR(compressed_data_offset);
                ADD_POINTER_FROM_INT32(reflexive.frame_data.pointer, compiled.data.size());
                reflexive.frame_data.size = static_cast<std::int32_t>(reflexive.frame_data.size - compressed_data_offset);
                std::size_t frame_size = reflexive.frame_data.size;
                compiled.data.insert(compiled.data.end(), data, data + frame_size);
                INCREMENT_DATA_PTR(frame_size);
                PAD_32_BIT
            }
            else {
                const auto *frame_data_big = data;
                if(reflexive.frame_data.size > 0) {
                    std::size_t frame_data_size = reflexive.frame_data.size;
                    ASSERT_SIZE(frame_data_size);
                    std::vector<std::byte> frame_data(frame_data_size, std::byte());
                    auto *frame_data_little = frame_data.data();
                    for(std::size_t frame = 0; frame < frame_count; frame++) {
                        for(std::size_t node = 0; node < node_count; node++) {
                            if(rotate[node]) {
                                const auto &rotation_big = *reinterpret_cast<const ModelAnimationRotation<BigEndian> *>(frame_data_big);
                                auto &rotation_little = *reinterpret_cast<ModelAnimationRotation<LittleEndian> *>(frame_data_little);
                                rotation_little = rotation_big;
                                frame_data_big += sizeof(rotation_big);
                                frame_data_little += sizeof(rotation_big);
                            }
                            if(transform[node]) {
                                const auto &transform_big = *reinterpret_cast<const ModelAnimationTransform<BigEndian> *>(frame_data_big);
                                auto &transform_little = *reinterpret_cast<ModelAnimationTransform<LittleEndian> *>(frame_data_little);
                                transform_little = transform_big;
                                frame_data_big += sizeof(transform_big);
                                frame_data_little += sizeof(transform_big);
                            }
                            if(scale[node]) {
                                const auto &scale_big = *reinterpret_cast<const ModelAnimationScale<BigEndian> *>(frame_data_big);
                                auto &scale_little = *reinterpret_cast<ModelAnimationScale<LittleEndian> *>(frame_data_little);
                                scale_little = scale_big;
                                frame_data_big += sizeof(scale_big);
                                frame_data_little += sizeof(scale_big);
                            }
                        }
                    }
                    INCREMENT_DATA_PTR(frame_data_size);

                    // Add default data
                    ADD_POINTER_FROM_INT32(reflexive.frame_data.pointer, compiled.data.size());
                    compiled.data.insert(compiled.data.end(), frame_data.data(), frame_data.data() + frame_data.size());
                }
            }

            reflexive.main_animation_index = 0;
        } ADD_REFLEXIVE_END;

        // Go through to set main_animation_index as well as the animation_progress value
        auto *animations = reinterpret_cast<ModelAnimationAnimation<LittleEndian> *>(compiled.data.data() + animations_offset);
        std::size_t animation_count = tag.animations.count;
        for(std::size_t i = 0; i < animation_count; i++) {
            auto *animation = animations + i;

            // Check if we already did things to this
            if(animation->main_animation_index) {
                continue;
            }

            bool multiple_animations = false;
            float total_weight = 0.0F;

            // Go through each animation. Make sure the weights are all correct.
            while(true) {
                // Set the main animation index
                animation->main_animation_index = static_cast<std::int32_t>(i);

                // Set weight to a default value
                animation->relative_weight = 1.0F;

                // Increment total weight
                total_weight += animation->weight > 0.0F ? animation->weight : 1.0F;

                // Get the next animation if there is one
                auto next_animation = static_cast<std::uint16_t>(animation->next_animation.read());
                if(next_animation == 0xFFFF) {
                    break;
                }
                else if(next_animation >= animation_count) {
                    throw OutOfBoundsException();
                }
                else {
                    animation = animations + next_animation;
                    multiple_animations = true;
                }
            }

            // We will need to go down the rabbit hole here if we have multiple animations
            if(multiple_animations) {
                animation = animations + i;
                float total_weight_second_pass = 0.0F;

                while(animation != nullptr) {
                    // Set the weight
                    total_weight_second_pass += animation->weight > 0.0F ? animation->weight : 1.0F;
                    animation->relative_weight = total_weight_second_pass / total_weight;

                    // Get the next animation if there is one
                    auto next_animation = static_cast<std::uint16_t>(animation->next_animation.read());
                    if(next_animation == 0xFFFF) {
                        break;
                    }
                    else {
                        animation = animations + next_animation;
                    }
                }

            }
        }

        FINISH_COMPILE
    }
}
