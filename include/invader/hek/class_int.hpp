// SPDX-License-Identifier: GPL-3.0-only

#ifndef INVADER__HEK__CLASS_INT_HPP
#define INVADER__HEK__CLASS_INT_HPP

#include <cstdint>

namespace Invader::HEK {
    enum TagClassInt : std::uint32_t {
        TAG_CLASS_ACTOR = 0x61637472,
        TAG_CLASS_ACTOR_VARIANT = 0x61637476,
        TAG_CLASS_ANTENNA = 0x616E7421,
        TAG_CLASS_MODEL_ANIMATIONS = 0x616E7472,
        TAG_CLASS_BIPED = 0x62697064,
        TAG_CLASS_BITMAP = 0x6269746D,
        TAG_CLASS_SPHEROID = 0x626F6F6D,
        TAG_CLASS_CONTINUOUS_DAMAGE_EFFECT = 0x63646D67,
        TAG_CLASS_MODEL_COLLISION_GEOMETRY = 0x636F6C6C,
        TAG_CLASS_COLOR_TABLE = 0x636F6C6F,
        TAG_CLASS_CONTRAIL = 0x636F6E74,
        TAG_CLASS_DEVICE_CONTROL = 0x6374726C,
        TAG_CLASS_DECAL = 0x64656361,
        TAG_CLASS_UI_WIDGET_DEFINITION = 0x44654C61,
        TAG_CLASS_INPUT_DEVICE_DEFAULTS = 0x64657663,
        TAG_CLASS_DEVICE = 0x64657669,
        TAG_CLASS_DETAIL_OBJECT_COLLECTION = 0x646F6263,
        TAG_CLASS_EFFECT = 0x65666665,
        TAG_CLASS_EQUIPMENT = 0x65716970,
        TAG_CLASS_FLAG = 0x666C6167,
        TAG_CLASS_FOG = 0x666F6720,
        TAG_CLASS_FONT = 0x666F6E74,
        TAG_CLASS_MATERIAL_EFFECTS = 0x666F6F74,
        TAG_CLASS_GARBAGE = 0x67617262,
        TAG_CLASS_GLOW = 0x676C7721,
        TAG_CLASS_GRENADE_HUD_INTERFACE = 0x67726869,
        TAG_CLASS_HUD_MESSAGE_TEXT = 0x686D7420,
        TAG_CLASS_HUD_NUMBER = 0x68756423,
        TAG_CLASS_HUD_GLOBALS = 0x68756467,
        TAG_CLASS_ITEM = 0x6974656D,
        TAG_CLASS_ITEM_COLLECTION = 0x69746D63,
        TAG_CLASS_DAMAGE_EFFECT = 0x6A707421,
        TAG_CLASS_LENS_FLARE = 0x6C656E73,
        TAG_CLASS_LIGHTNING = 0x656C6563,
        TAG_CLASS_DEVICE_LIGHT_FIXTURE = 0x6C696669,
        TAG_CLASS_LIGHT = 0x6C696768,
        TAG_CLASS_SOUND_LOOPING = 0x6C736E64,
        TAG_CLASS_DEVICE_MACHINE = 0x6D616368,
        TAG_CLASS_GLOBALS = 0x6D617467,
        TAG_CLASS_METER = 0x6D657472,
        TAG_CLASS_LIGHT_VOLUME = 0x6D677332,
        TAG_CLASS_GBXMODEL = 0x6D6F6432,
        TAG_CLASS_MODEL = 0x6D6F6465,
        TAG_CLASS_MULTIPLAYER_SCENARIO_DESCRIPTION = 0x6D706C79,
        TAG_CLASS_PREFERENCES_NETWORK_GAME = 0x6E677072,
        TAG_CLASS_NONE = ~static_cast<std::uint32_t>(0),
        TAG_CLASS_NULL = 0,
        TAG_CLASS_OBJECT = 0x6F626A65,
        TAG_CLASS_PARTICLE = 0x70617274,
        TAG_CLASS_PARTICLE_SYSTEM = 0x7063746C,
        TAG_CLASS_PHYSICS = 0x70687973,
        TAG_CLASS_PLACEHOLDER = 0x706C6163,
        TAG_CLASS_POINT_PHYSICS = 0x70706879,
        TAG_CLASS_PROJECTILE = 0x70726F6A,
        TAG_CLASS_WEATHER_PARTICLE_SYSTEM = 0x7261696E,
        TAG_CLASS_SCENARIO_STRUCTURE_BSP = 0x73627370,
        TAG_CLASS_SCENERY = 0x7363656E,
        TAG_CLASS_SHADER_TRANSPARENT_CHICAGO_EXTENDED = 0x73636578,
        TAG_CLASS_SHADER_TRANSPARENT_CHICAGO = 0x73636869,
        TAG_CLASS_SCENARIO = 0x73636E72,
        TAG_CLASS_SHADER_ENVIRONMENT = 0x73656E76,
        TAG_CLASS_SHADER_TRANSPARENT_GLASS = 0x73676C61,
        TAG_CLASS_SHADER = 0x73686472,
        TAG_CLASS_SKY = 0x736B7920,
        TAG_CLASS_SHADER_TRANSPARENT_METER = 0x736D6574,
        TAG_CLASS_SOUND = 0x736E6421,
        TAG_CLASS_SOUND_ENVIRONMENT = 0x736E6465,
        TAG_CLASS_SHADER_MODEL = 0x736F736F,
        TAG_CLASS_SHADER_TRANSPARENT_GENERIC = 0x736F7472,
        TAG_CLASS_UI_WIDGET_COLLECTION = 0x536F756C,
        TAG_CLASS_SHADER_TRANSPARENT_PLASMA = 0x73706C61,
        TAG_CLASS_SOUND_SCENERY = 0x73736365,
        TAG_CLASS_STRING_LIST = 0x73747223,
        TAG_CLASS_SHADER_TRANSPARENT_WATER = 0x73776174,
        TAG_CLASS_TAG_COLLECTION = 0x74616763,
        TAG_CLASS_CAMERA_TRACK = 0x7472616B,
        TAG_CLASS_DIALOGUE = 0x75646C67,
        TAG_CLASS_UNIT_HUD_INTERFACE = 0x756E6869,
        TAG_CLASS_UNIT = 0x756E6974,
        TAG_CLASS_UNICODE_STRING_LIST = 0x75737472,
        TAG_CLASS_VIRTUAL_KEYBOARD = 0x76636B79,
        TAG_CLASS_VEHICLE = 0x76656869,
        TAG_CLASS_WEAPON = 0x77656170,
        TAG_CLASS_WIND = 0x77696E64,
        TAG_CLASS_WEAPON_HUD_INTERFACE = 0x77706869
    };

