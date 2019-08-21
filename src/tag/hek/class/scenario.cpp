/*
 * Invader (c) 2018 Kavawuvi
 *
 * This program is free software under the GNU General Public License v3.0 or later. See LICENSE for more information.
 */

#include "../../../hek/constants.hpp"
#include "../compile.hpp"

#include "scenario.hpp"

namespace Invader::HEK {
    void compile_scenario_tag(CompiledTag &compiled, const std::byte *data, std::size_t size) {
        BEGIN_COMPILE(Scenario);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.don_t_use);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.won_t_use);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.can_t_use);
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

        std::size_t object_names_offset = compiled.data.size();
        ADD_REFLEXIVE(tag.object_names);

        #define ADD_REFLEXIVE_SCENARIO_OBJECT(ref, type) \
            ADD_REFLEXIVE_START(ref) { \
                std::size_t object_name_index = static_cast<std::size_t>(reflexive.name.read()); \
                if(object_name_index < tag.object_names.count) { \
                    auto &object_name = reinterpret_cast<ScenarioObjectName<BigEndian> *>(compiled.data.data() + object_names_offset)[object_name_index]; \
                    object_name.object_type = type; \
                    object_name.object_index = static_cast<std::int16_t>(i); \
                } \
            } ADD_REFLEXIVE_END

        ADD_REFLEXIVE_SCENARIO_OBJECT(tag.scenery, ObjectType::OBJECT_TYPE_SCENERY);
        ADD_BASIC_DEPENDENCY_REFLEXIVE(tag.scenery_palette, name);
        ADD_REFLEXIVE_SCENARIO_OBJECT(tag.bipeds, ObjectType::OBJECT_TYPE_BIPED);
        ADD_BASIC_DEPENDENCY_REFLEXIVE(tag.biped_palette, name);
        ADD_REFLEXIVE_SCENARIO_OBJECT(tag.vehicles, ObjectType::OBJECT_TYPE_VEHICLE);
        ADD_BASIC_DEPENDENCY_REFLEXIVE(tag.vehicle_palette, name);
        ADD_REFLEXIVE_SCENARIO_OBJECT(tag.equipment, ObjectType::OBJECT_TYPE_EQUIPMENT);
        ADD_BASIC_DEPENDENCY_REFLEXIVE(tag.equipment_palette, name);
        ADD_REFLEXIVE_SCENARIO_OBJECT(tag.weapons, ObjectType::OBJECT_TYPE_WEAPON);
        ADD_BASIC_DEPENDENCY_REFLEXIVE(tag.weapon_palette, name);
        ADD_REFLEXIVE(tag.device_groups);
        ADD_REFLEXIVE_SCENARIO_OBJECT(tag.machines, ObjectType::OBJECT_TYPE_DEVICE_MACHINE);
        ADD_BASIC_DEPENDENCY_REFLEXIVE(tag.machine_palette, name);
        ADD_REFLEXIVE_SCENARIO_OBJECT(tag.controls, ObjectType::OBJECT_TYPE_DEVICE_CONTROL);
        ADD_BASIC_DEPENDENCY_REFLEXIVE(tag.control_palette, name);
        ADD_REFLEXIVE_SCENARIO_OBJECT(tag.light_fixtures, ObjectType::OBJECT_TYPE_DEVICE_LIGHT_FIXTURE);
        ADD_BASIC_DEPENDENCY_REFLEXIVE(tag.light_fixture_palette, name);
        ADD_REFLEXIVE_SCENARIO_OBJECT(tag.sound_scenery, ObjectType::OBJECT_TYPE_SOUND_SCENERY);
        ADD_BASIC_DEPENDENCY_REFLEXIVE(tag.sound_scenery_palette, name);

        ADD_REFLEXIVE_START(tag.player_starting_profile) {
            ADD_DEPENDENCY_ADJUST_SIZES(reflexive.primary_weapon);
            ADD_DEPENDENCY_ADJUST_SIZES(reflexive.secondary_weapon);
        } ADD_REFLEXIVE_END;

        ADD_REFLEXIVE(tag.player_starting_locations);

        std::size_t trigger_volume_offset = compiled.data.size();
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

        skip_data = true;
        ADD_REFLEXIVE(tag.bsp_switch_trigger_volumes);
        skip_data = false;

        std::vector<ScenarioBSPSwitchTriggerVolume<LittleEndian>> switch_volumes;
        auto *volumes = reinterpret_cast<ScenarioTriggerVolume<LittleEndian> *>(compiled.data.data() + trigger_volume_offset);
        for(std::size_t i = 0; i < tag.trigger_volumes.count; i++) {
            auto &volume = volumes[i];
            volume.unknown = 1;

            // Check if it starts with "bsp" and it's null terminated
            if(volume.name.string[sizeof(volume.name.string) - 1] == 0 && std::memcmp(volume.name.string, "bsp", 3) == 0) {
                char *end = nullptr;

                // Get the first BSP index
                long bsp_1 = std::strtol(volume.name.string + 3, &end, 10);
                if(end && *end == ',' && end + 1 < volume.name.string + sizeof(volume.name.string) - 1) {
                    end++;
                    char *end2 = nullptr;

                    // Second BSP index
                    long bsp_2 = std::strtol(end, &end2, 10);
                    if(end2 && *end2 == 0) {
                        ScenarioBSPSwitchTriggerVolume<LittleEndian> new_volume = {};
                        new_volume.source = static_cast<std::int16_t>(bsp_1);
                        new_volume.destination = static_cast<std::int16_t>(bsp_2);
                        new_volume.trigger_volume = static_cast<std::int16_t>(i);
                        new_volume.unknown = 0xFFFF;
                        switch_volumes.push_back(new_volume);
                    }
                }
                else {
                    continue;
                }
            }
        }

        // Add the volumes we found
        auto switch_trigger_count = switch_volumes.size();
        tag.bsp_switch_trigger_volumes.count = static_cast<std::uint32_t>(switch_trigger_count);
        if(switch_trigger_count > 0) {
            ADD_POINTER_FROM_INT32(tag.bsp_switch_trigger_volumes.pointer, compiled.data.size());
            compiled.data.insert(compiled.data.end(), reinterpret_cast<std::byte *>(switch_volumes.data()), reinterpret_cast<std::byte *>(switch_volumes.data() + switch_volumes.size()));
        }

        ADD_REFLEXIVE(tag.decals);
        ADD_BASIC_DEPENDENCY_REFLEXIVE(tag.decal_palette, reference);
        ADD_BASIC_DEPENDENCY_REFLEXIVE(tag.detail_object_collection_palette, reference);
        ADD_BASIC_DEPENDENCY_REFLEXIVE(tag.actor_palette, reference);

        ADD_REFLEXIVE_START(tag.encounters) {
            reflexive.one = 1;
            ADD_REFLEXIVE_START(reflexive.squads) {
                ADD_REFLEXIVE(reflexive.move_positions);
                ADD_REFLEXIVE_START(reflexive.starting_locations) {
                    reflexive.unknown = 0;
                } ADD_REFLEXIVE_END;
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

        // Increment this
        INCREMENT_DATA_PTR(tag.script_syntax_data.size);
        ASSERT_SIZE(tag.script_string_data.size)

        // Hold a list of references here
        std::vector<CompiledTagDependency> script_dependencies;
        const char *script_string_data = reinterpret_cast<const char *>(data);

        // Iterate through the nodes to get references
        for(std::uint16_t c = 0; c < table.size.read(); c++) {
            // Check if we know the class
            HEK::TagClassInt tag_class = HEK::TAG_CLASS_NONE;
            auto &node = nodes[c];

            // Check the class type
            switch(node.type.read()) {
                case HEK::SCENARIO_SCRIPT_VALUE_TYPE_SOUND:
                    tag_class = HEK::TAG_CLASS_SOUND;
                    break;

                case HEK::SCENARIO_SCRIPT_VALUE_TYPE_EFFECT:
                    tag_class = HEK::TAG_CLASS_EFFECT;
                    break;

                case HEK::SCENARIO_SCRIPT_VALUE_TYPE_DAMAGE:
                    tag_class = HEK::TAG_CLASS_DAMAGE_EFFECT;
                    break;

                case HEK::SCENARIO_SCRIPT_VALUE_TYPE_LOOPING_SOUND:
                    tag_class = HEK::TAG_CLASS_SOUND_LOOPING;
                    break;

                case HEK::SCENARIO_SCRIPT_VALUE_TYPE_ANIMATION_GRAPH:
                    tag_class = HEK::TAG_CLASS_MODEL_ANIMATIONS;
                    break;

                case HEK::SCENARIO_SCRIPT_VALUE_TYPE_ACTOR_VARIANT:
                    tag_class = HEK::TAG_CLASS_ACTOR_VARIANT;
                    break;

                case HEK::SCENARIO_SCRIPT_VALUE_TYPE_DAMAGE_EFFECT:
                    tag_class = HEK::TAG_CLASS_DAMAGE_EFFECT;
                    break;

                case HEK::SCENARIO_SCRIPT_VALUE_TYPE_OBJECT_DEFINITION:
                    tag_class = HEK::TAG_CLASS_OBJECT;
                    break;

                default:
                    continue;
            }

            if(tag_class != HEK::TAG_CLASS_NONE) {
                // Check if we should leave it alone
                auto flags = nodes[c].flags.read();
                if(flags.is_global || flags.is_script_call) {
                    continue;
                }

                // Get the string
                const char *string = script_string_data + node.string_offset.read();
                std::size_t dependency_offset = reinterpret_cast<std::byte *>(&node.data) - compiled.data.data();

                // Add it to the list
                CompiledTagDependency dependency_to_add;
                dependency_to_add.offset = dependency_offset;
                dependency_to_add.path = string;
                dependency_to_add.tag_class_int = tag_class;
                dependency_to_add.tag_id_only = true;
                compiled.dependencies.push_back(dependency_to_add);
                script_dependencies.push_back(dependency_to_add);
            }
        }

        // Fix pointer and element size
        if(table.first_element_ptr == 0x40440028) {
            table.element_size = 0x7F4F;
            table.maximum_count = 0x764F;
        }

        ADD_POINTER_FROM_INT32(tag.script_string_data.pointer, compiled.data.size());
        compiled.data.insert(compiled.data.end(), data, data + tag.script_string_data.size);
        INCREMENT_DATA_PTR(tag.script_string_data.size);
        PAD_32_BIT

        ADD_REFLEXIVE(tag.scripts);
        ADD_REFLEXIVE(tag.globals);
        ADD_REFLEXIVE_START(tag.references) {
            const char *path = reinterpret_cast<const char *>(data);
            ADD_DEPENDENCY_ADJUST_SIZES(reflexive.reference);

            // If it's a script dependency that's already in the map, remove it
            if(reflexive.reference.path_size != 0) {
                auto tag_class_int = reflexive.reference.tag_class_int.read();
                for(auto dependency = script_dependencies.begin(); dependency != script_dependencies.end(); dependency++) {
                    if(dependency->path == path) {
                        if(dependency->tag_class_int == tag_class_int || (dependency->tag_class_int == HEK::TagClassInt::TAG_CLASS_OBJECT && IS_OBJECT_TAG(tag_class_int))) {
                            script_dependencies.erase(dependency);
                            break;
                        }
                    }
                }
            }
        } ADD_REFLEXIVE_END

        // Add the remaining dependencies
        std::size_t size_before_adding_references = compiled.data.size();
        for(auto &dependency : script_dependencies) {
            CompiledTagDependency dependency_reference = dependency;
            ScenarioReference<LittleEndian> dependency_reference_tag_data = {};
            dependency_reference.offset = compiled.data.size() + (reinterpret_cast<std::byte *>(&dependency_reference_tag_data.reference) - reinterpret_cast<std::byte *>(&dependency_reference_tag_data));
            dependency_reference_tag_data.reference.tag_class_int = dependency.tag_class_int;
            dependency_reference.tag_id_only = false;
            compiled.dependencies.push_back(dependency_reference);
            compiled.data.insert(compiled.data.end(), reinterpret_cast<std::byte *>(&dependency_reference_tag_data), reinterpret_cast<std::byte *>(&dependency_reference_tag_data + 1));
        }

        // Increment the dependency array count, making a brand new array if it was 0
        if(tag.references.count == 0) {
            tag.references.count = script_dependencies.size();
            ADD_POINTER_FROM_INT32(tag.references.pointer, size_before_adding_references);
        }
        else {
            tag.references.count = tag.references.count.read() + script_dependencies.size();
        }

        skip_data = true;
        ADD_REFLEXIVE_START(tag.source_files) {
            INCREMENT_DATA_PTR(reflexive.source.size);
        } ADD_REFLEXIVE_END
        skip_data = false;

        ADD_REFLEXIVE(tag.cutscene_flags);
        ADD_REFLEXIVE(tag.cutscene_camera_points);
        ADD_REFLEXIVE_START(tag.cutscene_titles) {
            reflexive.fade_in_time = reflexive.fade_in_time * TICK_RATE;
            reflexive.fade_out_time = reflexive.fade_out_time * TICK_RATE;
            reflexive.up_time = reflexive.up_time * TICK_RATE;
        } ADD_REFLEXIVE_END

        ADD_DEPENDENCY_ADJUST_SIZES(tag.custom_object_names);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.ingame_help_text);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.hud_messages);
        ADD_BASIC_DEPENDENCY_REFLEXIVE(tag.structure_bsps, structure_bsp);

        FINISH_COMPILE
    }
}
