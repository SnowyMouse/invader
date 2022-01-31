// SPDX-License-Identifier: GPL-3.0-only

#include <invader/map/map.hpp>
#include <invader/map/tag.hpp>
#include <invader/tag/parser/parser.hpp>
#include <invader/file/file.hpp>

namespace Invader::Parser {
    void Invader::Parser::ActorVariant::post_cache_deformat() {
        this->grenade_velocity *= TICK_RATE;
    }

    void Invader::Parser::DamageEffect::post_cache_deformat() {
        this->camera_shaking_wobble_period /= TICK_RATE;
    }

    void Invader::Parser::Glow::post_cache_deformat() {
        this->attachment_0 = static_cast<HEK::FunctionOut>(this->attachment_0 + 1);
        this->attachment_1 = static_cast<HEK::FunctionOut>(this->attachment_1 + 1);
        this->attachment_2 = static_cast<HEK::FunctionOut>(this->attachment_2 + 1);
        this->attachment_3 = static_cast<HEK::FunctionOut>(this->attachment_3 + 1);
        this->attachment_4 = static_cast<HEK::FunctionOut>(this->attachment_4 + 1);
        this->attachment_5 = static_cast<HEK::FunctionOut>(this->attachment_5 + 1);
    }

    void Invader::Parser::PointPhysics::post_cache_deformat() {
        this->air_friction /= 10000.0f;
        this->water_friction /= 10000.0f;
    }

    void Invader::Parser::Projectile::post_cache_deformat() {
        this->minimum_velocity *= TICK_RATE;
        this->initial_velocity *= TICK_RATE;
        this->final_velocity *= TICK_RATE;
    }

    void Invader::Parser::ProjectileMaterialResponse::post_cache_deformat() {
        this->potential_and.from *= TICK_RATE;
        this->potential_and.to *= TICK_RATE;
    }

    void Invader::Parser::ScenarioCutsceneTitle::post_cache_deformat() {
        this->fade_in_time /= TICK_RATE;
        this->fade_out_time /= TICK_RATE;
        this->up_time /= TICK_RATE;
    }

    void Invader::Parser::Light::post_cache_deformat() {
        this->duration /= TICK_RATE;
    }

    template <typename A, typename B> static void swap_endian_array(A *to, const B *from, std::size_t count) {
        for(std::size_t i = 0; i < count; i++) {
            to[i] = from[i];
        }
    }

