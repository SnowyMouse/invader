/*
 * Invader (c) 2018 Kavawuvi
 *
 * This program is free software under the GNU General Public License v3.0 or later. See LICENSE for more information.
 */

#ifndef INVADER__TAG__HEK__CLASS__SCENARIO_HPP
#define INVADER__TAG__HEK__CLASS__SCENARIO_HPP

#include "../../../hek/map.hpp"
#include "../../../hek/data_type.hpp"
#include "../../compiled_tag.hpp"
#include "../header.hpp"
#include "object.hpp"

namespace Invader::HEK {
    enum ScenarioSpawnType : TagEnum {
        SCENARIO_SPAWN_TYPE_NONE,
        SCENARIO_SPAWN_TYPE_CTF,
        SCENARIO_SPAWN_TYPE_SLAYER,
        SCENARIO_SPAWN_TYPE_ODDBALL,
        SCENARIO_SPAWN_TYPE_KING_OF_THE_HILL,
        SCENARIO_SPAWN_TYPE_RACE,
        SCENARIO_SPAWN_TYPE_TERMINATOR,
        SCENARIO_SPAWN_TYPE_STUB,
        SCENARIO_SPAWN_TYPE_IGNORED1,
        SCENARIO_SPAWN_TYPE_IGNORED2,
        SCENARIO_SPAWN_TYPE_IGNORED3,
        SCENARIO_SPAWN_TYPE_IGNORED4,
        SCENARIO_SPAWN_TYPE_ALL_GAMES,
        SCENARIO_SPAWN_TYPE_ALL_EXCEPT_CTF,
        SCENARIO_SPAWN_TYPE_ALL_EXCEPT_RACE__CTF
    };

    enum ScenarioNetgameFlagType : TagEnum {
        SCENARIO_NETGAME_FLAG_TYPE_CTF__FLAG,
        SCENARIO_NETGAME_FLAG_TYPE_CTF__VEHICLE,
        SCENARIO_NETGAME_FLAG_TYPE_ODDBALL__BALL_SPAWN,
        SCENARIO_NETGAME_FLAG_TYPE_RACE__TRACK,
        SCENARIO_NETGAME_FLAG_TYPE_RACE__VEHICLE,
        SCENARIO_NETGAME_FLAG_TYPE_VEGAS__BANK,
        SCENARIO_NETGAME_FLAG_TYPE_TELEPORT_FROM,
        SCENARIO_NETGAME_FLAG_TYPE_TELEPORT_TO,
        SCENARIO_NETGAME_FLAG_TYPE_HILL__FLAG
    };

    enum ScenarioReturnState : TagEnum {
        SCENARIO_RETURN_STATE_NONE,
        SCENARIO_RETURN_STATE_SLEEPING,
        SCENARIO_RETURN_STATE_ALERT,
        SCENARIO_RETURN_STATE_MOVING__REPEAT_SAME_POSITION,
        SCENARIO_RETURN_STATE_MOVING__LOOP,
        SCENARIO_RETURN_STATE_MOVING__LOOP_BACK_AND_FORTH,
        SCENARIO_RETURN_STATE_MOVING__LOOP_RANDOMLY,
        SCENARIO_RETURN_STATE_MOVING__RANDOMLY,
        SCENARIO_RETURN_STATE_GUARDING,
        SCENARIO_RETURN_STATE_GUARDING_AT_GUARD_POSITION,
        SCENARIO_RETURN_STATE_SEARCHING,
        SCENARIO_RETURN_STATE_FLEEING
    };

    enum ScenarioUniqueLeaderType : TagEnum {
        SCENARIO_UNIQUE_LEADER_TYPE_NORMAL,
        SCENARIO_UNIQUE_LEADER_TYPE_NONE,
        SCENARIO_UNIQUE_LEADER_TYPE_RANDOM,
        SCENARIO_UNIQUE_LEADER_TYPE_SGT_JOHNSON,
        SCENARIO_UNIQUE_LEADER_TYPE_SGT_LEHTO
    };

    enum ScenarioMajorUpgrade : TagEnum {
        SCENARIO_MAJOR_UPGRADE_NORMAL,
        SCENARIO_MAJOR_UPGRADE_FEW,
        SCENARIO_MAJOR_UPGRADE_MANY,
        SCENARIO_MAJOR_UPGRADE_NONE,
        SCENARIO_MAJOR_UPGRADE_ALL
    };

    enum ScenarioChangeAttackingDefendingStateWhen : TagEnum {
        SCENARIO_CHANGE_ATTACKING_DEFENDING_STATE_WHEN_NEVER,
        SCENARIO_CHANGE_ATTACKING_DEFENDING_STATE_WHEN__75_STRENGTH,
        SCENARIO_CHANGE_ATTACKING_DEFENDING_STATE_WHEN__50_STRENGTH,
        SCENARIO_CHANGE_ATTACKING_DEFENDING_STATE_WHEN__25_STRENGTH,
        SCENARIO_CHANGE_ATTACKING_DEFENDING_STATE_WHEN_ANYBODY_DEAD,
        SCENARIO_CHANGE_ATTACKING_DEFENDING_STATE_WHEN__25_DEAD,
        SCENARIO_CHANGE_ATTACKING_DEFENDING_STATE_WHEN__50_DEAD,
        SCENARIO_CHANGE_ATTACKING_DEFENDING_STATE_WHEN__75_DEAD,
        SCENARIO_CHANGE_ATTACKING_DEFENDING_STATE_WHEN_ALL_BUT_ONE_DEAD,
        SCENARIO_CHANGE_ATTACKING_DEFENDING_STATE_WHEN_ALL_DEAD
    };

    enum ScenarioGroupIndex : TagEnum {
        SCENARIO_GROUP_INDEX_A,
        SCENARIO_GROUP_INDEX_B,
        SCENARIO_GROUP_INDEX_C,
        SCENARIO_GROUP_INDEX_D,
        SCENARIO_GROUP_INDEX_E,
        SCENARIO_GROUP_INDEX_F,
        SCENARIO_GROUP_INDEX_G,
        SCENARIO_GROUP_INDEX_H,
        SCENARIO_GROUP_INDEX_I,
        SCENARIO_GROUP_INDEX_J,
        SCENARIO_GROUP_INDEX_K,
        SCENARIO_GROUP_INDEX_L,
        SCENARIO_GROUP_INDEX_M,
        SCENARIO_GROUP_INDEX_N,
        SCENARIO_GROUP_INDEX_O,
        SCENARIO_GROUP_INDEX_P,
        SCENARIO_GROUP_INDEX_Q,
        SCENARIO_GROUP_INDEX_R,
        SCENARIO_GROUP_INDEX_S,
        SCENARIO_GROUP_INDEX_T,
        SCENARIO_GROUP_INDEX_U,
        SCENARIO_GROUP_INDEX_V,
        SCENARIO_GROUP_INDEX_W,
        SCENARIO_GROUP_INDEX_X,
        SCENARIO_GROUP_INDEX_Y,
        SCENARIO_GROUP_INDEX_Z
    };

    enum ScenarioTeamIndex : TagEnum {
        SCENARIO_TEAM_INDEX__0__DEFAULT_BY_UNIT,
        SCENARIO_TEAM_INDEX__1__PLAYER,
        SCENARIO_TEAM_INDEX__2__HUMAN,
        SCENARIO_TEAM_INDEX__3__COVENANT,
        SCENARIO_TEAM_INDEX__4__FLOOD,
        SCENARIO_TEAM_INDEX__5__SENTINEL,
        SCENARIO_TEAM_INDEX__6__UNUSED6,
        SCENARIO_TEAM_INDEX__7__UNUSED7,
        SCENARIO_TEAM_INDEX__8__UNUSED8,
        SCENARIO_TEAM_INDEX__9__UNUSED9
    };

    enum ScenarioSearchBehavior : TagEnum {
        SCENARIO_SEARCH_BEHAVIOR_NORMAL,
        SCENARIO_SEARCH_BEHAVIOR_NEVER,
        SCENARIO_SEARCH_BEHAVIOR_TENACIOUS
    };

    enum ScenarioAtomType : TagEnum {
        SCENARIO_ATOM_TYPE_PAUSE,
        SCENARIO_ATOM_TYPE_GO_TO,
        SCENARIO_ATOM_TYPE_GO_TO_AND_FACE,
        SCENARIO_ATOM_TYPE_MOVE_IN_DIRECTION,
        SCENARIO_ATOM_TYPE_LOOK,
        SCENARIO_ATOM_TYPE_ANIMATION_MODE,
        SCENARIO_ATOM_TYPE_CROUCH,
        SCENARIO_ATOM_TYPE_SHOOT,
        SCENARIO_ATOM_TYPE_GRENADE,
        SCENARIO_ATOM_TYPE_VEHICLE,
        SCENARIO_ATOM_TYPE_RUNNING_JUMP,
        SCENARIO_ATOM_TYPE_TARGETED_JUMP,
        SCENARIO_ATOM_TYPE_SCRIPT,
        SCENARIO_ATOM_TYPE_ANIMATE,
        SCENARIO_ATOM_TYPE_RECORDING,
        SCENARIO_ATOM_TYPE_ACTION,
        SCENARIO_ATOM_TYPE_VOCALIZE,
        SCENARIO_ATOM_TYPE_TARGETING,
        SCENARIO_ATOM_TYPE_INITIATIVE,
        SCENARIO_ATOM_TYPE_WAIT,
        SCENARIO_ATOM_TYPE_LOOP,
        SCENARIO_ATOM_TYPE_DIE,
        SCENARIO_ATOM_TYPE_MOVE_IMMEDIATE,
        SCENARIO_ATOM_TYPE_LOOK_RANDOM,
        SCENARIO_ATOM_TYPE_LOOK_PLAYER,
        SCENARIO_ATOM_TYPE_LOOK_OBJECT,
        SCENARIO_ATOM_TYPE_SET_RADIUS,
        SCENARIO_ATOM_TYPE_TELEPORT
    };

