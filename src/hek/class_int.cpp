// SPDX-License-Identifier: GPL-3.0-only

#include <cstring>

#include <invader/hek/class_int.hpp>

namespace Invader::HEK {
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
