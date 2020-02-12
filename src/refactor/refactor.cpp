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

using namespace Invader::File;

std::size_t refactor_tags(const char *file_path, std::vector<std::pair<TagFilePath, TagFilePath>> replacements, bool check_only, bool dry_run) {
    // Open the tag
    auto tag = open_file(file_path);
    if(!tag.has_value()) {
        eprintf_error("Failed to open %s", file_path);
        std::exit(EXIT_FAILURE);
    }

    // Get the header
    std::vector<std::byte> file_data;
    std::size_t count = 0;

    try {
        const auto *header = reinterpret_cast<const Invader::HEK::TagFileHeader *>(tag->data());
        Invader::HEK::TagFileHeader::validate_header(header, tag->size());

        #define REFACTOR_TAG_CLASS(class_struct, tag_class_int) case Invader::TagClassInt::tag_class_int: { \
            auto tag_data = Invader::Parser::class_struct::parse_hek_tag_file(tag->data(), tag->size()); \
            for (auto &r : replacements) { \
                count += tag_data.refactor_reference(r.first.path.c_str(), r.first.class_int, r.second.path.c_str(), r.second.class_int); \
            } \
            if(count) { \
                file_data = tag_data.generate_hek_tag_data(Invader::TagClassInt::tag_class_int); \
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
        eprintf_error("Error: Failed to refactor in %s", file_path);
        throw;
    }

    if(!check_only) {
        if(!dry_run && !save_file(file_path, file_data)) {
            eprintf_error("Error: Failed to write to %s. This tag will need to be manually edited.", file_path);
            throw std::exception();
        }
        oprintf_success("Replaced %zu reference%s in %s", count, count == 1 ? "" : "s", file_path);
    }

    return count;
}

int main(int argc, char * const *argv) {
    EXIT_IF_INVADER_EXTRACT_HIDDEN_VALUES

    std::vector<Invader::CommandLineOption> options;
    options.emplace_back("info", 'i', 0, "Show license and credits.");
    options.emplace_back("tags", 't', 1, "Use the specified tags directory. Use multiple times to add more directories, ordered by precedence.", "<dir>");
    options.emplace_back("dry-run", 'D', 0, "Do not actually make any changes. This cannot be set with --move or --no-move.");
    options.emplace_back("move", 'M', 0, "Move files that are being refactored. This can only be set once and cannot be set with --no-move or --dry-run.");
    options.emplace_back("no-move", 'N', 0, "Do not move any files; just change the references in the tags. This can only be set once and cannot be set with --move, --dry-run, or --recursive.");
    options.emplace_back("recursive", 'r', 0, "Recursively move all tags in a directory. This will fail if a tag is present in both the old and new directories, and it cannot be used with --no-move.");
    options.emplace_back("single-tag", 's', 1, "Make changes to a single tag, only, rather than the whole tag directory.", "<path>");

    static constexpr char DESCRIPTION[] = "Find and replace tag references.";
    static constexpr char USAGE[] = "[options] <-M | -N | -D> < <from.class> <to.class> | -r <from-dir> <to-dir> >";

    struct RefactorOptions {
        std::vector<std::string> tags;
        bool no_move = false;
        bool recursive = false;
        bool set_move_or_no_move = false;
        bool dry_run = false;
        const char *single_tag = nullptr;
    } refactor_options;

    auto remaining_arguments = Invader::CommandLineOption::parse_arguments<RefactorOptions &>(argc, argv, options, USAGE, DESCRIPTION, 2, 2, refactor_options, [](char opt, const std::vector<const char *> &arguments, auto &refactor_options) {
        switch(opt) {
            case 't':
                refactor_options.tags.emplace_back(arguments[0]);
                break;
            case 'i':
                Invader::show_version_info();
                std::exit(EXIT_SUCCESS);
            case 'N':
                if(refactor_options.set_move_or_no_move) {
                    // For when you fuck up and need to get taught a lesson by a velociraptor programmer that is eating spaghetti
                    SPAGHETTI_CODE_VELOCIRAPTOR:
                    eprintf_error("Error: -D, -M, or -N can only be set once.");
                    std::exit(EXIT_FAILURE);
                }
                refactor_options.no_move = true;
                refactor_options.set_move_or_no_move = true;
                break;
            case 'M':
                if(refactor_options.set_move_or_no_move) {
                    goto SPAGHETTI_CODE_VELOCIRAPTOR;
                }
                refactor_options.set_move_or_no_move = true;
                break;
            case 'r':
                refactor_options.recursive = true;
                break;
            case 'D':
                if(refactor_options.set_move_or_no_move) {
                    goto SPAGHETTI_CODE_VELOCIRAPTOR;
                }
                refactor_options.dry_run = true;
                refactor_options.set_move_or_no_move = true;
                break;
            case 's':
                refactor_options.single_tag = arguments[0];
                break;
        }
    });

    if(refactor_options.no_move && refactor_options.recursive) {
        eprintf_error("Error: --no-move and --recursive cannot be used at the same time");
        return EXIT_FAILURE;
    }

    if(!refactor_options.set_move_or_no_move) {
        eprintf_error("Error: Either --dry-run, --no-move, or --move need must be set");
        return EXIT_FAILURE;
    }

    if(refactor_options.tags.size() == 0) {
        refactor_options.tags.emplace_back("tags");
    }

    // Figure out what we need to do
    std::vector<std::pair<TagFilePath, TagFilePath>> replacements;
    std::vector<TagFile *> replacements_files;
    std::vector<TagFile> all_tags = load_virtual_tag_folder(refactor_options.tags);
    std::vector<TagFile> single_tag;
    std::vector<TagFile> *tag_to_modify;

    // Do we only need to go through one tag?
    if(refactor_options.single_tag) {
        tag_to_modify = &single_tag;

        // Add this tag to the end
        auto &tag = single_tag.emplace_back();
        auto single_tag_maybe = split_tag_class_extension(halo_path_to_preferred_path(refactor_options.single_tag));
        if(!single_tag_maybe.has_value()) {
            eprintf_error("Error: %s is not a valid tag path", refactor_options.single_tag);
            return EXIT_FAILURE;
        }

        // Get the path together
        tag.tag_class_int = single_tag_maybe->class_int;
        tag.tag_path = single_tag_maybe->path + "." + tag_class_to_extension(single_tag_maybe->class_int);

        // Find it
        auto file_path_maybe = Invader::File::tag_path_to_file_path(tag.tag_path, refactor_options.tags, true);
        if(!file_path_maybe.has_value()) {
            eprintf_error("Error: %s was not found in any tag directory", refactor_options.single_tag);
            return EXIT_FAILURE;
        }

        tag.full_path = *file_path_maybe;
    }
    else {
        tag_to_modify = &all_tags;
    }

    auto unmaybe = [](const auto &value, const char *arg) -> const auto & {
        if(!value.has_value()) {
            eprintf_error("Error: %s is not a valid reference", arg);
            std::exit(EXIT_FAILURE);
        }
        return value.value();
    };

    // If recursive, we need to go through each tag in the tag directory for a match
    if(refactor_options.recursive) {
        auto from_halo = remove_trailing_slashes(preferred_path_to_halo_path(remaining_arguments[0]));
        auto from_halo_size = from_halo.size();
        auto to_halo = remove_trailing_slashes(preferred_path_to_halo_path(remaining_arguments[1]));

        // Go through each tag to find a match
        for(auto &t : all_tags) {
            auto halo_path = preferred_path_to_halo_path(t.tag_path);
            if(halo_path.find(from_halo) == 0 && halo_path[from_halo_size] == '\\') {
                TagFilePath from = unmaybe(split_tag_class_extension(halo_path), t.tag_path.c_str());
                TagFilePath to = {to_halo + from.path.substr(from_halo_size), t.tag_class_int};
                replacements.emplace_back(std::move(from), std::move(to));
                replacements_files.emplace_back(&t);
            }
        }

        if(replacements.size() == 0) {
            eprintf_error("No tags were found in %s", halo_path_to_preferred_path(from_halo).c_str());
            return EXIT_FAILURE;
        }
    }
    else {
        auto from_halo_path = preferred_path_to_halo_path(remaining_arguments[0]);
        auto from_maybe = split_tag_class_extension(from_halo_path);
        auto to_maybe = split_tag_class_extension(preferred_path_to_halo_path(remaining_arguments[1]));

        auto &from = unmaybe(from_maybe, remaining_arguments[0]);
        auto &to = unmaybe(to_maybe, remaining_arguments[1]);
        replacements.emplace_back(from, to);

        if(!refactor_options.no_move) {
            bool added = false;
            for(auto &t : all_tags) {
                if(preferred_path_to_halo_path(t.tag_path) == from_halo_path) {
                    replacements_files.emplace_back(&t);
                    added = true;
                    break;
                }
            }

            if(!added) {
                eprintf_error("Error: %s was not found.", remaining_arguments[0]);
                return EXIT_FAILURE;
            }
        }

        // If we're moving tags, we can't change tag classes
        if(!refactor_options.no_move && to.class_int != from.class_int) {
            eprintf_error("Error: Tag class cannot be changed if moving tags.");
            return EXIT_FAILURE;
        }
    }

    // Go through all the tags and see what needs edited
    std::size_t total_tags = 0;
    std::size_t total_replaced = 0;
    std::vector<TagFile *> tags_to_do;

    for(auto &tag : *tag_to_modify) {
        if(refactor_tags(tag.full_path.string().c_str(), replacements, true, refactor_options.dry_run)) {
            tags_to_do.emplace_back(&tag);
        }
    }

    // Now actually do it
    for(auto *tag : tags_to_do) {
        std::size_t count = refactor_tags(tag->full_path.string().c_str(), replacements, false, refactor_options.dry_run);
        if(count) {
            total_replaced += count;
            total_tags++;
        }
    }

    oprintf("Replaced %zu reference%s in %zu tag%s\n", total_replaced, total_replaced == 1 ? "" : "s", total_tags, total_tags == 1 ? "" : "s");

    // Move everything
    if(!refactor_options.dry_run && !refactor_options.no_move) {
        auto replacement_count = replacements.size();
        bool deleted_error_shown = false;
        for(std::size_t i = 0; i < replacement_count; i++) {
            auto *file = replacements_files[i];
            auto &replacement = replacements[i];

            auto new_path = std::filesystem::path(refactor_options.tags[file->tag_directory]) / (halo_path_to_preferred_path(replacement.second.path) + "." + tag_class_to_extension(replacement.second.class_int));

            // Create directories. If this fails, it probably matters, but it's not critical in and of itself
            try {
                std::filesystem::create_directories(new_path.parent_path());
            }
            catch(std::exception &) {}

            // Rename, copying as a last resort
            bool renamed = false;
            try {
                std::filesystem::rename(file->full_path, new_path);
                renamed = true;
            }
            catch(std::exception &e) {
                try {
                    std::filesystem::copy(file->full_path, new_path);
                    eprintf_error("Error: Failed to move %s to %s, thus it was copied instead: %s\n", file->full_path.string().c_str(), new_path.string().c_str(), e.what());
                }
                catch(std::exception &e) {
                    eprintf_error("Error: Failed to move or copy %s to %s: %s\n", file->full_path.string().c_str(), new_path.string().c_str(), e.what());
                }
            }

            // Lastly, delete empty directories if possible, obviously doing that only if renamed
            if(renamed) {
                auto delete_directory_if_empty = [](std::filesystem::path directory, auto &delete_directory_if_empty, int depth) -> bool {
                    if(++depth == 256) {
                        return false;
                    }
                    for(auto &i : std::filesystem::directory_iterator(directory)) {
                        if(!i.is_directory() || !delete_directory_if_empty(i.path(), delete_directory_if_empty, depth)) {
                            return false;
                        }
                    }
                    std::filesystem::remove_all(directory);
                    return true;
                };
                try {
                    auto parent = file->full_path.parent_path();
                    while(delete_directory_if_empty(parent, delete_directory_if_empty, 0)) {
                        parent = parent.parent_path();
                    }
                }
                catch(std::exception &) {
                    deleted_error_shown = true;
                }
            }
        }
        if(deleted_error_shown) {
            eprintf_error("Error: Failed to delete some empty directories");
        }
    }
}
