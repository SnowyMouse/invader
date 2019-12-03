// SPDX-License-Identifier: GPL-3.0-only

#include <invader/tag/parser/parser.hpp>
#include <invader/build/build_workload.hpp>

namespace Invader::Parser {
    void ModelAnimations::pre_compile(BuildWorkload &workload, std::size_t tag_index, std::size_t, std::size_t) {
        std::size_t animation_count = this->animations.size();
        std::size_t sound_count = this->sound_references.size();

        for(auto &a : this->animations) {
            a.main_animation_index = NULL_INDEX;
        }

        for(std::size_t i = 0; i < animation_count; i++) {
            auto *animation = this->animations.data() + i;

            // Null sound if needed
            if(sound_count == 0) {
                animation->sound = NULL_INDEX;
            }

            // Check to make sure indices aren't broken
            std::size_t sound = animation->sound;
            std::size_t sound_frame_index = animation->sound_frame_index;
            std::size_t frame_count = animation->frame_count;
            if(sound != NULL_INDEX) {
                if(sound >= sound_count) {
                    REPORT_ERROR_PRINTF(workload, ERROR_TYPE_ERROR, tag_index, "Animation #%zu has an invalid sound index (%zu >= %zu)", i, sound, sound_count);
                }
            }
            else if(animation->sound_frame_index != 0) {
                REPORT_ERROR_PRINTF(workload, ERROR_TYPE_WARNING, tag_index, "Animation #%zu has a sound frame index set, but no sound is referenced", i);
            }
            if(sound_frame_index >= animation->frame_count) {
                REPORT_ERROR_PRINTF(workload, ERROR_TYPE_ERROR, tag_index, "Animation #%zu has an invalid sound frame index (%zu >= %zu)", i, sound_frame_index, frame_count);
            }

            std::size_t key_frame_index = animation->key_frame_index;
            if(key_frame_index >= animation->frame_count) {
                REPORT_ERROR_PRINTF(workload, ERROR_TYPE_ERROR, tag_index, "Animation #%zu has an invalid key frame index (%zu >= %zu)", i, key_frame_index, frame_count);
            }

            std::size_t second_key_frame_index = animation->second_key_frame_index;
            if(second_key_frame_index >= animation->frame_count) {
                REPORT_ERROR_PRINTF(workload, ERROR_TYPE_ERROR, tag_index, "Animation #%zu has an invalid second key frame index (%zu >= %zu)", i, second_key_frame_index, frame_count);
            }

            // Check if we already did things to this
            if(animation->main_animation_index != NULL_INDEX) {
                continue;
            }

            bool multiple_animations = false;
            float total_weight = 0.0F;

            // Go through each animation. Make sure the weights are all correct.
            while(true) {
                // Set the main animation index
                animation->main_animation_index = static_cast<std::uint16_t>(i);

                // Set weight to a default value
                animation->relative_weight = 1.0F;

                // Increment total weight
                total_weight += animation->weight > 0.0F ? animation->weight : 1.0F;

                // Get the next animation if there is one
                std::size_t next_animation = animation->next_animation;
                if(next_animation == NULL_INDEX) {
                    break;
                }
                else if(next_animation >= animation_count) {
                    REPORT_ERROR_PRINTF(workload, ERROR_TYPE_FATAL_ERROR, tag_index, "Animation #%zu has an invalid next animation index (%zu >= %zu)", animation - this->animations.data(), next_animation, animation_count);
                    throw InvalidTagDataException();
                }
                else {
                    animation = this->animations.data() + next_animation;
                    multiple_animations = true;
                }
            }

            // We will need to go down the rabbit hole here if we have multiple animations
            if(multiple_animations) {
                animation = this->animations.data() + i;
                float total_weight_second_pass = 0.0F;

                while(animation != nullptr) {
                    // Set the weight
                    total_weight_second_pass += animation->weight > 0.0F ? animation->weight : 1.0F;
                    animation->relative_weight = total_weight_second_pass / total_weight;

                    // Get the next animation if there is one
                    auto next_animation = static_cast<std::uint16_t>(animation->next_animation);
                    if(next_animation == NULL_INDEX) {
                        break;
                    }
                    else {
                        animation = this->animations.data() + next_animation;
                    }
                }
            }
        }
    }

