// SPDX-License-Identifier: GPL-3.0-only

#include <invader/build2/build_workload.hpp>
#include <invader/hek/map.hpp>
#include <invader/file/file.hpp>

namespace Invader {
    using namespace HEK;

    std::vector<std::byte> BuildWorkload2::compile_map (
        const char *scenario,
        const std::vector<std::string> &tags_directories,
        HEK::CacheFileEngine engine_target,
        std::string maps_directory,
        bool no_external_tags,
        bool always_index_tags,
        bool verbose,
        const std::optional<std::vector<std::tuple<TagClassInt, std::string>>> &with_index,
        const std::optional<std::uint32_t> &forge_crc,
        const std::optional<std::uint32_t> &tag_data_address,
        const std::optional<std::string> &rename_scenario,
        std::size_t dedupe_tag_space
    ) {
        BuildWorkload2 workload;
        workload.scenario = scenario;
        workload.tags_directories = &tags_directories;
        workload.dedupe_tag_space = dedupe_tag_space;

        // Set the tag data address
        if(tag_data_address.has_value()) {
            workload.tag_data_address = *tag_data_address;
        }
        else {
            switch(engine_target) {
                case CacheFileEngine::CACHE_FILE_DARK_CIRCLET:
                    workload.tag_data_address = CacheFileTagDataBaseMemoryAddress::CACHE_FILE_DARK_CIRCLET_BASE_MEMORY_ADDRESS;
                    break;
                case CacheFileEngine::CACHE_FILE_DEMO:
                    workload.tag_data_address = CacheFileTagDataBaseMemoryAddress::CACHE_FILE_DEMO_BASE_MEMORY_ADDRESS;
                    break;
                default:
                    workload.tag_data_address = CacheFileTagDataBaseMemoryAddress::CACHE_FILE_PC_BASE_MEMORY_ADDRESS;
                    break;
            }
        }

        auto return_value = workload.build_cache_file();
        return return_value;
    }

    std::vector<std::byte> BuildWorkload2::build_cache_file() {
        // Before we begin, add all of the tags
        this->add_tags();

        // Dedupe structs
        if(this->dedupe_tag_space) {
            this->dedupe_structs();
        }

        std::terminate();
    }

    std::size_t BuildWorkload2::compile_tag_recursively(const char *tag_path, TagClassInt tag_class_int) {
        // Search for the tag
        std::size_t return_value = this->tags.size();
        bool found = false;
        for(std::size_t i = 0; i < return_value; i++) {
            auto &tag = this->tags[i];
            if(tag.tag_class_int == tag_class_int && tag.path == tag_path) {
                if(tag.base_struct.has_value()) {
                    return i;
                }
                return_value = i;
                found = true;
                break;
            }
        }

        // If it wasn't found in the current array list, add it to the list and let's begin
        if(!found) {
            auto &tag = this->tags.emplace_back();
            tag.path = tag_path;
            tag.tag_class_int = tag_class_int;
        }

        // Find it
        auto new_path = Invader::File::file_path_to_tag_path(tag_path, *this->tags_directories, true);
        if(!new_path.has_value()) {
            eprintf("failed to find %s.%s\n", tag_path, tag_class_to_extension(tag_class_int));
            throw InvalidTagPathException();
        }

        // Open it
        auto tag_file = Invader::File::open_file((*new_path).data());
        if(!tag_file.has_value()) {
            eprintf("failed to open %s.%s\n", tag_path, tag_class_to_extension(tag_class_int));
            throw FailedToOpenFileException();
        }
        auto &tag_file_data = *tag_file;

        #define COMPILE_TAG_CLASS(class_struct, class_int) case TagClassInt::class_int: { \
            auto tag_data = Parser::class_struct::parse_hek_tag_file(tag_file_data.data(), tag_file_data.size()); \
            auto &new_struct = this->structs.emplace_back(); \
            new_struct.data.resize(sizeof(Parser::class_struct::struct_little), std::byte()); \
            tag_data.compile(*this, &new_struct - this->structs.data()); \
            break; \
        }

