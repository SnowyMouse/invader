// SPDX-License-Identifier: GPL-3.0-only

#include "invader/tag/hek/compile.hpp"
#include "invader/tag/hek/class/ui_widget_definition.hpp"

namespace Invader::HEK {
    void compile_ui_widget_definition_tag(CompiledTag &compiled, const std::byte *data, std::size_t size) {
        BEGIN_COMPILE(UIWidgetDefinition)
        ADD_DEPENDENCY_ADJUST_SIZES(tag.background_bitmap);
        ADD_REFLEXIVE(tag.game_data_inputs);
        ADD_REFLEXIVE_START(tag.event_handlers) {
            ADD_DEPENDENCY_ADJUST_SIZES(reflexive.widget_tag);
            ADD_DEPENDENCY_ADJUST_SIZES(reflexive.sound_effect);
        } ADD_REFLEXIVE_END;
        ADD_REFLEXIVE(tag.search_and_replace_functions);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.text_label_unicode_strings_list);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.text_font);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.list_header_bitmap);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.list_footer_bitmap);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.extended_description_widget);
        ADD_BASIC_DEPENDENCY_REFLEXIVE(tag.conditional_widgets, widget_tag);
        ADD_BASIC_DEPENDENCY_REFLEXIVE(tag.child_widgets, widget_tag);
        FINISH_COMPILE
    }
}