    void ModelAnimationsAnimation::pre_compile(BuildWorkload &workload, std::size_t tag_index, std::size_t struct_index, std::size_t) {
        // Get the required frame size for sanity checking
        auto frame_type = this->frame_info_type;
        auto frame_count = static_cast<std::size_t>(this->frame_count);

        std::size_t required_frame_info_size;
        switch(frame_type) {
            case HEK::AnimationFrameInfoType::ANIMATION_FRAME_INFO_TYPE_NONE:
                required_frame_info_size = 0;
                break;
            case HEK::AnimationFrameInfoType::ANIMATION_FRAME_INFO_TYPE_DX_DY:
                required_frame_info_size = sizeof(ModelAnimationsFrameInfoDxDy::struct_little);
                break;
            case HEK::AnimationFrameInfoType::ANIMATION_FRAME_INFO_TYPE_DX_DY_DYAW:
                required_frame_info_size = sizeof(ModelAnimationsFrameInfoDxDyDyaw::struct_big);
                break;
            case HEK::AnimationFrameInfoType::ANIMATION_FRAME_INFO_TYPE_DX_DY_DZ_DYAW:
                required_frame_info_size = sizeof(ModelAnimationsFrameInfoDxDyDzDyaw::struct_big);
                break;
            default:
                std::terminate();
        }

        // If things don't add up, stop
        std::size_t expected_frame_info_size = required_frame_info_size * frame_count;
        std::size_t frame_info_size = this->frame_info.size();
        if(expected_frame_info_size != frame_info_size) {
            REPORT_ERROR_PRINTF(workload, ERROR_TYPE_FATAL_ERROR, tag_index, "Animation #%zu has an invalid frame info size (%zu expected, %zu gotten)", struct_index, expected_frame_info_size, frame_info_size);
            throw InvalidTagDataException();
        }

        // Do frame info stuff
        if(frame_info_size > 0) {
            const auto *frame_info_big = this->frame_info.data();
            std::vector<std::byte> frame_info_little_v(frame_info_size);
            std::byte *frame_info_little = frame_info_little_v.data();

            // Update frame_info data, updating everything to little endian endian
            switch(frame_type) {
                case HEK::AnimationFrameInfoType::ANIMATION_FRAME_INFO_TYPE_NONE:
                    break;
                case HEK::AnimationFrameInfoType::ANIMATION_FRAME_INFO_TYPE_DX_DY: {
                    const auto *big = reinterpret_cast<const ModelAnimationsFrameInfoDxDy::struct_big *>(frame_info_big);
                    auto *little = reinterpret_cast<ModelAnimationsFrameInfoDxDy::struct_little *>(frame_info_little);
                    std::copy(big, big + frame_count, little);
                    break;
                }
                case HEK::AnimationFrameInfoType::ANIMATION_FRAME_INFO_TYPE_DX_DY_DYAW: {
                    const auto *big = reinterpret_cast<const ModelAnimationsFrameInfoDxDyDyaw::struct_big *>(frame_info_big);
                    auto *little = reinterpret_cast<ModelAnimationsFrameInfoDxDyDyaw::struct_little *>(frame_info_little);
                    std::copy(big, big + frame_count, little);
                    break;
                }
                case HEK::AnimationFrameInfoType::ANIMATION_FRAME_INFO_TYPE_DX_DY_DZ_DYAW:
                    const auto *big = reinterpret_cast<const ModelAnimationsFrameInfoDxDyDzDyaw::struct_big *>(frame_info_big);
                    auto *little = reinterpret_cast<ModelAnimationsFrameInfoDxDyDzDyaw::struct_little *>(frame_info_little);
                    std::copy(big, big + frame_count, little);
                    break;
            }

            this->frame_info = frame_info_little_v;
        }

        auto node_count = static_cast<std::size_t>(this->node_count);
        std::size_t frame_data_size = this->frame_data.size();
        std::size_t default_data_size = this->default_data.size();

        // Get rotation, scale, and transform count
        std::size_t rotation_count = 0;
        std::size_t scale_count = 0;
        std::size_t transform_count = 0;
        std::uint32_t rotation_flag_0 = this->node_rotation_flag_data[0];
        std::uint32_t rotation_flag_1 = this->node_rotation_flag_data[1];
        std::uint32_t scale_flag_0 = this->node_scale_flag_data[0];
        std::uint32_t scale_flag_1 = this->node_scale_flag_data[1];
        std::uint32_t transform_flag_0 = this->node_transform_flag_data[0];
        std::uint32_t transform_flag_1 = this->node_transform_flag_data[1];
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
        std::size_t total_frame_size = rotation_count * sizeof(ModelAnimationsRotation::struct_big) + scale_count * sizeof(ModelAnimationscale::struct_big) + transform_count * sizeof(ModelAnimationsTransform::struct_big);
        std::size_t max_frame_size = node_count * (sizeof(ModelAnimationsRotation::struct_big) + sizeof(ModelAnimationscale::struct_big) + sizeof(ModelAnimationsTransform::struct_big));
        if(this->frame_size != total_frame_size) {
            REPORT_ERROR_PRINTF(workload, ERROR_TYPE_FATAL_ERROR, tag_index, "Animation #%zu has an invalid frame size (%zu expected, %zu gotten)", struct_index, total_frame_size, static_cast<std::size_t>(this->frame_size));
            throw InvalidTagDataException();
        }

        std::size_t expected_default_data_size = max_frame_size - total_frame_size;
        if(default_data_size != expected_default_data_size) {
            REPORT_ERROR_PRINTF(workload, ERROR_TYPE_FATAL_ERROR, tag_index, "Animation #%zu has an invalid default data size (%zu expected, %zu gotten)", struct_index, expected_default_data_size, default_data_size);
            throw InvalidTagDataException();
        }

        // Get whether or not it's compressed
        bool compressed = this->flags.compressed_data;
        std::size_t compressed_data_offset = this->offset_to_compressed_data;
        this->offset_to_compressed_data = 0;

        // Let's do default_data. Basically just add what isn't in frame_data, and only for one frame
        const auto *default_data_big = this->default_data.data();
        if(!compressed) {
            std::vector<std::byte> default_data(default_data_size);
            auto *default_data_little = default_data.data();
            for(std::size_t node = 0; node < node_count; node++) {
                if(!rotate[node]) {
                    const auto &rotation_big = *reinterpret_cast<const ModelAnimationsRotation::struct_big *>(default_data_big);
                    auto &rotation_little = *reinterpret_cast<ModelAnimationsRotation::struct_little *>(default_data_little);
                    rotation_little = rotation_big;
                    default_data_big += sizeof(rotation_big);
                    default_data_little += sizeof(rotation_big);
                }
                if(!transform[node]) {
                    const auto &transform_big = *reinterpret_cast<const ModelAnimationsTransform::struct_big *>(default_data_big);
                    auto &transform_little = *reinterpret_cast<ModelAnimationsTransform::struct_little *>(default_data_little);
                    transform_little = transform_big;
                    default_data_big += sizeof(transform_big);
                    default_data_little += sizeof(transform_big);
                }
                if(!scale[node]) {
                    const auto &scale_big = *reinterpret_cast<const ModelAnimationscale::struct_big *>(default_data_big);
                    auto &scale_little = *reinterpret_cast<ModelAnimationscale::struct_little *>(default_data_little);
                    scale_little = scale_big;
                    default_data_big += sizeof(scale_big);
                    default_data_little += sizeof(scale_big);
                }
            }
            this->default_data = default_data;
        }
        else {
            this->default_data.clear();
        }

        // Now let's do frame_data.
        if(compressed) {
            if(compressed_data_offset > frame_data_size) {
                REPORT_ERROR_PRINTF(workload, ERROR_TYPE_FATAL_ERROR, tag_index, "Animation #%zu has an invalid compressed data offset (%zu > %zu)", struct_index, compressed_data_offset, frame_data_size);
                throw InvalidTagDataException();
            }
            this->frame_data = std::vector<std::byte>(this->frame_data.begin() + compressed_data_offset, this->frame_data.end());
        }
        else {
            const auto *frame_data_big = this->frame_data.data();
            if(frame_data_size > 0) {
                std::vector<std::byte> frame_data(frame_data_size, std::byte());
                auto *frame_data_little = frame_data.data();
                for(std::size_t frame = 0; frame < frame_count; frame++) {
                    for(std::size_t node = 0; node < node_count; node++) {
                        if(rotate[node]) {
                            const auto &rotation_big = *reinterpret_cast<const ModelAnimationsRotation::struct_big *>(frame_data_big);
                            auto &rotation_little = *reinterpret_cast<ModelAnimationsRotation::struct_little *>(frame_data_little);
                            rotation_little = rotation_big;
                            frame_data_big += sizeof(rotation_big);
                            frame_data_little += sizeof(rotation_big);
                        }
                        if(transform[node]) {
                            const auto &transform_big = *reinterpret_cast<const ModelAnimationsTransform::struct_big *>(frame_data_big);
                            auto &transform_little = *reinterpret_cast<ModelAnimationsTransform::struct_little *>(frame_data_little);
                            transform_little = transform_big;
                            frame_data_big += sizeof(transform_big);
                            frame_data_little += sizeof(transform_big);
                        }
                        if(scale[node]) {
                            const auto &scale_big = *reinterpret_cast<const ModelAnimationscale::struct_big *>(frame_data_big);
                            auto &scale_little = *reinterpret_cast<ModelAnimationscale::struct_little *>(frame_data_little);
                            scale_little = scale_big;
                            frame_data_big += sizeof(scale_big);
                            frame_data_little += sizeof(scale_big);
                        }
                    }
                }
                this->frame_data = frame_data;
            }
        }
    }
}
