// SPDX-License-Identifier: GPL-3.0-only

#include <regex>
#include <invader/build/build_workload.hpp>
#include <invader/extract/extraction.hpp>
#include <invader/tag/hek/header.hpp>
#include <invader/tag/parser/parser.hpp>

namespace Invader {
    void ExtractionWorkload::extract_map(const Map &map, const std::string &tags, const std::vector<std::string> &queries, bool recursive, bool overwrite, bool non_mp_globals, ReportingLevel reporting_level) {
        // There's no need to extract recursively if we're extracting all tags
        if(queries.size() == 0) {
            recursive = false;
        }
        
        ExtractionWorkload workload(map, reporting_level);
        auto start = std::chrono::steady_clock::now();
        auto success = workload.perform_extraction(queries, tags, recursive, overwrite, non_mp_globals);
        auto matched = workload.matched_tags.size();
        auto warnings = workload.get_warnings();
        auto errors = workload.get_errors();
        
        char initial_message[256] = {};
        std::snprintf(initial_message, sizeof(initial_message), "Extracted %zu out of %zu matched tag%s", success, matched, matched == 1 ? "" : "s");
        
        char warnings_message[256] = {};
        char errors_message[256] = {};
        if(warnings) {
            std::snprintf(warnings_message, sizeof(warnings_message), " with %zu warning%s", warnings, warnings == 1 ? "" : "s");
        }
        if(errors) {
            std::snprintf(errors_message, sizeof(errors_message), "%s %zu error%s", warnings ? " and" : " with", errors, errors == 1 ? "" : "s");
        }
        
        char timer[256] = {};
        auto end = std::chrono::steady_clock::now();
        std::snprintf(timer, sizeof(timer), " in %zu ms", std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count());
        
        if(errors) {
            oprintf_success_warn("%s%s%s%s", initial_message, warnings_message, errors_message, timer);
        }
        else if(warnings) {
            oprintf_success_lesser_warn("%s%s%s", initial_message, warnings_message, timer);
        }
        else {
            oprintf_success("%s%s", initial_message, timer);
        }
    }
    
