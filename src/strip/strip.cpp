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

int strip_tag(const char *file_path, bool preprocess) {
    // Open the tag
    auto tag = Invader::File::open_file(file_path);
    if(!tag.has_value()) {
        eprintf_error("Failed to open %s", file_path);
        return EXIT_FAILURE;
    }

    // Get the header
    std::vector<std::byte> file_data;
    try {
        const auto *header = reinterpret_cast<const Invader::HEK::TagFileHeader *>(tag->data());
        Invader::HEK::TagFileHeader::validate_header(header, tag->size());

        #define STRIP_TAG_CLASS(class_struct, class_int) case Invader::TagClassInt::class_int: { \
            file_data = Invader::Parser::class_struct::parse_hek_tag_file(tag->data(), tag->size(), preprocess).generate_hek_tag_data(Invader::TagClassInt::class_int); \
            break; \
        }

        switch(header->tag_class_int) {
            STRIP_TAG_CLASS(Actor, TAG_CLASS_ACTOR)
            STRIP_TAG_CLASS(ActorVariant, TAG_CLASS_ACTOR_VARIANT)
            STRIP_TAG_CLASS(Antenna, TAG_CLASS_ANTENNA)
            STRIP_TAG_CLASS(ModelAnimations, TAG_CLASS_MODEL_ANIMATIONS)
            STRIP_TAG_CLASS(Biped, TAG_CLASS_BIPED)
            STRIP_TAG_CLASS(Bitmap, TAG_CLASS_BITMAP)
            STRIP_TAG_CLASS(ModelCollisionGeometry, TAG_CLASS_MODEL_COLLISION_GEOMETRY)
            STRIP_TAG_CLASS(ColorTable, TAG_CLASS_COLOR_TABLE)
            STRIP_TAG_CLASS(Contrail, TAG_CLASS_CONTRAIL)
            STRIP_TAG_CLASS(DeviceControl, TAG_CLASS_DEVICE_CONTROL)
            STRIP_TAG_CLASS(Decal, TAG_CLASS_DECAL)
            STRIP_TAG_CLASS(UIWidgetDefinition, TAG_CLASS_UI_WIDGET_DEFINITION)
            STRIP_TAG_CLASS(InputDeviceDefaults, TAG_CLASS_INPUT_DEVICE_DEFAULTS)
            STRIP_TAG_CLASS(Device, TAG_CLASS_DEVICE)
            STRIP_TAG_CLASS(DetailObjectCollection, TAG_CLASS_DETAIL_OBJECT_COLLECTION)
            STRIP_TAG_CLASS(Effect, TAG_CLASS_EFFECT)
            STRIP_TAG_CLASS(Equipment, TAG_CLASS_EQUIPMENT)
            STRIP_TAG_CLASS(Flag, TAG_CLASS_FLAG)
            STRIP_TAG_CLASS(Fog, TAG_CLASS_FOG)
            STRIP_TAG_CLASS(Font, TAG_CLASS_FONT)
            STRIP_TAG_CLASS(MaterialEffects, TAG_CLASS_MATERIAL_EFFECTS)
            STRIP_TAG_CLASS(Garbage, TAG_CLASS_GARBAGE)
            STRIP_TAG_CLASS(Glow, TAG_CLASS_GLOW)
            STRIP_TAG_CLASS(GrenadeHUDInterface, TAG_CLASS_GRENADE_HUD_INTERFACE)
            STRIP_TAG_CLASS(HUDMessageText, TAG_CLASS_HUD_MESSAGE_TEXT)
            STRIP_TAG_CLASS(HUDNumber, TAG_CLASS_HUD_NUMBER)
            STRIP_TAG_CLASS(HUDGlobals, TAG_CLASS_HUD_GLOBALS)
            STRIP_TAG_CLASS(Item, TAG_CLASS_ITEM)
            STRIP_TAG_CLASS(ItemCollection, TAG_CLASS_ITEM_COLLECTION)
            STRIP_TAG_CLASS(DamageEffect, TAG_CLASS_DAMAGE_EFFECT)
            STRIP_TAG_CLASS(LensFlare, TAG_CLASS_LENS_FLARE)
            STRIP_TAG_CLASS(Lightning, TAG_CLASS_LIGHTNING)
            STRIP_TAG_CLASS(DeviceLightFixture, TAG_CLASS_DEVICE_LIGHT_FIXTURE)
            STRIP_TAG_CLASS(Light, TAG_CLASS_LIGHT)
            STRIP_TAG_CLASS(SoundLooping, TAG_CLASS_SOUND_LOOPING)
            STRIP_TAG_CLASS(DeviceMachine, TAG_CLASS_DEVICE_MACHINE)
            STRIP_TAG_CLASS(Globals, TAG_CLASS_GLOBALS)
            STRIP_TAG_CLASS(Meter, TAG_CLASS_METER)
            STRIP_TAG_CLASS(LightVolume, TAG_CLASS_LIGHT_VOLUME)
            STRIP_TAG_CLASS(GBXModel, TAG_CLASS_GBXMODEL)
            STRIP_TAG_CLASS(MultiplayerScenarioDescription, TAG_CLASS_MULTIPLAYER_SCENARIO_DESCRIPTION)
            STRIP_TAG_CLASS(Object, TAG_CLASS_OBJECT)
            STRIP_TAG_CLASS(Particle, TAG_CLASS_PARTICLE)
            STRIP_TAG_CLASS(ParticleSystem, TAG_CLASS_PARTICLE_SYSTEM)
            STRIP_TAG_CLASS(Physics, TAG_CLASS_PHYSICS)
            STRIP_TAG_CLASS(Placeholder, TAG_CLASS_PLACEHOLDER)
            STRIP_TAG_CLASS(PointPhysics, TAG_CLASS_POINT_PHYSICS)
            STRIP_TAG_CLASS(Projectile, TAG_CLASS_PROJECTILE)
            STRIP_TAG_CLASS(WeatherParticleSystem, TAG_CLASS_WEATHER_PARTICLE_SYSTEM)
            STRIP_TAG_CLASS(Scenery, TAG_CLASS_SCENERY)
            STRIP_TAG_CLASS(ShaderTransparentChicagoExtended, TAG_CLASS_SHADER_TRANSPARENT_CHICAGO_EXTENDED)
            STRIP_TAG_CLASS(ShaderTransparentChicago, TAG_CLASS_SHADER_TRANSPARENT_CHICAGO)
            STRIP_TAG_CLASS(Scenario, TAG_CLASS_SCENARIO)
            STRIP_TAG_CLASS(ShaderEnvironment, TAG_CLASS_SHADER_ENVIRONMENT)
            STRIP_TAG_CLASS(ShaderTransparentGlass, TAG_CLASS_SHADER_TRANSPARENT_GLASS)
            STRIP_TAG_CLASS(Shader, TAG_CLASS_SHADER)
            STRIP_TAG_CLASS(Sky, TAG_CLASS_SKY)
            STRIP_TAG_CLASS(ShaderTransparentMeter, TAG_CLASS_SHADER_TRANSPARENT_METER)
            STRIP_TAG_CLASS(Sound, TAG_CLASS_SOUND)
            STRIP_TAG_CLASS(SoundEnvironment, TAG_CLASS_SOUND_ENVIRONMENT)
            STRIP_TAG_CLASS(ShaderModel, TAG_CLASS_SHADER_MODEL)
            STRIP_TAG_CLASS(ShaderTransparentGeneric, TAG_CLASS_SHADER_TRANSPARENT_GENERIC)
            STRIP_TAG_CLASS(TagCollection, TAG_CLASS_UI_WIDGET_COLLECTION)
            STRIP_TAG_CLASS(ShaderTransparentPlasma, TAG_CLASS_SHADER_TRANSPARENT_PLASMA)
            STRIP_TAG_CLASS(SoundScenery, TAG_CLASS_SOUND_SCENERY)
            STRIP_TAG_CLASS(StringList, TAG_CLASS_STRING_LIST)
            STRIP_TAG_CLASS(ShaderTransparentWater, TAG_CLASS_SHADER_TRANSPARENT_WATER)
            STRIP_TAG_CLASS(TagCollection, TAG_CLASS_TAG_COLLECTION)
            STRIP_TAG_CLASS(CameraTrack, TAG_CLASS_CAMERA_TRACK)
            STRIP_TAG_CLASS(Dialogue, TAG_CLASS_DIALOGUE)
            STRIP_TAG_CLASS(UnitHUDInterface, TAG_CLASS_UNIT_HUD_INTERFACE)
            STRIP_TAG_CLASS(Unit, TAG_CLASS_UNIT)
            STRIP_TAG_CLASS(UnicodeStringList, TAG_CLASS_UNICODE_STRING_LIST)
            STRIP_TAG_CLASS(VirtualKeyboard, TAG_CLASS_VIRTUAL_KEYBOARD)
            STRIP_TAG_CLASS(Vehicle, TAG_CLASS_VEHICLE)
            STRIP_TAG_CLASS(Weapon, TAG_CLASS_WEAPON)
            STRIP_TAG_CLASS(Wind, TAG_CLASS_WIND)
            STRIP_TAG_CLASS(WeaponHUDInterface, TAG_CLASS_WEAPON_HUD_INTERFACE)
            STRIP_TAG_CLASS(ScenarioStructureBSP, TAG_CLASS_SCENARIO_STRUCTURE_BSP)

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
        eprintf_error("Error: Failed to strip %s: %s", file_path, e.what());
        return EXIT_FAILURE;
    }

    if(!Invader::File::save_file(file_path, file_data)) {
        eprintf_error("Error: Failed to write to %s.", file_path);
        return EXIT_FAILURE;
    }

    oprintf_success("Stripped %s", file_path);

    return EXIT_SUCCESS;
}

