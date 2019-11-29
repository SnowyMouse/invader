// SPDX-License-Identifier: GPL-3.0-only

#include <invader/tag/parser/parser.hpp>
#include <invader/build2/build_workload.hpp>

namespace Invader::Parser {
    template <typename T> void compile_object(T &tag) {
        tag.has_change_colors = tag.change_colors.size() > 0;
    }

    void Biped::pre_compile(BuildWorkload2 &, std::size_t, std::size_t, std::size_t) {
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
    void Vehicle::pre_compile(BuildWorkload2 &, std::size_t, std::size_t, std::size_t) {
        this->object_type = HEK::ObjectType::OBJECT_TYPE_VEHICLE;
        compile_object(*this);
    }
    void Weapon::pre_compile(BuildWorkload2 &, std::size_t, std::size_t, std::size_t) {
        this->object_type = HEK::ObjectType::OBJECT_TYPE_WEAPON;
        compile_object(*this);
    }
    void Equipment::pre_compile(BuildWorkload2 &, std::size_t, std::size_t, std::size_t) {
        this->object_type = HEK::ObjectType::OBJECT_TYPE_EQUIPMENT;
        compile_object(*this);
    }
    void Garbage::pre_compile(BuildWorkload2 &, std::size_t, std::size_t, std::size_t) {
        this->object_type = HEK::ObjectType::OBJECT_TYPE_GARBAGE;
        compile_object(*this);
    }
    void Projectile::pre_compile(BuildWorkload2 &, std::size_t, std::size_t, std::size_t) {
        this->object_type = HEK::ObjectType::OBJECT_TYPE_PROJECTILE;
        compile_object(*this);
        this->initial_velocity /= TICK_RATE;
        this->final_velocity /= TICK_RATE;
    }
    void Scenery::pre_compile(BuildWorkload2 &, std::size_t, std::size_t, std::size_t) {
        this->object_type = HEK::ObjectType::OBJECT_TYPE_SCENERY;
        compile_object(*this);
    }
    void Placeholder::pre_compile(BuildWorkload2 &, std::size_t, std::size_t, std::size_t) {
        this->object_type = HEK::ObjectType::OBJECT_TYPE_PLACEHOLDER;
        compile_object(*this);
    }
    void SoundScenery::pre_compile(BuildWorkload2 &, std::size_t, std::size_t, std::size_t) {
        this->object_type = HEK::ObjectType::OBJECT_TYPE_SOUND_SCENERY;
        compile_object(*this);
    }

    template <typename T> void compile_device(T &tag) {
        tag.inverse_power_transition_time = 1.0F / (TICK_RATE * tag.power_transition_time);
        tag.inverse_power_acceleration_time = 1.0F / (TICK_RATE * tag.power_acceleration_time);
        tag.inverse_position_transition_time = 1.0f / (TICK_RATE * tag.position_transition_time);
        tag.inverse_position_acceleration_time = 1.0f / (TICK_RATE * tag.position_acceleration_time);
    }

    void DeviceMachine::pre_compile(BuildWorkload2 &, std::size_t, std::size_t, std::size_t) {
        this->object_type = HEK::ObjectType::OBJECT_TYPE_DEVICE_MACHINE;
        compile_object(*this);
        compile_device(*this);
        this->door_open_time_ticks = static_cast<std::uint32_t>(this->door_open_time * TICK_RATE);
    }
    void DeviceControl::pre_compile(BuildWorkload2 &, std::size_t, std::size_t, std::size_t) {
        this->object_type = HEK::ObjectType::OBJECT_TYPE_DEVICE_CONTROL;
        compile_object(*this);
        compile_device(*this);
    }
    void DeviceLightFixture::pre_compile(BuildWorkload2 &, std::size_t, std::size_t, std::size_t) {
        this->object_type = HEK::ObjectType::OBJECT_TYPE_DEVICE_LIGHT_FIXTURE;
        compile_object(*this);
        compile_device(*this);
    }

    void ObjectFunction::pre_compile(BuildWorkload2 &, std::size_t, std::size_t, std::size_t) {
        if(this->bounds.from == 0.0f && this->bounds.to == 0.0f) {
            this->bounds.to = 1.0f; \
        }
        this->inverse_bounds = 1.0f / (this->bounds.to - this->bounds.from);
        if(this->step_count > 1) {
            this->inverse_step = 1.0f / (this->step_count - 1);
        }
        this->inverse_period = 1.0f / this->period;
    }

    void WeaponTrigger::pre_compile(BuildWorkload2 &workload, std::size_t tag_index, std::size_t, std::size_t offset) {
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
