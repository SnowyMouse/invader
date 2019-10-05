// SPDX-License-Identifier: GPL-3.0-only

#include "../compile.hpp"
#include "grenade_hud_interface.hpp"

namespace Invader::HEK {
    void compile_grenade_hud_interface_tag(CompiledTag &compiled, const std::byte *data, std::size_t size) {
        BEGIN_COMPILE(GrenadeHUDInterface)
        DEFAULT_VALUE(tag.background_width_scale, 1.0f);
        DEFAULT_VALUE(tag.background_height_scale, 1.0f);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.background_interface_bitmap);
        COMPILE_MULTITEXTURE_OVERLAY(tag.background_multitex_overlay);
        DEFAULT_VALUE(tag.total_grenades_background_width_scale, 1.0f);
        DEFAULT_VALUE(tag.total_grenades_background_height_scale, 1.0f);
        DEFAULT_VALUE(tag.total_grenades_numbers_width_scale, 1.0f);
        DEFAULT_VALUE(tag.total_grenades_numbers_height_scale, 1.0f);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.total_grenades_background_interface_bitmap);
        COMPILE_MULTITEXTURE_OVERLAY(tag.total_grenades_background_multitex_overlay);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.total_grenades_overlay_bitmap);
        ADD_REFLEXIVE_START(tag.total_grenades_overlays) {
            DEFAULT_VALUE(reflexive.width_scale, 1.0f);
            DEFAULT_VALUE(reflexive.height_scale, 1.0f);
        } ADD_REFLEXIVE_END
        ADD_BASIC_DEPENDENCY_REFLEXIVE(tag.total_grenades_warning_sounds, sound);
        FINISH_COMPILE
    }
}
