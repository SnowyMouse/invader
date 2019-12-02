// SPDX-License-Identifier: GPL-3.0-only

#include <invader/build/build_workload.hpp>
#include <invader/hek/map.hpp>
#include <invader/file/file.hpp>
#include <invader/tag/hek/header.hpp>

namespace Invader {
    using namespace HEK;

    std::vector<std::byte> BuildWorkload::compile_map (
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
        unsigned int optimize_level
    ) {
        BuildWorkload workload;
        workload.scenario = scenario;
        workload.tags_directories = &tags_directories;

        if(optimize_level == 0) {
            workload.dedupe_tag_space = 0;
        }
        else if(optimize_level > 10) {
            std::terminate();
        }
        else if(optimize_level == 10) {
            workload.dedupe_tag_space = SIZE_MAX;
        }
        else {
            std::size_t s;
            if(engine_target == HEK::CacheFileEngine::CACHE_FILE_DARK_CIRCLET) {
                s = CACHE_FILE_MEMORY_LENGTH_DARK_CIRCLET / 2;
            }
            else {
                s = CACHE_FILE_MEMORY_LENGTH / 2;
            }
            s /= static_cast<std::size_t>(std::pow(10, 7));
            s *= static_cast<std::size_t>(std::pow(optimize_level, 7));
            workload.dedupe_tag_space = s;
        }

        // Set the tag data address
        if(tag_data_address.has_value()) {
            workload.tag_data_address = *tag_data_address;
        }
        else {
            switch(engine_target) {
                case CacheFileEngine::CACHE_FILE_DARK_CIRCLET:
                    workload.tag_data_address = CacheFileTagDataBaseMemoryAddress::CACHE_FILE_DARK_CIRCLET_BASE_MEMORY_ADDRESS;
                    workload.tag_data_size = CacheFileLimits::CACHE_FILE_MEMORY_LENGTH_DARK_CIRCLET;
                    break;
                case CacheFileEngine::CACHE_FILE_DEMO:
                    workload.tag_data_address = CacheFileTagDataBaseMemoryAddress::CACHE_FILE_DEMO_BASE_MEMORY_ADDRESS;
                    workload.tag_data_size = CacheFileLimits::CACHE_FILE_MEMORY_LENGTH;
                    break;
                default:
                    workload.tag_data_address = CacheFileTagDataBaseMemoryAddress::CACHE_FILE_PC_BASE_MEMORY_ADDRESS;
                    workload.tag_data_size = CacheFileLimits::CACHE_FILE_MEMORY_LENGTH;
                    break;
            }
        }

        auto return_value = workload.build_cache_file();
        if(workload.errors) {
            eprintf_error("Build failed with %zu error%s and %zu warning%s", workload.errors, workload.errors == 1 ? "" : "s", workload.warnings, workload.warnings == 1 ? "" : "s");
            throw InvalidTagDataException();
        }
        else if(workload.warnings) {
            eprintf_warn("Build successful with %zu warning%s", workload.warnings, workload.warnings == 1 ? "" : "s");
        }
        else {
            oprintf_success("Build successful");
        }
        return return_value;
    }

    std::vector<std::byte> BuildWorkload::build_cache_file() {
        // First, make our tag data header and array
        this->structs.resize(2);
        this->structs[0].unsafe_to_dedupe = true;
        auto &tag_data_ptr = this->structs[0].pointers.emplace_back();
        tag_data_ptr.offset = 0;
        tag_data_ptr.struct_index = 1;
        this->structs[1].unsafe_to_dedupe = true;

        // Add all of the tags
        this->add_tags();

        // Generate the tag array
        this->generate_tag_array();

        // Dedupe structs
        if(this->dedupe_tag_space) {
            this->dedupe_structs();
        }

        // Tag data
        std::vector<std::vector<std::byte>> tag_data;
        this->generate_tag_data(tag_data);

        std::terminate();
    }

    void BuildWorkload::compile_tag_data_recursively(const std::byte *tag_data, std::size_t tag_data_size, std::size_t tag_index, std::optional<TagClassInt> tag_class_int) {
        #define COMPILE_TAG_CLASS(class_struct, class_int) case TagClassInt::class_int: { \
            auto tag_data_parsed = Parser::class_struct::parse_hek_tag_file(tag_data, tag_data_size); \
            auto &new_struct = this->structs.emplace_back(); \
            this->tags[tag_index].base_struct = &new_struct - this->structs.data(); \
            new_struct.data.resize(sizeof(Parser::class_struct::struct_little), std::byte()); \
            tag_data_parsed.compile(*this, tag_index, &new_struct - this->structs.data()); \
            break; \
        }

