/*
 * Invader (c) 2018 Kavawuvi
 *
 * This program is free software under the GNU General Public License v3.0 or later. See LICENSE for more information.
 */

#include <string>
#include <cmath>

#include "salamander_scenario.hpp"

namespace Invader {
    struct SpawnStructBase {
        int type;
        int name;
        int not_placed_automatically;
        int not_placed_on_easy;
        int not_placed_on_hard;
        int permutation;
        HEK::Point3D<HEK::BigEndian> position;
        HEK::Euler3D<HEK::BigEndian> rotation;
    };

    SalamanderScenario parse_scenario_data(const std::vector<char> &file) {
        SalamanderScenario scenario;

        std::size_t offset = 0;

        auto seek_to_next = [&offset, file] () {
            const char *file_chars = file.data();
            while(offset < file.size() && (file_chars[offset] == ' ' || file_chars[offset] == '\n' || file_chars[offset] == '\r' || file_chars[offset] == '\t')) {
                offset++;
            }
            if(offset < file.size() && file_chars[offset] == ';') {
                while(offset < file.size() && (file_chars[offset] != '\n')) {
                    offset++;
                }
            }
        };

        auto read_int = [&offset, file, &seek_to_next] () -> int {
            seek_to_next();
            const char *file_chars = file.data();
            if(offset == file.size()) {
                throw std::exception();
            }

            std::size_t end;
            int i = std::stoi(file_chars + offset, &end, 10);
            offset += end;

            return i;
        };

        auto read_float = [&offset, file, &seek_to_next] () -> float {
            seek_to_next();
            const char *file_chars = file.data();
            if(offset == file.size()) {
                throw std::exception();
            }

            std::size_t end;
            float i = std::stof(file_chars + offset, &end);
            offset += end;

            return i;
        };

        auto read_string = [&offset, file, &seek_to_next] () -> std::string {
            seek_to_next();
            const char *file_chars = file.data();
            std::size_t file_size = file.size();

            if(offset + 1 >= file_size) {
                throw std::exception();
            }
            else if(file[offset] != '\"') {
                throw std::exception();
            }

            std::vector<char> string_to_add;

            offset++;

            while(file_chars[offset] != '\"') {
                if(file_chars[offset] == '\\') {
                    offset++;
                    if(offset >= file_size) {
                        throw std::exception();
                    }
                }
                string_to_add.push_back(file_chars[offset]);

                if(++offset >= file_size) {
                    throw std::exception();
                }
            }
            offset++;
            string_to_add.push_back(0);

            return string_to_add.data();
        };

        auto read_string_array = [&read_string, &read_int] () -> std::vector<std::string> {
            int count = read_int();
            std::vector<std::string> return_value;
            for(int i = 0; i < count; i++) {
                return_value.emplace_back(read_string());
            }
            return return_value;
        };

        #define READ_STRUCT_BASE(what) { \
            what.type = read_int(); \
            what.name = read_int(); \
            HEK::ScenarioSpawnNotPlaced not_placed = {}; \
            not_placed.automatically = read_int(); \
            not_placed.on_easy = read_int(); \
            not_placed.on_normal = read_int(); \
            not_placed.on_hard = read_int(); \
            what.not_placed = not_placed; \
            what.desired_permutation = read_int(); \
            HEK::Point3D<HEK::BigEndian> position; \
            position.x = read_float(); \
            position.y = read_float(); \
            position.z = read_float(); \
            HEK::Euler3D<HEK::BigEndian> rotation; \
            rotation.yaw = read_float() * HALO_PI / 180.0; \
            rotation.pitch = read_float() * HALO_PI / 180.0; \
            rotation.roll = read_float() * HALO_PI / 180.0; \
            what.position = position; \
            what.rotation = rotation; \
        }

        // Make sure version matches
        int version = read_int();
        if(version != 1) {
            throw std::exception();
        }

        // object names
        scenario.object_names = read_string_array();

        // scenery
        int scenery_count = read_int();
        for(int i = 0; i < scenery_count; i++) {
            HEK::ScenarioScenery<HEK::BigEndian> ss = {};
            READ_STRUCT_BASE(ss);
            scenario.scenery.push_back(ss);
        }
        scenario.scenery_palette = read_string_array();

