// SPDX-License-Identifier: GPL-3.0-only

#include <regex>
#include <invader/build/build_workload.hpp>
#include <invader/extract/extraction.hpp>
#include <invader/tag/hek/header.hpp>
#include <invader/tag/parser/parser.hpp>

namespace Invader {
    void ExtractionWorkload::extract_map(const Map &map, const std::string &tags, const std::vector<std::string> &queries, const std::vector<std::string> &queries_exclude, bool recursive, bool overwrite, bool non_mp_globals, ReportingLevel reporting_level) {
        // There's no need to extract recursively if we're extracting all tags
        if(queries.size() == 0) {
            recursive = false;
        }

        ExtractionWorkload workload(map, reporting_level);
        auto start = std::chrono::steady_clock::now();
        auto success = workload.perform_extraction(queries, queries_exclude, tags, recursive, overwrite, non_mp_globals);
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
        std::snprintf(timer, sizeof(timer), " in %lu ms", static_cast<unsigned long>(std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()));

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

    std::size_t ExtractionWorkload::perform_extraction(const std::vector<std::string> &queries, const std::vector<std::string> &queries_exclude, const std::filesystem::path &tags, bool recursive, bool overwrite, bool non_mp_globals) {
        // Set these variables up
        auto *map = &this->map;
        auto type = map->get_type();
        auto tag_count = map->get_tag_count();
        std::vector<bool> extracted_tags(tag_count);
        std::deque<std::size_t> all_tags_to_extract;
        auto &workload = *this;
        auto engine = map->get_cache_version();

        auto &scenario_tag = map->get_tag(map->get_scenario_tag_id()).get_base_struct<HEK::Scenario>();
        bool jason_jones = (scenario_tag.flags & HEK::ScenarioFlagsFlag::SCENARIO_FLAGS_FLAG_DO_NOT_APPLY_BUNGIE_CAMPAIGN_TAG_PATCHES) == 0;

        std::vector<float> detail_object_modifiers;
        auto detail_object_palette_size = static_cast<std::size_t>(scenario_tag.detail_object_collection_palette.count);
        if(detail_object_palette_size > 0) {
            auto *detail_object_collections = reinterpret_cast<const Parser::ScenarioDetailObjectCollectionPalette::struct_little *>(
                map->resolve_tag_data_pointer(scenario_tag.detail_object_collection_palette.pointer)
            );
            for(std::size_t dobby = 0; dobby < detail_object_palette_size; dobby++) {
                auto id = detail_object_collections[dobby].reference.tag_id.read();
                if(id.is_null()) {
                    detail_object_modifiers.emplace_back(0.0);
                    continue;
                }
                auto &dobby_destroyer_of_worlds = map->get_tag(id.index).get_base_struct<HEK::DetailObjectCollection>();
                detail_object_modifiers.emplace_back(dobby_destroyer_of_worlds.global_z_offset * 0.125);
            }
        }

        auto extract_tag = [&extracted_tags, &map, &tags, &all_tags_to_extract, &type, &recursive, &overwrite, &non_mp_globals, &workload, &engine, &jason_jones, &detail_object_modifiers](std::size_t tag_index) -> bool {
            // Do it
            extracted_tags[tag_index] = true;

            // Get the tag path
            const auto &tag = map->get_tag(tag_index);
            if(!tag.data_is_available()) {
                return false;
            }

            // See if we can extract this
            auto tag_path = tag.get_path();

            // Get the path
            if(tag_path.empty()) {
                workload.report_error(ErrorType::ERROR_TYPE_ERROR, "Tag path is invalid", tag_index);
                return false;
            }

            // Let's do this
            auto tfp = File::TagFilePath(Invader::File::halo_path_to_preferred_path(tag_path), tag.get_tag_fourcc());

            // Figure out the path we're writing to
            auto tag_path_to_write_to = Invader::File::tag_path_to_file_path(tfp, tags);

            if(!overwrite && std::filesystem::exists(tag_path_to_write_to)) {
                return false;
            }

            // Skip globals
            if(tfp.fourcc == Invader::TagFourCC::TAG_FOURCC_GLOBALS && !non_mp_globals && type != Invader::HEK::CacheFileType::SCENARIO_TYPE_MULTIPLAYER) {
                workload.report_error(ErrorType::ERROR_TYPE_WARNING_PEDANTIC, "Skipping the non-multiplayer map's globals tag", tag_index);
                return false;
            }

            // Get the tag data
            std::vector<std::byte> new_tag;
            try {
                new_tag = Invader::ExtractionWorkload::extract_single_tag(tag);

                // If we're recursive, we want to also get that stuff, too
                if(recursive) {
                    auto tag_compiled = BuildWorkload::compile_single_tag(new_tag.data(), new_tag.size(), std::vector<std::filesystem::path>(), false);
                    std::vector<std::pair<const std::string *, Invader::TagFourCC>> dependencies;
                    for(auto &s : tag_compiled.structs) {
                        for(auto &d : s.dependencies) {
                            auto &tag = tag_compiled.tags[d.tag_index];
                            dependencies.emplace_back(&tag.path, tag.tag_fourcc);
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
                REPORT_ERROR_PRINTF(workload, ERROR_TYPE_ERROR, tag_index, "Failed to extract %s.%s: %s", tfp.path.c_str(), HEK::tag_fourcc_to_extension(tfp.fourcc), e.what());
                return false;
            }

            // Jason Jones the tag
            if(jason_jones && type == Invader::HEK::CacheFileType::SCENARIO_TYPE_SINGLEPLAYER) {
                bool changed = false;
                switch(tfp.fourcc) {
                    case Invader::TagFourCC::TAG_FOURCC_WEAPON: {
                        #define ALTER_TAG_DATA(from, to) if(from != to) { from = to; changed = true; }
                        if(tag_path == "weapons\\pistol\\pistol") {
                            auto parsed = Invader::Parser::Weapon::parse_hek_tag_file(new_tag.data(), new_tag.size());
                            if(parsed.triggers.size() >= 1) {
                                auto &first_trigger = parsed.triggers[0];
                                ALTER_TAG_DATA(first_trigger.minimum_error, DEGREES_TO_RADIANS(0.0F));
                                ALTER_TAG_DATA(first_trigger.error_angle.from, DEGREES_TO_RADIANS(0.2F));
                                ALTER_TAG_DATA(first_trigger.error_angle.to, DEGREES_TO_RADIANS(2.0F));
                            }
                            new_tag = parsed.generate_hek_tag_data(TagFourCC::TAG_FOURCC_WEAPON);
                        }
                        else if(tag_path == "weapons\\plasma rifle\\plasma rifle") {
                            auto parsed = Invader::Parser::Weapon::parse_hek_tag_file(new_tag.data(), new_tag.size());
                            if(parsed.triggers.size() >= 1) {
                                auto &first_trigger = parsed.triggers[0];
                                ALTER_TAG_DATA(first_trigger.error_angle.from, DEGREES_TO_RADIANS(0.5F));
                                ALTER_TAG_DATA(first_trigger.error_angle.to, DEGREES_TO_RADIANS(5.0F));
                            }
                            new_tag = parsed.generate_hek_tag_data(TagFourCC::TAG_FOURCC_WEAPON);
                        }
                        break;
                    }
                    case Invader::TagFourCC::TAG_FOURCC_DAMAGE_EFFECT:
                        if(tag_path == "weapons\\pistol\\bullet") {
                            auto parsed = Invader::Parser::DamageEffect::parse_hek_tag_file(new_tag.data(), new_tag.size());
                            ALTER_TAG_DATA(parsed.elite_energy_shield, 1.0F);
                            new_tag = parsed.generate_hek_tag_data(TagFourCC::TAG_FOURCC_DAMAGE_EFFECT);
                        }
                        break;
                    default:
                        break;

                        #undef ALTER_TAG_DATA
                }

                if(changed) {
                    REPORT_ERROR_PRINTF(workload, ERROR_TYPE_WARNING_PEDANTIC, tag_index, "Weapon tag was changed due to being altered in singleplayer");
                }
            }

            // Fix detail object stuff
            if(tfp.fourcc == Invader::HEK::TagFourCC::TAG_FOURCC_SCENARIO_STRUCTURE_BSP) {
                auto bsp = Invader::Parser::ScenarioStructureBSP::parse_hek_tag_file(new_tag.data(), new_tag.size());
                if(!bsp.detail_objects.empty()) {
                    auto &detail_objects = bsp.detail_objects[0];
                    for(auto &cell : detail_objects.cells) {
                        auto count_index = static_cast<std::size_t>(cell.count_index);
                        for(std::uint32_t q = cell.valid_layers_flags, bitfield_index = 0; q != 0; q >>= 1, bitfield_index++) {
                            if(!(q & 1)) {
                                continue;
                            }

                            if(detail_object_modifiers.size() <= bitfield_index) {
                                continue; // this is bullshit and would've crashed the game upon loading the BSP, but tool.exe doesn't catch it and just ignores it
                            }

                            if(count_index >= detail_objects.z_reference_vectors.size()) {
                                continue; // also bullshit; wow
                            }

                            detail_objects.z_reference_vectors[count_index].z_reference_l -= detail_object_modifiers[bitfield_index];
                            count_index++;
                        }
                    }
                    new_tag = bsp.generate_hek_tag_data(TagFourCC::TAG_FOURCC_SCENARIO_STRUCTURE_BSP);
                }
            }

            // Create directories along the way
            std::error_code ec;
            std::filesystem::create_directories(tag_path_to_write_to.parent_path(), ec);

            // Save it
            auto tag_path_str = tag_path_to_write_to.string();
            if(!Invader::File::save_file(tag_path_str.c_str(), new_tag)) {
                REPORT_ERROR_PRINTF(workload, ERROR_TYPE_ERROR, tag_index, "Failed to save %s", tag_path_str.c_str());
                return false;
            }

            return true;
        };

        // Extract each tag?
        if(queries.empty() && queries_exclude.empty()) {
            for(std::size_t t = 0; t < tag_count; t++) {
                all_tags_to_extract.push_back(t);
            }
        }

        else {
            for(std::size_t t = 0; t < tag_count; t++) {
                // Get the full path
                const auto &tag = map->get_tag(t);
                auto full_tag_path = tag.get_path() + "." + HEK::tag_fourcc_to_extension(tag.get_tag_fourcc());

                // Match it
                if(File::path_matches(full_tag_path.c_str(), queries, queries_exclude)) {
                    all_tags_to_extract.emplace_back(t);
                }
            }

            if(all_tags_to_extract.empty()) {
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
            auto path_dot = File::TagFilePath(File::halo_path_to_preferred_path(tag_map.get_path()), tag_map.get_tag_fourcc());

            // Do it!
            bool result;
            try {
                result = extract_tag(tag);
            }
            catch(std::exception &e) {
                eprintf_error("Error while extracting %s: %s", path_dot.join().c_str(), e.what());
                result = false;
            }
            if(result) {
                oprintf_success("Extracted %s", path_dot.join().c_str());
                extracted++;
            }
            else {
                oprintf("Skipped %s\n", path_dot.join().c_str());
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
            paths.emplace_back(tag.get_path(), tag.get_tag_fourcc());
        }
    }

    std::vector<std::byte> ExtractionWorkload::extract_single_tag(const Tag &tag, ReportingLevel reporting_level) {
        ExtractionWorkload workload(tag.get_map(), reporting_level);
        auto result = workload.extract_tag(tag.get_tag_index());
        if(result.has_value()) {
            return result->get()->generate_hek_tag_data(tag.get_tag_fourcc());
        }
        else {
            throw InvalidTagDataException();
        }
    }

    std::optional<std::unique_ptr<Parser::ParserStruct>> ExtractionWorkload::extract_tag(std::size_t tag_index) {
        auto &tag = this->map.get_tag(tag_index);
        auto tag_fourcc = tag.get_tag_fourcc();

        #define EXTRACT_TAG_GROUP(class_struct, fourcc) case TagFourCC::fourcc: { \
            return std::make_unique<Parser::class_struct>(Parser::class_struct::parse_cache_file_data(tag)); \
        }

        switch(tag_fourcc) {
            EXTRACT_TAG_GROUP(Actor, TAG_FOURCC_ACTOR)
            EXTRACT_TAG_GROUP(ActorVariant, TAG_FOURCC_ACTOR_VARIANT)
            EXTRACT_TAG_GROUP(Antenna, TAG_FOURCC_ANTENNA)
            EXTRACT_TAG_GROUP(ModelAnimations, TAG_FOURCC_MODEL_ANIMATIONS)
            EXTRACT_TAG_GROUP(Biped, TAG_FOURCC_BIPED)
            EXTRACT_TAG_GROUP(Bitmap, TAG_FOURCC_BITMAP)
            EXTRACT_TAG_GROUP(ModelCollisionGeometry, TAG_FOURCC_MODEL_COLLISION_GEOMETRY)
            EXTRACT_TAG_GROUP(ColorTable, TAG_FOURCC_COLOR_TABLE)
            EXTRACT_TAG_GROUP(ContinuousDamageEffect, TAG_FOURCC_CONTINUOUS_DAMAGE_EFFECT)
            EXTRACT_TAG_GROUP(Contrail, TAG_FOURCC_CONTRAIL)
            EXTRACT_TAG_GROUP(DeviceControl, TAG_FOURCC_DEVICE_CONTROL)
            EXTRACT_TAG_GROUP(Decal, TAG_FOURCC_DECAL)
            EXTRACT_TAG_GROUP(UIWidgetDefinition, TAG_FOURCC_UI_WIDGET_DEFINITION)
            EXTRACT_TAG_GROUP(InputDeviceDefaults, TAG_FOURCC_INPUT_DEVICE_DEFAULTS)
            EXTRACT_TAG_GROUP(Device, TAG_FOURCC_DEVICE)
            EXTRACT_TAG_GROUP(DetailObjectCollection, TAG_FOURCC_DETAIL_OBJECT_COLLECTION)
            EXTRACT_TAG_GROUP(Effect, TAG_FOURCC_EFFECT)
            EXTRACT_TAG_GROUP(Equipment, TAG_FOURCC_EQUIPMENT)
            EXTRACT_TAG_GROUP(Flag, TAG_FOURCC_FLAG)
            EXTRACT_TAG_GROUP(Fog, TAG_FOURCC_FOG)
            EXTRACT_TAG_GROUP(Font, TAG_FOURCC_FONT)
            EXTRACT_TAG_GROUP(MaterialEffects, TAG_FOURCC_MATERIAL_EFFECTS)
            EXTRACT_TAG_GROUP(Garbage, TAG_FOURCC_GARBAGE)
            EXTRACT_TAG_GROUP(Glow, TAG_FOURCC_GLOW)
            EXTRACT_TAG_GROUP(GrenadeHUDInterface, TAG_FOURCC_GRENADE_HUD_INTERFACE)
            EXTRACT_TAG_GROUP(HUDMessageText, TAG_FOURCC_HUD_MESSAGE_TEXT)
            EXTRACT_TAG_GROUP(HUDNumber, TAG_FOURCC_HUD_NUMBER)
            EXTRACT_TAG_GROUP(HUDGlobals, TAG_FOURCC_HUD_GLOBALS)
            EXTRACT_TAG_GROUP(Item, TAG_FOURCC_ITEM)
            EXTRACT_TAG_GROUP(ItemCollection, TAG_FOURCC_ITEM_COLLECTION)
            EXTRACT_TAG_GROUP(DamageEffect, TAG_FOURCC_DAMAGE_EFFECT)
            EXTRACT_TAG_GROUP(LensFlare, TAG_FOURCC_LENS_FLARE)
            EXTRACT_TAG_GROUP(Lightning, TAG_FOURCC_LIGHTNING)
            EXTRACT_TAG_GROUP(DeviceLightFixture, TAG_FOURCC_DEVICE_LIGHT_FIXTURE)
            EXTRACT_TAG_GROUP(Light, TAG_FOURCC_LIGHT)
            EXTRACT_TAG_GROUP(SoundLooping, TAG_FOURCC_SOUND_LOOPING)
            EXTRACT_TAG_GROUP(DeviceMachine, TAG_FOURCC_DEVICE_MACHINE)
            EXTRACT_TAG_GROUP(Globals, TAG_FOURCC_GLOBALS)
            EXTRACT_TAG_GROUP(Meter, TAG_FOURCC_METER)
            EXTRACT_TAG_GROUP(LightVolume, TAG_FOURCC_LIGHT_VOLUME)
            EXTRACT_TAG_GROUP(GBXModel, TAG_FOURCC_GBXMODEL)
            EXTRACT_TAG_GROUP(Model, TAG_FOURCC_MODEL)
            EXTRACT_TAG_GROUP(MultiplayerScenarioDescription, TAG_FOURCC_MULTIPLAYER_SCENARIO_DESCRIPTION)
            EXTRACT_TAG_GROUP(PreferencesNetworkGame, TAG_FOURCC_PREFERENCES_NETWORK_GAME)
            EXTRACT_TAG_GROUP(Object, TAG_FOURCC_OBJECT)
            EXTRACT_TAG_GROUP(Particle, TAG_FOURCC_PARTICLE)
            EXTRACT_TAG_GROUP(ParticleSystem, TAG_FOURCC_PARTICLE_SYSTEM)
            EXTRACT_TAG_GROUP(Physics, TAG_FOURCC_PHYSICS)
            EXTRACT_TAG_GROUP(Placeholder, TAG_FOURCC_PLACEHOLDER)
            EXTRACT_TAG_GROUP(PointPhysics, TAG_FOURCC_POINT_PHYSICS)
            EXTRACT_TAG_GROUP(Projectile, TAG_FOURCC_PROJECTILE)
            EXTRACT_TAG_GROUP(WeatherParticleSystem, TAG_FOURCC_WEATHER_PARTICLE_SYSTEM)
            EXTRACT_TAG_GROUP(Scenery, TAG_FOURCC_SCENERY)
            EXTRACT_TAG_GROUP(ShaderTransparentChicagoExtended, TAG_FOURCC_SHADER_TRANSPARENT_CHICAGO_EXTENDED)
            EXTRACT_TAG_GROUP(ShaderTransparentChicago, TAG_FOURCC_SHADER_TRANSPARENT_CHICAGO)
            EXTRACT_TAG_GROUP(Scenario, TAG_FOURCC_SCENARIO)
            EXTRACT_TAG_GROUP(ShaderEnvironment, TAG_FOURCC_SHADER_ENVIRONMENT)
            EXTRACT_TAG_GROUP(ShaderTransparentGlass, TAG_FOURCC_SHADER_TRANSPARENT_GLASS)
            EXTRACT_TAG_GROUP(Shader, TAG_FOURCC_SHADER)
            EXTRACT_TAG_GROUP(Sky, TAG_FOURCC_SKY)
            EXTRACT_TAG_GROUP(ShaderTransparentMeter, TAG_FOURCC_SHADER_TRANSPARENT_METER)
            EXTRACT_TAG_GROUP(Sound, TAG_FOURCC_SOUND)
            EXTRACT_TAG_GROUP(SoundEnvironment, TAG_FOURCC_SOUND_ENVIRONMENT)
            EXTRACT_TAG_GROUP(ShaderModel, TAG_FOURCC_SHADER_MODEL)
            EXTRACT_TAG_GROUP(ShaderTransparentGeneric, TAG_FOURCC_SHADER_TRANSPARENT_GENERIC)
            EXTRACT_TAG_GROUP(TagCollection, TAG_FOURCC_UI_WIDGET_COLLECTION)
            EXTRACT_TAG_GROUP(ShaderTransparentPlasma, TAG_FOURCC_SHADER_TRANSPARENT_PLASMA)
            EXTRACT_TAG_GROUP(SoundScenery, TAG_FOURCC_SOUND_SCENERY)
            EXTRACT_TAG_GROUP(StringList, TAG_FOURCC_STRING_LIST)
            EXTRACT_TAG_GROUP(ShaderTransparentWater, TAG_FOURCC_SHADER_TRANSPARENT_WATER)
            EXTRACT_TAG_GROUP(TagCollection, TAG_FOURCC_TAG_COLLECTION)
            EXTRACT_TAG_GROUP(CameraTrack, TAG_FOURCC_CAMERA_TRACK)
            EXTRACT_TAG_GROUP(Dialogue, TAG_FOURCC_DIALOGUE)
            EXTRACT_TAG_GROUP(UnitHUDInterface, TAG_FOURCC_UNIT_HUD_INTERFACE)
            EXTRACT_TAG_GROUP(Unit, TAG_FOURCC_UNIT)
            EXTRACT_TAG_GROUP(UnicodeStringList, TAG_FOURCC_UNICODE_STRING_LIST)
            EXTRACT_TAG_GROUP(VirtualKeyboard, TAG_FOURCC_VIRTUAL_KEYBOARD)
            EXTRACT_TAG_GROUP(Vehicle, TAG_FOURCC_VEHICLE)
            EXTRACT_TAG_GROUP(Weapon, TAG_FOURCC_WEAPON)
            EXTRACT_TAG_GROUP(Wind, TAG_FOURCC_WIND)
            EXTRACT_TAG_GROUP(WeaponHUDInterface, TAG_FOURCC_WEAPON_HUD_INTERFACE)

            case TagFourCC::TAG_FOURCC_SCENARIO_STRUCTURE_BSP: {
                if(tag.get_map().get_cache_version() == HEK::CacheFileEngine::CACHE_FILE_NATIVE) {
                    return std::make_unique<Parser::ScenarioStructureBSP>(Parser::ScenarioStructureBSP::parse_cache_file_data(tag));
                }
                else {
                    HEK::TagFileHeader tag_data_header(TagFourCC::TAG_FOURCC_SCENARIO_STRUCTURE_BSP);
                    std::vector<std::byte> data(reinterpret_cast<std::byte *>(&tag_data_header), reinterpret_cast<std::byte *>(&tag_data_header + 1));
                    auto sbsp_header_data = tag.get_base_struct<HEK::ScenarioStructureBSPCompiledHeader>();
                    auto sbsp_header_pointer = sbsp_header_data.pointer.read();
                    return std::make_unique<Parser::ScenarioStructureBSP>(Parser::ScenarioStructureBSP::parse_cache_file_data(tag, sbsp_header_pointer));
                }
            }

            case TagFourCC::TAG_FOURCC_NONE:
            case TagFourCC::TAG_FOURCC_NULL:
            case TagFourCC::TAG_FOURCC_SPHEROID:
                break;
        }
        if(std::strcmp(tag_fourcc_to_extension(tag_fourcc), "unknown") == 0) {
            REPORT_ERROR_PRINTF(*this, ERROR_TYPE_ERROR, tag_index, "Tag group FourCC 0x%08X is unknown", tag_fourcc);
        }
        else {
            REPORT_ERROR_PRINTF(*this, ERROR_TYPE_ERROR, tag_index, "Tag group \"%s\" is unsupported", tag_fourcc_to_extension(tag_fourcc));
        }
        return std::nullopt;
    }
}
