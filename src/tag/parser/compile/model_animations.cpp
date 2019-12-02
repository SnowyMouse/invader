// SPDX-License-Identifier: GPL-3.0-only

#include <invader/tag/parser/parser.hpp>

namespace Invader::Parser {
    void ModelAnimations::pre_compile(BuildWorkload &workload, std::size_t tag_index, std::size_t, std::size_t) {
        std::size_t animation_count = this->animations.size();
        for(std::size_t i = 0; i < animation_count; i++) {
            auto *animation = this->animations.data() + i;

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
                auto next_animation = static_cast<std::uint16_t>(animation->next_animation);
                if(next_animation == 0xFFFF) {
                    break;
                }
                else if(next_animation >= animation_count) {
                    throw OutOfBoundsException();
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
                    if(next_animation == 0xFFFF) {
                        break;
                    }
                    else {
                        animation = this->animations.data() + next_animation;
                    }
                }

            }
        }
    }
}
