// SPDX-License-Identifier: GPL-3.0-only

#include <vector>
#include <string>
#include <filesystem>
#include <invader/printf.hpp>
#include <invader/version.hpp>
#include <invader/tag/hek/header.hpp>
#include <invader/tag/hek/definition.hpp>
#include <invader/command_line_option.hpp>
#include <invader/tag/parser/parser.hpp>
#include <invader/file/file.hpp>

int main(int argc, char * const *argv) {
    EXIT_IF_INVADER_EXTRACT_HIDDEN_VALUES

    std::vector<Invader::CommandLineOption> options;
    options.emplace_back("info", 'i', 0, "Show license and credits.");
    options.emplace_back("tags", 't', 1, "Use the specified tags directory.", "<dir>");
    options.emplace_back("fs-path", 'P', 0, "Use a filesystem path for the text file.");

    static constexpr char DESCRIPTION[] = "Strips extra hidden data from tags.";
    static constexpr char USAGE[] = "[options] <tag.class>";

    struct StripOptions {
        const char *tags = "tags";
        bool use_filesystem_path = false;
    } strip_options;

    auto remaining_arguments = Invader::CommandLineOption::parse_arguments<StripOptions &>(argc, argv, options, USAGE, DESCRIPTION, 1, 1, strip_options, [](char opt, const std::vector<const char *> &arguments, auto &strip_options) {
        switch(opt) {
            case 't':
                strip_options.tags = arguments[0];
                break;
            case 'i':
                Invader::show_version_info();
                std::exit(EXIT_SUCCESS);
            case 'P':
                strip_options.use_filesystem_path = true;
                break;
        }
    });

    // Check if there's a string tag
    std::filesystem::path file_path;
    if(strip_options.use_filesystem_path) {
        file_path = std::string(remaining_arguments[0]);
    }
    else {
        file_path = std::filesystem::path(strip_options.tags) / Invader::File::halo_path_to_preferred_path(remaining_arguments[0]);
    }
    std::string file_path_str = file_path.string();
    auto tag = Invader::File::open_file(file_path_str.c_str());
    if(!tag.has_value()) {
        eprintf_error("Failed to open %s", file_path_str.c_str());
        return EXIT_FAILURE;
    }

    // Get the header
    std::vector<std::byte> file_data;
    try {
        const auto *header = reinterpret_cast<const Invader::HEK::TagFileHeader *>(tag->data());
        Invader::HEK::TagFileHeader::validate_header(header, tag->size());

        #define TOUCH_TAG_CLASS(class_struct, class_int) case Invader::TagClassInt::class_int: { \
            file_data = Invader::Parser::class_struct::parse_hek_tag_file(tag->data(), tag->size()).generate_hek_tag_data(Invader::TagClassInt::class_int); \
            break; \
        }

        switch(header->tag_class_int) {
            TOUCH_TAG_CLASS(Actor, TAG_CLASS_ACTOR)
            TOUCH_TAG_CLASS(ActorVariant, TAG_CLASS_ACTOR_VARIANT)
            TOUCH_TAG_CLASS(Antenna, TAG_CLASS_ANTENNA)
            TOUCH_TAG_CLASS(ModelAnimations, TAG_CLASS_MODEL_ANIMATIONS)
            TOUCH_TAG_CLASS(Biped, TAG_CLASS_BIPED)
            TOUCH_TAG_CLASS(Bitmap, TAG_CLASS_BITMAP)
            TOUCH_TAG_CLASS(ModelCollisionGeometry, TAG_CLASS_MODEL_COLLISION_GEOMETRY)
            TOUCH_TAG_CLASS(ColorTable, TAG_CLASS_COLOR_TABLE)
            TOUCH_TAG_CLASS(Contrail, TAG_CLASS_CONTRAIL)
            TOUCH_TAG_CLASS(DeviceControl, TAG_CLASS_DEVICE_CONTROL)
            TOUCH_TAG_CLASS(Decal, TAG_CLASS_DECAL)
            TOUCH_TAG_CLASS(UIWidgetDefinition, TAG_CLASS_UI_WIDGET_DEFINITION)
            TOUCH_TAG_CLASS(InputDeviceDefaults, TAG_CLASS_INPUT_DEVICE_DEFAULTS)
            TOUCH_TAG_CLASS(Device, TAG_CLASS_DEVICE)
            TOUCH_TAG_CLASS(DetailObjectCollection, TAG_CLASS_DETAIL_OBJECT_COLLECTION)
            TOUCH_TAG_CLASS(Effect, TAG_CLASS_EFFECT)
            TOUCH_TAG_CLASS(Equipment, TAG_CLASS_EQUIPMENT)
            TOUCH_TAG_CLASS(Flag, TAG_CLASS_FLAG)
            TOUCH_TAG_CLASS(Fog, TAG_CLASS_FOG)
            TOUCH_TAG_CLASS(Font, TAG_CLASS_FONT)
            TOUCH_TAG_CLASS(MaterialEffects, TAG_CLASS_MATERIAL_EFFECTS)
            TOUCH_TAG_CLASS(Garbage, TAG_CLASS_GARBAGE)
            TOUCH_TAG_CLASS(Glow, TAG_CLASS_GLOW)
            TOUCH_TAG_CLASS(GrenadeHUDInterface, TAG_CLASS_GRENADE_HUD_INTERFACE)
            TOUCH_TAG_CLASS(HUDMessageText, TAG_CLASS_HUD_MESSAGE_TEXT)
            TOUCH_TAG_CLASS(HUDNumber, TAG_CLASS_HUD_NUMBER)
            TOUCH_TAG_CLASS(HUDGlobals, TAG_CLASS_HUD_GLOBALS)
            TOUCH_TAG_CLASS(Item, TAG_CLASS_ITEM)
            TOUCH_TAG_CLASS(ItemCollection, TAG_CLASS_ITEM_COLLECTION)
            TOUCH_TAG_CLASS(DamageEffect, TAG_CLASS_DAMAGE_EFFECT)
            TOUCH_TAG_CLASS(LensFlare, TAG_CLASS_LENS_FLARE)
            TOUCH_TAG_CLASS(Lightning, TAG_CLASS_LIGHTNING)
            TOUCH_TAG_CLASS(DeviceLightFixture, TAG_CLASS_DEVICE_LIGHT_FIXTURE)
            TOUCH_TAG_CLASS(Light, TAG_CLASS_LIGHT)
            TOUCH_TAG_CLASS(SoundLooping, TAG_CLASS_SOUND_LOOPING)
            TOUCH_TAG_CLASS(DeviceMachine, TAG_CLASS_DEVICE_MACHINE)
            TOUCH_TAG_CLASS(Globals, TAG_CLASS_GLOBALS)
            TOUCH_TAG_CLASS(Meter, TAG_CLASS_METER)
            TOUCH_TAG_CLASS(LightVolume, TAG_CLASS_LIGHT_VOLUME)
            TOUCH_TAG_CLASS(GBXModel, TAG_CLASS_GBXMODEL)
            TOUCH_TAG_CLASS(MultiplayerScenarioDescription, TAG_CLASS_MULTIPLAYER_SCENARIO_DESCRIPTION)
            TOUCH_TAG_CLASS(Object, TAG_CLASS_OBJECT)
            TOUCH_TAG_CLASS(Particle, TAG_CLASS_PARTICLE)
            TOUCH_TAG_CLASS(ParticleSystem, TAG_CLASS_PARTICLE_SYSTEM)
            TOUCH_TAG_CLASS(Physics, TAG_CLASS_PHYSICS)
            TOUCH_TAG_CLASS(Placeholder, TAG_CLASS_PLACEHOLDER)
            TOUCH_TAG_CLASS(PointPhysics, TAG_CLASS_POINT_PHYSICS)
            TOUCH_TAG_CLASS(Projectile, TAG_CLASS_PROJECTILE)
            TOUCH_TAG_CLASS(WeatherParticleSystem, TAG_CLASS_WEATHER_PARTICLE_SYSTEM)
            TOUCH_TAG_CLASS(Scenery, TAG_CLASS_SCENERY)
            TOUCH_TAG_CLASS(ShaderTransparentChicagoExtended, TAG_CLASS_SHADER_TRANSPARENT_CHICAGO_EXTENDED)
            TOUCH_TAG_CLASS(ShaderTransparentChicago, TAG_CLASS_SHADER_TRANSPARENT_CHICAGO)
            TOUCH_TAG_CLASS(Scenario, TAG_CLASS_SCENARIO)
            TOUCH_TAG_CLASS(ShaderEnvironment, TAG_CLASS_SHADER_ENVIRONMENT)
            TOUCH_TAG_CLASS(ShaderTransparentGlass, TAG_CLASS_SHADER_TRANSPARENT_GLASS)
            TOUCH_TAG_CLASS(Shader, TAG_CLASS_SHADER)
            TOUCH_TAG_CLASS(Sky, TAG_CLASS_SKY)
            TOUCH_TAG_CLASS(ShaderTransparentMeter, TAG_CLASS_SHADER_TRANSPARENT_METER)
            TOUCH_TAG_CLASS(Sound, TAG_CLASS_SOUND)
            TOUCH_TAG_CLASS(SoundEnvironment, TAG_CLASS_SOUND_ENVIRONMENT)
            TOUCH_TAG_CLASS(ShaderModel, TAG_CLASS_SHADER_MODEL)
            TOUCH_TAG_CLASS(ShaderTransparentGeneric, TAG_CLASS_SHADER_TRANSPARENT_GENERIC)
            TOUCH_TAG_CLASS(TagCollection, TAG_CLASS_UI_WIDGET_COLLECTION)
            TOUCH_TAG_CLASS(ShaderTransparentPlasma, TAG_CLASS_SHADER_TRANSPARENT_PLASMA)
            TOUCH_TAG_CLASS(SoundScenery, TAG_CLASS_SOUND_SCENERY)
            TOUCH_TAG_CLASS(StringList, TAG_CLASS_STRING_LIST)
            TOUCH_TAG_CLASS(ShaderTransparentWater, TAG_CLASS_SHADER_TRANSPARENT_WATER)
            TOUCH_TAG_CLASS(TagCollection, TAG_CLASS_TAG_COLLECTION)
            TOUCH_TAG_CLASS(CameraTrack, TAG_CLASS_CAMERA_TRACK)
            TOUCH_TAG_CLASS(Dialogue, TAG_CLASS_DIALOGUE)
            TOUCH_TAG_CLASS(UnitHUDInterface, TAG_CLASS_UNIT_HUD_INTERFACE)
            TOUCH_TAG_CLASS(Unit, TAG_CLASS_UNIT)
            TOUCH_TAG_CLASS(UnicodeStringList, TAG_CLASS_UNICODE_STRING_LIST)
            TOUCH_TAG_CLASS(VirtualKeyboard, TAG_CLASS_VIRTUAL_KEYBOARD)
            TOUCH_TAG_CLASS(Vehicle, TAG_CLASS_VEHICLE)
            TOUCH_TAG_CLASS(Weapon, TAG_CLASS_WEAPON)
            TOUCH_TAG_CLASS(Wind, TAG_CLASS_WIND)
            TOUCH_TAG_CLASS(WeaponHUDInterface, TAG_CLASS_WEAPON_HUD_INTERFACE)
            TOUCH_TAG_CLASS(ScenarioStructureBSP, TAG_CLASS_SCENARIO_STRUCTURE_BSP)

            case Invader::HEK::TagClassInt::TAG_CLASS_PREFERENCES_NETWORK_GAME:
            case Invader::HEK::TagClassInt::TAG_CLASS_SPHEROID:
            case Invader::HEK::TagClassInt::TAG_CLASS_CONTINUOUS_DAMAGE_EFFECT:
            case Invader::HEK::TagClassInt::TAG_CLASS_MODEL:
            case Invader::HEK::TagClassInt::TAG_CLASS_NONE:
            case Invader::HEK::TagClassInt::TAG_CLASS_NULL:
                eprintf_error("Error: Cannot handle %s.", tag_class_to_extension(header->tag_class_int));
                return EXIT_FAILURE;
        }
    }
    catch(std::exception &e) {
        eprintf_error("Error: Failed to strip %s: %s", file_path_str.c_str(), e.what());
        return EXIT_FAILURE;
    }

    if(!Invader::File::save_file(file_path_str.c_str(), file_data)) {
        eprintf_error("Error: Failed to write to %s.", file_path_str.c_str());
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
