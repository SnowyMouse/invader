/*
 * Invader (c) 2018 Kavawuvi
 *
 * This program is free software under the GNU General Public License v3.0 or later. See LICENSE for more information.
 */

#include "../compile.hpp"

#include "scenario.hpp"

namespace Invader::HEK {
    void compile_scenario_tag(CompiledTag &compiled, const std::byte *data, std::size_t size) {
        BEGIN_COMPILE(Scenario);

        ADD_BASIC_DEPENDENCY_REFLEXIVE(tag.skies, sky);
        ADD_BASIC_DEPENDENCY_REFLEXIVE(tag.child_scenarios, child_scenario);
        ADD_REFLEXIVE(tag.predicted_resources);
        ADD_REFLEXIVE(tag.functions);
        std::size_t editor_scenario_data_size = tag.editor_scenario_data.size;
        ASSERT_SIZE(editor_scenario_data_size);
        ADD_POINTER_FROM_INT32(tag.editor_scenario_data.pointer, compiled.data.size());
        compiled.data.insert(compiled.data.end(), data, data + editor_scenario_data_size);
        PAD_32_BIT
        INCREMENT_DATA_PTR(editor_scenario_data_size);

        ADD_REFLEXIVE_START(tag.comments) {
            std::size_t comment_size = reflexive.comment.size;
            ASSERT_SIZE(comment_size);
            ADD_POINTER_FROM_INT32(reflexive.comment.pointer, compiled.data.size());
            compiled.data.insert(compiled.data.end(), data, data + comment_size);
            PAD_32_BIT
            INCREMENT_DATA_PTR(comment_size);
        } ADD_REFLEXIVE_END;

        ADD_REFLEXIVE(tag.object_names);
        ADD_REFLEXIVE(tag.scenery);
        ADD_BASIC_DEPENDENCY_REFLEXIVE(tag.scenery_palette, name);
        ADD_REFLEXIVE(tag.bipeds);
        ADD_BASIC_DEPENDENCY_REFLEXIVE(tag.biped_palette, name);
        ADD_REFLEXIVE(tag.vehicles);
        ADD_BASIC_DEPENDENCY_REFLEXIVE(tag.vehicle_palette, name);
        ADD_REFLEXIVE(tag.equipment);
        ADD_BASIC_DEPENDENCY_REFLEXIVE(tag.equipment_palette, name);
        ADD_REFLEXIVE(tag.weapons);
        ADD_BASIC_DEPENDENCY_REFLEXIVE(tag.weapon_palette, name);
        ADD_REFLEXIVE(tag.device_groups);
        ADD_REFLEXIVE(tag.machines);
        ADD_BASIC_DEPENDENCY_REFLEXIVE(tag.machine_palette, name);
        ADD_REFLEXIVE(tag.controls);
        ADD_BASIC_DEPENDENCY_REFLEXIVE(tag.control_palette, name);
        ADD_REFLEXIVE(tag.light_fixtures);
        ADD_BASIC_DEPENDENCY_REFLEXIVE(tag.light_fixture_palette, name);
        ADD_REFLEXIVE(tag.sound_scenery);
        ADD_BASIC_DEPENDENCY_REFLEXIVE(tag.sound_scenery_palette, name);

        ADD_REFLEXIVE_START(tag.player_starting_profile) {
            ADD_DEPENDENCY_ADJUST_SIZES(reflexive.primary_weapon);
            ADD_DEPENDENCY_ADJUST_SIZES(reflexive.secondary_weapon);
        } ADD_REFLEXIVE_END;

        ADD_REFLEXIVE(tag.player_starting_locations);
        ADD_REFLEXIVE(tag.trigger_volumes);

        ADD_REFLEXIVE_START(tag.recorded_animations) {
            std::size_t animation_size = reflexive.recorded_animation_event_stream.size;
            ASSERT_SIZE(animation_size);
            ADD_POINTER_FROM_INT32(reflexive.recorded_animation_event_stream.pointer, compiled.data.size());
            compiled.data.insert(compiled.data.end(), data, data + animation_size);
            PAD_32_BIT
            INCREMENT_DATA_PTR(animation_size);
        } ADD_REFLEXIVE_END;

        ADD_BASIC_DEPENDENCY_REFLEXIVE(tag.netgame_flags, weapon_group);
        ADD_BASIC_DEPENDENCY_REFLEXIVE(tag.netgame_equipment, item_collection);
        ADD_REFLEXIVE_START(tag.starting_equipment) {
            ADD_DEPENDENCY_ADJUST_SIZES(reflexive.item_collection_1);
            ADD_DEPENDENCY_ADJUST_SIZES(reflexive.item_collection_2);
            ADD_DEPENDENCY_ADJUST_SIZES(reflexive.item_collection_3);
            ADD_DEPENDENCY_ADJUST_SIZES(reflexive.item_collection_4);
            ADD_DEPENDENCY_ADJUST_SIZES(reflexive.item_collection_5);
            ADD_DEPENDENCY_ADJUST_SIZES(reflexive.item_collection_6);
        } ADD_REFLEXIVE_END

        ADD_REFLEXIVE(tag.bsp_switch_trigger_volumes);
        ADD_REFLEXIVE(tag.decals);
        ADD_BASIC_DEPENDENCY_REFLEXIVE(tag.decal_palette, reference);
        ADD_BASIC_DEPENDENCY_REFLEXIVE(tag.detail_object_collection_palette, reference);
        ADD_BASIC_DEPENDENCY_REFLEXIVE(tag.actor_palette, reference);

        ADD_REFLEXIVE_START(tag.encounters) {
            ADD_REFLEXIVE_START(reflexive.squads) {
                ADD_REFLEXIVE(reflexive.move_positions);
                ADD_REFLEXIVE(reflexive.starting_locations);
            } ADD_REFLEXIVE_END
            ADD_REFLEXIVE(reflexive.platoons);
            ADD_REFLEXIVE(reflexive.firing_positions);
            ADD_REFLEXIVE(reflexive.player_starting_locations);
        } ADD_REFLEXIVE_END

        ADD_REFLEXIVE_START(tag.command_lists) {
            ADD_REFLEXIVE(reflexive.commands);
            ADD_REFLEXIVE(reflexive.points);
        } ADD_REFLEXIVE_END

        ADD_REFLEXIVE_START(tag.ai_animation_references) {
            ADD_DEPENDENCY_ADJUST_SIZES(reflexive.animation_graph);
        } ADD_REFLEXIVE_END

        ADD_REFLEXIVE(tag.ai_script_references);
        ADD_REFLEXIVE(tag.ai_recording_references);

        ADD_REFLEXIVE_START(tag.ai_conversations) {
            ADD_REFLEXIVE(reflexive.participants);
            ADD_REFLEXIVE_START(reflexive.lines) {
                ADD_DEPENDENCY_ADJUST_SIZES(reflexive.variant_1);
                ADD_DEPENDENCY_ADJUST_SIZES(reflexive.variant_2);
                ADD_DEPENDENCY_ADJUST_SIZES(reflexive.variant_3);
                ADD_DEPENDENCY_ADJUST_SIZES(reflexive.variant_4);
                ADD_DEPENDENCY_ADJUST_SIZES(reflexive.variant_5);
                ADD_DEPENDENCY_ADJUST_SIZES(reflexive.variant_6);
            } ADD_REFLEXIVE_END
        } ADD_REFLEXIVE_END

        ASSERT_SIZE(tag.script_syntax_data.size)
        ADD_POINTER_FROM_INT32(tag.script_syntax_data.pointer, compiled.data.size());
        compiled.data.insert(compiled.data.end(), tag.script_syntax_data.size, std::byte());
        auto &table = *reinterpret_cast<ScenarioScriptNodeTable<LittleEndian> *>(compiled.data.data() + compiled.data.size() - tag.script_syntax_data.size);
        table = *reinterpret_cast<const ScenarioScriptNodeTable<BigEndian> *>(data);

        auto node_count = static_cast<std::size_t>(table.maximum_count);

        if(static_cast<std::size_t>(tag.script_syntax_data.size) != sizeof(table) + node_count * sizeof(ScenarioScriptNode<BigEndian>)) {
            throw OutOfBoundsException();
        }

        auto *nodes = reinterpret_cast<ScenarioScriptNode<LittleEndian> *>(&table + 1);
        auto *nodes_big = reinterpret_cast<const ScenarioScriptNode<BigEndian> *>(data + sizeof(table));
        for(std::size_t i = 0; i < node_count; i++) {
            nodes[i] = nodes_big[i];
        }
        INCREMENT_DATA_PTR(tag.script_syntax_data.size);

        // Fix pointer and element size
        if(table.first_element_ptr == 0x40440028) {
            table.element_size = 0x7F4F;
            table.maximum_count = 0x764F;
        }

        ASSERT_SIZE(tag.script_string_data.size)
        ADD_POINTER_FROM_INT32(tag.script_string_data.pointer, compiled.data.size());
        compiled.data.insert(compiled.data.end(), data, data + tag.script_string_data.size);
        INCREMENT_DATA_PTR(tag.script_string_data.size);
        PAD_32_BIT

        ADD_REFLEXIVE(tag.scripts);
        ADD_REFLEXIVE(tag.globals);
        ADD_BASIC_DEPENDENCY_REFLEXIVE(tag.references, reference);

        skip_data = true;
        ADD_REFLEXIVE_START(tag.source_files) {
            INCREMENT_DATA_PTR(reflexive.source.size);
        } ADD_REFLEXIVE_END
        skip_data = false;

        ADD_REFLEXIVE(tag.cutscene_flags);
        ADD_REFLEXIVE(tag.cutscene_camera_points);
        ADD_REFLEXIVE(tag.cutscene_titles);

        ADD_DEPENDENCY_ADJUST_SIZES(tag.custom_object_names);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.ingame_help_text);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.hud_messages);
        ADD_BASIC_DEPENDENCY_REFLEXIVE(tag.structure_bsps, structure_bsp);

        FINISH_COMPILE
    }
}
