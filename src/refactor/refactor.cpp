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

std::size_t refactor_tag(const char *file_path, const char *from_path, Invader::TagClassInt from_class, const char *to_path, Invader::TagClassInt to_class) {
    // Open the tag
    auto tag = Invader::File::open_file(file_path);
    if(!tag.has_value()) {
        eprintf_error("Failed to open %s", file_path);
        return EXIT_FAILURE;
    }

    // Get the header
    std::vector<std::byte> file_data;
    std::size_t count;

    try {
        const auto *header = reinterpret_cast<const Invader::HEK::TagFileHeader *>(tag->data());
        Invader::HEK::TagFileHeader::validate_header(header, tag->size());

        #define REFACTOR_TAG_CLASS(class_struct, class_int) case Invader::TagClassInt::class_int: { \
            auto tag_data = Invader::Parser::class_struct::parse_hek_tag_file(tag->data(), tag->size()); \
            count = tag_data.refactor_reference(from_path, from_class, to_path, to_class); \
            if(count) { \
                file_data = tag_data.generate_hek_tag_data(Invader::TagClassInt::class_int); \
            } \
            else { \
                return count; \
            } \
            break; \
        }

        switch(header->tag_class_int) {
            REFACTOR_TAG_CLASS(Actor, TAG_CLASS_ACTOR)
            REFACTOR_TAG_CLASS(ActorVariant, TAG_CLASS_ACTOR_VARIANT)
            REFACTOR_TAG_CLASS(Antenna, TAG_CLASS_ANTENNA)
            REFACTOR_TAG_CLASS(ModelAnimations, TAG_CLASS_MODEL_ANIMATIONS)
            REFACTOR_TAG_CLASS(Biped, TAG_CLASS_BIPED)
            REFACTOR_TAG_CLASS(Bitmap, TAG_CLASS_BITMAP)
            REFACTOR_TAG_CLASS(ModelCollisionGeometry, TAG_CLASS_MODEL_COLLISION_GEOMETRY)
            REFACTOR_TAG_CLASS(ColorTable, TAG_CLASS_COLOR_TABLE)
            REFACTOR_TAG_CLASS(Contrail, TAG_CLASS_CONTRAIL)
            REFACTOR_TAG_CLASS(DeviceControl, TAG_CLASS_DEVICE_CONTROL)
            REFACTOR_TAG_CLASS(Decal, TAG_CLASS_DECAL)
            REFACTOR_TAG_CLASS(UIWidgetDefinition, TAG_CLASS_UI_WIDGET_DEFINITION)
            REFACTOR_TAG_CLASS(InputDeviceDefaults, TAG_CLASS_INPUT_DEVICE_DEFAULTS)
            REFACTOR_TAG_CLASS(Device, TAG_CLASS_DEVICE)
            REFACTOR_TAG_CLASS(DetailObjectCollection, TAG_CLASS_DETAIL_OBJECT_COLLECTION)
            REFACTOR_TAG_CLASS(Effect, TAG_CLASS_EFFECT)
            REFACTOR_TAG_CLASS(Equipment, TAG_CLASS_EQUIPMENT)
            REFACTOR_TAG_CLASS(Flag, TAG_CLASS_FLAG)
            REFACTOR_TAG_CLASS(Fog, TAG_CLASS_FOG)
            REFACTOR_TAG_CLASS(Font, TAG_CLASS_FONT)
            REFACTOR_TAG_CLASS(MaterialEffects, TAG_CLASS_MATERIAL_EFFECTS)
            REFACTOR_TAG_CLASS(Garbage, TAG_CLASS_GARBAGE)
            REFACTOR_TAG_CLASS(Glow, TAG_CLASS_GLOW)
            REFACTOR_TAG_CLASS(GrenadeHUDInterface, TAG_CLASS_GRENADE_HUD_INTERFACE)
            REFACTOR_TAG_CLASS(HUDMessageText, TAG_CLASS_HUD_MESSAGE_TEXT)
            REFACTOR_TAG_CLASS(HUDNumber, TAG_CLASS_HUD_NUMBER)
            REFACTOR_TAG_CLASS(HUDGlobals, TAG_CLASS_HUD_GLOBALS)
            REFACTOR_TAG_CLASS(Item, TAG_CLASS_ITEM)
            REFACTOR_TAG_CLASS(ItemCollection, TAG_CLASS_ITEM_COLLECTION)
            REFACTOR_TAG_CLASS(DamageEffect, TAG_CLASS_DAMAGE_EFFECT)
            REFACTOR_TAG_CLASS(LensFlare, TAG_CLASS_LENS_FLARE)
            REFACTOR_TAG_CLASS(Lightning, TAG_CLASS_LIGHTNING)
            REFACTOR_TAG_CLASS(DeviceLightFixture, TAG_CLASS_DEVICE_LIGHT_FIXTURE)
            REFACTOR_TAG_CLASS(Light, TAG_CLASS_LIGHT)
            REFACTOR_TAG_CLASS(SoundLooping, TAG_CLASS_SOUND_LOOPING)
            REFACTOR_TAG_CLASS(DeviceMachine, TAG_CLASS_DEVICE_MACHINE)
            REFACTOR_TAG_CLASS(Globals, TAG_CLASS_GLOBALS)
            REFACTOR_TAG_CLASS(Meter, TAG_CLASS_METER)
            REFACTOR_TAG_CLASS(LightVolume, TAG_CLASS_LIGHT_VOLUME)
            REFACTOR_TAG_CLASS(GBXModel, TAG_CLASS_GBXMODEL)
            REFACTOR_TAG_CLASS(MultiplayerScenarioDescription, TAG_CLASS_MULTIPLAYER_SCENARIO_DESCRIPTION)
            REFACTOR_TAG_CLASS(Object, TAG_CLASS_OBJECT)
            REFACTOR_TAG_CLASS(Particle, TAG_CLASS_PARTICLE)
            REFACTOR_TAG_CLASS(ParticleSystem, TAG_CLASS_PARTICLE_SYSTEM)
            REFACTOR_TAG_CLASS(Physics, TAG_CLASS_PHYSICS)
            REFACTOR_TAG_CLASS(Placeholder, TAG_CLASS_PLACEHOLDER)
            REFACTOR_TAG_CLASS(PointPhysics, TAG_CLASS_POINT_PHYSICS)
            REFACTOR_TAG_CLASS(Projectile, TAG_CLASS_PROJECTILE)
            REFACTOR_TAG_CLASS(WeatherParticleSystem, TAG_CLASS_WEATHER_PARTICLE_SYSTEM)
            REFACTOR_TAG_CLASS(Scenery, TAG_CLASS_SCENERY)
            REFACTOR_TAG_CLASS(ShaderTransparentChicagoExtended, TAG_CLASS_SHADER_TRANSPARENT_CHICAGO_EXTENDED)
            REFACTOR_TAG_CLASS(ShaderTransparentChicago, TAG_CLASS_SHADER_TRANSPARENT_CHICAGO)
            REFACTOR_TAG_CLASS(Scenario, TAG_CLASS_SCENARIO)
            REFACTOR_TAG_CLASS(ShaderEnvironment, TAG_CLASS_SHADER_ENVIRONMENT)
            REFACTOR_TAG_CLASS(ShaderTransparentGlass, TAG_CLASS_SHADER_TRANSPARENT_GLASS)
            REFACTOR_TAG_CLASS(Shader, TAG_CLASS_SHADER)
            REFACTOR_TAG_CLASS(Sky, TAG_CLASS_SKY)
            REFACTOR_TAG_CLASS(ShaderTransparentMeter, TAG_CLASS_SHADER_TRANSPARENT_METER)
            REFACTOR_TAG_CLASS(Sound, TAG_CLASS_SOUND)
            REFACTOR_TAG_CLASS(SoundEnvironment, TAG_CLASS_SOUND_ENVIRONMENT)
            REFACTOR_TAG_CLASS(ShaderModel, TAG_CLASS_SHADER_MODEL)
            REFACTOR_TAG_CLASS(ShaderTransparentGeneric, TAG_CLASS_SHADER_TRANSPARENT_GENERIC)
            REFACTOR_TAG_CLASS(TagCollection, TAG_CLASS_UI_WIDGET_COLLECTION)
            REFACTOR_TAG_CLASS(ShaderTransparentPlasma, TAG_CLASS_SHADER_TRANSPARENT_PLASMA)
            REFACTOR_TAG_CLASS(SoundScenery, TAG_CLASS_SOUND_SCENERY)
            REFACTOR_TAG_CLASS(StringList, TAG_CLASS_STRING_LIST)
            REFACTOR_TAG_CLASS(ShaderTransparentWater, TAG_CLASS_SHADER_TRANSPARENT_WATER)
            REFACTOR_TAG_CLASS(TagCollection, TAG_CLASS_TAG_COLLECTION)
            REFACTOR_TAG_CLASS(CameraTrack, TAG_CLASS_CAMERA_TRACK)
            REFACTOR_TAG_CLASS(Dialogue, TAG_CLASS_DIALOGUE)
            REFACTOR_TAG_CLASS(UnitHUDInterface, TAG_CLASS_UNIT_HUD_INTERFACE)
            REFACTOR_TAG_CLASS(Unit, TAG_CLASS_UNIT)
            REFACTOR_TAG_CLASS(UnicodeStringList, TAG_CLASS_UNICODE_STRING_LIST)
            REFACTOR_TAG_CLASS(VirtualKeyboard, TAG_CLASS_VIRTUAL_KEYBOARD)
            REFACTOR_TAG_CLASS(Vehicle, TAG_CLASS_VEHICLE)
            REFACTOR_TAG_CLASS(Weapon, TAG_CLASS_WEAPON)
            REFACTOR_TAG_CLASS(Wind, TAG_CLASS_WIND)
            REFACTOR_TAG_CLASS(WeaponHUDInterface, TAG_CLASS_WEAPON_HUD_INTERFACE)
            REFACTOR_TAG_CLASS(ScenarioStructureBSP, TAG_CLASS_SCENARIO_STRUCTURE_BSP)

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
        eprintf_error("Error: Failed to refactor %s: %s", file_path, e.what());
        return EXIT_FAILURE;
    }

    if(!Invader::File::save_file(file_path, file_data)) {
        eprintf_error("Error: Failed to write to %s.", file_path);
        return EXIT_FAILURE;
    }

    oprintf_success("Replaced %zu reference%s in %s", count, count == 1 ? "" : "s", file_path);

    return count;
}