    enum ScenarioSelectionType : TagEnum {
        SCENARIO_SELECTION_TYPE_FRIENDLY_ACTOR,
        SCENARIO_SELECTION_TYPE_DISEMBODIED,
        SCENARIO_SELECTION_TYPE_IN_PLAYER_S_VEHICLE,
        SCENARIO_SELECTION_TYPE_NOT_IN_A_VEHICLE,
        SCENARIO_SELECTION_TYPE_PREFER_SERGEANT,
        SCENARIO_SELECTION_TYPE_ANY_ACTOR,
        SCENARIO_SELECTION_TYPE_RADIO_UNIT,
        SCENARIO_SELECTION_TYPE_RADIO_SERGEANT
    };

    enum ScenarioActorType : TagEnum {
        SCENARIO_ACTOR_TYPE_ELITE,
        SCENARIO_ACTOR_TYPE_JACKAL,
        SCENARIO_ACTOR_TYPE_GRUNT,
        SCENARIO_ACTOR_TYPE_HUNTER,
        SCENARIO_ACTOR_TYPE_ENGINEER,
        SCENARIO_ACTOR_TYPE_ASSASSIN,
        SCENARIO_ACTOR_TYPE_PLAYER,
        SCENARIO_ACTOR_TYPE_MARINE,
        SCENARIO_ACTOR_TYPE_CREW,
        SCENARIO_ACTOR_TYPE_COMBAT_FORM,
        SCENARIO_ACTOR_TYPE_INFECTION_FORM,
        SCENARIO_ACTOR_TYPE_CARRIER_FORM,
        SCENARIO_ACTOR_TYPE_MONITOR,
        SCENARIO_ACTOR_TYPE_SENTINEL,
        SCENARIO_ACTOR_TYPE_NONE,
        SCENARIO_ACTOR_TYPE_MOUNTED_WEAPON
    };

    enum ScenarioAddressee : TagEnum {
        SCENARIO_ADDRESSEE_NONE,
        SCENARIO_ADDRESSEE_PLAYER,
        SCENARIO_ADDRESSEE_PARTICIPANT
    };

    enum ScenarioScriptType : TagEnum {
        SCENARIO_SCRIPT_TYPE_STARTUP,
        SCENARIO_SCRIPT_TYPE_DORMANT,
        SCENARIO_SCRIPT_TYPE_CONTINUOUS,
        SCENARIO_SCRIPT_TYPE_STATIC,
        SCENARIO_SCRIPT_TYPE_STUB
    };

    enum ScenarioScriptValueType : TagEnum {
        SCENARIO_SCRIPT_VALUE_TYPE_UNPARSED,
        SCENARIO_SCRIPT_VALUE_TYPE_SPECIAL_FORM,
        SCENARIO_SCRIPT_VALUE_TYPE_FUNCTION_NAME,
        SCENARIO_SCRIPT_VALUE_TYPE_PASSTHROUGH,
        SCENARIO_SCRIPT_VALUE_TYPE_VOID,
        SCENARIO_SCRIPT_VALUE_TYPE_BOOLEAN,
        SCENARIO_SCRIPT_VALUE_TYPE_REAL,
        SCENARIO_SCRIPT_VALUE_TYPE_SHORT,
        SCENARIO_SCRIPT_VALUE_TYPE_LONG,
        SCENARIO_SCRIPT_VALUE_TYPE_STRING,
        SCENARIO_SCRIPT_VALUE_TYPE_SCRIPT,
        SCENARIO_SCRIPT_VALUE_TYPE_TRIGGER_VOLUME,
        SCENARIO_SCRIPT_VALUE_TYPE_CUTSCENE_FLAG,
        SCENARIO_SCRIPT_VALUE_TYPE_CUTSCENE_CAMERA_POINT,
        SCENARIO_SCRIPT_VALUE_TYPE_CUTSCENE_TITLE,
        SCENARIO_SCRIPT_VALUE_TYPE_CUTSCENE_RECORDING,
        SCENARIO_SCRIPT_VALUE_TYPE_DEVICE_GROUP,
        SCENARIO_SCRIPT_VALUE_TYPE_AI,
        SCENARIO_SCRIPT_VALUE_TYPE_AI_COMMAND_LIST,
        SCENARIO_SCRIPT_VALUE_TYPE_STARTING_PROFILE,
        SCENARIO_SCRIPT_VALUE_TYPE_CONVERSATION,
        SCENARIO_SCRIPT_VALUE_TYPE_NAVPOINT,
        SCENARIO_SCRIPT_VALUE_TYPE_HUD_MESSAGE,
        SCENARIO_SCRIPT_VALUE_TYPE_OBJECT_LIST,
        SCENARIO_SCRIPT_VALUE_TYPE_SOUND,
        SCENARIO_SCRIPT_VALUE_TYPE_EFFECT,
        SCENARIO_SCRIPT_VALUE_TYPE_DAMAGE,
        SCENARIO_SCRIPT_VALUE_TYPE_LOOPING_SOUND,
        SCENARIO_SCRIPT_VALUE_TYPE_ANIMATION_GRAPH,
        SCENARIO_SCRIPT_VALUE_TYPE_ACTOR_VARIANT,
        SCENARIO_SCRIPT_VALUE_TYPE_DAMAGE_EFFECT,
        SCENARIO_SCRIPT_VALUE_TYPE_OBJECT_DEFINITION,
        SCENARIO_SCRIPT_VALUE_TYPE_GAME_DIFFICULTY,
        SCENARIO_SCRIPT_VALUE_TYPE_TEAM,
        SCENARIO_SCRIPT_VALUE_TYPE_AI_DEFAULT_STATE,
        SCENARIO_SCRIPT_VALUE_TYPE_ACTOR_TYPE,
        SCENARIO_SCRIPT_VALUE_TYPE_HUD_CORNER,
        SCENARIO_SCRIPT_VALUE_TYPE_OBJECT,
        SCENARIO_SCRIPT_VALUE_TYPE_UNIT,
        SCENARIO_SCRIPT_VALUE_TYPE_VEHICLE,
        SCENARIO_SCRIPT_VALUE_TYPE_WEAPON,
        SCENARIO_SCRIPT_VALUE_TYPE_DEVICE,
        SCENARIO_SCRIPT_VALUE_TYPE_SCENERY,
        SCENARIO_SCRIPT_VALUE_TYPE_OBJECT_NAME,
        SCENARIO_SCRIPT_VALUE_TYPE_UNIT_NAME,
        SCENARIO_SCRIPT_VALUE_TYPE_VEHICLE_NAME,
        SCENARIO_SCRIPT_VALUE_TYPE_WEAPON_NAME,
        SCENARIO_SCRIPT_VALUE_TYPE_DEVICE_NAME,
        SCENARIO_SCRIPT_VALUE_TYPE_SCENERY_NAME
    };

    enum ScenarioTextStyle : TagEnum {
        SCENARIO_TEXT_STYLE_PLAIN,
        SCENARIO_TEXT_STYLE_BOLD,
        SCENARIO_TEXT_STYLE_ITALIC,
        SCENARIO_TEXT_STYLE_CONDENSE,
        SCENARIO_TEXT_STYLE_UNDERLINE
    };

    enum ScenarioJustification : TagEnum {
        SCENARIO_JUSTIFICATION_LEFT,
        SCENARIO_JUSTIFICATION_RIGHT,
        SCENARIO_JUSTIFICATION_CENTER
    };

    SINGLE_DEPENDENCY_STRUCT(ScenarioSky, sky); // sky
    SINGLE_DEPENDENCY_STRUCT(ScenarioChildScenario, child_scenario); // scenario

    struct ScenarioFunctionFlags {
        std::uint32_t scripted : 1;
        std::uint32_t invert : 1;
        std::uint32_t additive : 1;
        std::uint32_t always_active : 1;
    };
    static_assert(sizeof(ScenarioFunctionFlags) == sizeof(std::uint32_t));

    ENDIAN_TEMPLATE(EndianType) struct ScenarioFunction {
        EndianType<ScenarioFunctionFlags> flags;
        TagString name;
        EndianType<float> period;
        EndianType<FunctionScaleBy> scale_period_by;
        EndianType<FunctionType2> function;
        EndianType<FunctionScaleBy> scale_function_by;
        EndianType<FunctionType2> wobble_function;
        EndianType<float> wobble_period;
        EndianType<float> wobble_magnitude;
        EndianType<Fraction> square_wave_threshold;
        EndianType<std::int16_t> step_count;
        EndianType<FunctionType> map_to;
        EndianType<std::int16_t> sawtooth_count;
        PAD(0x2);
        EndianType<FunctionScaleBy> scale_result_by;
        EndianType<FunctionBoundsMode> bounds_mode;
        Bounds<EndianType<float>> bounds;
        PAD(0x4);
        PAD(0x2);
        EndianType<FunctionScaleBy> turn_off_with;
        PAD(0x10);
        PAD(0x10);

        ENDIAN_TEMPLATE(NewType) operator ScenarioFunction<NewType>() const noexcept {
            ScenarioFunction<NewType> copy = {};
            COPY_THIS(flags);
            COPY_THIS(name);
            COPY_THIS(period);
            COPY_THIS(scale_period_by);
            COPY_THIS(function);
            COPY_THIS(scale_function_by);
            COPY_THIS(wobble_function);
            COPY_THIS(wobble_period);
            COPY_THIS(wobble_magnitude);
            COPY_THIS(square_wave_threshold);
            COPY_THIS(step_count);
            COPY_THIS(map_to);
            COPY_THIS(sawtooth_count);
            COPY_THIS(scale_result_by);
            COPY_THIS(bounds_mode);
            COPY_THIS(bounds);
            COPY_THIS(turn_off_with);
            return copy;
        }
    };
    static_assert(sizeof(ScenarioFunction<BigEndian>) == 0x78);

    ENDIAN_TEMPLATE(EndianType) struct ScenarioEditorComment {
        Point3D<EndianType> position;
        PAD(0x10);
        TagDataOffset<EndianType> comment;

        ENDIAN_TEMPLATE(NewType) operator ScenarioEditorComment<NewType>() const noexcept {
            ScenarioEditorComment<NewType> copy = {};
            COPY_THIS(position);
            COPY_THIS(comment);
            return copy;
        }
    };
    static_assert(sizeof(ScenarioEditorComment<BigEndian>) == 0x30);