        // bipeds
        int biped_count = read_int();
        for(int i = 0; i < biped_count; i++) {
            HEK::ScenarioBiped<HEK::BigEndian> ss = {};
            READ_STRUCT_BASE(ss);
            ss.body_vitality = read_float();
            HEK::ScenarioUnitFlags flags = {};
            flags.dead = read_int();
            ss.flags = flags;
            scenario.bipeds.push_back(ss);
        }
        scenario.biped_palette = read_string_array();

        // vehicles
        int vehicle_count = read_int();
        for(int i = 0; i < vehicle_count; i++) {
            HEK::ScenarioVehicle<HEK::BigEndian> ss = {};
            READ_STRUCT_BASE(ss);
            ss.body_vitality = read_float();
            HEK::ScenarioUnitFlags flags = {};
            flags.dead = read_int();
            ss.flags = flags;
            ss.multiplayer_team_index = read_int();

            HEK::ScenarioVehicleMultiplayerSpawnFlags msflags = {};
            msflags.slayer_default = read_int();
            msflags.ctf_default = read_int();
            msflags.king_default = read_int();
            msflags.oddball_default = read_int();
            msflags.unused = read_int();
            msflags.unused_1 = read_int();
            msflags.unused_2 = read_int();
            msflags.unused_3 = read_int();
            msflags.slayer_allowed = read_int();
            msflags.ctf_allowed = read_int();
            msflags.king_allowed = read_int();
            msflags.oddball_allowed = read_int();
            msflags.unused_4 = read_int();
            msflags.unused_5 = read_int();
            msflags.unused_6 = read_int();
            msflags.unused_7 = read_int();
            ss.multiplayer_spawn_flags = msflags;

            scenario.vehicles.push_back(ss);
        }
        scenario.vehicle_palette = read_string_array();

        // equipment
        int equipment_count = read_int();
        for(int i = 0; i < equipment_count; i++) {
            HEK::ScenarioEquipment<HEK::BigEndian> ss = {};
            READ_STRUCT_BASE(ss);
            HEK::ScenarioItemFlags flags = {};
            flags.initially_at_rest = read_int();
            flags.obsolete = read_int();
            flags.does_accelerate = read_int();
            ss.flags = flags;
            scenario.equipment.push_back(ss);
        }
        scenario.equipment_palette = read_string_array();

        // weapons
        int weapon_count = read_int();
        for(int i = 0; i < weapon_count; i++) {
            HEK::ScenarioWeapon<HEK::BigEndian> ss = {};
            READ_STRUCT_BASE(ss);
            ss.rounds_left = read_int();
            ss.rounds_loaded = read_int();
            HEK::ScenarioItemFlags flags = {};
            flags.initially_at_rest = read_int();
            flags.obsolete = read_int();
            flags.does_accelerate = read_int();
            ss.flags = flags;
            scenario.weapons.push_back(ss);
        }
        scenario.weapon_palette = read_string_array();

