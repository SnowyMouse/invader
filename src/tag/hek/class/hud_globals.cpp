// SPDX-License-Identifier: GPL-3.0-only

#include <invader/tag/hek/compile.hpp>
#include <invader/tag/hek/class/hud_globals.hpp>

namespace Invader::HEK {
    void compile_hud_globals_tag(CompiledTag &compiled, const std::byte *data, std::size_t size) {
        BEGIN_COMPILE(HUDGlobals)
        DEFAULT_VALUE(tag.width_scale, 1.0f);
        DEFAULT_VALUE(tag.height_scale, 1.0f);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.single_player_font);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.multi_player_font);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.item_message_text);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.icon_bitmap);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.alternate_icon_text);
        ADD_REFLEXIVE(tag.button_icons);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.hud_messages);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.arrow_bitmap);
        ADD_REFLEXIVE(tag.waypoint_arrows);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.default_weapon_hud);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.hud_damage_indicator_bitmap);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.carnage_report_bitmap);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.checkpoint_sound);
        FINISH_COMPILE
    }
}