    ENDIAN_TEMPLATE(EndianType) struct ScenarioObjectName {
        TagString name;
        LittleEndian<ObjectType> object_type;
        LittleEndian<std::uint16_t> object_index;

        ENDIAN_TEMPLATE(NewType) operator ScenarioObjectName<NewType>() const noexcept {
            ScenarioObjectName<NewType> copy = {};
            COPY_THIS(name);
            COPY_THIS(object_type);
            COPY_THIS(object_index);
            return copy;
        }
    };
    static_assert(sizeof(ScenarioObjectName<BigEndian>) == 0x24);

    struct ScenarioSpawnNotPlaced {
        std::uint16_t automatically : 1;
        std::uint16_t on_easy : 1;
        std::uint16_t on_normal : 1;
        std::uint16_t on_hard : 1;
    };
    static_assert(sizeof(ScenarioSpawnNotPlaced) == sizeof(std::uint16_t));

    ENDIAN_TEMPLATE(EndianType) struct SpawnPrelude {
        EndianType<std::int16_t> type;
        EndianType<std::int16_t> name;
        EndianType<ScenarioSpawnNotPlaced> not_placed;
        EndianType<std::int16_t> desired_permutation;
        Point3D<EndianType> position;
        Euler3D<EndianType> rotation;
    };

    #define COPY_SPAWN_PRELUDE COPY_THIS(type); \
                               COPY_THIS(name); \
                               COPY_THIS(not_placed); \
                               COPY_THIS(desired_permutation); \
                               COPY_THIS(position); \
                               COPY_THIS(rotation);

    ENDIAN_TEMPLATE(EndianType) struct ScenarioScenery : SpawnPrelude<EndianType> {
        PAD(0x8);
        PAD(0x10);
        PAD(0x8);
        PAD(0x8);

        ENDIAN_TEMPLATE(NewType) operator ScenarioScenery<NewType>() const noexcept {
            ScenarioScenery<NewType> copy = {};
            COPY_SPAWN_PRELUDE
            return copy;
        }
    };
    static_assert(sizeof(ScenarioScenery<BigEndian>) == 0x48);

    SINGLE_DEPENDENCY_PADDED_STRUCT(ScenarioSceneryPalette, name, 0x20); // scenery

    struct ScenarioUnitFlags {
        std::uint32_t dead : 1;
    };
    static_assert(sizeof(ScenarioUnitFlags) == sizeof(std::uint32_t));

    ENDIAN_TEMPLATE(EndianType) struct ScenarioBiped : SpawnPrelude<EndianType> {
        PAD(0x8);
        PAD(0x10);
        PAD(0x8);
        PAD(0x8);

        EndianType<float> body_vitality;
        EndianType<ScenarioUnitFlags> flags;
        PAD(0x8);
        PAD(0x20);

        ENDIAN_TEMPLATE(NewType) operator ScenarioBiped<NewType>() const noexcept {
            ScenarioBiped<NewType> copy = {};
            COPY_SPAWN_PRELUDE
            COPY_THIS(body_vitality);
            COPY_THIS(flags);
            return copy;
        }
    };
    static_assert(sizeof(ScenarioBiped<BigEndian>) == 0x78);

    SINGLE_DEPENDENCY_PADDED_STRUCT(ScenarioBipedPalette, name, 0x20); // biped

    struct ScenarioVehicleMultiplayerSpawnFlags {
        std::uint16_t slayer_default : 1;
        std::uint16_t ctf_default : 1;
        std::uint16_t king_default : 1;
        std::uint16_t oddball_default : 1;
        std::uint16_t unused : 1;
        std::uint16_t unused_1 : 1;
        std::uint16_t unused_2 : 1;
        std::uint16_t unused_3 : 1;
        std::uint16_t slayer_allowed : 1;
        std::uint16_t ctf_allowed : 1;
        std::uint16_t king_allowed : 1;
        std::uint16_t oddball_allowed : 1;
        std::uint16_t unused_4 : 1;
        std::uint16_t unused_5 : 1;
        std::uint16_t unused_6 : 1;
        std::uint16_t unused_7 : 1;
    };
    static_assert(sizeof(ScenarioVehicleMultiplayerSpawnFlags) == sizeof(std::uint16_t));

    ENDIAN_TEMPLATE(EndianType) struct ScenarioVehicle : SpawnPrelude<EndianType> {
        PAD(0x8);
        PAD(0x10);
        PAD(0x8);
        PAD(0x8);

        EndianType<float> body_vitality;
        EndianType<ScenarioUnitFlags> flags;
        PAD(0x8);
        std::int8_t multiplayer_team_index;
        PAD(0x1);
        EndianType<ScenarioVehicleMultiplayerSpawnFlags> multiplayer_spawn_flags;
        PAD(0x1C);

        ENDIAN_TEMPLATE(NewType) operator ScenarioVehicle<NewType>() const noexcept {
            ScenarioVehicle<NewType> copy = {};
            COPY_THIS(type);
            COPY_THIS(name);
            COPY_THIS(not_placed);
            COPY_THIS(desired_permutation);
            COPY_THIS(position);
            COPY_THIS(rotation);
            COPY_THIS(body_vitality);
            COPY_THIS(flags);
            COPY_THIS(multiplayer_team_index);
            COPY_THIS(multiplayer_spawn_flags);
            return copy;
        }
    };
    static_assert(sizeof(ScenarioVehicle<BigEndian>) == 0x78);

    SINGLE_DEPENDENCY_PADDED_STRUCT(ScenarioVehiclePalette, name, 0x20); // vehicle

    struct ScenarioItemFlags {
        std::uint16_t initially_at_rest : 1;
        std::uint16_t obsolete : 1;
        std::uint16_t does_accelerate : 1;
    };
    static_assert(sizeof(ScenarioItemFlags) == sizeof(std::uint16_t));

    ENDIAN_TEMPLATE(EndianType) struct ScenarioEquipment : SpawnPrelude<EndianType> {
        PAD(0x2);
        EndianType<ScenarioItemFlags> flags;
        PAD(0x4);

        ENDIAN_TEMPLATE(NewType) operator ScenarioEquipment<NewType>() const noexcept {
            ScenarioEquipment<NewType> copy = {};
            COPY_SPAWN_PRELUDE
            COPY_THIS(flags);
            return copy;
        }
    };
    static_assert(sizeof(ScenarioEquipment<BigEndian>) == 0x28);

    SINGLE_DEPENDENCY_PADDED_STRUCT(ScenarioEquipmentPalette, name, 0x20); // equipment

    ENDIAN_TEMPLATE(EndianType) struct ScenarioWeapon : SpawnPrelude<EndianType> {
        PAD(0x8);
        PAD(0x10);
        PAD(0x8);
        PAD(0x8);

        EndianType<std::int16_t> rounds_left;
        EndianType<std::int16_t> rounds_loaded;
        EndianType<ScenarioItemFlags> flags;
        PAD(0x2);
        PAD(0xC);

        ENDIAN_TEMPLATE(NewType) operator ScenarioWeapon<NewType>() const noexcept {
            ScenarioWeapon<NewType> copy = {};
            COPY_SPAWN_PRELUDE
            COPY_THIS(rounds_left);
            COPY_THIS(rounds_loaded);
            COPY_THIS(flags);
            return copy;
        }
    };
    static_assert(sizeof(ScenarioWeapon<BigEndian>) == 0x5C);

    SINGLE_DEPENDENCY_PADDED_STRUCT(ScenarioWeaponPalette, name, 0x20); // weapon

    struct ScenarioDeviceGroupFlags {
        std::uint32_t can_change_only_once : 1;
    };
    static_assert(sizeof(ScenarioDeviceGroupFlags) == sizeof(std::uint32_t));

    ENDIAN_TEMPLATE(EndianType) struct ScenarioDeviceGroup {
        TagString name;
        EndianType<float> initial_value;
        EndianType<ScenarioDeviceGroupFlags> flags;
        PAD(0xC);

        ENDIAN_TEMPLATE(NewType) operator ScenarioDeviceGroup<NewType>() const noexcept {
            ScenarioDeviceGroup<NewType> copy = {};
            COPY_THIS(name);
            COPY_THIS(initial_value);
            COPY_THIS(flags);
            return copy;
        }
    };
    static_assert(sizeof(ScenarioDeviceGroup<BigEndian>) == 0x34);

    struct ScenarioDeviceFlags {
        std::uint32_t initially_open : 1;
        std::uint32_t initially_off : 1;
        std::uint32_t can_change_only_once : 1;
        std::uint32_t position_reversed : 1;
        std::uint32_t not_usable_from_any_side : 1;
    };
    static_assert(sizeof(ScenarioDeviceFlags) == sizeof(std::uint32_t));

    struct ScenarioMachineFlags {
        std::uint32_t does_not_operate_automatically : 1;
        std::uint32_t one_sided : 1;
        std::uint32_t never_appears_locked : 1;
        std::uint32_t opened_by_melee_attack : 1;
    };
    static_assert(sizeof(ScenarioMachineFlags) == sizeof(std::uint32_t));

    ENDIAN_TEMPLATE(EndianType) struct ScenarioMachine : SpawnPrelude<EndianType> {
        PAD(0x8);
        EndianType<std::int16_t> power_group;
        EndianType<std::int16_t> position_group;
        EndianType<ScenarioDeviceFlags> device_flags;
        EndianType<ScenarioMachineFlags> machine_flags;
        PAD(0xC);

        ENDIAN_TEMPLATE(NewType) operator ScenarioMachine<NewType>() const noexcept {
            ScenarioMachine<NewType> copy = {};
            COPY_SPAWN_PRELUDE
            COPY_THIS(power_group);
            COPY_THIS(position_group);
            COPY_THIS(device_flags);
            COPY_THIS(machine_flags);
            return copy;
        }
    };
    static_assert(sizeof(ScenarioMachine<BigEndian>) == 0x40);

    SINGLE_DEPENDENCY_PADDED_STRUCT(ScenarioMachinePalette, name, 0x20); // device_machine

    struct ScenarioControlFlags {
        std::uint32_t usable_from_both_sides : 1;
    };
    static_assert(sizeof(ScenarioControlFlags) == sizeof(std::uint32_t));