    void Invader::Parser::ModelAnimationsAnimation::post_cache_deformat() {
        // Get whether or not it's compressed
        bool compressed = this->flags & HEK::ModelAnimationsAnimationFlagsFlag::MODEL_ANIMATIONS_ANIMATION_FLAGS_FLAG_COMPRESSED_DATA;
        std::vector<std::byte> frame_data = this->frame_data;
        std::vector<std::byte> frame_info = this->frame_info;
        std::vector<std::byte> default_data = this->default_data;

        // Frame info
        std::size_t required_frame_info_size;
        switch(this->frame_info_type) {
            case HEK::AnimationFrameInfoType::ANIMATION_FRAME_INFO_TYPE_NONE:
                required_frame_info_size = 0;
                break;
            case HEK::AnimationFrameInfoType::ANIMATION_FRAME_INFO_TYPE_DX_DY:
                required_frame_info_size = sizeof(ModelAnimationsFrameInfoDxDy::struct_little);
                break;
            case HEK::AnimationFrameInfoType::ANIMATION_FRAME_INFO_TYPE_DX_DY_DYAW:
                required_frame_info_size = sizeof(ModelAnimationsFrameInfoDxDyDyaw::struct_little);
                break;
            case HEK::AnimationFrameInfoType::ANIMATION_FRAME_INFO_TYPE_DX_DY_DZ_DYAW:
                required_frame_info_size = sizeof(ModelAnimationsFrameInfoDxDyDzDyaw::struct_little);
                break;
            default:
                eprintf_error("unknown frame info type");
                throw InvalidTagDataException();
        }
        if(required_frame_info_size * this->frame_count != frame_info.size()) {
            throw OutOfBoundsException();
        }

        // Convert endianness
        if(frame_info.size() > 0) {
            switch(this->frame_info_type) {
                case HEK::AnimationFrameInfoType::ANIMATION_FRAME_INFO_TYPE_DX_DY:
                    swap_endian_array(
                        reinterpret_cast<ModelAnimationsFrameInfoDxDy::struct_big *>(this->frame_info.data()),
                        reinterpret_cast<ModelAnimationsFrameInfoDxDy::struct_little *>(frame_info.data()),
                        this->frame_count
                    );
                    break;
                case HEK::AnimationFrameInfoType::ANIMATION_FRAME_INFO_TYPE_DX_DY_DYAW:
                    swap_endian_array(
                        reinterpret_cast<ModelAnimationsFrameInfoDxDyDyaw::struct_big *>(this->frame_info.data()),
                        reinterpret_cast<ModelAnimationsFrameInfoDxDyDyaw::struct_little *>(frame_info.data()),
                        this->frame_count
                    );
                    break;
                case HEK::AnimationFrameInfoType::ANIMATION_FRAME_INFO_TYPE_DX_DY_DZ_DYAW:
                    swap_endian_array(
                        reinterpret_cast<ModelAnimationsFrameInfoDxDyDzDyaw::struct_big *>(this->frame_info.data()),
                        reinterpret_cast<ModelAnimationsFrameInfoDxDyDzDyaw::struct_little *>(frame_info.data()),
                        this->frame_count
                    );
                    break;
                default:
                    break;
            }
        }

        // Now do nodes
        if(this->node_count > 64) {
            throw OutOfBoundsException();
        }

        // Count this stuff
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
        std::size_t frame_data_size_expected = 0;
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

            if(has_rotation) {
                frame_data_size_expected += sizeof(ModelAnimationsRotation::struct_little) * frame_count;
            }
            if(has_transform) {
                frame_data_size_expected += sizeof(ModelAnimationsTransform::struct_little) * frame_count;
            }
            if(has_scale) {
                frame_data_size_expected += sizeof(ModelAnimationscale::struct_little) * frame_count;
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
        if(frame_size != total_frame_size) {
            eprintf_error("Frame size is invalid (%zu != %zu)", static_cast<std::size_t>(frame_size), total_frame_size);
            throw InvalidTagDataException();
        }

        // Do default data
        std::size_t expected_default_data_size = (max_frame_size - total_frame_size);
        if(!compressed) {
            std::size_t default_data_size = default_data.size();
            if(default_data_size > 0) {
                if(default_data.size() != expected_default_data_size) {
                    eprintf_error("Default data size is invalid (%zu > 0 && %zu != %zu)", default_data_size, static_cast<std::size_t>(default_data_size), expected_default_data_size);
                    throw InvalidTagDataException();
                }

                if(expected_default_data_size > 0) {
                    auto *default_data_big = this->default_data.data();
                    auto *default_data_little = default_data.data();

                    for(std::size_t node = 0; node < this->node_count; node++) {
                        if(!rotate[node]) {
                            auto &rotation_big = *reinterpret_cast<ModelAnimationsRotation::struct_big *>(default_data_big);
                            const auto &rotation_little = *reinterpret_cast<const ModelAnimationsRotation::struct_little *>(default_data_little);
                            rotation_big = rotation_little;
                            default_data_big += sizeof(rotation_big);
                            default_data_little += sizeof(rotation_big);
                        }
                        if(!transform[node]) {
                            auto &transform_big = *reinterpret_cast<ModelAnimationsTransform::struct_big *>(default_data_big);
                            const auto &transform_little = *reinterpret_cast<const ModelAnimationsTransform::struct_little *>(default_data_little);
                            transform_big = transform_little;
                            default_data_big += sizeof(transform_big);
                            default_data_little += sizeof(transform_big);
                        }
                        if(!scale[node]) {
                            auto &scale_big = *reinterpret_cast<ModelAnimationscale::struct_big *>(default_data_big);
                            const auto &scale_little = *reinterpret_cast<const ModelAnimationscale::struct_little *>(default_data_little);
                            scale_big = scale_little;
                            default_data_big += sizeof(scale_big);
                            default_data_little += sizeof(scale_big);
                        }
                    }
                }
            }
        }
        // Zero out default data if there is none
        else {
            this->default_data.clear();
            this->default_data.resize(expected_default_data_size, std::byte());
        }

        if(compressed) {
            this->offset_to_compressed_data = static_cast<std::uint32_t>(frame_data_size_expected);
            this->frame_data.insert(this->frame_data.begin(), frame_data_size_expected, std::byte());
        }
        else {
            if(frame_data.size() != frame_data_size_expected) {
                eprintf_error("Frame data size is invalid (%zu != %zu)", frame_data.size(), frame_data_size_expected);
                throw InvalidTagDataException();
            }

            if(frame_data_size_expected) {
                auto *frame_data_big = this->frame_data.data();
                auto *frame_data_little = frame_data.data();
                for(std::size_t frame = 0; frame < frame_count; frame++) {
                    for(std::size_t node = 0; node < this->node_count; node++) {
                        if(rotate[node]) {
                            auto &rotation_big = *reinterpret_cast<ModelAnimationsRotation::struct_little *>(frame_data_big);
                            const auto &rotation_little = *reinterpret_cast<const ModelAnimationsRotation::struct_big *>(frame_data_little);
                            rotation_big = rotation_little;
                            frame_data_big += sizeof(rotation_big);
                            frame_data_little += sizeof(rotation_big);
                        }
                        if(transform[node]) {
                            auto &transform_big = *reinterpret_cast<ModelAnimationsTransform::struct_little *>(frame_data_big);
                            const auto &transform_little = *reinterpret_cast<const ModelAnimationsTransform::struct_big *>(frame_data_little);
                            transform_big = transform_little;
                            frame_data_big += sizeof(transform_big);
                            frame_data_little += sizeof(transform_big);
                        }
                        if(scale[node]) {
                            auto &scale_big = *reinterpret_cast<ModelAnimationscale::struct_little *>(frame_data_big);
                            const auto &scale_little = *reinterpret_cast<const ModelAnimationscale::struct_big *>(frame_data_little);
                            scale_big = scale_little;
                            frame_data_big += sizeof(scale_big);
                            frame_data_little += sizeof(scale_big);
                        }
                    }
                }
            }
        }
    }
}
