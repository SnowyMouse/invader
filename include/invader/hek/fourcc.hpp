// SPDX-License-Identifier: GPL-3.0-only

#ifndef INVADER__HEK__FOURCC_HPP
#define INVADER__HEK__FOURCC_HPP

#include <cstdint>

namespace Invader::HEK {
    enum TagFourCC : std::uint32_t {
        TAG_FOURCC_ACTOR = 0x61637472,
        TAG_FOURCC_ACTOR_VARIANT = 0x61637476,
        TAG_FOURCC_ANTENNA = 0x616E7421,
        TAG_FOURCC_MODEL_ANIMATIONS = 0x616E7472,
        TAG_FOURCC_BIPED = 0x62697064,
        TAG_FOURCC_BITMAP = 0x6269746D,
        TAG_FOURCC_SPHEROID = 0x626F6F6D,
        TAG_FOURCC_CONTINUOUS_DAMAGE_EFFECT = 0x63646D67,
        TAG_FOURCC_MODEL_COLLISION_GEOMETRY = 0x636F6C6C,
        TAG_FOURCC_COLOR_TABLE = 0x636F6C6F,
        TAG_FOURCC_CONTRAIL = 0x636F6E74,
        TAG_FOURCC_DEVICE_CONTROL = 0x6374726C,
        TAG_FOURCC_DECAL = 0x64656361,
        TAG_FOURCC_UI_WIDGET_DEFINITION = 0x44654C61,
        TAG_FOURCC_INPUT_DEVICE_DEFAULTS = 0x64657663,
        TAG_FOURCC_DEVICE = 0x64657669,
        TAG_FOURCC_DETAIL_OBJECT_COLLECTION = 0x646F6263,
        TAG_FOURCC_EFFECT = 0x65666665,
        TAG_FOURCC_EQUIPMENT = 0x65716970,
        TAG_FOURCC_FLAG = 0x666C6167,
        TAG_FOURCC_FOG = 0x666F6720,
        TAG_FOURCC_FONT = 0x666F6E74,
        TAG_FOURCC_MATERIAL_EFFECTS = 0x666F6F74,
        TAG_FOURCC_GARBAGE = 0x67617262,
        TAG_FOURCC_GLOW = 0x676C7721,
        TAG_FOURCC_GRENADE_HUD_INTERFACE = 0x67726869,
        TAG_FOURCC_HUD_MESSAGE_TEXT = 0x686D7420,
        TAG_FOURCC_HUD_NUMBER = 0x68756423,
        TAG_FOURCC_HUD_GLOBALS = 0x68756467,
        TAG_FOURCC_ITEM = 0x6974656D,
        TAG_FOURCC_ITEM_COLLECTION = 0x69746D63,
        TAG_FOURCC_DAMAGE_EFFECT = 0x6A707421,
        TAG_FOURCC_LENS_FLARE = 0x6C656E73,
        TAG_FOURCC_LIGHTNING = 0x656C6563,
        TAG_FOURCC_DEVICE_LIGHT_FIXTURE = 0x6C696669,
        TAG_FOURCC_LIGHT = 0x6C696768,
        TAG_FOURCC_SOUND_LOOPING = 0x6C736E64,
        TAG_FOURCC_DEVICE_MACHINE = 0x6D616368,
        TAG_FOURCC_GLOBALS = 0x6D617467,
        TAG_FOURCC_METER = 0x6D657472,
        TAG_FOURCC_LIGHT_VOLUME = 0x6D677332,
        TAG_FOURCC_GBXMODEL = 0x6D6F6432,
        TAG_FOURCC_MODEL = 0x6D6F6465,
        TAG_FOURCC_MULTIPLAYER_SCENARIO_DESCRIPTION = 0x6D706C79,
        TAG_FOURCC_PREFERENCES_NETWORK_GAME = 0x6E677072,
        TAG_FOURCC_NONE = ~static_cast<std::uint32_t>(0),
        TAG_FOURCC_NULL = 0,
        TAG_FOURCC_OBJECT = 0x6F626A65,
        TAG_FOURCC_PARTICLE = 0x70617274,
        TAG_FOURCC_PARTICLE_SYSTEM = 0x7063746C,
        TAG_FOURCC_PHYSICS = 0x70687973,
        TAG_FOURCC_PLACEHOLDER = 0x706C6163,
        TAG_FOURCC_POINT_PHYSICS = 0x70706879,
        TAG_FOURCC_PROJECTILE = 0x70726F6A,
        TAG_FOURCC_WEATHER_PARTICLE_SYSTEM = 0x7261696E,
        TAG_FOURCC_SCENARIO_STRUCTURE_BSP = 0x73627370,
        TAG_FOURCC_SCENERY = 0x7363656E,
        TAG_FOURCC_SHADER_TRANSPARENT_CHICAGO_EXTENDED = 0x73636578,
        TAG_FOURCC_SHADER_TRANSPARENT_CHICAGO = 0x73636869,
        TAG_FOURCC_SCENARIO = 0x73636E72,
        TAG_FOURCC_SHADER_ENVIRONMENT = 0x73656E76,
        TAG_FOURCC_SHADER_TRANSPARENT_GLASS = 0x73676C61,
        TAG_FOURCC_SHADER = 0x73686472,
        TAG_FOURCC_SKY = 0x736B7920,
        TAG_FOURCC_SHADER_TRANSPARENT_METER = 0x736D6574,
        TAG_FOURCC_SOUND = 0x736E6421,
        TAG_FOURCC_SOUND_ENVIRONMENT = 0x736E6465,
        TAG_FOURCC_SHADER_MODEL = 0x736F736F,
        TAG_FOURCC_SHADER_TRANSPARENT_GENERIC = 0x736F7472,
        TAG_FOURCC_UI_WIDGET_COLLECTION = 0x536F756C,
        TAG_FOURCC_SHADER_TRANSPARENT_PLASMA = 0x73706C61,
        TAG_FOURCC_SOUND_SCENERY = 0x73736365,
        TAG_FOURCC_STRING_LIST = 0x73747223,
        TAG_FOURCC_SHADER_TRANSPARENT_WATER = 0x73776174,
        TAG_FOURCC_TAG_COLLECTION = 0x74616763,
        TAG_FOURCC_CAMERA_TRACK = 0x7472616B,
        TAG_FOURCC_DIALOGUE = 0x75646C67,
        TAG_FOURCC_UNIT_HUD_INTERFACE = 0x756E6869,
        TAG_FOURCC_UNIT = 0x756E6974,
        TAG_FOURCC_UNICODE_STRING_LIST = 0x75737472,
        TAG_FOURCC_VIRTUAL_KEYBOARD = 0x76636B79,
        TAG_FOURCC_VEHICLE = 0x76656869,
        TAG_FOURCC_WEAPON = 0x77656170,
        TAG_FOURCC_WIND = 0x77696E64,
        TAG_FOURCC_WEAPON_HUD_INTERFACE = 0x77706869
    };