    ENDIAN_TEMPLATE(EndianType) struct ScenarioControl : SpawnPrelude<EndianType> {
        PAD(0x8);
        EndianType<std::int16_t> power_group;
        EndianType<std::int16_t> position_group;
        EndianType<ScenarioDeviceFlags> device_flags;
        EndianType<ScenarioControlFlags> control_flags;
        EndianType<std::int16_t> no_name;
        PAD(0x2);
        PAD(0x8);

        ENDIAN_TEMPLATE(NewType) operator ScenarioControl<NewType>() const noexcept {
            ScenarioControl<NewType> copy = {};
            COPY_SPAWN_PRELUDE
            COPY_THIS(power_group);
            COPY_THIS(position_group);
            COPY_THIS(device_flags);
            COPY_THIS(control_flags);
            COPY_THIS(no_name);
            return copy;
        }
    };
    static_assert(sizeof(ScenarioControl<BigEndian>) == 0x40);

    SINGLE_DEPENDENCY_PADDED_STRUCT(ScenarioControlPalette, name, 0x20); // device_control

    ENDIAN_TEMPLATE(EndianType) struct ScenarioLightFixture : SpawnPrelude<EndianType> {
        PAD(0x8);
        EndianType<std::int16_t> power_group;
        EndianType<std::int16_t> position_group;
        EndianType<ScenarioDeviceFlags> device_flags;
        ColorRGB<EndianType> color;
        EndianType<float> intensity;
        EndianType<Angle> falloff_angle;
        EndianType<Angle> cutoff_angle;
        PAD(0x10);

        ENDIAN_TEMPLATE(NewType) operator ScenarioLightFixture<NewType>() const noexcept {
            ScenarioLightFixture<NewType> copy = {};
            COPY_SPAWN_PRELUDE
            COPY_THIS(power_group);
            COPY_THIS(position_group);
            COPY_THIS(device_flags);
            COPY_THIS(color);
            COPY_THIS(intensity);
            COPY_THIS(falloff_angle);
            COPY_THIS(cutoff_angle);
            return copy;
        }
    };
    static_assert(sizeof(ScenarioLightFixture<BigEndian>) == 0x58);

    SINGLE_DEPENDENCY_PADDED_STRUCT(ScenarioLightFixturePalette, name, 0x20); // device_light_fixture

    ENDIAN_TEMPLATE(EndianType) struct ScenarioSoundScenery : SpawnPrelude<EndianType> {
        PAD(0x8);

        ENDIAN_TEMPLATE(NewType) operator ScenarioSoundScenery<NewType>() const noexcept {
            ScenarioSoundScenery<NewType> copy = {};
            COPY_SPAWN_PRELUDE
            return copy;
        }
    };
    static_assert(sizeof(ScenarioSoundScenery<BigEndian>) == 0x28);

    SINGLE_DEPENDENCY_PADDED_STRUCT(ScenarioSoundSceneryPalette, name, 0x20); // sound_scenery

    ENDIAN_TEMPLATE(EndianType) struct ScenarioPlayerStartingProfile {
        TagString name;
        EndianType<Fraction> starting_health_modifier;
        EndianType<Fraction> starting_shield_modifier;
        TagDependency<EndianType> primary_weapon; // weapon
        EndianType<std::int16_t> rounds_loaded;
        EndianType<std::int16_t> rounds_total;
        TagDependency<EndianType> secondary_weapon; // weapon
        EndianType<std::int16_t> rounds_loaded_1;
        EndianType<std::int16_t> rounds_total_1;
        std::int8_t starting_fragmentation_grenade_count;
        std::int8_t starting_plasma_grenade_count;
        std::int8_t starting_custom_2_grenade_count;
        std::int8_t starting_custom_3_grenade_count;
        PAD(0x14);

        ENDIAN_TEMPLATE(NewType) operator ScenarioPlayerStartingProfile<NewType>() const noexcept {
            ScenarioPlayerStartingProfile<NewType> copy = {};
            COPY_THIS(name);
            COPY_THIS(starting_health_modifier);
            COPY_THIS(starting_shield_modifier);
            COPY_THIS(primary_weapon);
            COPY_THIS(rounds_loaded);
            COPY_THIS(rounds_total);
            COPY_THIS(secondary_weapon);
            COPY_THIS(rounds_loaded_1);
            COPY_THIS(rounds_total_1);
            COPY_THIS(starting_fragmentation_grenade_count);
            COPY_THIS(starting_plasma_grenade_count);
            COPY_THIS(starting_custom_2_grenade_count);
            COPY_THIS(starting_custom_3_grenade_count);
            return copy;
        }
    };
    static_assert(sizeof(ScenarioPlayerStartingProfile<BigEndian>) == 0x68);

    ENDIAN_TEMPLATE(EndianType) struct ScenarioPlayerStartingLocation {
        Point3D<EndianType> position;
        EndianType<Angle> facing;
        EndianType<std::int16_t> team_index;
        EndianType<std::int16_t> bsp_index;
        EndianType<ScenarioSpawnType> type_0;
        EndianType<ScenarioSpawnType> type_1;
        EndianType<ScenarioSpawnType> type_2;
        EndianType<ScenarioSpawnType> type_3;
        PAD(0x18);

        ENDIAN_TEMPLATE(NewType) operator ScenarioPlayerStartingLocation<NewType>() const noexcept {
            ScenarioPlayerStartingLocation<NewType> copy = {};
            COPY_THIS(position);
            COPY_THIS(facing);
            COPY_THIS(team_index);
            COPY_THIS(bsp_index);
            COPY_THIS(type_0);
            COPY_THIS(type_1);
            COPY_THIS(type_2);
            COPY_THIS(type_3);
            return copy;
        }
    };
    static_assert(sizeof(ScenarioPlayerStartingLocation<BigEndian>) == 0x34);

    ENDIAN_TEMPLATE(EndianType) struct ScenarioTriggerVolume {
        LittleEndian<std::uint16_t> unknown; // 1 = enabled? idk
        PAD(0x2);
        TagString name;
        EndianType<float> unknown_values[9];
        Point3D<EndianType> corners[2];

        ENDIAN_TEMPLATE(NewType) operator ScenarioTriggerVolume<NewType>() const noexcept {
            ScenarioTriggerVolume<NewType> copy = {};
            COPY_THIS(unknown);
            COPY_THIS(name);
            COPY_THIS_ARRAY(unknown_values);
            COPY_THIS_ARRAY(corners);
            return copy;
        }
    };
    static_assert(sizeof(ScenarioTriggerVolume<BigEndian>) == 0x60);

    ENDIAN_TEMPLATE(EndianType) struct ScenarioRecordedAnimation {
        TagString name;
        std::int8_t version;
        std::int8_t raw_animation_data;
        std::int8_t unit_control_data_version;
        PAD(0x1);
        EndianType<std::int16_t> length_of_animation;
        PAD(0x2);
        PAD(0x4);
        TagDataOffset<EndianType> recorded_animation_event_stream;

        ENDIAN_TEMPLATE(NewType) operator ScenarioRecordedAnimation<NewType>() const noexcept {
            ScenarioRecordedAnimation<NewType> copy = {};
            COPY_THIS(name);
            COPY_THIS(version);
            COPY_THIS(raw_animation_data);
            COPY_THIS(unit_control_data_version);
            COPY_THIS(length_of_animation);
            COPY_THIS(recorded_animation_event_stream);
            return copy;
        }
    };
    static_assert(sizeof(ScenarioRecordedAnimation<BigEndian>) == 0x40);

    ENDIAN_TEMPLATE(EndianType) struct ScenarioNetgameFlags {
        Point3D<EndianType> position;
        EndianType<Angle> facing;
        EndianType<ScenarioNetgameFlagType> type;
        EndianType<std::int16_t> team_index;
        TagDependency<EndianType> weapon_group; // item_collection
        PAD(0x70);

        ENDIAN_TEMPLATE(NewType) operator ScenarioNetgameFlags<NewType>() const noexcept {
            ScenarioNetgameFlags<NewType> copy = {};
            COPY_THIS(position);
            COPY_THIS(facing);
            COPY_THIS(type);
            COPY_THIS(team_index);
            COPY_THIS(weapon_group);
            return copy;
        }
    };
    static_assert(sizeof(ScenarioNetgameFlags<BigEndian>) == 0x94);

    struct ScenarioNetgameEquipmentFlags {
        std::uint32_t levitate : 1;
    };

    ENDIAN_TEMPLATE(EndianType) struct ScenarioNetgameEquipment {
        EndianType<ScenarioNetgameEquipmentFlags> flags;
        EndianType<ScenarioSpawnType> type_0;
        EndianType<ScenarioSpawnType> type_1;
        EndianType<ScenarioSpawnType> type_2;
        EndianType<ScenarioSpawnType> type_3;
        EndianType<std::int16_t> team_index;
        EndianType<std::int16_t> spawn_time;
        PAD(0x30);
        Point3D<EndianType> position;
        EndianType<Angle> facing;
        TagDependency<EndianType> item_collection; // item_collection
        PAD(0x30);

        ENDIAN_TEMPLATE(NewType) operator ScenarioNetgameEquipment<NewType>() const noexcept {
            ScenarioNetgameEquipment<NewType> copy = {};
            COPY_THIS(flags);
            COPY_THIS(type_0);
            COPY_THIS(type_1);
            COPY_THIS(type_2);
            COPY_THIS(type_3);
            COPY_THIS(team_index);
            COPY_THIS(spawn_time);
            COPY_THIS(position);
            COPY_THIS(facing);
            COPY_THIS(item_collection);
            return copy;
        }
    };
    static_assert(sizeof(ScenarioNetgameEquipment<BigEndian>) == 0x90);

    struct ScenarioStartingEquipmentFlags {
        std::uint32_t no_grenades : 1;
        std::uint32_t plasma_grenades : 1;
    };