    /**
     * Check if the tag class int is an object-type class
     * @param  tag_class_int the tag class int
     * @return               true if an object, false if not
     */
    #define IS_OBJECT_TAG(tag_class_int) (tag_class_int == TagClassInt::TAG_CLASS_BIPED || \
                                          tag_class_int == TagClassInt::TAG_CLASS_DEVICE || \
                                          tag_class_int == TagClassInt::TAG_CLASS_DEVICE_CONTROL || \
                                          tag_class_int == TagClassInt::TAG_CLASS_DEVICE_LIGHT_FIXTURE || \
                                          tag_class_int == TagClassInt::TAG_CLASS_DEVICE_MACHINE || \
                                          tag_class_int == TagClassInt::TAG_CLASS_EQUIPMENT || \
                                          tag_class_int == TagClassInt::TAG_CLASS_GARBAGE || \
                                          tag_class_int == TagClassInt::TAG_CLASS_ITEM || \
                                          tag_class_int == TagClassInt::TAG_CLASS_OBJECT || \
                                          tag_class_int == TagClassInt::TAG_CLASS_PLACEHOLDER || \
                                          tag_class_int == TagClassInt::TAG_CLASS_PROJECTILE || \
                                          tag_class_int == TagClassInt::TAG_CLASS_SCENERY || \
                                          tag_class_int == TagClassInt::TAG_CLASS_SOUND_SCENERY || \
                                          tag_class_int == TagClassInt::TAG_CLASS_VEHICLE || \
                                          tag_class_int == TagClassInt::TAG_CLASS_WEAPON)

    /**
     * Return the extension of the tag class or nullptr if none exists.
     * @param  tag_class This is the tag class being looked up.
     * @return           This is a C string containing the extension.
     */
     constexpr const char *tag_class_to_extension(TagClassInt tag_class) noexcept {
         switch(tag_class) {
         case TagClassInt::TAG_CLASS_ACTOR:
             return "actor";
         case TagClassInt::TAG_CLASS_ACTOR_VARIANT:
             return "actor_variant";
         case TagClassInt::TAG_CLASS_ANTENNA:
             return "antenna";
         case TagClassInt::TAG_CLASS_MODEL_ANIMATIONS:
             return "model_animations";
         case TagClassInt::TAG_CLASS_BIPED:
             return "biped";
         case TagClassInt::TAG_CLASS_BITMAP:
             return "bitmap";
         case TagClassInt::TAG_CLASS_SPHEROID:
             return "spheroid";
         case TagClassInt::TAG_CLASS_CONTINUOUS_DAMAGE_EFFECT:
             return "continuous_damage_effect";
         case TagClassInt::TAG_CLASS_MODEL_COLLISION_GEOMETRY:
             return "model_collision_geometry";
         case TagClassInt::TAG_CLASS_COLOR_TABLE:
             return "color_table";
         case TagClassInt::TAG_CLASS_CONTRAIL:
             return "contrail";
         case TagClassInt::TAG_CLASS_DEVICE_CONTROL:
             return "device_control";
         case TagClassInt::TAG_CLASS_DECAL:
             return "decal";
         case TagClassInt::TAG_CLASS_UI_WIDGET_DEFINITION:
             return "ui_widget_definition";
         case TagClassInt::TAG_CLASS_INPUT_DEVICE_DEFAULTS:
             return "input_device_defaults";
         case TagClassInt::TAG_CLASS_DEVICE:
             return "device";
         case TagClassInt::TAG_CLASS_DETAIL_OBJECT_COLLECTION:
             return "detail_object_collection";
         case TagClassInt::TAG_CLASS_EFFECT:
             return "effect";
         case TagClassInt::TAG_CLASS_EQUIPMENT:
             return "equipment";
         case TagClassInt::TAG_CLASS_FLAG:
             return "flag";
         case TagClassInt::TAG_CLASS_FOG:
             return "fog";
         case TagClassInt::TAG_CLASS_FONT:
             return "font";
         case TagClassInt::TAG_CLASS_MATERIAL_EFFECTS:
             return "material_effects";
         case TagClassInt::TAG_CLASS_GARBAGE:
             return "garbage";
         case TagClassInt::TAG_CLASS_GLOW:
             return "glow";
         case TagClassInt::TAG_CLASS_GRENADE_HUD_INTERFACE:
             return "grenade_hud_interface";
         case TagClassInt::TAG_CLASS_HUD_MESSAGE_TEXT:
             return "hud_message_text";
         case TagClassInt::TAG_CLASS_HUD_NUMBER:
             return "hud_number";
         case TagClassInt::TAG_CLASS_HUD_GLOBALS:
             return "hud_globals";
         case TagClassInt::TAG_CLASS_ITEM:
             return "item";
         case TagClassInt::TAG_CLASS_ITEM_COLLECTION:
             return "item_collection";
         case TagClassInt::TAG_CLASS_DAMAGE_EFFECT:
             return "damage_effect";
         case TagClassInt::TAG_CLASS_LENS_FLARE:
             return "lens_flare";
         case TagClassInt::TAG_CLASS_LIGHTNING:
             return "lightning";
         case TagClassInt::TAG_CLASS_DEVICE_LIGHT_FIXTURE:
             return "device_light_fixture";
         case TagClassInt::TAG_CLASS_LIGHT:
             return "light";
         case TagClassInt::TAG_CLASS_SOUND_LOOPING:
             return "sound_looping";
         case TagClassInt::TAG_CLASS_DEVICE_MACHINE:
             return "device_machine";
         case TagClassInt::TAG_CLASS_GLOBALS:
             return "globals";
         case TagClassInt::TAG_CLASS_METER:
             return "meter";
         case TagClassInt::TAG_CLASS_LIGHT_VOLUME:
             return "light_volume";
         case TagClassInt::TAG_CLASS_GBXMODEL:
             return "gbxmodel";
         case TagClassInt::TAG_CLASS_MODEL:
             return "model";
         case TagClassInt::TAG_CLASS_MULTIPLAYER_SCENARIO_DESCRIPTION:
             return "multiplayer_scenario_description";
         case TagClassInt::TAG_CLASS_PREFERENCES_NETWORK_GAME:
             return "preferences_network_game";
         case TagClassInt::TAG_CLASS_NONE:
             return "none";
         case TagClassInt::TAG_CLASS_OBJECT:
             return "object";
         case TagClassInt::TAG_CLASS_PARTICLE:
             return "particle";
         case TagClassInt::TAG_CLASS_PARTICLE_SYSTEM:
             return "particle_system";
         case TagClassInt::TAG_CLASS_PHYSICS:
             return "physics";
         case TagClassInt::TAG_CLASS_PLACEHOLDER:
             return "placeholder";
         case TagClassInt::TAG_CLASS_POINT_PHYSICS:
             return "point_physics";
         case TagClassInt::TAG_CLASS_PROJECTILE:
             return "projectile";
         case TagClassInt::TAG_CLASS_WEATHER_PARTICLE_SYSTEM:
             return "weather_particle_system";
         case TagClassInt::TAG_CLASS_SCENARIO_STRUCTURE_BSP:
             return "scenario_structure_bsp";
         case TagClassInt::TAG_CLASS_SCENERY:
             return "scenery";
         case TagClassInt::TAG_CLASS_SHADER_TRANSPARENT_CHICAGO_EXTENDED:
             return "shader_transparent_chicago_extended";
         case TagClassInt::TAG_CLASS_SHADER_TRANSPARENT_CHICAGO:
             return "shader_transparent_chicago";
         case TagClassInt::TAG_CLASS_SCENARIO:
             return "scenario";
         case TagClassInt::TAG_CLASS_SHADER_ENVIRONMENT:
             return "shader_environment";
         case TagClassInt::TAG_CLASS_SHADER_TRANSPARENT_GLASS:
             return "shader_transparent_glass";
         case TagClassInt::TAG_CLASS_SHADER:
             return "shader";
         case TagClassInt::TAG_CLASS_SKY:
             return "sky";
         case TagClassInt::TAG_CLASS_SHADER_TRANSPARENT_METER:
             return "shader_transparent_meter";
         case TagClassInt::TAG_CLASS_SOUND:
             return "sound";
         case TagClassInt::TAG_CLASS_SOUND_ENVIRONMENT:
             return "sound_environment";
         case TagClassInt::TAG_CLASS_SHADER_MODEL:
             return "shader_model";
         case TagClassInt::TAG_CLASS_SHADER_TRANSPARENT_GENERIC:
             return "shader_transparent_generic";
         case TagClassInt::TAG_CLASS_UI_WIDGET_COLLECTION:
             return "ui_widget_collection";
         case TagClassInt::TAG_CLASS_SHADER_TRANSPARENT_PLASMA:
             return "shader_transparent_plasma";
         case TagClassInt::TAG_CLASS_SOUND_SCENERY:
             return "sound_scenery";
         case TagClassInt::TAG_CLASS_STRING_LIST:
             return "string_list";
         case TagClassInt::TAG_CLASS_SHADER_TRANSPARENT_WATER:
             return "shader_transparent_water";
         case TagClassInt::TAG_CLASS_TAG_COLLECTION:
             return "tag_collection";
         case TagClassInt::TAG_CLASS_CAMERA_TRACK:
             return "camera_track";
         case TagClassInt::TAG_CLASS_DIALOGUE:
             return "dialogue";
         case TagClassInt::TAG_CLASS_UNIT_HUD_INTERFACE:
             return "unit_hud_interface";
         case TagClassInt::TAG_CLASS_UNIT:
             return "unit";
         case TagClassInt::TAG_CLASS_UNICODE_STRING_LIST:
             return "unicode_string_list";
         case TagClassInt::TAG_CLASS_VIRTUAL_KEYBOARD:
             return "virtual_keyboard";
         case TagClassInt::TAG_CLASS_VEHICLE:
             return "vehicle";
         case TagClassInt::TAG_CLASS_WEAPON:
             return "weapon";
         case TagClassInt::TAG_CLASS_WIND:
             return "wind";
         case TagClassInt::TAG_CLASS_WEAPON_HUD_INTERFACE:
             return "weapon_hud_interface";
         default:
             return "unknown";
         }
     }

    /**
     * Return the tag class of the extension or TagClass::TAG_CLASS_NULL if failed or extension is nullptr.
     * @param  extension This is the extension being looked up.
     * @return           This is the equivalent tag class int of the extesion or TagClass::TAG_CLASS_NULL if failed.
     */
    TagClassInt extension_to_tag_class(const char *extension) noexcept;
}

namespace Invader {
    using TagClassInt = HEK::TagClassInt;
}

#endif