int main(int argc, char * const *argv) {
    EXIT_IF_INVADER_EXTRACT_HIDDEN_VALUES

    std::vector<Invader::CommandLineOption> options;
    options.emplace_back("info", 'i', 0, "Show license and credits.");
    options.emplace_back("tags", 't', 1, "Use the specified tags directory.", "<dir>");
    options.emplace_back("fs-path", 'P', 0, "Use a filesystem path for the tag path if specifying a tag.");
    options.emplace_back("preprocessor", 'p', 0, "Save the result of the tag preprocessor rather than just a strip. This is to allow easier tag comparison.");
    options.emplace_back("all", 'a', 0, "Strip all tags in the tags directory.");

    static constexpr char DESCRIPTION[] = "Strips extra hidden data from tags.";
    static constexpr char USAGE[] = "[options] <-a | tag.class>";

    struct StripOptions {
        const char *tags = "tags";
        bool use_filesystem_path = false;
        bool all = false;
        bool preprocess = false;
    } strip_options;

    auto remaining_arguments = Invader::CommandLineOption::parse_arguments<StripOptions &>(argc, argv, options, USAGE, DESCRIPTION, 0, 1, strip_options, [](char opt, const std::vector<const char *> &arguments, auto &strip_options) {
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
            case 'a':
                strip_options.all = true;
                break;
            case 'p':
                strip_options.preprocess = true;
                break;
        }
    });

    bool preprocess = strip_options.preprocess;

    if(remaining_arguments.size() == 0) {
        if(!strip_options.all) {
            eprintf_error("Expected --all to be used OR a tag path. Use -h for more information.");
            return EXIT_FAILURE;
        }

        std::size_t success = 0;
        std::size_t total = 0;

        auto recursively_strip_dir = [&total, &success, &preprocess](const std::filesystem::path &dir, auto &recursively_strip_dir) -> void {
            for(auto i : std::filesystem::directory_iterator(dir)) {
                if(i.is_directory()) {
                    recursively_strip_dir(i, recursively_strip_dir);
                }
                else if(i.is_regular_file()) {
                    total++;
                    success += strip_tag(i.path().string().c_str(), preprocess) == EXIT_SUCCESS;
                }
            }
        };

        recursively_strip_dir(std::filesystem::path(strip_options.tags), recursively_strip_dir);

        oprintf("Stripped %zu out of %zu tag%s\n", success, total, total == 1 ? "" : "s");

        return EXIT_SUCCESS;
    }
    else if(strip_options.all) {
        eprintf_error("--all and a tag path cannot be used at the same time. Use -h for more information.");
        return EXIT_FAILURE;
    }
    else {
        std::filesystem::path file_path;
        if(strip_options.use_filesystem_path) {
            file_path = std::string(remaining_arguments[0]);
        }
        else {
            file_path = std::filesystem::path(strip_options.tags) / Invader::File::halo_path_to_preferred_path(remaining_arguments[0]);
        }
        std::string file_path_str = file_path.string();
        return strip_tag(file_path_str.c_str(), preprocess);
    }
}