    ENDIAN_TEMPLATE(EndianType) struct ScenarioStartingEquipment {
        EndianType<ScenarioStartingEquipmentFlags> flags;
        EndianType<ScenarioSpawnType> type_0;
        EndianType<ScenarioSpawnType> type_1;
        EndianType<ScenarioSpawnType> type_2;
        EndianType<ScenarioSpawnType> type_3;
        PAD(0x30);
        TagDependency<EndianType> item_collection_1; // item_collection
        TagDependency<EndianType> item_collection_2; // item_collection
        TagDependency<EndianType> item_collection_3; // item_collection
        TagDependency<EndianType> item_collection_4; // item_collection
        TagDependency<EndianType> item_collection_5; // item_collection
        TagDependency<EndianType> item_collection_6; // item_collection
        PAD(0x30);

        ENDIAN_TEMPLATE(NewType) operator ScenarioStartingEquipment<NewType>() const noexcept {
            ScenarioStartingEquipment<NewType> copy = {};
            COPY_THIS(flags);
            COPY_THIS(type_0);
            COPY_THIS(type_1);
            COPY_THIS(type_2);
            COPY_THIS(type_3);
            COPY_THIS(item_collection_1);
            COPY_THIS(item_collection_2);
            COPY_THIS(item_collection_3);
            COPY_THIS(item_collection_4);
            COPY_THIS(item_collection_5);
            COPY_THIS(item_collection_6);
            return copy;
        }
    };
    static_assert(sizeof(ScenarioStartingEquipment<BigEndian>) == 0xCC);

    ENDIAN_TEMPLATE(EndianType) struct ScenarioBSPSwitchTriggerVolume {
        EndianType<std::int16_t> trigger_volume;
        EndianType<std::int16_t> source;
        EndianType<std::int16_t> destination;
        LittleEndian<std::uint16_t> unknown;

        ENDIAN_TEMPLATE(NewType) operator ScenarioBSPSwitchTriggerVolume<NewType>() const noexcept {
            ScenarioBSPSwitchTriggerVolume<NewType> copy = {};
            COPY_THIS(trigger_volume);
            COPY_THIS(source);
            COPY_THIS(destination);
            COPY_THIS(unknown);
            return copy;
        }
    };
    static_assert(sizeof(ScenarioBSPSwitchTriggerVolume<BigEndian>) == 0x8);

    ENDIAN_TEMPLATE(EndianType) struct ScenarioDecal {
        EndianType<std::int16_t> decal_type;
        std::int8_t yaw;
        std::int8_t pitch;
        Point3D<EndianType> position;

        ENDIAN_TEMPLATE(NewType) operator ScenarioDecal<NewType>() const noexcept {
            ScenarioDecal<NewType> copy = {};
            COPY_THIS(decal_type);
            COPY_THIS(yaw);
            COPY_THIS(pitch);
            COPY_THIS(position);
            return copy;
        }
    };
    static_assert(sizeof(ScenarioDecal<BigEndian>) == 0x10);

    SINGLE_DEPENDENCY_STRUCT(ScenarioDecalPallete, reference); // decal
    SINGLE_DEPENDENCY_PADDED_STRUCT(ScenarioDetailObjectCollectionPalette, reference, 0x20); // detail_object_collection
    SINGLE_DEPENDENCY_STRUCT(ScenarioActorPalette, reference); // actor_variant

    ENDIAN_TEMPLATE(EndianType) struct ScenarioMovePosition {
        Point3D<EndianType> position;
        EndianType<Angle> facing;
        EndianType<float> weight;
        Bounds<EndianType<float>> time;
        EndianType<std::int16_t> animation;
        std::int8_t sequence_id;
        PAD(0x1);
        PAD(0x2C);
        EndianType<std::int32_t> surface_index;

        ENDIAN_TEMPLATE(NewType) operator ScenarioMovePosition<NewType>() const noexcept {
            ScenarioMovePosition<NewType> copy = {};
            COPY_THIS(position);
            COPY_THIS(facing);
            COPY_THIS(weight);
            COPY_THIS(time);
            COPY_THIS(animation);
            COPY_THIS(sequence_id);
            COPY_THIS(surface_index);
            return copy;
        }
    };
    static_assert(sizeof(ScenarioMovePosition<BigEndian>) == 0x50);

    struct ScenarioActorStartingLocationFlags {
        std::uint8_t required : 1;
    };

    ENDIAN_TEMPLATE(EndianType) struct ScenarioActorStartingLocation {
        Point3D<EndianType> position;
        EndianType<Angle> facing;
        EndianType<std::uint16_t> unknown;
        std::int8_t sequence_id;
        EndianType<ScenarioActorStartingLocationFlags> flags;
        EndianType<ScenarioReturnState> return_state;
        EndianType<ScenarioReturnState> initial_state;
        EndianType<std::int16_t> actor_type;
        EndianType<std::int16_t> command_list;

        ENDIAN_TEMPLATE(NewType) operator ScenarioActorStartingLocation<NewType>() const noexcept {
            ScenarioActorStartingLocation<NewType> copy = {};
            COPY_THIS(position);
            COPY_THIS(facing);
            COPY_THIS(unknown);
            COPY_THIS(sequence_id);
            COPY_THIS(flags);
            COPY_THIS(return_state);
            COPY_THIS(initial_state);
            COPY_THIS(actor_type);
            COPY_THIS(command_list);
            return copy;
        }
    };
    static_assert(sizeof(ScenarioActorStartingLocation<BigEndian>) == 0x1C);

    struct ScenarioSquadFlags {
        std::uint32_t unused : 1;
        std::uint32_t never_search : 1;
        std::uint32_t start_timer_immediately : 1;
        std::uint32_t no_timer_delay_forever : 1;
        std::uint32_t magic_sight_after_timer : 1;
        std::uint32_t automatic_migration : 1;
    };

    struct ScenarioSquadAttacking {
        std::uint32_t a : 1;
        std::uint32_t b : 1;
        std::uint32_t c : 1;
        std::uint32_t d : 1;
        std::uint32_t e : 1;
        std::uint32_t f : 1;
        std::uint32_t g : 1;
        std::uint32_t h : 1;
        std::uint32_t i : 1;
        std::uint32_t j : 1;
        std::uint32_t k : 1;
        std::uint32_t l : 1;
        std::uint32_t m : 1;
        std::uint32_t n : 1;
        std::uint32_t o : 1;
        std::uint32_t p : 1;
        std::uint32_t q : 1;
        std::uint32_t r : 1;
        std::uint32_t s : 1;
        std::uint32_t t : 1;
        std::uint32_t u : 1;
        std::uint32_t v : 1;
        std::uint32_t w : 1;
        std::uint32_t x : 1;
        std::uint32_t y : 1;
        std::uint32_t z : 1;
    };

    ENDIAN_TEMPLATE(EndianType) struct ScenarioSquad {
        TagString name;
        EndianType<std::int16_t> actor_type;
        EndianType<std::int16_t> platoon;
        EndianType<ScenarioReturnState> Tinitial_state;
        EndianType<ScenarioReturnState> Treturn_state;
        EndianType<ScenarioSquadFlags> flags;
        EndianType<ScenarioUniqueLeaderType> unique_leader_type;
        PAD(0x2);
        PAD(0x1C);
        PAD(0x2);
        EndianType<std::int16_t> maneuver_to_squad;
        EndianType<float> squad_delay_time;
        EndianType<ScenarioSquadAttacking> attacking;
        EndianType<ScenarioSquadAttacking> attacking_search;
        EndianType<ScenarioSquadAttacking> attacking_guard;
        EndianType<ScenarioSquadAttacking> defending;
        EndianType<ScenarioSquadAttacking> defending_search;
        EndianType<ScenarioSquadAttacking> defending_guard;
        EndianType<ScenarioSquadAttacking> pursuing;
        PAD(0x4);
        PAD(0x8);
        EndianType<std::int16_t> normal_diff_count;
        EndianType<std::int16_t> insane_diff_count;
        EndianType<ScenarioMajorUpgrade> major_upgrade;
        PAD(0x2);
        EndianType<std::int16_t> respawn_min_actors;
        EndianType<std::int16_t> respawn_max_actors;
        EndianType<std::int16_t> respawn_total;
        PAD(0x2);
        Bounds<EndianType<float>> respawn_delay;
        PAD(0x30);
        TagReflexive<EndianType, ScenarioMovePosition> move_positions;
        TagReflexive<EndianType, ScenarioActorStartingLocation> starting_locations;
        PAD(0xC);

        ENDIAN_TEMPLATE(NewType) operator ScenarioSquad<NewType>() const noexcept {
            ScenarioSquad<NewType> copy = {};
            COPY_THIS(name);
            COPY_THIS(actor_type);
            COPY_THIS(platoon);
            COPY_THIS(Tinitial_state);
            COPY_THIS(Treturn_state);
            COPY_THIS(flags);
            COPY_THIS(unique_leader_type);
            COPY_THIS(maneuver_to_squad);
            COPY_THIS(squad_delay_time);
            COPY_THIS(attacking);
            COPY_THIS(attacking_search);
            COPY_THIS(attacking_guard);
            COPY_THIS(defending);
            COPY_THIS(defending_search);
            COPY_THIS(defending_guard);
            COPY_THIS(pursuing);
            COPY_THIS(normal_diff_count);
            COPY_THIS(insane_diff_count);
            COPY_THIS(major_upgrade);
            COPY_THIS(respawn_min_actors);
            COPY_THIS(respawn_max_actors);
            COPY_THIS(respawn_total);
            COPY_THIS(respawn_delay);
            COPY_THIS(move_positions);
            COPY_THIS(starting_locations);
            return copy;
        }
    };
    static_assert(sizeof(ScenarioSquad<BigEndian>) == 0xE8);

    struct ScenarioPlatoonFlags {
        std::uint32_t flee_when_maneuvering : 1;
        std::uint32_t say_advancing_when_maneuver : 1;
        std::uint32_t start_in_defending_state : 1;
    };

