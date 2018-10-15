/*
 * Invader (c) 2018 Kavawuvi
 *
 * This program is free software under the GNU General Public License v3.0 or later. See LICENSE for more information.
 */

#include "../compile.hpp"

#include "unit_hud_interface.hpp"

namespace Invader::HEK {
    void compile_unit_hud_interface_tag(CompiledTag &compiled, const std::byte *data, std::size_t size) {
        BEGIN_COMPILE(UnitHUDInterface)
        DEFAULT_VALUE(tag.hud_background_width_scale, 1.0f);
        DEFAULT_VALUE(tag.hud_background_height_scale, 1.0f);
        DEFAULT_VALUE(tag.shield_panel_background_width_scale, 1.0f);
        DEFAULT_VALUE(tag.shield_panel_background_height_scale, 1.0f);
        DEFAULT_VALUE(tag.shield_panel_meter_width_scale, 1.0f);
        DEFAULT_VALUE(tag.shield_panel_meter_height_scale, 1.0f);
        //DEFAULT_VALUE(tag.shield_panel_meter_value_scale, 1.0f);
        DEFAULT_VALUE(tag.health_panel_background_width_scale, 1.0f);
        DEFAULT_VALUE(tag.health_panel_background_height_scale, 1.0f);
        DEFAULT_VALUE(tag.health_panel_meter_width_scale, 1.0f);
        DEFAULT_VALUE(tag.health_panel_meter_height_scale, 1.0f);
        //DEFAULT_VALUE(tag.health_panel_meter_value_scale, 1.0f);
        DEFAULT_VALUE(tag.motion_sensor_background_width_scale, 1.0f);
        DEFAULT_VALUE(tag.motion_sensor_background_height_scale, 1.0f);
        DEFAULT_VALUE(tag.motion_sensor_foreground_width_scale, 1.0f);
        DEFAULT_VALUE(tag.motion_sensor_foreground_height_scale, 1.0f);
        DEFAULT_VALUE(tag.motion_sensor_center_width_scale, 1.0f);
        DEFAULT_VALUE(tag.motion_sensor_center_height_scale, 1.0f);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.hud_background_interface_bitmap);
        COMPILE_MULTITEXTURE_OVERLAY(tag.hud_background_multitex_overlay);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.shield_panel_background_interface_bitmap);
        COMPILE_MULTITEXTURE_OVERLAY(tag.shield_panel_background_multitex_overlay);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.shield_panel_meter_meter_bitmap);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.health_panel_background_interface_bitmap);
        COMPILE_MULTITEXTURE_OVERLAY(tag.health_panel_background_multitex_overlay);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.health_panel_meter_meter_bitmap);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.motion_sensor_background_interface_bitmap);
        COMPILE_MULTITEXTURE_OVERLAY(tag.motion_sensor_background_multitex_overlays);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.motion_sensor_foreground_interface_bitmap);
        COMPILE_MULTITEXTURE_OVERLAY(tag.motion_sensor_foreground_multitex_overlays);
        ADD_REFLEXIVE_START(tag.overlays) {
            DEFAULT_VALUE(reflexive.height_scale, 1.0f);
            DEFAULT_VALUE(reflexive.width_scale, 1.0f);
            ADD_DEPENDENCY_ADJUST_SIZES(reflexive.interface_bitmap);
        } ADD_REFLEXIVE_END
        ADD_REFLEXIVE_START(tag.sounds) {
            ADD_DEPENDENCY_ADJUST_SIZES(reflexive.sound);
        } ADD_REFLEXIVE_END
        ADD_REFLEXIVE_START(tag.meters) {
            DEFAULT_VALUE(reflexive.background_width_scale, 1.0f);
            DEFAULT_VALUE(reflexive.background_height_scale, 1.0f);
            DEFAULT_VALUE(reflexive.meter_width_scale, 1.0f);
            DEFAULT_VALUE(reflexive.meter_height_scale, 1.0f);
            ADD_DEPENDENCY_ADJUST_SIZES(reflexive.background_interface_bitmap);
            ADD_DEPENDENCY_ADJUST_SIZES(reflexive.meter_meter_bitmap);
        } ADD_REFLEXIVE_END
        FINISH_COMPILE
    }
}
