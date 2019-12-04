// SPDX-License-Identifier: GPL-3.0-only

#include <invader/tag/parser/parser.hpp>
#include <invader/build/build_workload.hpp>

namespace Invader::Parser {
    template <typename T> void compile_object(T &tag) {
        tag.has_change_colors = tag.change_colors.size() > 0;
        tag.render_bounding_radius = tag.render_bounding_radius < tag.bounding_radius ? tag.bounding_radius : tag.render_bounding_radius;
    }

    void Biped::pre_compile(BuildWorkload &, std::size_t, std::size_t, std::size_t) {
        this->object_type = HEK::ObjectType::OBJECT_TYPE_BIPED;
        compile_object(*this);
        this->cosine_stationary_turning_threshold = std::cos(this->stationary_turning_threshold);
        this->crouch_camera_velocity = 1.0f / this->crouch_transition_time / TICK_RATE;
        this->cosine_maximum_slope_angle = static_cast<float>(std::cos(this->maximum_slope_angle));
        this->negative_sine_downhill_falloff_angle = static_cast<float>(-std::sin(this->downhill_falloff_angle));
        this->negative_sine_downhill_cutoff_angle = static_cast<float>(-std::sin(this->downhill_cutoff_angle));
        this->sine_uphill_falloff_angle = static_cast<float>(std::sin(this->uphill_falloff_angle));
        this->sine_uphill_cutoff_angle = static_cast<float>(std::sin(this->uphill_cutoff_angle));
    }
    void Biped::post_compile(BuildWorkload &workload, std::size_t tag_index, std::size_t struct_index, std::size_t offset) {
        auto &head_index = reinterpret_cast<struct_little *>(workload.structs[struct_index].data.data() + offset)->head_model_node_index;
        auto &model_id = this->model.tag_id;

        this->head_model_node_index = NULL_INDEX;
        if(model_id.is_null()) {
            workload.report_error(BuildWorkload::ErrorType::ERROR_TYPE_WARNING, "Biped is missing a model tag, so it will not spawn", tag_index);
        }
        else {
            auto &model_tag = workload.tags[model_id.index];
            auto &model_tag_header = workload.structs[*model_tag.base_struct];
            auto &model_tag_header_struct = *reinterpret_cast<GBXModel::struct_little *>(model_tag_header.data.data());
            std::size_t node_count = model_tag_header_struct.nodes.count.read();
            if(node_count) {
                auto *nodes = reinterpret_cast<GBXModelNode::struct_little *>(workload.structs[*model_tag_header.resolve_pointer(&model_tag_header_struct.nodes.pointer)].data.data());
                for(std::size_t n = 0; n < node_count; n++) {
                    auto &node = nodes[n];
                    if(std::strncmp(node.name.string, "bip01 head", sizeof(node.name.string) - 1) == 0) {
                        this->head_model_node_index = static_cast<HEK::Index>(n);
                    }
                }

                if(this->head_model_node_index == NULL_INDEX) {
                    workload.report_error(BuildWorkload::ErrorType::ERROR_TYPE_WARNING, "Biped model has no \"bip01 head\" node, so the biped will not spawn", tag_index);
                }
            }
            else {
                workload.report_error(BuildWorkload::ErrorType::ERROR_TYPE_WARNING, "Biped model has no nodes, so the biped will not spawn", tag_index);
            }
        }

        head_index = this->head_model_node_index;
    }
    void Vehicle::pre_compile(BuildWorkload &, std::size_t, std::size_t, std::size_t) {
        this->object_type = HEK::ObjectType::OBJECT_TYPE_VEHICLE;
        compile_object(*this);
    }
    void Weapon::pre_compile(BuildWorkload &, std::size_t, std::size_t, std::size_t) {
        this->object_type = HEK::ObjectType::OBJECT_TYPE_WEAPON;
        compile_object(*this);
    }
    void Equipment::pre_compile(BuildWorkload &, std::size_t, std::size_t, std::size_t) {
        this->object_type = HEK::ObjectType::OBJECT_TYPE_EQUIPMENT;
        compile_object(*this);
    }
    void Garbage::pre_compile(BuildWorkload &, std::size_t, std::size_t, std::size_t) {
        this->object_type = HEK::ObjectType::OBJECT_TYPE_GARBAGE;
        compile_object(*this);
    }
    void Projectile::pre_compile(BuildWorkload &, std::size_t, std::size_t, std::size_t) {
        this->object_type = HEK::ObjectType::OBJECT_TYPE_PROJECTILE;
        compile_object(*this);
        this->initial_velocity /= TICK_RATE;
        this->final_velocity /= TICK_RATE;
    }
    void Scenery::pre_compile(BuildWorkload &, std::size_t, std::size_t, std::size_t) {
        this->object_type = HEK::ObjectType::OBJECT_TYPE_SCENERY;
        compile_object(*this);
    }
    void Placeholder::pre_compile(BuildWorkload &, std::size_t, std::size_t, std::size_t) {
        this->object_type = HEK::ObjectType::OBJECT_TYPE_PLACEHOLDER;
        compile_object(*this);
    }
    void SoundScenery::pre_compile(BuildWorkload &, std::size_t, std::size_t, std::size_t) {
        this->object_type = HEK::ObjectType::OBJECT_TYPE_SOUND_SCENERY;
        compile_object(*this);
    }

