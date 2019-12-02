// SPDX-License-Identifier: GPL-3.0-only

#include <invader/tag/parser/parser.hpp>
#include <invader/build/build_workload.hpp>

namespace Invader::Parser {
    void ModelAnimations::pre_compile(BuildWorkload &workload, std::size_t tag_index, std::size_t, std::size_t) {
        std::size_t animation_count = this->animations.size();
        std::size_t sound_count = this->sound_references.size();
        for(std::size_t i = 0; i < animation_count; i++) {
            auto *animation = this->animations.data() + i;

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

            std::size_t loop_frame_index = animation->loop_frame_index;
            if(loop_frame_index >= animation->frame_count) {
                REPORT_ERROR_PRINTF(workload, ERROR_TYPE_ERROR, tag_index, "Animation #%zu has an invalid loop frame index (%zu >= %zu)", i, loop_frame_index, frame_count);
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
            if(animation->main_animation_index) {
                continue;
            }

            bool multiple_animations = false;
            float total_weight = 0.0F;

            // Go through each animation. Make sure the weights are all correct.
            while(true) {
                // Set the main animation index
                animation->main_animation_index = static_cast<std::uint32_t>(i);

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

    void ModelAnimationsAnimation::pre_compile(BuildWorkload &workload, std::size_t tag_index, std::size_t, std::size_t) {

    }
}
