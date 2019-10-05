// SPDX-License-Identifier: GPL-3.0-only

#include "../compile.hpp"

#include "weapon_hud_interface.hpp"

namespace Invader::HEK {
    void compile_weapon_hud_interface_tag(CompiledTag &compiled, const std::byte *data, std::size_t size) {
        BEGIN_COMPILE(WeaponHUDInterface)
        ADD_DEPENDENCY_ADJUST_SIZES(tag.child_hud);
        ADD_REFLEXIVE_START(tag.static_elements) {
            DEFAULT_VALUE(reflexive.height_scale, 1.0f);
            DEFAULT_VALUE(reflexive.width_scale, 1.0f);
            ADD_DEPENDENCY_ADJUST_SIZES(reflexive.interface_bitmap);
            COMPILE_MULTITEXTURE_OVERLAY(reflexive.multitex_overlay);
        } ADD_REFLEXIVE_END
        ADD_REFLEXIVE_START(tag.meter_elements) {
            DEFAULT_VALUE(reflexive.height_scale, 1.0f);
            DEFAULT_VALUE(reflexive.width_scale, 1.0f);
            ADD_DEPENDENCY_ADJUST_SIZES(reflexive.meter_bitmap);
        } ADD_REFLEXIVE_END
        ADD_REFLEXIVE_START(tag.number_elements)  {
            DEFAULT_VALUE(reflexive.height_scale, 1.0f);
            DEFAULT_VALUE(reflexive.width_scale, 1.0f);
        } ADD_REFLEXIVE_END

        std::uint32_t crosshair_types = 0;
        ADD_REFLEXIVE_START(tag.crosshairs) {
            crosshair_types |= (1 << reflexive.crosshair_type.read());
            ADD_DEPENDENCY_ADJUST_SIZES(reflexive.crosshair_bitmap);
            ADD_REFLEXIVE_START(reflexive.crosshair_overlays) {
                DEFAULT_VALUE_POSITIVE(reflexive.height_scale, 1.0f);
                DEFAULT_VALUE_POSITIVE(reflexive.width_scale, 1.0f);
            } ADD_REFLEXIVE_END
        } ADD_REFLEXIVE_END

        tag.crosshair_types = crosshair_types;

        ADD_REFLEXIVE_START(tag.overlay_elements) {
            ADD_DEPENDENCY_ADJUST_SIZES(reflexive.overlay_bitmap);
            ADD_REFLEXIVE_START(reflexive.overlays) {
                DEFAULT_VALUE(reflexive.height_scale, 1.0f);
                DEFAULT_VALUE(reflexive.width_scale, 1.0f);
            } ADD_REFLEXIVE_END
        } ADD_REFLEXIVE_END
        ADD_REFLEXIVE_START(tag.screen_effect) {
            ADD_DEPENDENCY_ADJUST_SIZES(reflexive.mask_fullscreen);
            ADD_DEPENDENCY_ADJUST_SIZES(reflexive.mask_splitscreen);
        } ADD_REFLEXIVE_END
        FINISH_COMPILE
    }
}
