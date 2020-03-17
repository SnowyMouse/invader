// SPDX-License-Identifier: GPL-3.0-only

#include <invader/extract/extraction.hpp>
#include <invader/tag/hek/header.hpp>
#include <invader/tag/parser/parser.hpp>

namespace Invader::Extraction {
    /**
     * Extract the tag into an HEK tag file
     * @param tag tag to extract
     */
    std::vector<std::byte> extract_tag(const Tag &tag) {
        auto tag_class_int = tag.get_tag_class_int();

        #define EXTRACT_TAG_CLASS(class_struct, class_int) case TagClassInt::class_int: { \
            return Parser::class_struct::parse_cache_file_data(tag).generate_hek_tag_data(TagClassInt::class_int); \
        }

        switch(tag_class_int) {
            EXTRACT_TAG_CLASS(Actor, TAG_CLASS_ACTOR)
            EXTRACT_TAG_CLASS(ActorVariant, TAG_CLASS_ACTOR_VARIANT)
            EXTRACT_TAG_CLASS(Antenna, TAG_CLASS_ANTENNA)
            EXTRACT_TAG_CLASS(ModelAnimations, TAG_CLASS_MODEL_ANIMATIONS)
            EXTRACT_TAG_CLASS(Biped, TAG_CLASS_BIPED)
            EXTRACT_TAG_CLASS(Bitmap, TAG_CLASS_BITMAP)
            EXTRACT_TAG_CLASS(ModelCollisionGeometry, TAG_CLASS_MODEL_COLLISION_GEOMETRY)
            EXTRACT_TAG_CLASS(ColorTable, TAG_CLASS_COLOR_TABLE)
            EXTRACT_TAG_CLASS(Contrail, TAG_CLASS_CONTRAIL)
            EXTRACT_TAG_CLASS(DeviceControl, TAG_CLASS_DEVICE_CONTROL)
            EXTRACT_TAG_CLASS(Decal, TAG_CLASS_DECAL)
            EXTRACT_TAG_CLASS(UIWidgetDefinition, TAG_CLASS_UI_WIDGET_DEFINITION)
            EXTRACT_TAG_CLASS(InputDeviceDefaults, TAG_CLASS_INPUT_DEVICE_DEFAULTS)
            EXTRACT_TAG_CLASS(Device, TAG_CLASS_DEVICE)
            EXTRACT_TAG_CLASS(DetailObjectCollection, TAG_CLASS_DETAIL_OBJECT_COLLECTION)
            EXTRACT_TAG_CLASS(Effect, TAG_CLASS_EFFECT)
            EXTRACT_TAG_CLASS(Equipment, TAG_CLASS_EQUIPMENT)
            EXTRACT_TAG_CLASS(Flag, TAG_CLASS_FLAG)
            EXTRACT_TAG_CLASS(Fog, TAG_CLASS_FOG)
            EXTRACT_TAG_CLASS(Font, TAG_CLASS_FONT)
            EXTRACT_TAG_CLASS(MaterialEffects, TAG_CLASS_MATERIAL_EFFECTS)
            EXTRACT_TAG_CLASS(Garbage, TAG_CLASS_GARBAGE)
            EXTRACT_TAG_CLASS(Glow, TAG_CLASS_GLOW)
            EXTRACT_TAG_CLASS(GrenadeHUDInterface, TAG_CLASS_GRENADE_HUD_INTERFACE)
            EXTRACT_TAG_CLASS(HUDMessageText, TAG_CLASS_HUD_MESSAGE_TEXT)
            EXTRACT_TAG_CLASS(HUDNumber, TAG_CLASS_HUD_NUMBER)
            EXTRACT_TAG_CLASS(HUDGlobals, TAG_CLASS_HUD_GLOBALS)
            EXTRACT_TAG_CLASS(Item, TAG_CLASS_ITEM)
            EXTRACT_TAG_CLASS(ItemCollection, TAG_CLASS_ITEM_COLLECTION)
            EXTRACT_TAG_CLASS(DamageEffect, TAG_CLASS_DAMAGE_EFFECT)
            EXTRACT_TAG_CLASS(LensFlare, TAG_CLASS_LENS_FLARE)
            EXTRACT_TAG_CLASS(Lightning, TAG_CLASS_LIGHTNING)
            EXTRACT_TAG_CLASS(DeviceLightFixture, TAG_CLASS_DEVICE_LIGHT_FIXTURE)
            EXTRACT_TAG_CLASS(Light, TAG_CLASS_LIGHT)
            EXTRACT_TAG_CLASS(SoundLooping, TAG_CLASS_SOUND_LOOPING)
            EXTRACT_TAG_CLASS(DeviceMachine, TAG_CLASS_DEVICE_MACHINE)
            EXTRACT_TAG_CLASS(Globals, TAG_CLASS_GLOBALS)
            EXTRACT_TAG_CLASS(Meter, TAG_CLASS_METER)
            EXTRACT_TAG_CLASS(LightVolume, TAG_CLASS_LIGHT_VOLUME)
            EXTRACT_TAG_CLASS(GBXModel, TAG_CLASS_GBXMODEL)
            EXTRACT_TAG_CLASS(MultiplayerScenarioDescription, TAG_CLASS_MULTIPLAYER_SCENARIO_DESCRIPTION)
            EXTRACT_TAG_CLASS(Object, TAG_CLASS_OBJECT)
            EXTRACT_TAG_CLASS(Particle, TAG_CLASS_PARTICLE)
            EXTRACT_TAG_CLASS(ParticleSystem, TAG_CLASS_PARTICLE_SYSTEM)
            EXTRACT_TAG_CLASS(Physics, TAG_CLASS_PHYSICS)
            EXTRACT_TAG_CLASS(Placeholder, TAG_CLASS_PLACEHOLDER)
            EXTRACT_TAG_CLASS(PointPhysics, TAG_CLASS_POINT_PHYSICS)
            EXTRACT_TAG_CLASS(Projectile, TAG_CLASS_PROJECTILE)
            EXTRACT_TAG_CLASS(WeatherParticleSystem, TAG_CLASS_WEATHER_PARTICLE_SYSTEM)
            EXTRACT_TAG_CLASS(Scenery, TAG_CLASS_SCENERY)
            EXTRACT_TAG_CLASS(ShaderTransparentChicagoExtended, TAG_CLASS_SHADER_TRANSPARENT_CHICAGO_EXTENDED)
            EXTRACT_TAG_CLASS(ShaderTransparentChicago, TAG_CLASS_SHADER_TRANSPARENT_CHICAGO)
            EXTRACT_TAG_CLASS(Scenario, TAG_CLASS_SCENARIO)
            EXTRACT_TAG_CLASS(ShaderEnvironment, TAG_CLASS_SHADER_ENVIRONMENT)
            EXTRACT_TAG_CLASS(ShaderTransparentGlass, TAG_CLASS_SHADER_TRANSPARENT_GLASS)
            EXTRACT_TAG_CLASS(Shader, TAG_CLASS_SHADER)
            EXTRACT_TAG_CLASS(Sky, TAG_CLASS_SKY)
            EXTRACT_TAG_CLASS(ShaderTransparentMeter, TAG_CLASS_SHADER_TRANSPARENT_METER)
            EXTRACT_TAG_CLASS(Sound, TAG_CLASS_SOUND)
            EXTRACT_TAG_CLASS(SoundEnvironment, TAG_CLASS_SOUND_ENVIRONMENT)
            EXTRACT_TAG_CLASS(ShaderModel, TAG_CLASS_SHADER_MODEL)
            EXTRACT_TAG_CLASS(ShaderTransparentGeneric, TAG_CLASS_SHADER_TRANSPARENT_GENERIC)
            EXTRACT_TAG_CLASS(TagCollection, TAG_CLASS_UI_WIDGET_COLLECTION)
            EXTRACT_TAG_CLASS(ShaderTransparentPlasma, TAG_CLASS_SHADER_TRANSPARENT_PLASMA)
            EXTRACT_TAG_CLASS(SoundScenery, TAG_CLASS_SOUND_SCENERY)
            EXTRACT_TAG_CLASS(StringList, TAG_CLASS_STRING_LIST)
            EXTRACT_TAG_CLASS(ShaderTransparentWater, TAG_CLASS_SHADER_TRANSPARENT_WATER)
            EXTRACT_TAG_CLASS(TagCollection, TAG_CLASS_TAG_COLLECTION)
            EXTRACT_TAG_CLASS(CameraTrack, TAG_CLASS_CAMERA_TRACK)
            EXTRACT_TAG_CLASS(Dialogue, TAG_CLASS_DIALOGUE)
            EXTRACT_TAG_CLASS(UnitHUDInterface, TAG_CLASS_UNIT_HUD_INTERFACE)
            EXTRACT_TAG_CLASS(Unit, TAG_CLASS_UNIT)
            EXTRACT_TAG_CLASS(UnicodeStringList, TAG_CLASS_UNICODE_STRING_LIST)
            EXTRACT_TAG_CLASS(VirtualKeyboard, TAG_CLASS_VIRTUAL_KEYBOARD)
            EXTRACT_TAG_CLASS(Vehicle, TAG_CLASS_VEHICLE)
            EXTRACT_TAG_CLASS(Weapon, TAG_CLASS_WEAPON)
            EXTRACT_TAG_CLASS(Wind, TAG_CLASS_WIND)
            EXTRACT_TAG_CLASS(WeaponHUDInterface, TAG_CLASS_WEAPON_HUD_INTERFACE)
            EXTRACT_TAG_CLASS(ExtendedBitmap, TAG_CLASS_EXTENDED_BITMAP)

            case TagClassInt::TAG_CLASS_SCENARIO_STRUCTURE_BSP: {
                if(tag.get_map().get_engine() == HEK::CacheFileEngine::CACHE_FILE_DARK_CIRCLET) {
                    return Parser::ScenarioStructureBSP::parse_cache_file_data(tag).generate_hek_tag_data(TagClassInt::TAG_CLASS_SCENARIO_STRUCTURE_BSP);
                }
                else {
                    HEK::TagFileHeader tag_data_header(TagClassInt::TAG_CLASS_SCENARIO_STRUCTURE_BSP);
                    std::vector<std::byte> data(reinterpret_cast<std::byte *>(&tag_data_header), reinterpret_cast<std::byte *>(&tag_data_header + 1));
                    auto sbsp_header_data = tag.get_base_struct<HEK::ScenarioStructureBSPCompiledHeader>();
                    auto sbsp_header_pointer = sbsp_header_data.pointer.read();
                    return Parser::ScenarioStructureBSP::parse_cache_file_data(tag, sbsp_header_pointer).generate_hek_tag_data(TagClassInt::TAG_CLASS_SCENARIO_STRUCTURE_BSP);
                }
            }

            case TagClassInt::TAG_CLASS_PREFERENCES_NETWORK_GAME:
            case TagClassInt::TAG_CLASS_SPHEROID:
            case TagClassInt::TAG_CLASS_CONTINUOUS_DAMAGE_EFFECT:
            case TagClassInt::TAG_CLASS_MODEL:
            case TagClassInt::TAG_CLASS_NONE:
            case TagClassInt::TAG_CLASS_NULL:
            case TagClassInt::TAG_CLASS_EXTENDED_SOUND:
            case TagClassInt::TAG_CLASS_EXTENDED_SCENARIO:
            case TagClassInt::TAG_CLASS_NEW_FONT:
            case TagClassInt::TAG_CLASS_NEW_UI_WIDGET_DEFINITION:
            case TagClassInt::TAG_CLASS_NEW_UNIT_HUD_INTERFACE:
            case TagClassInt::TAG_CLASS_NEW_WEAPON_HUD_INTERFACE:
            case TagClassInt::TAG_CLASS_SHADER_TRANSPARENT_GLSL:
                break;
        }

        eprintf_error("Tag class %s is unsupported", tag_class_to_extension(tag_class_int));

        throw InvalidTagDataException();
    }
}