    std::size_t ExtractionWorkload::perform_extraction(const std::vector<std::string> &queries, const std::filesystem::path &tags, bool recursive, bool overwrite, bool non_mp_globals) {
        // Set these variables up
        auto *map = &this->map;
        auto type = map->get_type();
        auto tag_count = map->get_tag_count();
        std::vector<bool> extracted_tags(tag_count);
        std::deque<std::size_t> all_tags_to_extract;
        auto &workload = *this;

        auto extract_tag = [&extracted_tags, &map, &tags, &all_tags_to_extract, &type, &recursive, &overwrite, &non_mp_globals, &workload](std::size_t tag_index) -> bool {
            // Do it
            extracted_tags[tag_index] = true;

            // Get the tag path
            const auto &tag = map->get_tag(tag_index);

            // See if we can extract this
            auto tag_class_int = tag.get_tag_class_int();
            const char *tag_extension = Invader::HEK::tag_class_to_extension(tag_class_int);
            if(!tag.data_is_available()) {
                return false;
            }

            // Get the path
            auto path = Invader::File::halo_path_to_preferred_path(tag.get_path());

            // Lowercase everything
            for(char &c : path) {
                c = std::tolower(c);
            }

            // Figure out the path we're writing to
            auto tag_path_to_write_to = tags / (path + "." + tag_extension);
            if(!overwrite && std::filesystem::exists(tag_path_to_write_to)) {
                return false;
            }

            // Skip globals
            if(tag_class_int == Invader::TagClassInt::TAG_CLASS_GLOBALS && !non_mp_globals && type != Invader::HEK::CacheFileType::SCENARIO_TYPE_MULTIPLAYER) {
                workload.report_error(ErrorType::ERROR_TYPE_WARNING_PEDANTIC, "Skipping the non-multiplayer map's globals tag", tag_index);
                return false;
            }

            // Get the tag data
            std::vector<std::byte> new_tag;
            try {
                new_tag = Invader::ExtractionWorkload::extract_single_tag(tag);

                // If we're recursive, we want to also get that stuff, too
                if(recursive) {
                    auto tag_compiled = BuildWorkload::compile_single_tag(new_tag.data(), new_tag.size(), std::vector<std::string>(), false);
                    std::vector<std::pair<const std::string *, Invader::TagClassInt>> dependencies;
                    for(auto &s : tag_compiled.structs) {
                        for(auto &d : s.dependencies) {
                            auto &tag = tag_compiled.tags[d.tag_index];
                            dependencies.emplace_back(&tag.path, tag.tag_class_int);
                        }
                    }
                    for(auto &d : dependencies) {
                        auto tag_index = map->find_tag(d.first->c_str(), d.second);
                        if(tag_index.has_value() && extracted_tags[*tag_index] == false) {
                            all_tags_to_extract.push_back(*tag_index);
                        }
                    }
                }
            }
            catch (std::exception &e) {
                REPORT_ERROR_PRINTF(workload, ERROR_TYPE_ERROR, tag_index, "Failed to extract %s.%s: %s", Invader::File::halo_path_to_preferred_path(tag.get_path()).c_str(), tag_extension, e.what());
                return false;
            }

            // Jason Jones the tag
            if(type == Invader::HEK::CacheFileType::SCENARIO_TYPE_SINGLEPLAYER) {
                auto tag_path = tag.get_path();
                bool changed = false;
                switch(tag_class_int) {
                    case Invader::TagClassInt::TAG_CLASS_WEAPON: {
                        #define ALTER_TAG_DATA(from, to) if(from != to) { from = to; changed = true; } 
                        
                        if(tag_path == "weapons\\pistol\\pistol") {
                            auto parsed = Invader::Parser::Weapon::parse_hek_tag_file(new_tag.data(), new_tag.size());
                            if(parsed.triggers.size() >= 1) {
                                auto &first_trigger = parsed.triggers[0];
                                ALTER_TAG_DATA(first_trigger.minimum_error, DEGREES_TO_RADIANS(0.0F));
                                ALTER_TAG_DATA(first_trigger.error_angle.from, DEGREES_TO_RADIANS(0.2F));
                                ALTER_TAG_DATA(first_trigger.error_angle.to, DEGREES_TO_RADIANS(2.0F));
                            }
                            new_tag = parsed.generate_hek_tag_data(TagClassInt::TAG_CLASS_WEAPON);
                        }
                        else if(tag_path == "weapons\\plasma rifle\\plasma rifle") {
                            auto parsed = Invader::Parser::Weapon::parse_hek_tag_file(new_tag.data(), new_tag.size());
                            if(parsed.triggers.size() >= 1) {
                                auto &first_trigger = parsed.triggers[0];
                                ALTER_TAG_DATA(first_trigger.error_angle.from, DEGREES_TO_RADIANS(0.5F));
                                ALTER_TAG_DATA(first_trigger.error_angle.to, DEGREES_TO_RADIANS(5.0F));
                            }
                            new_tag = parsed.generate_hek_tag_data(TagClassInt::TAG_CLASS_WEAPON);
                        }
                        break;
                    }
                    case Invader::TagClassInt::TAG_CLASS_DAMAGE_EFFECT:
                        if(tag_path == "weapons\\pistol\\bullet") {
                            auto parsed = Invader::Parser::DamageEffect::parse_hek_tag_file(new_tag.data(), new_tag.size());
                            ALTER_TAG_DATA(parsed.elite_energy_shield, 1.0F);
                            new_tag = parsed.generate_hek_tag_data(TagClassInt::TAG_CLASS_DAMAGE_EFFECT);
                        }
                        break;
                    default:
                        break;
                        
                        #undef ALTER_TAG_DATA
                }
                
                if(changed) {
                    REPORT_ERROR_PRINTF(workload, ERROR_TYPE_WARNING_PEDANTIC, tag_index, "%s.%s was changed due to being altered in singleplayer", Invader::File::halo_path_to_preferred_path(tag.get_path()).c_str(), tag_extension);
                }
            }

            // Create directories along the way
            try {
                if(!std::filesystem::exists(tag_path_to_write_to.parent_path())) {
                    std::filesystem::create_directories(tag_path_to_write_to.parent_path());
                }
            }
            catch(std::exception &e) {
                REPORT_ERROR_PRINTF(workload, ERROR_TYPE_ERROR, tag_index, "Failed to create a directory: %s", e.what());
                return false;
            }

            // Save it
            auto tag_path_str = tag_path_to_write_to.string();
            if(!Invader::File::save_file(tag_path_str.c_str(), new_tag)) {
                REPORT_ERROR_PRINTF(workload, ERROR_TYPE_ERROR, tag_index, "Failed to save %s", tag_path_str.c_str());
                return false;
            }

            return true;
        };

        // Extract each tag?
        if(queries.size() == 0) {
            for(std::size_t t = 0; t < tag_count; t++) {
                all_tags_to_extract.push_back(t);
            }
        }

        else {
            std::size_t tag_count = map->get_tag_count();
            for(std::size_t t = 0; t < tag_count; t++) {
                // See if we already added
                bool already_added = false;
                for(auto &tag : all_tags_to_extract) {
                    if(tag == t) {
                        already_added = true;
                        break;
                    }
                }
                if(already_added) {
                    continue;
                }

                const auto &tag = map->get_tag(t);
                auto full_tag_path = Invader::File::halo_path_to_preferred_path(tag.get_path()) + "." + HEK::tag_class_to_extension(tag.get_tag_class_int());

                for(auto &query : queries) {
                    if(File::path_matches(full_tag_path.c_str(), query.c_str())) {
                        all_tags_to_extract.emplace_back(t);
                        break;
                    }
                }
            }

            if(all_tags_to_extract.size() == 0) {
                workload.report_error(ErrorType::ERROR_TYPE_ERROR, "No tags were found with the given search parameter(s).");
                return 0;
            }
        }

        // Extract tags
        std::size_t total = 0;
        std::size_t extracted = 0;
        while(all_tags_to_extract.size() > 0) {
            std::size_t tag = all_tags_to_extract[0];
            all_tags_to_extract.erase(all_tags_to_extract.begin());
            if(extracted_tags[tag]) {
                continue;
            }
            const auto &tag_map = map->get_tag(tag);
            if(extract_tag(tag)) {
                oprintf_success("Extracted %s.%s", Invader::File::halo_path_to_preferred_path(tag_map.get_path()).c_str(), HEK::tag_class_to_extension(tag_map.get_tag_class_int()));
                extracted++;
            }
            else {
                eprintf("Skipped %s.%s\n", Invader::File::halo_path_to_preferred_path(tag_map.get_path()).c_str(), HEK::tag_class_to_extension(tag_map.get_tag_class_int()));
            }
        }
        
        this->matched_tags.reserve(total);
        for(std::size_t i = 0; i < tag_count; i++) {
            if(extracted_tags[i]) {
                this->matched_tags.push_back(i);
            }
        }
        
        return extracted;
    }
    
