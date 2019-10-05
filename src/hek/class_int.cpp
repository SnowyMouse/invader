// SPDX-License-Identifier: GPL-3.0-only

#include <cstring>

#include "class_int.hpp"

namespace Invader::HEK {
    /**
     * Convert a tag class integer to its respective extension
     * @param  tag_class tag class integer
     * @return           string representation of the tag class integer or "unknown" if none
     */
    const char *tag_class_to_extension(TagClassInt tag_class) noexcept {
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

    // Check the first byte. If they're equal, do strcmp. Otherwise it's not the same. This is a little faster than just doing strcmp.
    #define EQUAL(a, b) a[0] == b[0] ? std::strcmp(a, b) == 0 : false

    /**
     * Convert a string extension to its respective tag class integer
     * @param  extension string name of the tag class
     * @return           respective tag class integer or TAG_CLASS_NULL if nullptr was given or the extension was not valid
     */
    TagClassInt extension_to_tag_class(const char *extension) noexcept {
        if(extension == nullptr) {
            return TagClassInt::TAG_CLASS_NULL;
        }
        if(EQUAL(extension,"actor")) {
            return TagClassInt::TAG_CLASS_ACTOR;
        }
        else if(EQUAL(extension,"actor_variant")) {
            return TagClassInt::TAG_CLASS_ACTOR_VARIANT;
        }
        else if(EQUAL(extension,"antenna")) {
            return TagClassInt::TAG_CLASS_ANTENNA;
        }
        else if(EQUAL(extension,"model_animations")) {
            return TagClassInt::TAG_CLASS_MODEL_ANIMATIONS;
        }
        else if(EQUAL(extension,"biped")) {
            return TagClassInt::TAG_CLASS_BIPED;
        }
        else if(EQUAL(extension,"bitmap")) {
            return TagClassInt::TAG_CLASS_BITMAP;
        }
        else if(EQUAL(extension,"spheroid")) {
            return TagClassInt::TAG_CLASS_SPHEROID;
        }
        else if(EQUAL(extension,"continuous_damage_effect")) {
            return TagClassInt::TAG_CLASS_CONTINUOUS_DAMAGE_EFFECT;
        }
        else if(EQUAL(extension,"model_collision_geometry")) {
            return TagClassInt::TAG_CLASS_MODEL_COLLISION_GEOMETRY;
        }
        else if(EQUAL(extension,"color_table")) {
            return TagClassInt::TAG_CLASS_COLOR_TABLE;
        }
        else if(EQUAL(extension,"contrail")) {
            return TagClassInt::TAG_CLASS_CONTRAIL;
        }
        else if(EQUAL(extension,"device_control")) {
            return TagClassInt::TAG_CLASS_DEVICE_CONTROL;
        }
        else if(EQUAL(extension,"decal")) {
            return TagClassInt::TAG_CLASS_DECAL;
        }
        else if(EQUAL(extension,"ui_widget_definition")) {
            return TagClassInt::TAG_CLASS_UI_WIDGET_DEFINITION;
        }
        else if(EQUAL(extension,"input_device_defaults")) {
            return TagClassInt::TAG_CLASS_INPUT_DEVICE_DEFAULTS;
        }
        else if(EQUAL(extension,"device")) {
            return TagClassInt::TAG_CLASS_DEVICE;
        }
        else if(EQUAL(extension,"detail_object_collection")) {
            return TagClassInt::TAG_CLASS_DETAIL_OBJECT_COLLECTION;
        }
        else if(EQUAL(extension,"effect")) {
            return TagClassInt::TAG_CLASS_EFFECT;
        }
        else if(EQUAL(extension,"equipment")) {
            return TagClassInt::TAG_CLASS_EQUIPMENT;
        }
        else if(EQUAL(extension,"flag")) {
            return TagClassInt::TAG_CLASS_FLAG;
        }
        else if(EQUAL(extension,"fog")) {
            return TagClassInt::TAG_CLASS_FOG;
        }
        else if(EQUAL(extension,"font")) {
            return TagClassInt::TAG_CLASS_FONT;
        }
        else if(EQUAL(extension,"material_effects")) {
            return TagClassInt::TAG_CLASS_MATERIAL_EFFECTS;
        }
        else if(EQUAL(extension,"garbage")) {
            return TagClassInt::TAG_CLASS_GARBAGE;
        }
        else if(EQUAL(extension,"glow")) {
            return TagClassInt::TAG_CLASS_GLOW;
        }
        else if(EQUAL(extension,"grenade_hud_interface")) {
            return TagClassInt::TAG_CLASS_GRENADE_HUD_INTERFACE;
        }
        else if(EQUAL(extension,"hud_message_text")) {
            return TagClassInt::TAG_CLASS_HUD_MESSAGE_TEXT;
        }
        else if(EQUAL(extension,"hud_number")) {
            return TagClassInt::TAG_CLASS_HUD_NUMBER;
        }
        else if(EQUAL(extension,"hud_globals")) {
            return TagClassInt::TAG_CLASS_HUD_GLOBALS;
        }
        else if(EQUAL(extension,"item")) {
            return TagClassInt::TAG_CLASS_ITEM;
        }
        else if(EQUAL(extension,"item_collection")) {
            return TagClassInt::TAG_CLASS_ITEM_COLLECTION;
        }
        else if(EQUAL(extension,"damage_effect")) {
            return TagClassInt::TAG_CLASS_DAMAGE_EFFECT;
        }
        else if(EQUAL(extension,"lens_flare")) {
            return TagClassInt::TAG_CLASS_LENS_FLARE;
        }
        else if(EQUAL(extension,"lightning")) {
            return TagClassInt::TAG_CLASS_LIGHTNING;
        }
        else if(EQUAL(extension,"device_light_fixture")) {
            return TagClassInt::TAG_CLASS_DEVICE_LIGHT_FIXTURE;
        }
        else if(EQUAL(extension,"light")) {
            return TagClassInt::TAG_CLASS_LIGHT;
        }
        else if(EQUAL(extension,"sound_looping")) {
            return TagClassInt::TAG_CLASS_SOUND_LOOPING;
        }
        else if(EQUAL(extension,"device_machine")) {
            return TagClassInt::TAG_CLASS_DEVICE_MACHINE;
        }
        else if(EQUAL(extension,"globals")) {
            return TagClassInt::TAG_CLASS_GLOBALS;
        }
        else if(EQUAL(extension,"meter")) {
            return TagClassInt::TAG_CLASS_METER;
        }
        else if(EQUAL(extension,"light_volume")) {
            return TagClassInt::TAG_CLASS_LIGHT_VOLUME;
        }
        else if(EQUAL(extension,"gbxmodel")) {
            return TagClassInt::TAG_CLASS_GBXMODEL;
        }
        else if(EQUAL(extension,"model")) {
            return TagClassInt::TAG_CLASS_MODEL;
        }
        else if(EQUAL(extension,"multiplayer_scenario_description")) {
            return TagClassInt::TAG_CLASS_MULTIPLAYER_SCENARIO_DESCRIPTION;
        }
        else if(EQUAL(extension,"preferences_network_game")) {
            return TagClassInt::TAG_CLASS_PREFERENCES_NETWORK_GAME;
        }
        else if(EQUAL(extension,"none")) {
            return TagClassInt::TAG_CLASS_NONE;
        }
        else if(EQUAL(extension,"object")) {
            return TagClassInt::TAG_CLASS_OBJECT;
        }
        else if(EQUAL(extension,"particle")) {
            return TagClassInt::TAG_CLASS_PARTICLE;
        }
        else if(EQUAL(extension,"particle_system")) {
            return TagClassInt::TAG_CLASS_PARTICLE_SYSTEM;
        }
        else if(EQUAL(extension,"physics")) {
            return TagClassInt::TAG_CLASS_PHYSICS;
        }
        else if(EQUAL(extension,"placeholder")) {
            return TagClassInt::TAG_CLASS_PLACEHOLDER;
        }
        else if(EQUAL(extension,"point_physics")) {
            return TagClassInt::TAG_CLASS_POINT_PHYSICS;
        }
        else if(EQUAL(extension,"projectile")) {
            return TagClassInt::TAG_CLASS_PROJECTILE;
        }
        else if(EQUAL(extension,"weather_particle_system")) {
            return TagClassInt::TAG_CLASS_WEATHER_PARTICLE_SYSTEM;
        }
        else if(EQUAL(extension,"scenario_structure_bsp")) {
            return TagClassInt::TAG_CLASS_SCENARIO_STRUCTURE_BSP;
        }
        else if(EQUAL(extension,"scenery")) {
            return TagClassInt::TAG_CLASS_SCENERY;
        }
        else if(EQUAL(extension,"shader_transparent_chicago_extended")) {
            return TagClassInt::TAG_CLASS_SHADER_TRANSPARENT_CHICAGO_EXTENDED;
        }
        else if(EQUAL(extension,"shader_transparent_chicago")) {
            return TagClassInt::TAG_CLASS_SHADER_TRANSPARENT_CHICAGO;
        }
        else if(EQUAL(extension,"scenario")) {
            return TagClassInt::TAG_CLASS_SCENARIO;
        }
        else if(EQUAL(extension,"shader_environment")) {
            return TagClassInt::TAG_CLASS_SHADER_ENVIRONMENT;
        }
        else if(EQUAL(extension,"shader_transparent_glass")) {
            return TagClassInt::TAG_CLASS_SHADER_TRANSPARENT_GLASS;
        }
        else if(EQUAL(extension,"shader")) {
            return TagClassInt::TAG_CLASS_SHADER;
        }
        else if(EQUAL(extension,"sky")) {
            return TagClassInt::TAG_CLASS_SKY;
        }
        else if(EQUAL(extension,"shader_transparent_meter")) {
            return TagClassInt::TAG_CLASS_SHADER_TRANSPARENT_METER;
        }
        else if(EQUAL(extension,"sound")) {
            return TagClassInt::TAG_CLASS_SOUND;
        }
        else if(EQUAL(extension,"sound_environment")) {
            return TagClassInt::TAG_CLASS_SOUND_ENVIRONMENT;
        }
        else if(EQUAL(extension,"shader_model")) {
            return TagClassInt::TAG_CLASS_SHADER_MODEL;
        }
        else if(EQUAL(extension,"shader_transparent_generic")) {
            return TagClassInt::TAG_CLASS_SHADER_TRANSPARENT_GENERIC;
        }
        else if(EQUAL(extension,"ui_widget_collection")) {
            return TagClassInt::TAG_CLASS_UI_WIDGET_COLLECTION;
        }
        else if(EQUAL(extension,"shader_transparent_plasma")) {
            return TagClassInt::TAG_CLASS_SHADER_TRANSPARENT_PLASMA;
        }
        else if(EQUAL(extension,"sound_scenery")) {
            return TagClassInt::TAG_CLASS_SOUND_SCENERY;
        }
        else if(EQUAL(extension,"string_list")) {
            return TagClassInt::TAG_CLASS_STRING_LIST;
        }
        else if(EQUAL(extension,"shader_transparent_water")) {
            return TagClassInt::TAG_CLASS_SHADER_TRANSPARENT_WATER;
        }
        else if(EQUAL(extension,"tag_collection")) {
            return TagClassInt::TAG_CLASS_TAG_COLLECTION;
        }
        else if(EQUAL(extension,"camera_track")) {
            return TagClassInt::TAG_CLASS_CAMERA_TRACK;
        }
        else if(EQUAL(extension,"dialogue")) {
            return TagClassInt::TAG_CLASS_DIALOGUE;
        }
        else if(EQUAL(extension,"unit_hud_interface")) {
            return TagClassInt::TAG_CLASS_UNIT_HUD_INTERFACE;
        }
        else if(EQUAL(extension,"unit")) {
            return TagClassInt::TAG_CLASS_UNIT;
        }
        else if(EQUAL(extension,"unicode_string_list")) {
            return TagClassInt::TAG_CLASS_UNICODE_STRING_LIST;
        }
        else if(EQUAL(extension,"virtual_keyboard")) {
            return TagClassInt::TAG_CLASS_VIRTUAL_KEYBOARD;
        }
        else if(EQUAL(extension,"vehicle")) {
            return TagClassInt::TAG_CLASS_VEHICLE;
        }
        else if(EQUAL(extension,"weapon")) {
            return TagClassInt::TAG_CLASS_WEAPON;
        }
        else if(EQUAL(extension,"wind")) {
            return TagClassInt::TAG_CLASS_WIND;
        }
        else if(EQUAL(extension,"weapon_hud_interface")) {
            return TagClassInt::TAG_CLASS_WEAPON_HUD_INTERFACE;
        }
        else {
            return TagClassInt::TAG_CLASS_NULL;
        }
    }
}