        try {
            switch(tag_class_int) {
                COMPILE_TAG_CLASS(Actor, TAG_CLASS_ACTOR)
                COMPILE_TAG_CLASS(ActorVariant, TAG_CLASS_ACTOR_VARIANT)
                COMPILE_TAG_CLASS(Antenna, TAG_CLASS_ANTENNA)
                COMPILE_TAG_CLASS(ModelAnimations, TAG_CLASS_MODEL_ANIMATIONS)
                COMPILE_TAG_CLASS(Biped, TAG_CLASS_BIPED)
                COMPILE_TAG_CLASS(Bitmap, TAG_CLASS_BITMAP)
                COMPILE_TAG_CLASS(ModelCollisionGeometry, TAG_CLASS_MODEL_COLLISION_GEOMETRY)
                COMPILE_TAG_CLASS(ColorTable, TAG_CLASS_COLOR_TABLE)
                COMPILE_TAG_CLASS(Contrail, TAG_CLASS_CONTRAIL)
                COMPILE_TAG_CLASS(DeviceControl, TAG_CLASS_DEVICE_CONTROL)
                COMPILE_TAG_CLASS(Decal, TAG_CLASS_DECAL)
                COMPILE_TAG_CLASS(UIWidgetDefinition, TAG_CLASS_UI_WIDGET_DEFINITION)
                COMPILE_TAG_CLASS(InputDeviceDefaults, TAG_CLASS_INPUT_DEVICE_DEFAULTS)
                COMPILE_TAG_CLASS(Device, TAG_CLASS_DEVICE)
                COMPILE_TAG_CLASS(DetailObjectCollection, TAG_CLASS_DETAIL_OBJECT_COLLECTION)
                COMPILE_TAG_CLASS(Effect, TAG_CLASS_EFFECT)
                COMPILE_TAG_CLASS(Equipment, TAG_CLASS_EQUIPMENT)
                COMPILE_TAG_CLASS(Flag, TAG_CLASS_FLAG)
                COMPILE_TAG_CLASS(Fog, TAG_CLASS_FOG)
                COMPILE_TAG_CLASS(Font, TAG_CLASS_FONT)
                COMPILE_TAG_CLASS(MaterialEffects, TAG_CLASS_MATERIAL_EFFECTS)
                COMPILE_TAG_CLASS(Garbage, TAG_CLASS_GARBAGE)
                COMPILE_TAG_CLASS(Glow, TAG_CLASS_GLOW)
                COMPILE_TAG_CLASS(GrenadeHUDInterface, TAG_CLASS_GRENADE_HUD_INTERFACE)
                COMPILE_TAG_CLASS(HUDMessageText, TAG_CLASS_HUD_MESSAGE_TEXT)
                COMPILE_TAG_CLASS(HUDNumber, TAG_CLASS_HUD_NUMBER)
                COMPILE_TAG_CLASS(HUDGlobals, TAG_CLASS_HUD_GLOBALS)
                COMPILE_TAG_CLASS(Item, TAG_CLASS_ITEM)
                COMPILE_TAG_CLASS(ItemCollection, TAG_CLASS_ITEM_COLLECTION)
                COMPILE_TAG_CLASS(DamageEffect, TAG_CLASS_DAMAGE_EFFECT)
                COMPILE_TAG_CLASS(LensFlare, TAG_CLASS_LENS_FLARE)
                COMPILE_TAG_CLASS(Lightning, TAG_CLASS_LIGHTNING)
                COMPILE_TAG_CLASS(DeviceLightFixture, TAG_CLASS_DEVICE_LIGHT_FIXTURE)
                COMPILE_TAG_CLASS(Light, TAG_CLASS_LIGHT)
                COMPILE_TAG_CLASS(SoundLooping, TAG_CLASS_SOUND_LOOPING)
                COMPILE_TAG_CLASS(DeviceMachine, TAG_CLASS_DEVICE_MACHINE)
                COMPILE_TAG_CLASS(Globals, TAG_CLASS_GLOBALS)
                COMPILE_TAG_CLASS(Meter, TAG_CLASS_METER)
                COMPILE_TAG_CLASS(LightVolume, TAG_CLASS_LIGHT_VOLUME)
                COMPILE_TAG_CLASS(GBXModel, TAG_CLASS_GBXMODEL)
                COMPILE_TAG_CLASS(MultiplayerScenarioDescription, TAG_CLASS_MULTIPLAYER_SCENARIO_DESCRIPTION)
                COMPILE_TAG_CLASS(Object, TAG_CLASS_OBJECT)
                COMPILE_TAG_CLASS(Particle, TAG_CLASS_PARTICLE)
                COMPILE_TAG_CLASS(ParticleSystem, TAG_CLASS_PARTICLE_SYSTEM)
                COMPILE_TAG_CLASS(Physics, TAG_CLASS_PHYSICS)
                COMPILE_TAG_CLASS(Placeholder, TAG_CLASS_PLACEHOLDER)
                COMPILE_TAG_CLASS(PointPhysics, TAG_CLASS_POINT_PHYSICS)
                COMPILE_TAG_CLASS(Projectile, TAG_CLASS_PROJECTILE)
                COMPILE_TAG_CLASS(WeatherParticleSystem, TAG_CLASS_WEATHER_PARTICLE_SYSTEM)
                COMPILE_TAG_CLASS(Scenery, TAG_CLASS_SCENERY)
                COMPILE_TAG_CLASS(ShaderTransparentChicagoExtended, TAG_CLASS_SHADER_TRANSPARENT_CHICAGO_EXTENDED)
                COMPILE_TAG_CLASS(ShaderTransparentChicago, TAG_CLASS_SHADER_TRANSPARENT_CHICAGO)
                COMPILE_TAG_CLASS(Scenario, TAG_CLASS_SCENARIO)
                COMPILE_TAG_CLASS(ShaderEnvironment, TAG_CLASS_SHADER_ENVIRONMENT)
                COMPILE_TAG_CLASS(ShaderTransparentGlass, TAG_CLASS_SHADER_TRANSPARENT_GLASS)
                COMPILE_TAG_CLASS(Shader, TAG_CLASS_SHADER)
                COMPILE_TAG_CLASS(Sky, TAG_CLASS_SKY)
                COMPILE_TAG_CLASS(ShaderTransparentMeter, TAG_CLASS_SHADER_TRANSPARENT_METER)
                COMPILE_TAG_CLASS(Sound, TAG_CLASS_SOUND)
                COMPILE_TAG_CLASS(SoundEnvironment, TAG_CLASS_SOUND_ENVIRONMENT)
                COMPILE_TAG_CLASS(ShaderModel, TAG_CLASS_SHADER_MODEL)
                COMPILE_TAG_CLASS(ShaderTransparentGeneric, TAG_CLASS_SHADER_TRANSPARENT_GENERIC)
                COMPILE_TAG_CLASS(TagCollection, TAG_CLASS_UI_WIDGET_COLLECTION)
                COMPILE_TAG_CLASS(ShaderTransparentPlasma, TAG_CLASS_SHADER_TRANSPARENT_PLASMA)
                COMPILE_TAG_CLASS(Scenery, TAG_CLASS_SOUND_SCENERY)
                COMPILE_TAG_CLASS(StringList, TAG_CLASS_STRING_LIST)
                COMPILE_TAG_CLASS(ShaderTransparentWater, TAG_CLASS_SHADER_TRANSPARENT_WATER)
                COMPILE_TAG_CLASS(TagCollection, TAG_CLASS_TAG_COLLECTION)
                COMPILE_TAG_CLASS(CameraTrack, TAG_CLASS_CAMERA_TRACK)
                COMPILE_TAG_CLASS(Dialogue, TAG_CLASS_DIALOGUE)
                COMPILE_TAG_CLASS(UnitHUDInterface, TAG_CLASS_UNIT_HUD_INTERFACE)
                COMPILE_TAG_CLASS(Unit, TAG_CLASS_UNIT)
                COMPILE_TAG_CLASS(UnicodeStringList, TAG_CLASS_UNICODE_STRING_LIST)
                COMPILE_TAG_CLASS(VirtualKeyboard, TAG_CLASS_VIRTUAL_KEYBOARD)
                COMPILE_TAG_CLASS(Vehicle, TAG_CLASS_VEHICLE)
                COMPILE_TAG_CLASS(Weapon, TAG_CLASS_WEAPON)
                COMPILE_TAG_CLASS(Wind, TAG_CLASS_WIND)
                COMPILE_TAG_CLASS(WeaponHUDInterface, TAG_CLASS_WEAPON_HUD_INTERFACE)
                COMPILE_TAG_CLASS(ScenarioStructureBSP, TAG_CLASS_SCENARIO_STRUCTURE_BSP)
                default:
                    throw UnknownTagClassException();
            }
        }
        catch(std::exception &e) {
            eprintf("failed to compile %s.%s\n", tag_path, tag_class_to_extension(tag_class_int));
            throw;
        }