    ExtractionWorkload::ExtractionWorkload(const Map &map, ReportingLevel reporting_level) : ErrorHandler(reporting_level), map(map) {
        auto &paths = this->get_tag_paths();
        auto tag_count = map.get_tag_count();
        paths.reserve(tag_count);
        for(std::size_t i = 0; i < tag_count; i++) {
            auto &tag = map.get_tag(i);
            paths.emplace_back(tag.get_path(), tag.get_tag_class_int());
        }
    }
    
    std::vector<std::byte> ExtractionWorkload::extract_single_tag(const Tag &tag, ReportingLevel reporting_level) {
        ExtractionWorkload workload(tag.get_map(), reporting_level);
        auto result = workload.extract_tag(tag.get_tag_index());
        if(result.has_value()) {
            return result->get()->generate_hek_tag_data(tag.get_tag_class_int());
        }
        else {
            throw InvalidTagDataException();
        }
    }
    
    std::optional<std::unique_ptr<Parser::ParserStruct>> ExtractionWorkload::extract_tag(std::size_t tag_index) {
        auto &tag = this->map.get_tag(tag_index);
        auto tag_class_int = tag.get_tag_class_int();

        #define EXTRACT_TAG_CLASS(class_struct, class_int) case TagClassInt::class_int: { \
            return std::make_unique<Parser::class_struct>(Parser::class_struct::parse_cache_file_data(tag)); \
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
            EXTRACT_TAG_CLASS(Model, TAG_CLASS_MODEL)
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
                if(tag.get_map().get_engine() == HEK::CacheFileEngine::CACHE_FILE_NATIVE) {
                    return std::make_unique<Parser::ScenarioStructureBSP>(Parser::ScenarioStructureBSP::parse_cache_file_data(tag));
                }
                else {
                    HEK::TagFileHeader tag_data_header(TagClassInt::TAG_CLASS_SCENARIO_STRUCTURE_BSP);
                    std::vector<std::byte> data(reinterpret_cast<std::byte *>(&tag_data_header), reinterpret_cast<std::byte *>(&tag_data_header + 1));
                    auto sbsp_header_data = tag.get_base_struct<HEK::ScenarioStructureBSPCompiledHeader>();
                    auto sbsp_header_pointer = sbsp_header_data.pointer.read();
                    return std::make_unique<Parser::ScenarioStructureBSP>(Parser::ScenarioStructureBSP::parse_cache_file_data(tag, sbsp_header_pointer));
                }
            }

            case TagClassInt::TAG_CLASS_PREFERENCES_NETWORK_GAME:
            case TagClassInt::TAG_CLASS_SPHEROID:
            case TagClassInt::TAG_CLASS_CONTINUOUS_DAMAGE_EFFECT:
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

        REPORT_ERROR_PRINTF(*this, ERROR_TYPE_ERROR, tag_index, "Tag class %s is unsupported", tag_class_to_extension(tag_class_int));
        return std::nullopt;
    }
}
