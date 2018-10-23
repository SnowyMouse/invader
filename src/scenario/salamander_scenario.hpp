/*
 * Invader (c) 2018 Kavawuvi
 *
 * This program is free software under the GNU General Public License v3.0 or later. See LICENSE for more information.
 */

#pragma once

#include <vector>

#include "../tag/hek/class/scenario.hpp"

namespace Invader {
    struct SalamanderScenario {
        std::vector<std::string> object_names;

        std::vector<HEK::ScenarioScenery<HEK::BigEndian>> scenery;
        std::vector<std::string> scenery_palette;

        std::vector<HEK::ScenarioBiped<HEK::BigEndian>> bipeds;
        std::vector<std::string> biped_palette;

        std::vector<HEK::ScenarioVehicle<HEK::BigEndian>> vehicles;
        std::vector<std::string> vehicle_palette;

        std::vector<HEK::ScenarioEquipment<HEK::BigEndian>> equipment;
        std::vector<std::string> equipment_palette;

        std::vector<HEK::ScenarioWeapon<HEK::BigEndian>> weapons;
        std::vector<std::string> weapon_palette;

        std::vector<HEK::ScenarioMachine<HEK::BigEndian>> machines;
        std::vector<std::string> machine_palette;

        std::vector<HEK::ScenarioControl<HEK::BigEndian>> controls;
        std::vector<std::string> control_palette;

        std::vector<HEK::ScenarioLightFixture<HEK::BigEndian>> light_fixtures;
        std::vector<std::string> light_fixture_palette;

        std::vector<HEK::ScenarioSoundScenery<HEK::BigEndian>> sound_scenery;
        std::vector<std::string> sound_scenery_palette;

        std::vector<HEK::ScenarioPlayerStartingLocation<HEK::BigEndian>> player_starting_locations;
        std::vector<HEK::ScenarioNetgameFlags<HEK::BigEndian>> netgame_flags;
        std::vector<HEK::ScenarioNetgameEquipment<HEK::BigEndian>> netgame_equipment;
        std::vector<std::string> netgame_equipment_items;
    };

    /**
     * Parse the scenario data
     * @param  data data for the scenario data
     * @return      the parsed data
     * @throws      std::exception if data is invalid
     */
    SalamanderScenario parse_scenario_data(const std::vector<char> &data);

    /**
     * Write the scenario data to the tag
     * @param scenario_tag scenario tag data from Halo Editing Kit
     * @param scenario     scenario data from parse_scenario_data
     */
    std::vector<std::byte> scenario_to_tag(const std::vector<std::byte> &scenario_tag, const SalamanderScenario &scenario);
}