        return return_value;
    }

    void BuildWorkload2::add_tags() {
        this->scenario_index = this->compile_tag_recursively(this->scenario, TagClassInt::TAG_CLASS_SCENARIO);
        this->cache_file_type = reinterpret_cast<Scenario<LittleEndian> *>(this->structs[*this->tags[this->scenario_index].base_struct].data.data())->type;

        this->compile_tag_recursively("globals\\globals", TagClassInt::TAG_CLASS_GLOBALS);
        this->compile_tag_recursively("ui\\ui_tags_loaded_all_scenario_types", TagClassInt::TAG_CLASS_TAG_COLLECTION);

        // Load the correct tag collection tag
        switch(this->cache_file_type) {
            case ScenarioType::CACHE_FILE_SINGLEPLAYER:
                this->compile_tag_recursively("ui\\ui_tags_loaded_solo_scenario_type", TagClassInt::TAG_CLASS_TAG_COLLECTION);
                break;
            case ScenarioType::CACHE_FILE_MULTIPLAYER:
                this->compile_tag_recursively("ui\\ui_tags_loaded_multiplayer_scenario_type", TagClassInt::TAG_CLASS_TAG_COLLECTION);
                break;
            case ScenarioType::CACHE_FILE_USER_INTERFACE:
                this->compile_tag_recursively("ui\\ui_tags_loaded_mainmenu_scenario_type", TagClassInt::TAG_CLASS_TAG_COLLECTION);
                break;
        }

        // These are required for UI elements and other things
        this->compile_tag_recursively("sound\\sfx\\ui\\cursor", TagClassInt::TAG_CLASS_SOUND);
        this->compile_tag_recursively("sound\\sfx\\ui\\back", TagClassInt::TAG_CLASS_SOUND);
        this->compile_tag_recursively("sound\\sfx\\ui\\flag_failure", TagClassInt::TAG_CLASS_SOUND);
        this->compile_tag_recursively("ui\\shell\\main_menu\\mp_map_list", TagClassInt::TAG_CLASS_UNICODE_STRING_LIST);
        this->compile_tag_recursively("ui\\shell\\strings\\loading", TagClassInt::TAG_CLASS_UNICODE_STRING_LIST);
        this->compile_tag_recursively("ui\\shell\\bitmaps\\trouble_brewing", TagClassInt::TAG_CLASS_BITMAP);
        this->compile_tag_recursively("ui\\shell\\bitmaps\\background", TagClassInt::TAG_CLASS_BITMAP);
    }

    std::size_t BuildWorkload2::dedupe_structs() {
        bool found_something = true;
        std::size_t total_savings = 0;

        while(found_something && total_savings < this->dedupe_tag_space) {
            found_something = false;
            for(std::size_t i = 0; i < this->structs.size() && !found_something; i++) {
                for(std::size_t j = i + 1; j < this->structs.size(); j++) {
                    // Check if the structs are the same
                    if(this->structs[i] == this->structs[j]) {
                        // If so, go through every struct pointer. If they equal j, set to i. If they're greater than j, decrement
                        for(std::size_t k = 0; k < i; k++) {
                            for(auto &pointer : this->structs[k].pointers) {
                                if(pointer.struct_index > j) {
                                    pointer.struct_index--;
                                }
                                else if(pointer.struct_index == j) {
                                    pointer.struct_index = i;
                                }
                            }
                        }

                        // Also go through every tag, too
                        for(auto &tag : this->tags) {
                            if(tag.base_struct > j) {
                                (*tag.base_struct)--;
                            }
                            else if(tag.base_struct == j) {
                                tag.base_struct = i;
                            }
                        }

                        total_savings += this->structs[j].data.size();
                        this->structs.erase(this->structs.begin() + j);
                        found_something = true;
                        j--;

                        if(total_savings >= this->dedupe_tag_space) {
                            return total_savings;
                        }
                    }
                }
            }
        }

        return total_savings;
    }
}