    ENDIAN_TEMPLATE(EndianType) struct ScenarioPlatoon {
        TagString name;
        EndianType<ScenarioPlatoonFlags> flags;
        PAD(0xC);
        EndianType<ScenarioChangeAttackingDefendingStateWhen> change_attacking_defending_state_when;
        EndianType<std::int16_t> happens_to;
        PAD(0x4);
        PAD(0x4);
        EndianType<ScenarioChangeAttackingDefendingStateWhen> maneuver_when;
        EndianType<std::int16_t> happens_to_1;
        PAD(0x4);
        PAD(0x4);
        PAD(0x40);
        PAD(0x24);

        ENDIAN_TEMPLATE(NewType) operator ScenarioPlatoon<NewType>() const noexcept {
            ScenarioPlatoon<NewType> copy = {};
            COPY_THIS(name);
            COPY_THIS(flags);
            COPY_THIS(change_attacking_defending_state_when);
            COPY_THIS(happens_to);
            COPY_THIS(maneuver_when);
            COPY_THIS(happens_to_1);
            return copy;
        }
    };
    static_assert(sizeof(ScenarioPlatoon<BigEndian>) == 0xAC);

    ENDIAN_TEMPLATE(EndianType) struct ScenarioFiringPosition {
        Point3D<EndianType> position;
        EndianType<ScenarioGroupIndex> group_index;
        LittleEndian<std::uint16_t> cluster_index;
        PAD(0x4);
        LittleEndian<std::uint32_t> surface_index;

        ENDIAN_TEMPLATE(NewType) operator ScenarioFiringPosition<NewType>() const noexcept {
            ScenarioFiringPosition<NewType> copy = {};
            COPY_THIS(position);
            COPY_THIS(group_index);
            COPY_THIS(cluster_index);
            COPY_THIS(surface_index);
            return copy;
        }
    };
    static_assert(sizeof(ScenarioFiringPosition<BigEndian>) == 0x18);

    struct ScenarioEncounterFlags {
        std::uint32_t not_initially_created : 1;
        std::uint32_t respawn_enabled : 1;
        std::uint32_t initially_blind : 1;
        std::uint32_t initially_deaf : 1;
        std::uint32_t initially_braindead : 1;
        std::uint32_t _3d_firing_positions : 1;
        std::uint32_t manual_bsp_index_specified : 1;
    };
    ENDIAN_TEMPLATE(EndianType) struct ScenarioEncounter {
        TagString name;
        EndianType<ScenarioEncounterFlags> flags;
        EndianType<ScenarioTeamIndex> team_index;
        LittleEndian<std::int16_t> one;
        EndianType<ScenarioSearchBehavior> search_behavior;
        EndianType<std::int16_t> manual_bsp_index;
        Bounds<EndianType<float>> respawn_delay;
        PAD(0x4A);
        LittleEndian<std::uint16_t> precomputed_bsp_index;
        TagReflexive<EndianType, ScenarioSquad> squads;
        TagReflexive<EndianType, ScenarioPlatoon> platoons;
        TagReflexive<EndianType, ScenarioFiringPosition> firing_positions;
        TagReflexive<EndianType, ScenarioPlayerStartingLocation> player_starting_locations;

        ENDIAN_TEMPLATE(NewType) operator ScenarioEncounter<NewType>() const noexcept {
            ScenarioEncounter<NewType> copy = {};
            COPY_THIS(name);
            COPY_THIS(flags);
            COPY_THIS(team_index);
            COPY_THIS(one);
            COPY_THIS(search_behavior);
            COPY_THIS(manual_bsp_index);
            COPY_THIS(respawn_delay);
            COPY_THIS(precomputed_bsp_index);
            COPY_THIS(squads);
            COPY_THIS(platoons);
            COPY_THIS(firing_positions);
            COPY_THIS(player_starting_locations);
            return copy;
        }
    };
    static_assert(sizeof(ScenarioEncounter<BigEndian>) == 0xB0);

    ENDIAN_TEMPLATE(EndianType) struct ScenarioCommand {
        EndianType<ScenarioAtomType> atom_type;
        EndianType<std::int16_t> atom_modifier;
        EndianType<float> parameter1;
        EndianType<float> parameter2;
        EndianType<std::int16_t> point_1;
        EndianType<std::int16_t> point_2;
        EndianType<std::int16_t> animation;
        EndianType<std::int16_t> script;
        EndianType<std::int16_t> recording;
        EndianType<std::int16_t> command;
        EndianType<std::int16_t> object_name;
        PAD(0x6);

        ENDIAN_TEMPLATE(NewType) operator ScenarioCommand<NewType>() const noexcept {
            ScenarioCommand<NewType> copy = {};
            COPY_THIS(atom_type);
            COPY_THIS(atom_modifier);
            COPY_THIS(parameter1);
            COPY_THIS(parameter2);
            COPY_THIS(point_1);
            COPY_THIS(point_2);
            COPY_THIS(animation);
            COPY_THIS(script);
            COPY_THIS(recording);
            COPY_THIS(command);
            COPY_THIS(object_name);
            return copy;
        }
    };
    static_assert(sizeof(ScenarioCommand<BigEndian>) == 0x20);

    ENDIAN_TEMPLATE(EndianType) struct ScenarioCommandPoint {
        Point3D<EndianType> position;
        PAD(0x8);

        ENDIAN_TEMPLATE(NewType) operator ScenarioCommandPoint<NewType>() const noexcept {
            ScenarioCommandPoint<NewType> copy = {};
            COPY_THIS(position);
            return copy;
        }
    };
    static_assert(sizeof(ScenarioCommandPoint<BigEndian>) == 0x14);

    struct ScenarioCommandListFlags {
        std::uint32_t allow_initiative : 1;
        std::uint32_t allow_targeting : 1;
        std::uint32_t disable_looking : 1;
        std::uint32_t disable_communication : 1;
        std::uint32_t disable_falling_damage : 1;
        std::uint32_t manual_bsp_index : 1;
    };

    ENDIAN_TEMPLATE(EndianType) struct ScenarioCommandList {
        TagString name;
        EndianType<ScenarioCommandListFlags> flags;
        PAD(0x8);
        EndianType<std::int16_t> manual_bsp_index;
        EndianType<std::int16_t> precomputed_bsp_index;
        TagReflexive<EndianType, ScenarioCommand> commands;
        TagReflexive<EndianType, ScenarioCommandPoint> points;
        PAD(0x18);

        ENDIAN_TEMPLATE(NewType) operator ScenarioCommandList<NewType>() const noexcept {
            ScenarioCommandList<NewType> copy = {};
            COPY_THIS(name);
            COPY_THIS(flags);
            COPY_THIS(manual_bsp_index);
            COPY_THIS(precomputed_bsp_index);
            COPY_THIS(commands);
            COPY_THIS(points);
            return copy;
        }
    };
    static_assert(sizeof(ScenarioCommandList<BigEndian>) == 0x60);

    ENDIAN_TEMPLATE(EndianType) struct ScenarioAIAnimationReference {
        TagString animation_name;
        TagDependency<EndianType> animation_graph; // model_animations
        PAD(0xC);

        ENDIAN_TEMPLATE(NewType) operator ScenarioAIAnimationReference<NewType>() const noexcept {
            ScenarioAIAnimationReference<NewType> copy = {};
            COPY_THIS(animation_name);
            COPY_THIS(animation_graph);
            return copy;
        }
    };
    static_assert(sizeof(ScenarioAIAnimationReference<BigEndian>) == 0x3C);

    ENDIAN_TEMPLATE(EndianType) struct ScenarioAIScriptReference {
        TagString script_name;
        PAD(0x8);

        ENDIAN_TEMPLATE(NewType) operator ScenarioAIScriptReference<NewType>() const noexcept {
            ScenarioAIScriptReference<NewType> copy = {};
            COPY_THIS(script_name);
            return copy;
        }
    };
    static_assert(sizeof(ScenarioAIScriptReference<BigEndian>) == 0x28);

    ENDIAN_TEMPLATE(EndianType) struct ScenarioAIRecordingReference {
        TagString recording_name;
        PAD(0x8);

        ENDIAN_TEMPLATE(NewType) operator ScenarioAIRecordingReference<NewType>() const noexcept {
            ScenarioAIRecordingReference<NewType> copy = {};
            COPY_THIS(recording_name);
            return copy;
        }
    };
    static_assert(sizeof(ScenarioAIRecordingReference<BigEndian>) == 0x28);

    struct ScenarioAIConversationParticipantFlags {
        std::uint16_t optional : 1;
        std::uint16_t has_alternate : 1;
        std::uint16_t is_alternate : 1;
    };

    ENDIAN_TEMPLATE(EndianType) struct ScenarioAIConversationParticipant {
        PAD(0x2);
        EndianType<ScenarioAIConversationParticipantFlags> flags;
        EndianType<ScenarioSelectionType> selection_type;
        EndianType<ScenarioActorType> actor_type;
        EndianType<std::int16_t> use_this_object;
        EndianType<std::int16_t> set_new_name;
        PAD(0xC);
        PAD(0xC);
        TagString encounter_name;
        PAD(0x4);
        PAD(0xC);

        ENDIAN_TEMPLATE(NewType) operator ScenarioAIConversationParticipant<NewType>() const noexcept {
            ScenarioAIConversationParticipant<NewType> copy = {};
            COPY_THIS(flags);
            COPY_THIS(selection_type);
            COPY_THIS(actor_type);
            COPY_THIS(use_this_object);
            COPY_THIS(set_new_name);
            COPY_THIS(encounter_name);
            return copy;
        }
    };
    static_assert(sizeof(ScenarioAIConversationParticipant<BigEndian>) == 0x54);

    struct ScenarioAIConversationLineFlags {
        std::uint16_t addressee_look_at_speaker : 1;
        std::uint16_t everyone_look_at_speaker : 1;
        std::uint16_t everyone_look_at_addressee : 1;
        std::uint16_t wait_after_until_told_to_advance : 1;
        std::uint16_t wait_until_speaker_nearby : 1;
        std::uint16_t wait_until_everyone_nearby : 1;
    };