int main(int argc, char * const *argv) {
    EXIT_IF_INVADER_EXTRACT_HIDDEN_VALUES

    std::vector<Invader::CommandLineOption> options;
    options.emplace_back("info", 'i', 0, "Show license and credits.");
    options.emplace_back("tags", 't', 1, "Use the specified tags directory.", "<dir>");

    static constexpr char DESCRIPTION[] = "Find and replace tag references.";
    static constexpr char USAGE[] = "[options] <from.class> <to.class>";

    struct RefactorOptions {
        const char *tags = "tags";
    } refactor_options;

    auto remaining_arguments = Invader::CommandLineOption::parse_arguments<RefactorOptions &>(argc, argv, options, USAGE, DESCRIPTION, 2, 2, refactor_options, [](char opt, const std::vector<const char *> &arguments, auto &refactor_options) {
        switch(opt) {
            case 't':
                refactor_options.tags = arguments[0];
                break;
            case 'i':
                Invader::show_version_info();
                std::exit(EXIT_SUCCESS);
        }
    });

    // Parse the stuff
    auto from_maybe = Invader::File::split_tag_class_extension(Invader::File::preferred_path_to_halo_path(remaining_arguments[0]));
    auto to_maybe = Invader::File::split_tag_class_extension(Invader::File::preferred_path_to_halo_path(remaining_arguments[1]));

    auto unmaybe = [](auto &value, const char *arg) {
        if(!value.has_value()) {
            eprintf_error("%s is not a valid reference", arg);
            std::exit(EXIT_FAILURE);
        }
        return value.value();
    };

    auto from = unmaybe(from_maybe, remaining_arguments[0]);
    auto to = unmaybe(to_maybe, remaining_arguments[1]);

    // Go through all the tags
    std::size_t total_tags = 0;
    std::size_t total_replaced = 0;
    auto recursively_refactor_dir = [&total_tags, &total_replaced, &from, &to](const std::filesystem::path &dir, auto &recursively_refactor_dir) -> void {
        for(auto i : std::filesystem::directory_iterator(dir)) {
            if(i.is_directory()) {
                recursively_refactor_dir(i, recursively_refactor_dir);
            }
            else if(i.is_regular_file()) {
                std::size_t count = refactor_tag(i.path().string().c_str(), from.first.c_str(), from.second, to.first.c_str(), to.second);
                if(count) {
                    total_tags++;
                    total_replaced += count;
                }
            }
        }
    };

    // Go!
    recursively_refactor_dir(std::filesystem::path(refactor_options.tags), recursively_refactor_dir);
    oprintf("Replaced %zu reference%s in %zu tag%s\n", total_replaced, total_replaced == 1 ? "" : "s", total_tags, total_tags == 1 ? "" : "s");

    return EXIT_SUCCESS;
}