    template <typename T> void compile_device(T &tag) {
        tag.inverse_power_transition_time = 1.0F / (TICK_RATE * tag.power_transition_time);
        tag.inverse_power_acceleration_time = 1.0F / (TICK_RATE * tag.power_acceleration_time);
        tag.inverse_position_transition_time = 1.0f / (TICK_RATE * tag.position_transition_time);
        tag.inverse_position_acceleration_time = 1.0f / (TICK_RATE * tag.position_acceleration_time);
    }

    void DeviceMachine::pre_compile(BuildWorkload &, std::size_t, std::size_t, std::size_t) {
        this->object_type = HEK::ObjectType::OBJECT_TYPE_DEVICE_MACHINE;
        compile_object(*this);
        compile_device(*this);
        this->door_open_time_ticks = static_cast<std::uint32_t>(this->door_open_time * TICK_RATE);
    }
    void DeviceControl::pre_compile(BuildWorkload &, std::size_t, std::size_t, std::size_t) {
        this->object_type = HEK::ObjectType::OBJECT_TYPE_DEVICE_CONTROL;
        compile_object(*this);
        compile_device(*this);
    }
    void DeviceLightFixture::pre_compile(BuildWorkload &, std::size_t, std::size_t, std::size_t) {
        this->object_type = HEK::ObjectType::OBJECT_TYPE_DEVICE_LIGHT_FIXTURE;
        compile_object(*this);
        compile_device(*this);
    }

    void ObjectFunction::pre_compile(BuildWorkload &, std::size_t, std::size_t, std::size_t) {
        if(this->bounds.from == 0.0f && this->bounds.to == 0.0f) {
            this->bounds.to = 1.0f; \
        }
        this->inverse_bounds = 1.0f / (this->bounds.to - this->bounds.from);
        if(this->step_count > 1) {
            this->inverse_step = 1.0f / (this->step_count - 1);
        }
        this->inverse_period = 1.0f / this->period;
    }

    void WeaponTrigger::pre_compile(BuildWorkload &workload, std::size_t tag_index, std::size_t, std::size_t offset) {
        this->illumination_recovery_rate = 1.0f / TICK_RATE / this->illumination_recovery_time;
        this->ejection_port_recovery_rate = 1.0f / TICK_RATE / this->ejection_port_recovery_time;

        this->firing_acceleration_rate = 1.0f / TICK_RATE / this->acceleration_time;
        this->firing_deceleration_rate = 1.0f / TICK_RATE / this->deceleration_time;

        this->error_acceleration_rate = 1.0f / TICK_RATE / this->error_acceleration_time;
        this->error_deceleration_rate = 1.0f / TICK_RATE / this->error_deceleration_time;

        // Jason Jones the accuracy of the weapon
        if(offset == 0) {
            auto &tag = workload.tags[tag_index];
            if(tag.path == "weapons\\pistol\\pistol") {
                this->minimum_error = DEGREES_TO_RADIANS(0.2F);
                this->error_angle.from = DEGREES_TO_RADIANS(0.2F);
                this->error_angle.to = DEGREES_TO_RADIANS(0.4F);
            }
            else if(tag.path == "weapons\\plasma rifle\\plasma rifle") {
                this->error_angle.from = DEGREES_TO_RADIANS(0.25F);
                this->error_angle.to = DEGREES_TO_RADIANS(2.5F);
            }
        }
    }
}