    ENDIAN_TEMPLATE(EndianType) struct ScenarioAIConversationLine {
        EndianType<ScenarioAIConversationLineFlags> flags;
        EndianType<std::int16_t> participant;
        EndianType<ScenarioAddressee> addressee;
        EndianType<std::int16_t> addressee_participant;
        PAD(0x4);
        EndianType<float> line_delay_time;
        PAD(0xC);
        TagDependency<EndianType> variant_1; // sound
        TagDependency<EndianType> variant_2; // sound
        TagDependency<EndianType> variant_3; // sound
        TagDependency<EndianType> variant_4; // sound
        TagDependency<EndianType> variant_5; // sound
        TagDependency<EndianType> variant_6; // sound

        ENDIAN_TEMPLATE(NewType) operator ScenarioAIConversationLine<NewType>() const noexcept {
            ScenarioAIConversationLine<NewType> copy = {};
            COPY_THIS(flags);
            COPY_THIS(participant);
            COPY_THIS(addressee);
            COPY_THIS(addressee_participant);
            COPY_THIS(line_delay_time);
            COPY_THIS(variant_1);
            COPY_THIS(variant_2);
            COPY_THIS(variant_3);
            COPY_THIS(variant_4);
            COPY_THIS(variant_5);
            COPY_THIS(variant_6);
            return copy;
        }
    };
    static_assert(sizeof(ScenarioAIConversationLine<BigEndian>) == 0x7C);

    struct ScenarioAIConversationFlags {
        std::uint16_t stop_if_death : 1;
        std::uint16_t stop_if_damaged : 1;
        std::uint16_t stop_if_visible_enemy : 1;
        std::uint16_t stop_if_alerted_to_enemy : 1;
        std::uint16_t player_must_be_visible : 1;
        std::uint16_t stop_other_actions : 1;
        std::uint16_t keep_trying_to_play : 1;
        std::uint16_t player_must_be_looking : 1;
    };

    ENDIAN_TEMPLATE(EndianType) struct ScenarioAIConversation {
        TagString name;
        EndianType<ScenarioAIConversationFlags> flags;
        PAD(0x2);
        EndianType<float> trigger_distance;
        EndianType<float> run_to_player_dist;
        PAD(0x24);
        TagReflexive<EndianType, ScenarioAIConversationParticipant> participants;
        TagReflexive<EndianType, ScenarioAIConversationLine> lines;
        PAD(0xC);

        ENDIAN_TEMPLATE(NewType) operator ScenarioAIConversation<NewType>() const noexcept {
            ScenarioAIConversation<NewType> copy = {};
            COPY_THIS(name);
            COPY_THIS(flags);
            COPY_THIS(trigger_distance);
            COPY_THIS(run_to_player_dist);
            COPY_THIS(participants);
            COPY_THIS(lines);
            return copy;
        }
    };
    static_assert(sizeof(ScenarioAIConversation<BigEndian>) == 0x74);

    ENDIAN_TEMPLATE(EndianType) struct ScenarioScript {
        TagString name;
        EndianType<ScenarioScriptType> script_type;
        EndianType<ScenarioScriptValueType> return_type;
        EndianType<std::int32_t> root_expression_index;
        PAD(0x34);

        ENDIAN_TEMPLATE(NewType) operator ScenarioScript<NewType>() const noexcept {
            ScenarioScript<NewType> copy = {};
            COPY_THIS(name);
            COPY_THIS(script_type);
            COPY_THIS(return_type);
            COPY_THIS(root_expression_index);
            return copy;
        }
    };
    static_assert(sizeof(ScenarioScript<BigEndian>) == 0x5C);

    ENDIAN_TEMPLATE(EndianType) struct ScenarioGlobal {
        TagString name;
        EndianType<ScenarioScriptValueType> type;
        PAD(0x2);
        PAD(0x4);
        EndianType<std::int32_t> initialization_expression_index;
        PAD(0x30);

        ENDIAN_TEMPLATE(NewType) operator ScenarioGlobal<NewType>() const noexcept {
            ScenarioGlobal<NewType> copy = {};
            COPY_THIS(name);
            COPY_THIS(type);
            COPY_THIS(initialization_expression_index);
            return copy;
        }
    };
    static_assert(sizeof(ScenarioGlobal<BigEndian>) == 0x5C);

    ENDIAN_TEMPLATE(EndianType) struct ScenarioReference {
        PAD(0x18);
        TagDependency<EndianType> reference;

        ENDIAN_TEMPLATE(NewType) operator ScenarioReference<NewType>() const noexcept {
            ScenarioReference<NewType> copy = {};
            COPY_THIS(reference);
            return copy;
        }
    };
    static_assert(sizeof(ScenarioReference<BigEndian>) == 0x28);

    ENDIAN_TEMPLATE(EndianType) struct ScenarioSourceFile {
        TagString name;
        TagDataOffset<EndianType> source;

        ENDIAN_TEMPLATE(NewType) operator ScenarioSourceFile<NewType>() const noexcept {
            ScenarioSourceFile<NewType> copy = {};
            COPY_THIS(name);
            COPY_THIS(source);
            return copy;
        }
    };
    static_assert(sizeof(ScenarioSourceFile<BigEndian>) == 0x34);

    ENDIAN_TEMPLATE(EndianType) struct ScenarioCutsceneFlag {
        LittleEndian<std::uint32_t> unknown;
        TagString name;
        Point3D<EndianType> position;
        Euler2D<EndianType> facing;
        PAD(0x24);

        ENDIAN_TEMPLATE(NewType) operator ScenarioCutsceneFlag<NewType>() const noexcept {
            ScenarioCutsceneFlag<NewType> copy = {};
            COPY_THIS(unknown);
            COPY_THIS(name);
            COPY_THIS(position);
            COPY_THIS(facing);
            return copy;
        }
    };
    static_assert(sizeof(ScenarioCutsceneFlag<BigEndian>) == 0x5C);

    ENDIAN_TEMPLATE(EndianType) struct ScenarioCutsceneCameraPoint {
        LittleEndian<std::uint32_t> unknown;
        TagString name;
        PAD(0x4);
        Point3D<EndianType> position;
        Euler3D<EndianType> orientation;
        EndianType<Angle> field_of_view;
        PAD(0x24);

        ENDIAN_TEMPLATE(NewType) operator ScenarioCutsceneCameraPoint<NewType>() const noexcept {
            ScenarioCutsceneCameraPoint<NewType> copy = {};
            COPY_THIS(unknown);
            COPY_THIS(name);
            COPY_THIS(position);
            COPY_THIS(orientation);
            COPY_THIS(field_of_view);
            return copy;
        }
    };
    static_assert(sizeof(ScenarioCutsceneCameraPoint<BigEndian>) == 0x68);

    ENDIAN_TEMPLATE(EndianType) struct ScenarioCutsceneTitle {
        LittleEndian<std::uint32_t> unknown;
        TagString name;
        PAD(0x4);
        Rectangle2D<EndianType> text_bounds;
        EndianType<std::int16_t> string_index;
        EndianType<ScenarioTextStyle> text_style;
        EndianType<ScenarioJustification> justification;
        PAD(0x2);
        PAD(0x4);
        EndianType<ColorARGBInt> text_color;
        EndianType<ColorARGBInt> shadow_color;
        EndianType<float> fade_in_time;
        EndianType<float> up_time;
        EndianType<float> fade_out_time;
        PAD(0x10);

        ENDIAN_TEMPLATE(NewType) operator ScenarioCutsceneTitle<NewType>() const noexcept {
            ScenarioCutsceneTitle<NewType> copy = {};
            COPY_THIS(unknown);
            COPY_THIS(name);
            COPY_THIS(text_bounds);
            COPY_THIS(string_index);
            COPY_THIS(text_style);
            COPY_THIS(justification);
            COPY_THIS(text_color);
            COPY_THIS(shadow_color);
            COPY_THIS(fade_in_time);
            COPY_THIS(up_time);
            COPY_THIS(fade_out_time);
            return copy;
        }
    };
    static_assert(sizeof(ScenarioCutsceneTitle<BigEndian>) == 0x60);

    ENDIAN_TEMPLATE(EndianType) struct ScenarioBSP {
        /** File offset for bsp when compiled */
        EndianType<std::uint32_t> bsp_start;

        /** File size for bsp when compiled */
        EndianType<std::uint32_t> bsp_size;

        /** Memory address for bsp when compiled and loaded */
        EndianType<std::uint32_t> bsp_address;

        PAD(0x4);

        TagDependency<EndianType> structure_bsp; // scenario_structure_bsp

        ENDIAN_TEMPLATE(NewType) operator ScenarioBSP<NewType>() const noexcept {
            ScenarioBSP<NewType> copy = {};
            COPY_THIS(bsp_start);
            COPY_THIS(bsp_size);
            COPY_THIS(bsp_address);
            COPY_THIS(structure_bsp);
            return copy;
        }
    };
    static_assert(sizeof(ScenarioBSP<BigEndian>) == 0x20);

    struct ScenarioFlags {
        std::uint16_t cortana_hack : 1;
        std::uint16_t use_demo_ui : 1;
    };
    static_assert(sizeof(ScenarioFlags) == sizeof(std::uint16_t));

    union ScenarioScriptNodeValue {
        std::uint8_t bool_int;
        std::uint16_t short_int;
        std::uint32_t long_int;
        float real;
        TagID tag_id;

        ScenarioScriptNodeValue() = default;
        ScenarioScriptNodeValue(const ScenarioScriptNodeValue &copy) = default;

        ScenarioScriptNodeValue(std::uint8_t v) {
            this->bool_int = v;
        }

        ScenarioScriptNodeValue(std::uint16_t v) {
            this->short_int = v;
        }

        ScenarioScriptNodeValue(std::uint32_t v) {
            this->long_int = v;
        }

        ScenarioScriptNodeValue(float v) {
            this->real = v;
        }

        ScenarioScriptNodeValue(TagID v) {
            this->tag_id = v;
        }

        void operator=(std::uint8_t v) {
            this->long_int = 0xFFFFFFFF;
            this->bool_int = v;
        }

        void operator=(std::uint16_t v) {
            this->long_int = 0xFFFFFFFF;
            this->short_int = v;
        }

        void operator=(std::uint32_t v) {
            this->long_int = v;
        }

        void operator=(float v) {
            this->real = v;
        }

        void operator=(TagID v) {
            this->tag_id = v;
        }
    };

    struct ScenarioScriptNodeFlags {
        std::uint16_t is_primitive : 1;
        std::uint16_t is_script_call : 1;
        std::uint16_t is_global : 1;
        std::uint16_t is_garbage_collectable : 1;
    };

