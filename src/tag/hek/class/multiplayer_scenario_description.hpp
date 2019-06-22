/*
 * Invader (c) 2018 Kavawuvi
 *
 * This program is free software under the GNU General Public License v3.0 or later. See LICENSE for more information.
 */

#ifndef INVADER__TAG__HEK__CLASS__MULTIPLAYER_SCENARIO_DESCRIPTION_HPP
#define INVADER__TAG__HEK__CLASS__MULTIPLAYER_SCENARIO_DESCRIPTION_HPP

#include "../../compiled_tag.hpp"
#include "../../../hek/data_type.hpp"
#include "../header.hpp"

namespace Invader::HEK {
    ENDIAN_TEMPLATE(EndianType) struct MultiplayerScenarioDescriptionScenarioDescription {
        TagDependency<EndianType> descriptive_bitmap; // bitmap
        TagDependency<EndianType> displayed_map_name; // unicode_string_list
        TagString scenario_tag_directory_path;
        PAD(0x4);

        ENDIAN_TEMPLATE(NewType) operator MultiplayerScenarioDescriptionScenarioDescription<NewType>() const noexcept {
            MultiplayerScenarioDescriptionScenarioDescription<NewType> copy = {};
            COPY_THIS(descriptive_bitmap);
            COPY_THIS(displayed_map_name);
            COPY_THIS(scenario_tag_directory_path);
            return copy;
        }
    };
    static_assert(sizeof(MultiplayerScenarioDescriptionScenarioDescription<NativeEndian>) == 0x44);


    ENDIAN_TEMPLATE(EndianType) struct MultiplayerScenarioDescription {
        TagReflexive<EndianType, MultiplayerScenarioDescriptionScenarioDescription> multiplayer_scenarios;

        ENDIAN_TEMPLATE(NewType) operator MultiplayerScenarioDescription<NewType>() const noexcept {
            MultiplayerScenarioDescription<NewType> copy;
            COPY_THIS(multiplayer_scenarios);
            return copy;
        }
    };
    static_assert(sizeof(MultiplayerScenarioDescription<BigEndian>) == 0xC);

    void compile_multiplayer_scenario_description_tag(CompiledTag &compiled, const std::byte *data, std::size_t size);
}
#endif