        #define READ_DEVICE(what) { \
            READ_STRUCT_BASE(what); \
            what.power_group = read_int(); \
            what.position_group = read_int(); \
            HEK::ScenarioDeviceFlags sdf = {}; \
            sdf.initially_open = read_int(); \
            sdf.initially_off = read_int(); \
            sdf.can_change_only_once = read_int(); \
            sdf.position_reversed = read_int(); \
            sdf.not_usable_from_any_side = read_int(); \
            what.device_flags = sdf; \
        }

        // machines
        int machine_count = read_int();
        for(int i = 0; i < machine_count; i++) {
            HEK::ScenarioMachine<HEK::BigEndian> ss = {};
            READ_DEVICE(ss);
            HEK::ScenarioMachineFlags smf = {};
            smf.does_not_operate_automatically = read_int();
            smf.one_sided = read_int();
            smf.never_appears_locked = read_int();
            smf.opened_by_melee_attack = read_int();
            ss.machine_flags = smf;
            scenario.machines.push_back(ss);
        }
        scenario.machine_palette = read_string_array();

        // controls
        int control_count = read_int();
        for(int i = 0; i < control_count; i++) {
            HEK::ScenarioControl<HEK::BigEndian> ss = {};
            READ_DEVICE(ss);
            HEK::ScenarioControlFlags smf = {};
            smf.usable_from_both_sides = read_int();
            ss.control_flags = smf;
            scenario.controls.push_back(ss);
        }
        scenario.control_palette = read_string_array();

        // light fixtures
        int light_fixtures_count = read_int();
        for(int i = 0; i < light_fixtures_count; i++) {
            HEK::ScenarioLightFixture<HEK::BigEndian> ss = {};
            READ_DEVICE(ss);
            ss.color.red = read_float();
            ss.color.green = read_float();
            ss.color.blue = read_float();
            ss.intensity = read_float();
            ss.falloff_angle = read_float() * HALO_PI / 180.0;
            ss.cutoff_angle = read_float() * HALO_PI / 180.0;
            scenario.light_fixtures.push_back(ss);
        }
        scenario.light_fixture_palette = read_string_array();

        // sound scenery
        int sound_scenery_count = read_int();
        for(int i = 0; i < sound_scenery_count; i++) {
            HEK::ScenarioSoundScenery<HEK::BigEndian> ss = {};
            READ_STRUCT_BASE(ss);
            scenario.sound_scenery.push_back(ss);
        }
        scenario.sound_scenery_palette = read_string_array();

        // player starting locations
        int player_starting_locations_count = read_int();
        for(int i = 0; i < player_starting_locations_count; i++) {
            HEK::ScenarioPlayerStartingLocation<HEK::BigEndian> ss = {};
            ss.position.x = read_float();
            ss.position.y = read_float();
            ss.position.z = read_float();
            ss.facing = read_float() * HALO_PI / 180.0;
            ss.team_index = read_int();
            ss.bsp_index = read_int();
            ss.type_0 = static_cast<HEK::ScenarioSpawnType>(read_int());
            ss.type_1 = static_cast<HEK::ScenarioSpawnType>(read_int());
            ss.type_2 = static_cast<HEK::ScenarioSpawnType>(read_int());
            ss.type_3 = static_cast<HEK::ScenarioSpawnType>(read_int());
            scenario.player_starting_locations.push_back(ss);
        }

        // netgame flags
        int netgame_flags_count = read_int();
        for(int i = 0; i < netgame_flags_count; i++) {
            HEK::ScenarioNetgameFlags<HEK::BigEndian> ss = {};
            ss.position.x = read_float();
            ss.position.y = read_float();
            ss.position.z = read_float();
            ss.facing = read_float() * HALO_PI / 180.0;
            ss.team_index = read_int();
            scenario.netgame_flags.push_back(ss);
        }

        // netgame equipment
        int netgame_equipment_count = read_int();
        for(int i = 0; i < netgame_equipment_count; i++) {
            HEK::ScenarioNetgameEquipment<HEK::BigEndian> ss = {};
            HEK::ScenarioNetgameEquipmentFlags flags = {};
            flags.levitate = read_int();
            ss.flags = flags;
            ss.type_0 = static_cast<HEK::ScenarioSpawnType>(read_int());
            ss.type_1 = static_cast<HEK::ScenarioSpawnType>(read_int());
            ss.type_2 = static_cast<HEK::ScenarioSpawnType>(read_int());
            ss.type_3 = static_cast<HEK::ScenarioSpawnType>(read_int());
            ss.team_index = read_int();
            ss.spawn_time = read_float();
            ss.position.x = read_float();
            ss.position.y = read_float();
            ss.position.z = read_float();
            ss.facing = read_float() * HALO_PI / 180.0;
            scenario.netgame_equipment_items.push_back(read_string());
            scenario.netgame_equipment.push_back(ss);
        }

        return scenario;
    }

    std::vector<std::byte> scenario_to_tag(const std::vector<std::byte> &scenario_tag, const SalamanderScenario &scenario) {
        using namespace Invader::HEK;

        std::vector<std::byte> return_value;

        std::size_t offset = 0;

        // Make sure the offset is correct
        #define INCREMENT_OFFSET(amt) { offset += amt; }

        // Make sure the size is valid
        #define ASSERT_SIZE(amt) { if(offset + amt > scenario_tag.size()) { throw std::exception(); } }

        // Increment offset by the amt plus 1 if amt is nonzero
        #define INCREMENT_BY_STRLEN(amt) { if(amt != 0) { INCREMENT_OFFSET(amt + 1) } }

        // Append and add an object
        #define APPEND_ANYTHING(what) { return_value.insert(return_value.end(), reinterpret_cast<const std::byte *>(&what), reinterpret_cast<const std::byte *>(&what + 1)); }

        // Append and add a vector
        #define APPEND_EVERYTHING_EVER_MADE_EVER(what) { return_value.insert(return_value.end(), reinterpret_cast<const std::byte *>(what.data()), reinterpret_cast<const std::byte *>(what.data() + what.size())); }

        // Append and add a pallete
        #define APPEND_A_BLACK_HOLE(what, type) { \
            for(std::size_t i = 0; i < scenario_header.what.count; i++) {\
                const auto *thing = reinterpret_cast<const decltype(scenario_header.what)::struct_type_big *>(scenario_tag_data + offset);\
                ASSERT_SIZE(sizeof(*thing));\
                INCREMENT_OFFSET(sizeof(*thing));\
                INCREMENT_BY_STRLEN(thing->name.path_size)\
            }\
            ASSERT_SIZE(0);\
            for(auto &dependency : scenario.what) { \
                decltype(scenario_header.what)::struct_type_big the_object = {}; \
                the_object.name.path_size = dependency.length(); \
                the_object.name.tag_class_int = type; \
                APPEND_ANYTHING(the_object); \
                if(the_object.name.path_size != 0) { \
                    return_value.insert(return_value.end(), reinterpret_cast<const std::byte *>(dependency.data()), reinterpret_cast<const std::byte *>(dependency.data() + dependency.length())); \
                    return_value.insert(return_value.end(), std::byte()); \
                } \
            } \
            scenario_header.what.count = scenario.what.size();\
        }

        const auto *scenario_tag_data = scenario_tag.data();

        INCREMENT_OFFSET(sizeof(HEK::TagFileHeader));
        ASSERT_SIZE(sizeof(Scenario<BigEndian>));
        auto scenario_header = *reinterpret_cast<const Scenario<BigEndian> *>(scenario_tag_data + offset);
        INCREMENT_OFFSET(sizeof(Scenario<BigEndian>));

        {
            INCREMENT_BY_STRLEN(scenario_header.don_t_use.path_size);
            INCREMENT_BY_STRLEN(scenario_header.won_t_use.path_size);
            INCREMENT_BY_STRLEN(scenario_header.can_t_use.path_size);

            for(std::size_t i = 0; i < scenario_header.skies.count; i++) {
                const auto *sky = reinterpret_cast<const ScenarioSky<BigEndian> *>(scenario_tag_data + offset);
                ASSERT_SIZE(sizeof(*sky));
                INCREMENT_OFFSET(sizeof(*sky));
                INCREMENT_BY_STRLEN(sky->sky.path_size);
            }

            for(std::size_t i = 0; i < scenario_header.child_scenarios.count; i++) {
                const auto *child_scenario = reinterpret_cast<const ScenarioChildScenario<BigEndian> *>(scenario_tag_data + offset);
                ASSERT_SIZE(sizeof(*child_scenario));
                INCREMENT_OFFSET(sizeof(*child_scenario));
                INCREMENT_BY_STRLEN(child_scenario->child_scenario.path_size);
            }

            INCREMENT_OFFSET(scenario_header.functions.count * sizeof(ScenarioFunction<BigEndian>));
            INCREMENT_OFFSET(scenario_header.editor_scenario_data.size);

            for(std::size_t i = 0; i < scenario_header.comments.count; i++) {
                const auto *comment = reinterpret_cast<const ScenarioEditorComment<BigEndian> *>(scenario_tag_data + offset);
                ASSERT_SIZE(sizeof(*comment));
                INCREMENT_OFFSET(sizeof(*comment));
                ASSERT_SIZE(comment->comment.size);
                INCREMENT_OFFSET(comment->comment.size);
            }

            return_value.insert(return_value.end(), scenario_tag_data, scenario_tag_data + offset);
        }

        // Object names
        INCREMENT_OFFSET(scenario_header.object_names.count * sizeof(ScenarioObjectName<BigEndian>));
        scenario_header.object_names.count = scenario.object_names.size();
        for(auto &name : scenario.object_names) {
            ScenarioObjectName<BigEndian> copy_name = {};
            copy_name.name = name;
            APPEND_ANYTHING(copy_name);
        }

        // Scenery
        INCREMENT_OFFSET(scenario_header.scenery.count * sizeof(ScenarioScenery<BigEndian>));
        scenario_header.scenery.count = scenario.scenery.size();

        APPEND_EVERYTHING_EVER_MADE_EVER(scenario.scenery);
        APPEND_A_BLACK_HOLE(scenery_palette, TagClassInt::TAG_CLASS_SCENERY);

        // Bipeds
        INCREMENT_OFFSET(scenario_header.bipeds.count * sizeof(ScenarioBiped<BigEndian>));
        scenario_header.bipeds.count = scenario.bipeds.size();
        APPEND_EVERYTHING_EVER_MADE_EVER(scenario.bipeds);
        APPEND_A_BLACK_HOLE(biped_palette, TagClassInt::TAG_CLASS_BIPED);

        // Vehicles
        INCREMENT_OFFSET(scenario_header.vehicles.count * sizeof(ScenarioVehicle<BigEndian>));
        scenario_header.vehicles.count = scenario.vehicles.size();
        APPEND_EVERYTHING_EVER_MADE_EVER(scenario.vehicles);
        APPEND_A_BLACK_HOLE(vehicle_palette, TagClassInt::TAG_CLASS_VEHICLE);

        // Equipment
        INCREMENT_OFFSET(scenario_header.equipment.count * sizeof(ScenarioEquipment<BigEndian>));
        scenario_header.equipment.count = scenario.equipment.size();
        APPEND_EVERYTHING_EVER_MADE_EVER(scenario.equipment);
        APPEND_A_BLACK_HOLE(equipment_palette, TagClassInt::TAG_CLASS_EQUIPMENT);

        // Weapons
        INCREMENT_OFFSET(scenario_header.weapons.count * sizeof(ScenarioWeapon<BigEndian>));
        scenario_header.weapons.count = scenario.weapons.size();
        APPEND_EVERYTHING_EVER_MADE_EVER(scenario.weapons);
        APPEND_A_BLACK_HOLE(weapon_palette, TagClassInt::TAG_CLASS_WEAPON);

        // Machines
        INCREMENT_OFFSET(scenario_header.machines.count * sizeof(ScenarioMachine<BigEndian>));
        scenario_header.machines.count = scenario.machines.size();
        APPEND_EVERYTHING_EVER_MADE_EVER(scenario.machines);
        APPEND_A_BLACK_HOLE(machine_palette, TagClassInt::TAG_CLASS_DEVICE_MACHINE);

        // Controls
        INCREMENT_OFFSET(scenario_header.controls.count * sizeof(ScenarioControl<BigEndian>));
        scenario_header.controls.count = scenario.controls.size();
        APPEND_EVERYTHING_EVER_MADE_EVER(scenario.controls);
        APPEND_A_BLACK_HOLE(control_palette, TagClassInt::TAG_CLASS_DEVICE_CONTROL);

        // Light Fixture
        INCREMENT_OFFSET(scenario_header.light_fixtures.count * sizeof(ScenarioLightFixture<BigEndian>));
        scenario_header.light_fixtures.count = scenario.light_fixtures.size();
        APPEND_EVERYTHING_EVER_MADE_EVER(scenario.light_fixtures);
        APPEND_A_BLACK_HOLE(light_fixture_palette, TagClassInt::TAG_CLASS_DEVICE_LIGHT_FIXTURE);

        // Sound Scenery
        INCREMENT_OFFSET(scenario_header.sound_scenery.count * sizeof(ScenarioSoundScenery<BigEndian>));
        scenario_header.sound_scenery.count = scenario.sound_scenery.size();
        APPEND_EVERYTHING_EVER_MADE_EVER(scenario.sound_scenery);
        APPEND_A_BLACK_HOLE(sound_scenery_palette, TagClassInt::TAG_CLASS_SOUND_SCENERY);

        {
            std::size_t before = offset;
            for(std::size_t i = 0; i < scenario_header.player_starting_profile.count; i++) {
                const auto *starting_profile = reinterpret_cast<const ScenarioPlayerStartingProfile<BigEndian> *>(scenario_tag_data + offset);
                ASSERT_SIZE(sizeof(*starting_profile));
                INCREMENT_OFFSET(sizeof(*starting_profile));
                INCREMENT_BY_STRLEN(starting_profile->primary_weapon.path_size);
                INCREMENT_BY_STRLEN(starting_profile->secondary_weapon.path_size);
            }
            ASSERT_SIZE(0);
            if(offset > before) {
                return_value.insert(return_value.end(), scenario_tag_data + before, scenario_tag_data + offset);
            }
        }

        // Player starting location
        INCREMENT_OFFSET(scenario_header.player_starting_locations.count * sizeof(ScenarioPlayerStartingLocation<BigEndian>));
        scenario_header.player_starting_locations.count = scenario.player_starting_locations.size();
        APPEND_EVERYTHING_EVER_MADE_EVER(scenario.player_starting_locations);

        {
            std::size_t before = offset;
            INCREMENT_OFFSET(scenario_header.trigger_volumes.count * sizeof(ScenarioTriggerVolume<BigEndian>));

            for(std::size_t i = 0; i < scenario_header.comments.count; i++) {
                const auto *animation = reinterpret_cast<const ScenarioRecordedAnimation<BigEndian> *>(scenario_tag_data + offset);
                ASSERT_SIZE(sizeof(*animation));
                INCREMENT_OFFSET(sizeof(*animation));
                ASSERT_SIZE(animation->recorded_animation_event_stream.size);
                INCREMENT_OFFSET(animation->recorded_animation_event_stream.size);
            }

            INCREMENT_OFFSET(scenario_header.trigger_volumes.count * sizeof(ScenarioTriggerVolume<BigEndian>));
            ASSERT_SIZE(0);
            return_value.insert(return_value.end(), scenario_tag_data + before, scenario_tag_data + offset);
        }

        // Netgame flags
        for(std::size_t i = 0; i < scenario_header.netgame_flags.count; i++) {
            const auto *netgame_flags = reinterpret_cast<const ScenarioNetgameFlags<BigEndian> *>(scenario_tag_data + offset);
            ASSERT_SIZE(sizeof(*netgame_flags));
            INCREMENT_OFFSET(sizeof(*netgame_flags));
            INCREMENT_BY_STRLEN(netgame_flags->weapon_group.path_size);
        }
        scenario_header.netgame_flags.count = scenario.netgame_flags.size();
        APPEND_EVERYTHING_EVER_MADE_EVER(scenario.netgame_flags);

        // Netgame equipment
        for(std::size_t i = 0; i < scenario_header.netgame_equipment.count; i++) {
            const auto *netgame_equipment = reinterpret_cast<const ScenarioNetgameEquipment<BigEndian> *>(scenario_tag_data + offset);
            ASSERT_SIZE(sizeof(*netgame_equipment));
            INCREMENT_OFFSET(sizeof(*netgame_equipment));
            INCREMENT_BY_STRLEN(netgame_equipment->item_collection.path_size);
        }
        scenario_header.netgame_equipment.count = scenario.netgame_equipment.size();
        for(std::size_t i = 0; i < scenario_header.netgame_equipment.count; i++) {
            auto equipment = scenario.netgame_equipment[i];
            const auto &item = scenario.netgame_equipment_items[i];
            equipment.item_collection.path_size = item.size();
            APPEND_ANYTHING(equipment);
            if(equipment.item_collection.path_size != 0) { \
                return_value.insert(return_value.end(), reinterpret_cast<const std::byte *>(item.data()), reinterpret_cast<const std::byte *>(item.data() + item.size()));
                return_value.insert(return_value.end(), std::byte());
            }
        }

        *reinterpret_cast<Scenario<BigEndian> *>(return_value.data() + sizeof(TagFileHeader)) = scenario_header;

        // The rest
        ASSERT_SIZE(0);
        return_value.insert(return_value.end(), scenario_tag_data + offset, scenario_tag_data + scenario_tag.size());

        return return_value;
    }
}