        auto *header = reinterpret_cast<const HEK::TagFileHeader *>(tag_data);
        HEK::TagFileHeader::validate_header(header, tag_data_size, true, tag_class_int);

        switch(*tag_class_int) {
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
            COMPILE_TAG_CLASS(SoundScenery, TAG_CLASS_SOUND_SCENERY)
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
            case TagClassInt::TAG_CLASS_SCENARIO_STRUCTURE_BSP: {
                // First thing's first - parse the tag data
                auto tag_data_parsed = Parser::ScenarioStructureBSP::parse_hek_tag_file(tag_data, tag_data_size);
                auto &new_bsp_header_struct = this->structs.emplace_back();
                std::size_t bsp = this->bsp_count++;
                new_bsp_header_struct.bsp = bsp;

                // Make the header struct
                this->tags[tag_index].base_struct = &new_bsp_header_struct - this->structs.data();
                Parser::ScenarioStructureBSPCompiledHeader::struct_little *bsp_data;
                new_bsp_header_struct.data.resize(sizeof(*bsp_data), std::byte());
                bsp_data = reinterpret_cast<decltype(bsp_data)>(new_bsp_header_struct.data.data());
                auto &new_ptr = new_bsp_header_struct.pointers.emplace_back();
                new_ptr.offset = reinterpret_cast<std::byte *>(&bsp_data->pointer) - reinterpret_cast<std::byte *>(bsp_data);
                bsp_data->signature = TagClassInt::TAG_CLASS_SCENARIO_STRUCTURE_BSP;

                // Make the new BSP struct thingy and make the header point to it
                auto &new_bsp_struct = this->structs.emplace_back();
                new_ptr.struct_index = &new_bsp_struct - this->structs.data();
                new_bsp_struct.data.resize(sizeof(Parser::ScenarioStructureBSP::struct_little), std::byte());
                tag_data_parsed.compile(*this, tag_index, new_ptr.struct_index, bsp);
                break;
            }
            default:
                throw UnknownTagClassException();
        }
    }

    std::size_t BuildWorkload::compile_tag_recursively(const char *tag_path, TagClassInt tag_class_int) {
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

        // And we're done! Maybe?
        if(this->disable_recursion) {
            return return_value;
        }

        // Find it
        char formatted_path[256];
        if(tag_class_int == TagClassInt::TAG_CLASS_MODEL) {
            tag_class_int = TagClassInt::TAG_CLASS_GBXMODEL;
        }
        std::snprintf(formatted_path, sizeof(formatted_path), "%s.%s", tag_path, tag_class_to_extension(tag_class_int));
        Invader::File::halo_path_to_preferred_path_chars(formatted_path);
        auto new_path = Invader::File::tag_path_to_file_path(formatted_path, *this->tags_directories, true);
        if(!new_path.has_value()) {
            eprintf_error("Failed to find %s\n", formatted_path);
            throw InvalidTagPathException();
        }

        // Open it
        auto tag_file = Invader::File::open_file((*new_path).data());
        if(!tag_file.has_value()) {
            eprintf_error("Failed to open %s\n", formatted_path);
            throw FailedToOpenFileException();
        }
        auto &tag_file_data = *tag_file;

        try {
            this->compile_tag_data_recursively(tag_file_data.data(), tag_file_data.size(), return_value, tag_class_int);
        }
        catch(std::exception &e) {
            eprintf("Failed to compile tag %s\n", formatted_path);
            throw;
        }

        return return_value;
    }

    void BuildWorkload::add_tags() {
        this->scenario_index = this->compile_tag_recursively(this->scenario, TagClassInt::TAG_CLASS_SCENARIO);

        this->compile_tag_recursively("globals\\globals", TagClassInt::TAG_CLASS_GLOBALS);
        this->compile_tag_recursively("ui\\ui_tags_loaded_all_scenario_types", TagClassInt::TAG_CLASS_TAG_COLLECTION);

        // Load the correct tag collection tag
        switch(*this->cache_file_type) {
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

    std::size_t BuildWorkload::dedupe_structs() {
        bool found_something = true;
        std::size_t total_savings = 0;

        while(found_something && total_savings < this->dedupe_tag_space) {
            found_something = false;
            for(std::size_t i = 0; i < this->structs.size() && !found_something; i++) {
                for(std::size_t j = i + 1; j < this->structs.size(); j++) {
                    // Check if the structs are the same
                    if(this->structs[i].can_dedupe(this->structs[j])) {
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

    void BuildWorkload::report_error(ErrorType type, const char *error, std::optional<std::size_t> tag_index) {
        switch(type) {
            case ErrorType::ERROR_TYPE_WARNING_PEDANTIC:
                if(this->hide_pedantic_warnings) {
                    return;
                }
                // fallthrough
            case ErrorType::ERROR_TYPE_WARNING:
                eprintf_warn("WARNING: %s", error);
                this->warnings++;
                break;
            case ErrorType::ERROR_TYPE_ERROR:
                eprintf_error("ERROR: %s", error);
                this->errors++;
                break;
            case ErrorType::ERROR_TYPE_FATAL_ERROR:
                eprintf_error("FATAL ERROR: %s", error);
                this->errors++;
                break;
        }
        if(tag_index.has_value()) {
            auto &tag = this->tags[*tag_index];
            eprintf("...in %s.%s\n", File::halo_path_to_preferred_path(tag.path).data(), tag_class_to_extension(tag.tag_class_int));
        }
    }

    BuildWorkload BuildWorkload::compile_single_tag(const std::byte *tag_data, std::size_t tag_data_size, const std::vector<std::string> &tags_directories, bool recursion) {
        BuildWorkload workload;
        workload.disable_recursion = !recursion;
        workload.cache_file_type = HEK::CacheFileType::CACHE_FILE_MULTIPLAYER;
        workload.tags_directories = &tags_directories;
        workload.tags.emplace_back();
        workload.compile_tag_data_recursively(tag_data, tag_data_size, 0);
        return workload;
    }

    BuildWorkload BuildWorkload::compile_single_tag(const char *tag, TagClassInt tag_class_int, const std::vector<std::string> &tags_directories, bool recursion) {
        BuildWorkload workload;
        workload.disable_recursion = !recursion;
        workload.cache_file_type = HEK::CacheFileType::CACHE_FILE_MULTIPLAYER;
        workload.tags_directories = &tags_directories;
        workload.compile_tag_recursively(tag, tag_class_int);
        return workload;
    }

    void BuildWorkload::generate_tag_array() {
        this->structs[1].data.resize(sizeof(HEK::CacheFileTagDataTag) * this->tags.size());
        auto *tag_array = reinterpret_cast<HEK::CacheFileTagDataTag *>(this->structs[1].data.data());

        // Set tag classes, paths, etc.
        std::size_t tag_count = this->tags.size();
        for(std::size_t t = 0; t < tag_count; t++) {
            auto &tag_index = tag_array[tag_count];
            auto &tag = this->tags[t];

            // Tag path
            auto &new_path = this->structs.emplace_back();
            std::size_t new_path_struct = this->structs.size();
            auto &tag_path_ptr = this->structs[1].pointers.emplace_back();
            tag_path_ptr.offset = reinterpret_cast<std::byte *>(&tag_index.tag_path) - reinterpret_cast<std::byte *>(tag_array);
            tag_path_ptr.struct_index = new_path_struct;
            tag.tag_path = new_path_struct;
            new_path.data.insert(new_path.data.end(), reinterpret_cast<const std::byte *>(tag.path.data()), reinterpret_cast<const std::byte *>(tag.path.data() + tag.path.size() + 1));

            // Tag data
            auto primary_class = tag.tag_class_int;
            if(!tag.tag_index.has_value() || primary_class != TagClassInt::TAG_CLASS_SCENARIO_STRUCTURE_BSP) {
                auto &tag_data_ptr = this->structs[1].pointers.emplace_back();
                tag_data_ptr.offset = reinterpret_cast<std::byte *>(&tag_index.tag_data) - reinterpret_cast<std::byte *>(tag_array);
                tag_data_ptr.struct_index = *tag.base_struct;
            }

            // Tag ID
            auto &tag_id = this->structs[1].dependencies.emplace_back();
            tag_id.tag_id_only = true;
            tag_id.offset = reinterpret_cast<std::byte *>(&tag_index.tag_id) - reinterpret_cast<std::byte *>(tag_array);
            tag_id.tag_index = t;

            // Not strictly required to set the secondary or tertiary classes, but we do it anyway
            tag_index.primary_class = tag.tag_class_int;
            tag_index.secondary_class = TagClassInt::TAG_CLASS_NULL;
            tag_index.tertiary_class = TagClassInt::TAG_CLASS_NULL;
            switch(tag.tag_class_int) {
                case TagClassInt::TAG_CLASS_BIPED:
                case TagClassInt::TAG_CLASS_VEHICLE:
                    tag_index.secondary_class = TagClassInt::TAG_CLASS_UNIT;
                    tag_index.tertiary_class = TagClassInt::TAG_CLASS_OBJECT;
                    break;
                case TagClassInt::TAG_CLASS_WEAPON:
                case TagClassInt::TAG_CLASS_GARBAGE:
                case TagClassInt::TAG_CLASS_EQUIPMENT:
                    tag_index.secondary_class = TagClassInt::TAG_CLASS_ITEM;
                    tag_index.tertiary_class = TagClassInt::TAG_CLASS_OBJECT;
                    break;
                case TagClassInt::TAG_CLASS_DEVICE_CONTROL:
                case TagClassInt::TAG_CLASS_DEVICE_LIGHT_FIXTURE:
                case TagClassInt::TAG_CLASS_DEVICE_MACHINE:
                    tag_index.secondary_class = TagClassInt::TAG_CLASS_DEVICE;
                    tag_index.tertiary_class = TagClassInt::TAG_CLASS_OBJECT;
                    break;
                case TagClassInt::TAG_CLASS_SCENERY:
                case TagClassInt::TAG_CLASS_SOUND_SCENERY:
                case TagClassInt::TAG_CLASS_PLACEHOLDER:
                case TagClassInt::TAG_CLASS_PROJECTILE:
                    tag_index.secondary_class = TagClassInt::TAG_CLASS_OBJECT;
                    break;
                case TagClassInt::TAG_CLASS_SHADER_ENVIRONMENT:
                case TagClassInt::TAG_CLASS_SHADER_MODEL:
                case TagClassInt::TAG_CLASS_SHADER_TRANSPARENT_GENERIC:
                case TagClassInt::TAG_CLASS_SHADER_TRANSPARENT_CHICAGO:
                case TagClassInt::TAG_CLASS_SHADER_TRANSPARENT_CHICAGO_EXTENDED:
                case TagClassInt::TAG_CLASS_SHADER_TRANSPARENT_WATER:
                case TagClassInt::TAG_CLASS_SHADER_TRANSPARENT_PLASMA:
                case TagClassInt::TAG_CLASS_SHADER_TRANSPARENT_METER:
                case TagClassInt::TAG_CLASS_SHADER_TRANSPARENT_GLASS:
                    tag_index.secondary_class = TagClassInt::TAG_CLASS_SHADER;
                    break;
                default:
                    break;
            }
        }
    }

    void BuildWorkload::generate_tag_data(std::vector<std::vector<std::byte>> &tag_data) {
        auto &structs = this->structs;
        auto &tags = this->tags;

        // Pointer offset to where
        std::vector<std::pair<std::size_t, std::size_t>> pointers;
        auto name_tag_data_pointer = this->tag_data_address;

        auto recursively_generate_data = [&structs, &tags, &pointers, &name_tag_data_pointer](std::vector<std::byte> &data, std::size_t struct_index, auto &recursively_generate_data) -> std::size_t {
            auto &s = structs[struct_index];

            // Return the pointer thingy
            if(s.offset.has_value()) {
                return *s.offset;
            }

            // Set the offset thingy
            std::size_t offset = data.size();
            s.offset = offset;
            data.insert(data.end(), s.data.begin(), s.data.end());

            // Get the pointers
            for(auto &pointer : s.pointers) {
                pointers.emplace_back(offset, recursively_generate_data(data, pointer.struct_index, recursively_generate_data));
            }

            // Get the pointers
            for(auto &dependency : s.dependencies) {
                HEK::TagID new_tag_id = { static_cast<std::uint32_t>(dependency.tag_index) };

                if(dependency.tag_id_only) {
                    *reinterpret_cast<HEK::LittleEndian<HEK::TagID> *>(data.data() + dependency.offset) = new_tag_id;
                }
                else {
                    std::size_t tag_path_offset = recursively_generate_data(data, *tags[new_tag_id.index].tag_path, recursively_generate_data);
                    auto &dependency_struct = *reinterpret_cast<HEK::TagDependency<HEK::LittleEndian> *>(data.data() + dependency.offset);
                    dependency_struct.tag_class_int = tags[new_tag_id.index].tag_class_int;
                    dependency_struct.tag_id = new_tag_id;
                    dependency_struct.path_pointer = name_tag_data_pointer + static_cast<HEK::Pointer>(tag_path_offset);
                }
            }

            // Append stuff
            data.insert(data.end(), REQUIRED_PADDING_32_BIT(data.size()), std::byte());
            return offset;
        };

        // Build the tag data for the main tag data
        auto &tag_data_struct = tag_data.emplace_back();
        recursively_generate_data(tag_data_struct, 0, recursively_generate_data);
        auto *tag_data_b = tag_data_struct.data();

        // Adjust the pointers
        for(auto &p : pointers) {
            *reinterpret_cast<HEK::LittleEndian<HEK::Pointer> *>(tag_data_b + p.first) = static_cast<HEK::Pointer>(name_tag_data_pointer + p.second);
        }

        // Get the scenario tag
        auto &scenario_tag = tags[this->scenario_index];
        auto &scenario_tag_data = *reinterpret_cast<const Parser::Scenario::struct_little *>(structs[*scenario_tag.base_struct].data.data());
        std::size_t bsp_count = scenario_tag_data.structure_bsps.count.read();
        if(bsp_count != this->bsp_count) {
            REPORT_ERROR_PRINTF(*this, ERROR_TYPE_FATAL_ERROR, this->scenario_index, "BSP count in scenario tag is wrong (%zu expected, %zu gotten)", bsp_count, this->bsp_count);
            throw InvalidTagDataException();
        }
        else if(bsp_count) {
            auto scenario_bsps_struct_index = *structs[*scenario_tag.base_struct].resolve_pointer(reinterpret_cast<const std::byte *>(&scenario_tag_data.structure_bsps.pointer) - reinterpret_cast<const std::byte *>(&scenario_tag_data));
            std::size_t file_offset = sizeof(HEK::CacheFileHeader);
            auto *scenario_bsps_struct_data = reinterpret_cast<Parser::ScenarioBSP::struct_little *>(tag_data[0].data() + *structs[scenario_bsps_struct_index].offset);

            // Go through each BSP tag
            for(std::size_t i = 0; i < tags.size(); i++) {
                auto &t = tags[i];
                if(t.tag_class_int != TagClassInt::TAG_CLASS_SCENARIO_STRUCTURE_BSP) {
                    continue;
                }

                // Build the tag data for the BSP data now
                pointers.clear();
                auto &bsp_data_struct = tag_data.emplace_back();
                recursively_generate_data(bsp_data_struct, 0, recursively_generate_data);
                std::size_t bsp_size = bsp_data_struct.size();
                HEK::Pointer tag_data_base = this->tag_data_address + this->tag_data_size - bsp_size;
                tag_data_b = bsp_data_struct.data();

                // Chu
                for(auto &p : pointers) {
                    *reinterpret_cast<HEK::LittleEndian<HEK::Pointer> *>(tag_data_b + p.first) = static_cast<HEK::Pointer>(tag_data_base + p.second);
                }

                // Do the thing now
                file_offset += bsp_size;

                // Find the BSP in the scenario array thingy
                bool found = false;
                for(std::size_t b = 0; b < bsp_count; b++) {
                    if(scenario_bsps_struct_data[b].structure_bsp.tag_id.read().index == i) {
                        scenario_bsps_struct_data[b].bsp_address = tag_data_base;
                        scenario_bsps_struct_data[b].bsp_size = bsp_size;
                        scenario_bsps_struct_data[b].bsp_start = file_offset;
                        found = true;
                    }
                }
                if(!found) {
                    REPORT_ERROR_PRINTF(*this, ERROR_TYPE_FATAL_ERROR, this->scenario_index, "Scenario does not reference BSP %s", t.path.data());
                    throw InvalidTagDataException();
                }
            }
        }
    }
}
