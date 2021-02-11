// SPDX-License-Identifier: GPL-3.0-only

#include <cstring>

#include <invader/hek/fourcc.hpp>

namespace Invader::HEK {
    #define MATCH_TAG_CLASS(check_extension,tag_fourcc) else if(std::strcmp(extension, check_extension) == 0) return TagClassInt::tag_fourcc;

    /**
     * Convert a string extension to its respective tag class integer
     * @param  extension string name of the tag class
     * @return           respective tag class integer or TAG_CLASS_NULL if nullptr was given or the extension was not valid
     */
    TagClassInt tag_extension_to_fourcc(const char *extension) noexcept {
        if(extension == nullptr) {
            return TagClassInt::TAG_CLASS_NULL;
        }

        MATCH_TAG_CLASS("actor", TAG_CLASS_ACTOR)
        MATCH_TAG_CLASS("actor_variant", TAG_CLASS_ACTOR_VARIANT)
        MATCH_TAG_CLASS("antenna", TAG_CLASS_ANTENNA)
        MATCH_TAG_CLASS("model_animations", TAG_CLASS_MODEL_ANIMATIONS)
        MATCH_TAG_CLASS("biped", TAG_CLASS_BIPED)
        MATCH_TAG_CLASS("bitmap", TAG_CLASS_BITMAP)
        MATCH_TAG_CLASS("spheroid", TAG_CLASS_SPHEROID)
        MATCH_TAG_CLASS("continuous_damage_effect", TAG_CLASS_CONTINUOUS_DAMAGE_EFFECT)
        MATCH_TAG_CLASS("model_collision_geometry", TAG_CLASS_MODEL_COLLISION_GEOMETRY)
        MATCH_TAG_CLASS("color_table", TAG_CLASS_COLOR_TABLE)
        MATCH_TAG_CLASS("contrail", TAG_CLASS_CONTRAIL)
        MATCH_TAG_CLASS("device_control", TAG_CLASS_DEVICE_CONTROL)
        MATCH_TAG_CLASS("decal", TAG_CLASS_DECAL)
        MATCH_TAG_CLASS("ui_widget_definition", TAG_CLASS_UI_WIDGET_DEFINITION)
        MATCH_TAG_CLASS("input_device_defaults", TAG_CLASS_INPUT_DEVICE_DEFAULTS)
        MATCH_TAG_CLASS("device", TAG_CLASS_DEVICE)
        MATCH_TAG_CLASS("detail_object_collection", TAG_CLASS_DETAIL_OBJECT_COLLECTION)
        MATCH_TAG_CLASS("effect", TAG_CLASS_EFFECT)
        MATCH_TAG_CLASS("equipment", TAG_CLASS_EQUIPMENT)
        MATCH_TAG_CLASS("flag", TAG_CLASS_FLAG)
        MATCH_TAG_CLASS("fog", TAG_CLASS_FOG)
        MATCH_TAG_CLASS("font", TAG_CLASS_FONT)
        MATCH_TAG_CLASS("material_effects", TAG_CLASS_MATERIAL_EFFECTS)
        MATCH_TAG_CLASS("garbage", TAG_CLASS_GARBAGE)
        MATCH_TAG_CLASS("glow", TAG_CLASS_GLOW)
        MATCH_TAG_CLASS("grenade_hud_interface", TAG_CLASS_GRENADE_HUD_INTERFACE)
        MATCH_TAG_CLASS("hud_message_text", TAG_CLASS_HUD_MESSAGE_TEXT)
        MATCH_TAG_CLASS("hud_number", TAG_CLASS_HUD_NUMBER)
        MATCH_TAG_CLASS("hud_globals", TAG_CLASS_HUD_GLOBALS)
        MATCH_TAG_CLASS("item", TAG_CLASS_ITEM)
        MATCH_TAG_CLASS("item_collection", TAG_CLASS_ITEM_COLLECTION)
        MATCH_TAG_CLASS("damage_effect", TAG_CLASS_DAMAGE_EFFECT)
        MATCH_TAG_CLASS("lens_flare", TAG_CLASS_LENS_FLARE)
        MATCH_TAG_CLASS("lightning", TAG_CLASS_LIGHTNING)
        MATCH_TAG_CLASS("device_light_fixture", TAG_CLASS_DEVICE_LIGHT_FIXTURE)
        MATCH_TAG_CLASS("light", TAG_CLASS_LIGHT)
        MATCH_TAG_CLASS("sound_looping", TAG_CLASS_SOUND_LOOPING)
        MATCH_TAG_CLASS("device_machine", TAG_CLASS_DEVICE_MACHINE)
        MATCH_TAG_CLASS("globals", TAG_CLASS_GLOBALS)
        MATCH_TAG_CLASS("meter", TAG_CLASS_METER)
        MATCH_TAG_CLASS("light_volume", TAG_CLASS_LIGHT_VOLUME)
        MATCH_TAG_CLASS("gbxmodel", TAG_CLASS_GBXMODEL)
        MATCH_TAG_CLASS("model", TAG_CLASS_MODEL)
        MATCH_TAG_CLASS("multiplayer_scenario_description", TAG_CLASS_MULTIPLAYER_SCENARIO_DESCRIPTION)
        MATCH_TAG_CLASS("preferences_network_game", TAG_CLASS_PREFERENCES_NETWORK_GAME)
        MATCH_TAG_CLASS("none", TAG_CLASS_NONE)
        MATCH_TAG_CLASS("object", TAG_CLASS_OBJECT)
        MATCH_TAG_CLASS("particle", TAG_CLASS_PARTICLE)
        MATCH_TAG_CLASS("particle_system", TAG_CLASS_PARTICLE_SYSTEM)
        MATCH_TAG_CLASS("physics", TAG_CLASS_PHYSICS)
        MATCH_TAG_CLASS("placeholder", TAG_CLASS_PLACEHOLDER)
        MATCH_TAG_CLASS("point_physics", TAG_CLASS_POINT_PHYSICS)
        MATCH_TAG_CLASS("projectile", TAG_CLASS_PROJECTILE)
        MATCH_TAG_CLASS("weather_particle_system", TAG_CLASS_WEATHER_PARTICLE_SYSTEM)
        MATCH_TAG_CLASS("scenario_structure_bsp", TAG_CLASS_SCENARIO_STRUCTURE_BSP)
        MATCH_TAG_CLASS("scenery", TAG_CLASS_SCENERY)
        MATCH_TAG_CLASS("shader_transparent_chicago_extended", TAG_CLASS_SHADER_TRANSPARENT_CHICAGO_EXTENDED)
        MATCH_TAG_CLASS("shader_transparent_chicago", TAG_CLASS_SHADER_TRANSPARENT_CHICAGO)
        MATCH_TAG_CLASS("scenario", TAG_CLASS_SCENARIO)
        MATCH_TAG_CLASS("shader_environment", TAG_CLASS_SHADER_ENVIRONMENT)
        MATCH_TAG_CLASS("shader_transparent_glass", TAG_CLASS_SHADER_TRANSPARENT_GLASS)
        MATCH_TAG_CLASS("shader", TAG_CLASS_SHADER)
        MATCH_TAG_CLASS("sky", TAG_CLASS_SKY)
        MATCH_TAG_CLASS("shader_transparent_meter", TAG_CLASS_SHADER_TRANSPARENT_METER)
        MATCH_TAG_CLASS("sound", TAG_CLASS_SOUND)
        MATCH_TAG_CLASS("sound_environment", TAG_CLASS_SOUND_ENVIRONMENT)
        MATCH_TAG_CLASS("shader_model", TAG_CLASS_SHADER_MODEL)
        MATCH_TAG_CLASS("shader_transparent_generic", TAG_CLASS_SHADER_TRANSPARENT_GENERIC)
        MATCH_TAG_CLASS("ui_widget_collection", TAG_CLASS_UI_WIDGET_COLLECTION)
        MATCH_TAG_CLASS("shader_transparent_plasma", TAG_CLASS_SHADER_TRANSPARENT_PLASMA)
        MATCH_TAG_CLASS("sound_scenery", TAG_CLASS_SOUND_SCENERY)
        MATCH_TAG_CLASS("string_list", TAG_CLASS_STRING_LIST)
        MATCH_TAG_CLASS("shader_transparent_water", TAG_CLASS_SHADER_TRANSPARENT_WATER)
        MATCH_TAG_CLASS("tag_collection", TAG_CLASS_TAG_COLLECTION)
        MATCH_TAG_CLASS("camera_track", TAG_CLASS_CAMERA_TRACK)
        MATCH_TAG_CLASS("dialogue", TAG_CLASS_DIALOGUE)
        MATCH_TAG_CLASS("unit_hud_interface", TAG_CLASS_UNIT_HUD_INTERFACE)
        MATCH_TAG_CLASS("unit", TAG_CLASS_UNIT)
        MATCH_TAG_CLASS("unicode_string_list", TAG_CLASS_UNICODE_STRING_LIST)
        MATCH_TAG_CLASS("virtual_keyboard", TAG_CLASS_VIRTUAL_KEYBOARD)
        MATCH_TAG_CLASS("vehicle", TAG_CLASS_VEHICLE)
        MATCH_TAG_CLASS("weapon", TAG_CLASS_WEAPON)
        MATCH_TAG_CLASS("wind", TAG_CLASS_WIND)
        MATCH_TAG_CLASS("weapon_hud_interface", TAG_CLASS_WEAPON_HUD_INTERFACE)

        MATCH_TAG_CLASS("invader_bitmap", TAG_CLASS_INVADER_BITMAP)
        MATCH_TAG_CLASS("invader_sound", TAG_CLASS_INVADER_SOUND)
        MATCH_TAG_CLASS("invader_font", TAG_CLASS_INVADER_FONT)
        MATCH_TAG_CLASS("invader_ui_widget_definition", TAG_CLASS_INVADER_UI_WIDGET_DEFINITION)
        MATCH_TAG_CLASS("invader_unit_hud_interface", TAG_CLASS_INVADER_UNIT_HUD_INTERFACE)
        MATCH_TAG_CLASS("invader_weapon_hud_interface", TAG_CLASS_INVADER_WEAPON_HUD_INTERFACE)
        MATCH_TAG_CLASS("shader_transparent_glsl", TAG_CLASS_SHADER_TRANSPARENT_GLSL)

        return TAG_CLASS_NULL;
    }

    #undef MATCH_TAG_CLASS
}