    // TODO: Figure out what this is
    ENDIAN_TEMPLATE(EndianType) struct ScenarioScriptNode {
        EndianType<std::uint16_t> salt;
        EndianType<std::uint16_t> index_union;
        EndianType<ScenarioScriptValueType> type;
        EndianType<ScenarioScriptNodeFlags> flags;
        EndianType<std::uint32_t> next_node;
        EndianType<std::uint32_t> string_offset;
        EndianType<ScenarioScriptNodeValue> data;

        ENDIAN_TEMPLATE(NewType) operator ScenarioScriptNode<NewType>() const noexcept {
            ScenarioScriptNode<NewType> copy = {};
            COPY_THIS(salt);
            COPY_THIS(index_union);
            COPY_THIS(type);
            COPY_THIS(flags);
            COPY_THIS(next_node);
            COPY_THIS(string_offset);
            COPY_THIS(data);
            return copy;
        }
    };
    static_assert(sizeof(ScenarioScriptNode<BigEndian>) == 0x14);

    // This is the same thing as any other Halo table, but in big endian when in a tag file
    ENDIAN_TEMPLATE(EndianType) struct ScenarioScriptNodeTable {
        TagString name;
        EndianType<std::uint16_t> maximum_count;
        EndianType<std::uint16_t> element_size;
        std::uint8_t one;
        PAD(0x3);
        EndianType<std::uint32_t> data;
        PAD(0x2);
        EndianType<std::uint16_t> size;
        EndianType<std::uint16_t> count;
        EndianType<std::uint16_t> next_id;
        EndianType<std::uint32_t> first_element_ptr; // only means something when loaded. otherwise this is set to 0 upon compiling

        ENDIAN_TEMPLATE(NewType) operator ScenarioScriptNodeTable<NewType>() const noexcept {
            ScenarioScriptNodeTable<NewType> copy = {};
            COPY_THIS(name);
            COPY_THIS(maximum_count);
            COPY_THIS(element_size);
            COPY_THIS(one);
            COPY_THIS(data);
            COPY_THIS(size);
            COPY_THIS(count);
            COPY_THIS(next_id);
            COPY_THIS(first_element_ptr);
            return copy;
        }
    };
    static_assert(sizeof(ScenarioScriptNodeTable<BigEndian>) == 0x38);

    ENDIAN_TEMPLATE(EndianType) struct Scenario {
        TagDependency<EndianType> don_t_use; // scenario_structure_bsp
        TagDependency<EndianType> won_t_use; // scenario_structure_bsp
        TagDependency<EndianType> can_t_use; // sky
        TagReflexive<EndianType, ScenarioSky> skies;
        EndianType<CacheFileType> type;
        EndianType<ScenarioFlags> flags;
        TagReflexive<EndianType, ScenarioChildScenario> child_scenarios;
        EndianType<float> local_north;
        PAD(0x14);
        PAD(0x88);
        TagReflexive<EndianType, PredictedResource> predicted_resources;
        TagReflexive<EndianType, ScenarioFunction> functions;
        TagDataOffset<EndianType> editor_scenario_data;
        TagReflexive<EndianType, ScenarioEditorComment> comments;
        PAD(0xE0);
        TagReflexive<EndianType, ScenarioObjectName> object_names;
        TagReflexive<EndianType, ScenarioScenery> scenery;
        TagReflexive<EndianType, ScenarioSceneryPalette> scenery_palette;
        TagReflexive<EndianType, ScenarioBiped> bipeds;
        TagReflexive<EndianType, ScenarioBipedPalette> biped_palette;
        TagReflexive<EndianType, ScenarioVehicle> vehicles;
        TagReflexive<EndianType, ScenarioVehiclePalette> vehicle_palette;
        TagReflexive<EndianType, ScenarioEquipment> equipment;
        TagReflexive<EndianType, ScenarioEquipmentPalette> equipment_palette;
        TagReflexive<EndianType, ScenarioWeapon> weapons;
        TagReflexive<EndianType, ScenarioWeaponPalette> weapon_palette;
        TagReflexive<EndianType, ScenarioDeviceGroup> device_groups;
        TagReflexive<EndianType, ScenarioMachine> machines;
        TagReflexive<EndianType, ScenarioMachinePalette> machine_palette;
        TagReflexive<EndianType, ScenarioControl> controls;
        TagReflexive<EndianType, ScenarioControlPalette> control_palette;
        TagReflexive<EndianType, ScenarioLightFixture> light_fixtures;
        TagReflexive<EndianType, ScenarioLightFixturePalette> light_fixture_palette;
        TagReflexive<EndianType, ScenarioSoundScenery> sound_scenery;
        TagReflexive<EndianType, ScenarioSoundSceneryPalette> sound_scenery_palette;
        PAD(0x54);
        TagReflexive<EndianType, ScenarioPlayerStartingProfile> player_starting_profile;
        TagReflexive<EndianType, ScenarioPlayerStartingLocation> player_starting_locations;
        TagReflexive<EndianType, ScenarioTriggerVolume> trigger_volumes;
        TagReflexive<EndianType, ScenarioRecordedAnimation> recorded_animations;
        TagReflexive<EndianType, ScenarioNetgameFlags> netgame_flags;
        TagReflexive<EndianType, ScenarioNetgameEquipment> netgame_equipment;
        TagReflexive<EndianType, ScenarioStartingEquipment> starting_equipment;
        TagReflexive<EndianType, ScenarioBSPSwitchTriggerVolume> bsp_switch_trigger_volumes;
        TagReflexive<EndianType, ScenarioDecal> decals;
        TagReflexive<EndianType, ScenarioDecalPallete> decal_palette;
        TagReflexive<EndianType, ScenarioDetailObjectCollectionPalette> detail_object_collection_palette;
        PAD(0x54);
        TagReflexive<EndianType, ScenarioActorPalette> actor_palette;
        TagReflexive<EndianType, ScenarioEncounter> encounters;
        TagReflexive<EndianType, ScenarioCommandList> command_lists;
        TagReflexive<EndianType, ScenarioAIAnimationReference> ai_animation_references;
        TagReflexive<EndianType, ScenarioAIScriptReference> ai_script_references;
        TagReflexive<EndianType, ScenarioAIRecordingReference> ai_recording_references;
        TagReflexive<EndianType, ScenarioAIConversation> ai_conversations;
        TagDataOffset<EndianType> script_syntax_data;
        TagDataOffset<EndianType> script_string_data;
        TagReflexive<EndianType, ScenarioScript> scripts;
        TagReflexive<EndianType, ScenarioGlobal> globals;
        TagReflexive<EndianType, ScenarioReference> references;
        TagReflexive<EndianType, ScenarioSourceFile> source_files;
        PAD(0x18);
        TagReflexive<EndianType, ScenarioCutsceneFlag> cutscene_flags;
        TagReflexive<EndianType, ScenarioCutsceneCameraPoint> cutscene_camera_points;
        TagReflexive<EndianType, ScenarioCutsceneTitle> cutscene_titles;
        PAD(0x6C);
        TagDependency<EndianType> custom_object_names; // unicode_string
        TagDependency<EndianType> ingame_help_text; // unicode_string
        TagDependency<EndianType> hud_messages; // hud_messages
        TagReflexive<EndianType, ScenarioBSP> structure_bsps;

        ENDIAN_TEMPLATE(NewEndian) operator Scenario<NewEndian>() const noexcept {
            Scenario<NewEndian> copy = {};
            COPY_THIS(don_t_use);
            COPY_THIS(won_t_use);
            COPY_THIS(can_t_use);
            COPY_THIS(skies);
            COPY_THIS(type);
            COPY_THIS(flags);
            COPY_THIS(child_scenarios);
            COPY_THIS(local_north);
            COPY_THIS(predicted_resources);
            COPY_THIS(functions);
            COPY_THIS(editor_scenario_data);
            COPY_THIS(comments);
            COPY_THIS(object_names);
            COPY_THIS(scenery);
            COPY_THIS(scenery_palette);
            COPY_THIS(bipeds);
            COPY_THIS(biped_palette);
            COPY_THIS(vehicles);
            COPY_THIS(vehicle_palette);
            COPY_THIS(equipment);
            COPY_THIS(equipment_palette);
            COPY_THIS(weapons);
            COPY_THIS(weapon_palette);
            COPY_THIS(device_groups);
            COPY_THIS(machines);
            COPY_THIS(machine_palette);
            COPY_THIS(controls);
            COPY_THIS(control_palette);
            COPY_THIS(light_fixtures);
            COPY_THIS(light_fixture_palette);
            COPY_THIS(sound_scenery);
            COPY_THIS(sound_scenery_palette);
            COPY_THIS(player_starting_profile);
            COPY_THIS(player_starting_locations);
            COPY_THIS(trigger_volumes);
            COPY_THIS(recorded_animations);
            COPY_THIS(netgame_flags);
            COPY_THIS(netgame_equipment);
            COPY_THIS(starting_equipment);
            COPY_THIS(bsp_switch_trigger_volumes);
            COPY_THIS(decals);
            COPY_THIS(decal_palette);
            COPY_THIS(detail_object_collection_palette);
            COPY_THIS(actor_palette);
            COPY_THIS(encounters);
            COPY_THIS(command_lists);
            COPY_THIS(ai_animation_references);
            COPY_THIS(ai_script_references);
            COPY_THIS(ai_recording_references);
            COPY_THIS(ai_conversations);
            COPY_THIS(script_syntax_data);
            COPY_THIS(script_string_data);
            COPY_THIS(scripts);
            COPY_THIS(globals);
            COPY_THIS(references);
            COPY_THIS(source_files);
            COPY_THIS(cutscene_flags);
            COPY_THIS(cutscene_camera_points);
            COPY_THIS(cutscene_titles);
            COPY_THIS(custom_object_names);
            COPY_THIS(ingame_help_text);
            COPY_THIS(hud_messages);
            COPY_THIS(structure_bsps);
            return copy;
        }
    };
    static_assert(sizeof(Scenario<BigEndian>) == 0x5B0);

    void compile_scenario_tag(CompiledTag &compiled, const std::byte *data, std::size_t size);
}
#endif
