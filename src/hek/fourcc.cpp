// SPDX-License-Identifier: GPL-3.0-only

#include <cstring>

#include <invader/hek/fourcc.hpp>

namespace Invader::HEK {
    #define MATCH_TAG_CLASS(check_extension,tag_fourcc) else if(std::strcmp(extension, check_extension) == 0) return TagFourCC::tag_fourcc;

    /**
     * Convert a string extension to its respective tag class integer
     * @param  extension string name of the tag class
     * @return           respective tag class integer or TAG_FOURCC_NULL if nullptr was given or the extension was not valid
     */
    TagFourCC tag_extension_to_fourcc(const char *extension) noexcept {
        if(extension == nullptr) {
            return TagFourCC::TAG_FOURCC_NULL;
        }

        MATCH_TAG_CLASS("actor", TAG_FOURCC_ACTOR)
        MATCH_TAG_CLASS("actor_variant", TAG_FOURCC_ACTOR_VARIANT)
        MATCH_TAG_CLASS("antenna", TAG_FOURCC_ANTENNA)
        MATCH_TAG_CLASS("model_animations", TAG_FOURCC_MODEL_ANIMATIONS)
        MATCH_TAG_CLASS("biped", TAG_FOURCC_BIPED)
        MATCH_TAG_CLASS("bitmap", TAG_FOURCC_BITMAP)
        MATCH_TAG_CLASS("spheroid", TAG_FOURCC_SPHEROID)
        MATCH_TAG_CLASS("continuous_damage_effect", TAG_FOURCC_CONTINUOUS_DAMAGE_EFFECT)
        MATCH_TAG_CLASS("model_collision_geometry", TAG_FOURCC_MODEL_COLLISION_GEOMETRY)
        MATCH_TAG_CLASS("color_table", TAG_FOURCC_COLOR_TABLE)
        MATCH_TAG_CLASS("contrail", TAG_FOURCC_CONTRAIL)
        MATCH_TAG_CLASS("device_control", TAG_FOURCC_DEVICE_CONTROL)
        MATCH_TAG_CLASS("decal", TAG_FOURCC_DECAL)
        MATCH_TAG_CLASS("ui_widget_definition", TAG_FOURCC_UI_WIDGET_DEFINITION)
        MATCH_TAG_CLASS("input_device_defaults", TAG_FOURCC_INPUT_DEVICE_DEFAULTS)
        MATCH_TAG_CLASS("device", TAG_FOURCC_DEVICE)
        MATCH_TAG_CLASS("detail_object_collection", TAG_FOURCC_DETAIL_OBJECT_COLLECTION)
        MATCH_TAG_CLASS("effect", TAG_FOURCC_EFFECT)
        MATCH_TAG_CLASS("equipment", TAG_FOURCC_EQUIPMENT)
        MATCH_TAG_CLASS("flag", TAG_FOURCC_FLAG)
        MATCH_TAG_CLASS("fog", TAG_FOURCC_FOG)
        MATCH_TAG_CLASS("font", TAG_FOURCC_FONT)
        MATCH_TAG_CLASS("material_effects", TAG_FOURCC_MATERIAL_EFFECTS)
        MATCH_TAG_CLASS("garbage", TAG_FOURCC_GARBAGE)
        MATCH_TAG_CLASS("glow", TAG_FOURCC_GLOW)
        MATCH_TAG_CLASS("grenade_hud_interface", TAG_FOURCC_GRENADE_HUD_INTERFACE)
        MATCH_TAG_CLASS("hud_message_text", TAG_FOURCC_HUD_MESSAGE_TEXT)
        MATCH_TAG_CLASS("hud_number", TAG_FOURCC_HUD_NUMBER)
        MATCH_TAG_CLASS("hud_globals", TAG_FOURCC_HUD_GLOBALS)
        MATCH_TAG_CLASS("item", TAG_FOURCC_ITEM)
        MATCH_TAG_CLASS("item_collection", TAG_FOURCC_ITEM_COLLECTION)
        MATCH_TAG_CLASS("damage_effect", TAG_FOURCC_DAMAGE_EFFECT)
        MATCH_TAG_CLASS("lens_flare", TAG_FOURCC_LENS_FLARE)
        MATCH_TAG_CLASS("lightning", TAG_FOURCC_LIGHTNING)
        MATCH_TAG_CLASS("device_light_fixture", TAG_FOURCC_DEVICE_LIGHT_FIXTURE)
        MATCH_TAG_CLASS("light", TAG_FOURCC_LIGHT)
        MATCH_TAG_CLASS("sound_looping", TAG_FOURCC_SOUND_LOOPING)
        MATCH_TAG_CLASS("device_machine", TAG_FOURCC_DEVICE_MACHINE)
        MATCH_TAG_CLASS("globals", TAG_FOURCC_GLOBALS)
        MATCH_TAG_CLASS("meter", TAG_FOURCC_METER)
        MATCH_TAG_CLASS("light_volume", TAG_FOURCC_LIGHT_VOLUME)
        MATCH_TAG_CLASS("gbxmodel", TAG_FOURCC_GBXMODEL)
        MATCH_TAG_CLASS("model", TAG_FOURCC_MODEL)
        MATCH_TAG_CLASS("multiplayer_scenario_description", TAG_FOURCC_MULTIPLAYER_SCENARIO_DESCRIPTION)
        MATCH_TAG_CLASS("preferences_network_game", TAG_FOURCC_PREFERENCES_NETWORK_GAME)
        MATCH_TAG_CLASS("none", TAG_FOURCC_NONE)
        MATCH_TAG_CLASS("object", TAG_FOURCC_OBJECT)
        MATCH_TAG_CLASS("particle", TAG_FOURCC_PARTICLE)
        MATCH_TAG_CLASS("particle_system", TAG_FOURCC_PARTICLE_SYSTEM)
        MATCH_TAG_CLASS("physics", TAG_FOURCC_PHYSICS)
        MATCH_TAG_CLASS("placeholder", TAG_FOURCC_PLACEHOLDER)
        MATCH_TAG_CLASS("point_physics", TAG_FOURCC_POINT_PHYSICS)
        MATCH_TAG_CLASS("projectile", TAG_FOURCC_PROJECTILE)
        MATCH_TAG_CLASS("weather_particle_system", TAG_FOURCC_WEATHER_PARTICLE_SYSTEM)
        MATCH_TAG_CLASS("scenario_structure_bsp", TAG_FOURCC_SCENARIO_STRUCTURE_BSP)
        MATCH_TAG_CLASS("scenery", TAG_FOURCC_SCENERY)
        MATCH_TAG_CLASS("shader_transparent_chicago_extended", TAG_FOURCC_SHADER_TRANSPARENT_CHICAGO_EXTENDED)
        MATCH_TAG_CLASS("shader_transparent_chicago", TAG_FOURCC_SHADER_TRANSPARENT_CHICAGO)
        MATCH_TAG_CLASS("scenario", TAG_FOURCC_SCENARIO)
        MATCH_TAG_CLASS("shader_environment", TAG_FOURCC_SHADER_ENVIRONMENT)
        MATCH_TAG_CLASS("shader_transparent_glass", TAG_FOURCC_SHADER_TRANSPARENT_GLASS)
        MATCH_TAG_CLASS("shader", TAG_FOURCC_SHADER)
        MATCH_TAG_CLASS("sky", TAG_FOURCC_SKY)
        MATCH_TAG_CLASS("shader_transparent_meter", TAG_FOURCC_SHADER_TRANSPARENT_METER)
        MATCH_TAG_CLASS("sound", TAG_FOURCC_SOUND)
        MATCH_TAG_CLASS("sound_environment", TAG_FOURCC_SOUND_ENVIRONMENT)
        MATCH_TAG_CLASS("shader_model", TAG_FOURCC_SHADER_MODEL)
        MATCH_TAG_CLASS("shader_transparent_generic", TAG_FOURCC_SHADER_TRANSPARENT_GENERIC)
        MATCH_TAG_CLASS("ui_widget_collection", TAG_FOURCC_UI_WIDGET_COLLECTION)
        MATCH_TAG_CLASS("shader_transparent_plasma", TAG_FOURCC_SHADER_TRANSPARENT_PLASMA)
        MATCH_TAG_CLASS("sound_scenery", TAG_FOURCC_SOUND_SCENERY)
        MATCH_TAG_CLASS("string_list", TAG_FOURCC_STRING_LIST)
        MATCH_TAG_CLASS("shader_transparent_water", TAG_FOURCC_SHADER_TRANSPARENT_WATER)
        MATCH_TAG_CLASS("tag_collection", TAG_FOURCC_TAG_COLLECTION)
        MATCH_TAG_CLASS("camera_track", TAG_FOURCC_CAMERA_TRACK)
        MATCH_TAG_CLASS("dialogue", TAG_FOURCC_DIALOGUE)
        MATCH_TAG_CLASS("unit_hud_interface", TAG_FOURCC_UNIT_HUD_INTERFACE)
        MATCH_TAG_CLASS("unit", TAG_FOURCC_UNIT)
        MATCH_TAG_CLASS("unicode_string_list", TAG_FOURCC_UNICODE_STRING_LIST)
        MATCH_TAG_CLASS("virtual_keyboard", TAG_FOURCC_VIRTUAL_KEYBOARD)
        MATCH_TAG_CLASS("vehicle", TAG_FOURCC_VEHICLE)
        MATCH_TAG_CLASS("weapon", TAG_FOURCC_WEAPON)
        MATCH_TAG_CLASS("wind", TAG_FOURCC_WIND)
        MATCH_TAG_CLASS("weapon_hud_interface", TAG_FOURCC_WEAPON_HUD_INTERFACE)

        return TAG_FOURCC_NULL;
    }

    #undef MATCH_TAG_CLASS
}