    /**
     * Check if the tag class int is an object-type class
     * @param  tag_fourcc the tag class int
     * @return               true if an object, false if not
     */
    #define IS_OBJECT_TAG(tag_fourcc) (tag_fourcc == TagFourCC::TAG_FOURCC_BIPED || \
                                       tag_fourcc == TagFourCC::TAG_FOURCC_DEVICE || \
                                       tag_fourcc == TagFourCC::TAG_FOURCC_DEVICE_CONTROL || \
                                       tag_fourcc == TagFourCC::TAG_FOURCC_DEVICE_LIGHT_FIXTURE || \
                                       tag_fourcc == TagFourCC::TAG_FOURCC_DEVICE_MACHINE || \
                                       tag_fourcc == TagFourCC::TAG_FOURCC_EQUIPMENT || \
                                       tag_fourcc == TagFourCC::TAG_FOURCC_GARBAGE || \
                                       tag_fourcc == TagFourCC::TAG_FOURCC_ITEM || \
                                       tag_fourcc == TagFourCC::TAG_FOURCC_OBJECT || \
                                       tag_fourcc == TagFourCC::TAG_FOURCC_PLACEHOLDER || \
                                       tag_fourcc == TagFourCC::TAG_FOURCC_PROJECTILE || \
                                       tag_fourcc == TagFourCC::TAG_FOURCC_SCENERY || \
                                       tag_fourcc == TagFourCC::TAG_FOURCC_SOUND_SCENERY || \
                                       tag_fourcc == TagFourCC::TAG_FOURCC_VEHICLE || \
                                       tag_fourcc == TagFourCC::TAG_FOURCC_WEAPON)

    /**
     * Check if the tag class int is an shader-type class
     * @param  tag_fourcc the tag class int
     * @return               true if an shader, false if not
     */
    #define IS_SHADER_TAG(tag_fourcc) (tag_fourcc == TagFourCC::TAG_FOURCC_SHADER || \
                                       tag_fourcc == TagFourCC::TAG_FOURCC_SHADER_ENVIRONMENT || \
                                       tag_fourcc == TagFourCC::TAG_FOURCC_SHADER_MODEL || \
                                       tag_fourcc == TagFourCC::TAG_FOURCC_SHADER_TRANSPARENT_CHICAGO || \
                                       tag_fourcc == TagFourCC::TAG_FOURCC_SHADER_TRANSPARENT_CHICAGO_EXTENDED || \
                                       tag_fourcc == TagFourCC::TAG_FOURCC_SHADER_TRANSPARENT_GENERIC || \
                                       tag_fourcc == TagFourCC::TAG_FOURCC_SHADER_TRANSPARENT_GLASS || \
                                       tag_fourcc == TagFourCC::TAG_FOURCC_SHADER_TRANSPARENT_METER || \
                                       tag_fourcc == TagFourCC::TAG_FOURCC_SHADER_TRANSPARENT_PLASMA || \
                                       tag_fourcc == TagFourCC::TAG_FOURCC_SHADER_TRANSPARENT_WATER)

    /**
     * Return the extension of the tag class or nullptr if none exists.
     * @param  tag_class This is the tag class being looked up.
     * @return           This is a C string containing the extension.
     */
     constexpr const char *tag_fourcc_to_extension(TagFourCC tag_class) noexcept {
         switch(tag_class) {
         case TagFourCC::TAG_FOURCC_ACTOR:
             return "actor";
         case TagFourCC::TAG_FOURCC_ACTOR_VARIANT:
             return "actor_variant";
         case TagFourCC::TAG_FOURCC_ANTENNA:
             return "antenna";
         case TagFourCC::TAG_FOURCC_MODEL_ANIMATIONS:
             return "model_animations";
         case TagFourCC::TAG_FOURCC_BIPED:
             return "biped";
         case TagFourCC::TAG_FOURCC_BITMAP:
             return "bitmap";
         case TagFourCC::TAG_FOURCC_SPHEROID:
             return "spheroid";
         case TagFourCC::TAG_FOURCC_CONTINUOUS_DAMAGE_EFFECT:
             return "continuous_damage_effect";
         case TagFourCC::TAG_FOURCC_MODEL_COLLISION_GEOMETRY:
             return "model_collision_geometry";
         case TagFourCC::TAG_FOURCC_COLOR_TABLE:
             return "color_table";
         case TagFourCC::TAG_FOURCC_CONTRAIL:
             return "contrail";
         case TagFourCC::TAG_FOURCC_DEVICE_CONTROL:
             return "device_control";
         case TagFourCC::TAG_FOURCC_DECAL:
             return "decal";
         case TagFourCC::TAG_FOURCC_UI_WIDGET_DEFINITION:
             return "ui_widget_definition";
         case TagFourCC::TAG_FOURCC_INPUT_DEVICE_DEFAULTS:
             return "input_device_defaults";
         case TagFourCC::TAG_FOURCC_DEVICE:
             return "device";
         case TagFourCC::TAG_FOURCC_DETAIL_OBJECT_COLLECTION:
             return "detail_object_collection";
         case TagFourCC::TAG_FOURCC_EFFECT:
             return "effect";
         case TagFourCC::TAG_FOURCC_EQUIPMENT:
             return "equipment";
         case TagFourCC::TAG_FOURCC_FLAG:
             return "flag";
         case TagFourCC::TAG_FOURCC_FOG:
             return "fog";
         case TagFourCC::TAG_FOURCC_FONT:
             return "font";
         case TagFourCC::TAG_FOURCC_MATERIAL_EFFECTS:
             return "material_effects";
         case TagFourCC::TAG_FOURCC_GARBAGE:
             return "garbage";
         case TagFourCC::TAG_FOURCC_GLOW:
             return "glow";
         case TagFourCC::TAG_FOURCC_GRENADE_HUD_INTERFACE:
             return "grenade_hud_interface";
         case TagFourCC::TAG_FOURCC_HUD_MESSAGE_TEXT:
             return "hud_message_text";
         case TagFourCC::TAG_FOURCC_HUD_NUMBER:
             return "hud_number";
         case TagFourCC::TAG_FOURCC_HUD_GLOBALS:
             return "hud_globals";
         case TagFourCC::TAG_FOURCC_ITEM:
             return "item";
         case TagFourCC::TAG_FOURCC_ITEM_COLLECTION:
             return "item_collection";
         case TagFourCC::TAG_FOURCC_DAMAGE_EFFECT:
             return "damage_effect";
         case TagFourCC::TAG_FOURCC_LENS_FLARE:
             return "lens_flare";
         case TagFourCC::TAG_FOURCC_LIGHTNING:
             return "lightning";
         case TagFourCC::TAG_FOURCC_DEVICE_LIGHT_FIXTURE:
             return "device_light_fixture";
         case TagFourCC::TAG_FOURCC_LIGHT:
             return "light";
         case TagFourCC::TAG_FOURCC_SOUND_LOOPING:
             return "sound_looping";
         case TagFourCC::TAG_FOURCC_DEVICE_MACHINE:
             return "device_machine";
         case TagFourCC::TAG_FOURCC_GLOBALS:
             return "globals";
         case TagFourCC::TAG_FOURCC_METER:
             return "meter";
         case TagFourCC::TAG_FOURCC_LIGHT_VOLUME:
             return "light_volume";
         case TagFourCC::TAG_FOURCC_GBXMODEL:
             return "gbxmodel";
         case TagFourCC::TAG_FOURCC_MODEL:
             return "model";
         case TagFourCC::TAG_FOURCC_MULTIPLAYER_SCENARIO_DESCRIPTION:
             return "multiplayer_scenario_description";
         case TagFourCC::TAG_FOURCC_PREFERENCES_NETWORK_GAME:
             return "preferences_network_game";
         case TagFourCC::TAG_FOURCC_NONE:
             return "none";
         case TagFourCC::TAG_FOURCC_OBJECT:
             return "object";
         case TagFourCC::TAG_FOURCC_PARTICLE:
             return "particle";
         case TagFourCC::TAG_FOURCC_PARTICLE_SYSTEM:
             return "particle_system";
         case TagFourCC::TAG_FOURCC_PHYSICS:
             return "physics";
         case TagFourCC::TAG_FOURCC_PLACEHOLDER:
             return "placeholder";
         case TagFourCC::TAG_FOURCC_POINT_PHYSICS:
             return "point_physics";
         case TagFourCC::TAG_FOURCC_PROJECTILE:
             return "projectile";
         case TagFourCC::TAG_FOURCC_WEATHER_PARTICLE_SYSTEM:
             return "weather_particle_system";
         case TagFourCC::TAG_FOURCC_SCENARIO_STRUCTURE_BSP:
             return "scenario_structure_bsp";
         case TagFourCC::TAG_FOURCC_SCENERY:
             return "scenery";
         case TagFourCC::TAG_FOURCC_SHADER_TRANSPARENT_CHICAGO_EXTENDED:
             return "shader_transparent_chicago_extended";
         case TagFourCC::TAG_FOURCC_SHADER_TRANSPARENT_CHICAGO:
             return "shader_transparent_chicago";
         case TagFourCC::TAG_FOURCC_SCENARIO:
             return "scenario";
         case TagFourCC::TAG_FOURCC_SHADER_ENVIRONMENT:
             return "shader_environment";
         case TagFourCC::TAG_FOURCC_SHADER_TRANSPARENT_GLASS:
             return "shader_transparent_glass";
         case TagFourCC::TAG_FOURCC_SHADER:
             return "shader";
         case TagFourCC::TAG_FOURCC_SKY:
             return "sky";
         case TagFourCC::TAG_FOURCC_SHADER_TRANSPARENT_METER:
             return "shader_transparent_meter";
         case TagFourCC::TAG_FOURCC_SOUND:
             return "sound";
         case TagFourCC::TAG_FOURCC_SOUND_ENVIRONMENT:
             return "sound_environment";
         case TagFourCC::TAG_FOURCC_SHADER_MODEL:
             return "shader_model";
         case TagFourCC::TAG_FOURCC_SHADER_TRANSPARENT_GENERIC:
             return "shader_transparent_generic";
         case TagFourCC::TAG_FOURCC_UI_WIDGET_COLLECTION:
             return "ui_widget_collection";
         case TagFourCC::TAG_FOURCC_SHADER_TRANSPARENT_PLASMA:
             return "shader_transparent_plasma";
         case TagFourCC::TAG_FOURCC_SOUND_SCENERY:
             return "sound_scenery";
         case TagFourCC::TAG_FOURCC_STRING_LIST:
             return "string_list";
         case TagFourCC::TAG_FOURCC_SHADER_TRANSPARENT_WATER:
             return "shader_transparent_water";
         case TagFourCC::TAG_FOURCC_TAG_COLLECTION:
             return "tag_collection";
         case TagFourCC::TAG_FOURCC_CAMERA_TRACK:
             return "camera_track";
         case TagFourCC::TAG_FOURCC_DIALOGUE:
             return "dialogue";
         case TagFourCC::TAG_FOURCC_UNIT_HUD_INTERFACE:
             return "unit_hud_interface";
         case TagFourCC::TAG_FOURCC_UNIT:
             return "unit";
         case TagFourCC::TAG_FOURCC_UNICODE_STRING_LIST:
             return "unicode_string_list";
         case TagFourCC::TAG_FOURCC_VIRTUAL_KEYBOARD:
             return "virtual_keyboard";
         case TagFourCC::TAG_FOURCC_VEHICLE:
             return "vehicle";
         case TagFourCC::TAG_FOURCC_WEAPON:
             return "weapon";
         case TagFourCC::TAG_FOURCC_WIND:
             return "wind";
         case TagFourCC::TAG_FOURCC_WEAPON_HUD_INTERFACE:
             return "weapon_hud_interface";
         case TagFourCC::TAG_FOURCC_NULL:
             break;
         }
         return "unknown";
     }

    /**
     * Return the tag class of the extension or TagClass::TAG_FOURCC_NULL if failed or extension is nullptr.
     * @param  extension This is the extension being looked up.
     * @return           This is the equivalent tag class int of the extension or TagClass::TAG_FOURCC_NULL if failed.
     */
    TagFourCC tag_extension_to_fourcc(const char *extension) noexcept;
}

namespace Invader {
    using TagFourCC = HEK::TagFourCC;
}

#endif
