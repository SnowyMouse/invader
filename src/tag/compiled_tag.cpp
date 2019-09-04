/*
 * Invader (c) 2018 Kavawuvi
 *
 * This program is free software under the GNU General Public License v3.0 or later. See LICENSE for more information.
 */

#ifdef _WIN32
#include <windows.h>
#define PATH_BUFFER_LENGTH MAX_PATH
#else
#include <linux/limits.h>
#define PATH_BUFFER_LENGTH PATH_MAX
#endif

#include <memory>
#include <cstdio>

#include "compiled_tag.hpp"

#include "../error.hpp"
#include "hek/header.hpp"
#include "hek/class/globals.hpp"
#include "hek/class/damage_effect.hpp"
#include "hek/class/object.hpp"
#include "hek/class/weapon.hpp"

namespace Invader::HEK {
    /**
     * Compile the tag
     * @param compiled  CompiledTag reference
     * @param data      pointer to tag data
     * @param size      size of the data
     * @param class_int tag type to compile
     * @param type      cache file type
     * @param path      path of the tag
     */
    static inline void compile_tag_fn(CompiledTag &compiled, const std::byte *data, std::size_t size, TagClassInt class_int, CacheFileType type, const std::string &path) {
        #define COMPILE_EXTERN_TAG_FN(function_name) {\
            extern void function_name(CompiledTag &compiled, const std::byte *data, std::size_t size); \
            function_name(compiled, data, size);\
        }

        switch(class_int) {
            case TagClassInt::TAG_CLASS_DAMAGE_EFFECT: {
                DamageEffectJasonJones jason_jones = DAMAGE_EFFECT_JASON_JONES_NONE;
                if(type == CACHE_FILE_SINGLEPLAYER) {
                    if(path == "weapons\\pistol\\bullet") {
                        jason_jones = DAMAGE_EFFECT_JASON_JONES_PISTOL_SINGLEPLAYER;
                    }
                }
                compile_damage_effect_tag(compiled, data, size, jason_jones);
                break;
            }
            case TagClassInt::TAG_CLASS_ACTOR:
                COMPILE_EXTERN_TAG_FN(compile_actor_tag);
                break;
            case TagClassInt::TAG_CLASS_ACTOR_VARIANT:
                COMPILE_EXTERN_TAG_FN(compile_actor_variant_tag);
                break;
            case TagClassInt::TAG_CLASS_ANTENNA:
                COMPILE_EXTERN_TAG_FN(compile_antenna_tag);
                break;
            case TagClassInt::TAG_CLASS_BIPED:
                COMPILE_EXTERN_TAG_FN(compile_biped_tag);
                break;
            case TagClassInt::TAG_CLASS_BITMAP:
                COMPILE_EXTERN_TAG_FN(compile_bitmap_tag);
                break;
            case TagClassInt::TAG_CLASS_CAMERA_TRACK:
                COMPILE_EXTERN_TAG_FN(compile_camera_track_tag);
                break;
            case TagClassInt::TAG_CLASS_CONTRAIL:
                COMPILE_EXTERN_TAG_FN(compile_contrail_tag);
                break;
            case TagClassInt::TAG_CLASS_COLOR_TABLE:
                COMPILE_EXTERN_TAG_FN(compile_color_table_tag);
                break;
            case TagClassInt::TAG_CLASS_DECAL:
                COMPILE_EXTERN_TAG_FN(compile_decal_tag);
                break;
            case TagClassInt::TAG_CLASS_DETAIL_OBJECT_COLLECTION:
                COMPILE_EXTERN_TAG_FN(compile_detail_object_collection_tag);
                break;
            case TagClassInt::TAG_CLASS_DEVICE_CONTROL:
                COMPILE_EXTERN_TAG_FN(compile_device_control_tag);
                break;
            case TagClassInt::TAG_CLASS_DEVICE_LIGHT_FIXTURE:
                COMPILE_EXTERN_TAG_FN(compile_device_light_fixture_tag);
                break;
            case TagClassInt::TAG_CLASS_DEVICE_MACHINE:
                COMPILE_EXTERN_TAG_FN(compile_device_machine_tag);
                break;
            case TagClassInt::TAG_CLASS_DIALOGUE:
                COMPILE_EXTERN_TAG_FN(compile_dialogue_tag);
                break;
            case TagClassInt::TAG_CLASS_EFFECT:
                COMPILE_EXTERN_TAG_FN(compile_effect_tag);
                break;
            case TagClassInt::TAG_CLASS_EQUIPMENT:
                COMPILE_EXTERN_TAG_FN(compile_equipment_tag);
                break;
            case TagClassInt::TAG_CLASS_FLAG:
                COMPILE_EXTERN_TAG_FN(compile_flag_tag);
                break;
            case TagClassInt::TAG_CLASS_FOG:
                COMPILE_EXTERN_TAG_FN(compile_fog_tag);
                break;
            case TagClassInt::TAG_CLASS_FONT:
                COMPILE_EXTERN_TAG_FN(compile_font_tag);
                break;
            case TagClassInt::TAG_CLASS_GARBAGE:
                COMPILE_EXTERN_TAG_FN(compile_garbage_tag);
                break;
            case TagClassInt::TAG_CLASS_GBXMODEL:
                COMPILE_EXTERN_TAG_FN(compile_gbxmodel_tag);
                break;
            case TagClassInt::TAG_CLASS_GLOBALS:
                compile_globals_tag(compiled, data, size, type);
                break;
            case TagClassInt::TAG_CLASS_GLOW:
                COMPILE_EXTERN_TAG_FN(compile_glow_tag);
                break;
            case TagClassInt::TAG_CLASS_GRENADE_HUD_INTERFACE:
                COMPILE_EXTERN_TAG_FN(compile_grenade_hud_interface_tag);
                break;
            case TagClassInt::TAG_CLASS_HUD_GLOBALS:
                COMPILE_EXTERN_TAG_FN(compile_hud_globals_tag);
                break;
            case TagClassInt::TAG_CLASS_HUD_MESSAGE_TEXT:
                COMPILE_EXTERN_TAG_FN(compile_hud_message_text_tag);
                break;
            case TagClassInt::TAG_CLASS_HUD_NUMBER:
                COMPILE_EXTERN_TAG_FN(compile_hud_number_tag);
                break;
            case TagClassInt::TAG_CLASS_INPUT_DEVICE_DEFAULTS:
                COMPILE_EXTERN_TAG_FN(compile_input_device_defaults_tag);
                break;
            case TagClassInt::TAG_CLASS_ITEM_COLLECTION:
                COMPILE_EXTERN_TAG_FN(compile_item_collection_tag);
                break;
            case TagClassInt::TAG_CLASS_LENS_FLARE:
                COMPILE_EXTERN_TAG_FN(compile_lens_flare_tag);
                break;
            case TagClassInt::TAG_CLASS_LIGHT:
                COMPILE_EXTERN_TAG_FN(compile_light_tag);
                break;
            case TagClassInt::TAG_CLASS_LIGHTNING:
                COMPILE_EXTERN_TAG_FN(compile_lightning_tag);
                break;
            case TagClassInt::TAG_CLASS_LIGHT_VOLUME:
                COMPILE_EXTERN_TAG_FN(compile_light_volume_tag);
                break;
            case TagClassInt::TAG_CLASS_MATERIAL_EFFECTS:
                COMPILE_EXTERN_TAG_FN(compile_material_effects_tag);
                break;
            case TagClassInt::TAG_CLASS_METER:
                COMPILE_EXTERN_TAG_FN(compile_meter_tag);
                break;
            case TagClassInt::TAG_CLASS_MODEL_ANIMATIONS:
                COMPILE_EXTERN_TAG_FN(compile_model_animations_tag);
                break;
            case TagClassInt::TAG_CLASS_MODEL_COLLISION_GEOMETRY:
                COMPILE_EXTERN_TAG_FN(compile_model_collision_geometry_tag);
                break;
            case TagClassInt::TAG_CLASS_MULTIPLAYER_SCENARIO_DESCRIPTION:
                COMPILE_EXTERN_TAG_FN(compile_multiplayer_scenario_description_tag);
                break;
            case TagClassInt::TAG_CLASS_PARTICLE:
                COMPILE_EXTERN_TAG_FN(compile_particle_tag);
                break;
            case TagClassInt::TAG_CLASS_PARTICLE_SYSTEM:
                COMPILE_EXTERN_TAG_FN(compile_particle_system_tag);
                break;
            case TagClassInt::TAG_CLASS_PLACEHOLDER:
                compile_object_tag(compiled, data, size, ObjectType::OBJECT_TYPE_PLACEHOLDER);
                break;
            case TagClassInt::TAG_CLASS_PROJECTILE:
                COMPILE_EXTERN_TAG_FN(compile_projectile_tag);
                break;
            case TagClassInt::TAG_CLASS_POINT_PHYSICS:
                COMPILE_EXTERN_TAG_FN(compile_point_physics_tag);
                break;
            case TagClassInt::TAG_CLASS_PHYSICS:
                COMPILE_EXTERN_TAG_FN(compile_physics_tag);
                break;
            case TagClassInt::TAG_CLASS_SCENARIO:
                COMPILE_EXTERN_TAG_FN(compile_scenario_tag);
                break;
            case TagClassInt::TAG_CLASS_SCENARIO_STRUCTURE_BSP:
                COMPILE_EXTERN_TAG_FN(compile_scenario_structure_bsp_tag);
                break;
            case TagClassInt::TAG_CLASS_SCENERY:
                compile_object_tag(compiled, data, size, ObjectType::OBJECT_TYPE_SCENERY);
                break;
            case TagClassInt::TAG_CLASS_SHADER_ENVIRONMENT:
                COMPILE_EXTERN_TAG_FN(compile_shader_environment_tag);
                break;
            case TagClassInt::TAG_CLASS_SHADER_MODEL:
                COMPILE_EXTERN_TAG_FN(compile_shader_model_tag);
                break;
            case TagClassInt::TAG_CLASS_SHADER_TRANSPARENT_CHICAGO:
                COMPILE_EXTERN_TAG_FN(compile_shader_transparent_chicago_tag);
                break;
            case TagClassInt::TAG_CLASS_SHADER_TRANSPARENT_CHICAGO_EXTENDED:
                COMPILE_EXTERN_TAG_FN(compile_shader_transparent_chicago_extended_tag);
                break;
            case TagClassInt::TAG_CLASS_SHADER_TRANSPARENT_GENERIC:
                COMPILE_EXTERN_TAG_FN(compile_shader_transparent_generic_tag);
                break;
            case TagClassInt::TAG_CLASS_SHADER_TRANSPARENT_GLASS:
                COMPILE_EXTERN_TAG_FN(compile_shader_transparent_glass_tag);
                break;
            case TagClassInt::TAG_CLASS_SHADER_TRANSPARENT_METER:
                COMPILE_EXTERN_TAG_FN(compile_shader_transparent_meter_tag);
                break;
            case TagClassInt::TAG_CLASS_SHADER_TRANSPARENT_PLASMA:
                COMPILE_EXTERN_TAG_FN(compile_shader_transparent_plasma_tag);
                break;
            case TagClassInt::TAG_CLASS_SHADER_TRANSPARENT_WATER:
                COMPILE_EXTERN_TAG_FN(compile_shader_transparent_water_tag);
                break;
            case TagClassInt::TAG_CLASS_SKY:
                COMPILE_EXTERN_TAG_FN(compile_sky_tag);
                break;
            case TagClassInt::TAG_CLASS_SOUND:
                COMPILE_EXTERN_TAG_FN(compile_sound_tag);
                break;
            case TagClassInt::TAG_CLASS_SOUND_ENVIRONMENT:
                COMPILE_EXTERN_TAG_FN(compile_sound_environment_tag);
                break;
            case TagClassInt::TAG_CLASS_SOUND_LOOPING:
                COMPILE_EXTERN_TAG_FN(compile_sound_looping_tag);
                break;
            case TagClassInt::TAG_CLASS_SOUND_SCENERY:
                compile_object_tag(compiled, data, size, ObjectType::OBJECT_TYPE_SOUND_SCENERY);
                break;
            case TagClassInt::TAG_CLASS_STRING_LIST:
                COMPILE_EXTERN_TAG_FN(compile_string_list_tag);
                break;
            case TagClassInt::TAG_CLASS_TAG_COLLECTION:
                COMPILE_EXTERN_TAG_FN(compile_tag_collection_tag);
                break;
            case TagClassInt::TAG_CLASS_UI_WIDGET_COLLECTION:
                COMPILE_EXTERN_TAG_FN(compile_tag_collection_tag);
                break;
            case TagClassInt::TAG_CLASS_UI_WIDGET_DEFINITION:
                COMPILE_EXTERN_TAG_FN(compile_ui_widget_definition_tag);
                break;
            case TagClassInt::TAG_CLASS_UNICODE_STRING_LIST:
                COMPILE_EXTERN_TAG_FN(compile_string_list_tag);
                break;
            case TagClassInt::TAG_CLASS_UNIT_HUD_INTERFACE:
                COMPILE_EXTERN_TAG_FN(compile_unit_hud_interface_tag);
                break;
            case TagClassInt::TAG_CLASS_VEHICLE:
                COMPILE_EXTERN_TAG_FN(compile_vehicle_tag);
                break;
            case TagClassInt::TAG_CLASS_VIRTUAL_KEYBOARD:
                COMPILE_EXTERN_TAG_FN(compile_virtual_keyboard_tag);
                break;
            case TagClassInt::TAG_CLASS_WEAPON: {
                WeaponJasonJones jason_jones = WEAPON_JASON_JONES_NONE;
                if(type == CACHE_FILE_SINGLEPLAYER) {
                    if(path == "weapons\\pistol\\pistol") {
                        jason_jones = WEAPON_JASON_JONES_PISTOL_SINGLEPLAYER;
                    }
                    else if(path == "weapons\\plasma rifle\\plasma rifle") {
                        jason_jones = WEAPON_JASON_JONES_PLASMA_RIFLE_SINGLEPLAYER;
                    }
                }
                compile_weapon_tag(compiled, data, size, jason_jones);
                break;
            }
            case TagClassInt::TAG_CLASS_WEAPON_HUD_INTERFACE:
                COMPILE_EXTERN_TAG_FN(compile_weapon_hud_interface_tag);
                break;
            case TagClassInt::TAG_CLASS_WEATHER_PARTICLE_SYSTEM:
                COMPILE_EXTERN_TAG_FN(compile_weather_particle_system_tag);
                break;
            case TagClassInt::TAG_CLASS_WIND:
                COMPILE_EXTERN_TAG_FN(compile_wind_tag);
                break;
            default:
                throw UnknownTagClassException();
        }
    }
}

namespace Invader {
    using namespace Invader::HEK;

    std::size_t CompiledTag::resolve_pointer(std::size_t offset) noexcept {
        for(auto &pointer : this->pointers) {
            if(pointer.offset == offset) {
                return pointer.offset_pointed;
            }
        }
        return INVALID_POINTER;
    }

    std::size_t CompiledTag::resolve_pointer(HEK::LittleEndian<HEK::Pointer> *offset) noexcept {
        return this->resolve_pointer(static_cast<std::size_t>(reinterpret_cast<std::byte *>(offset) - this->data.data()));
    }

    CompiledTag::CompiledTag(const std::string &path, HEK::TagClassInt class_int) : path(path), tag_class_int(class_int), p_stub(true) {}

    CompiledTag::CompiledTag(const std::string &path, HEK::TagClassInt class_int, const std::byte *data, std::size_t size, CacheFileType type) : CompiledTag(path, data, size, type) {
        if(this->tag_class_int != class_int) {
            throw UnexpectedTagClassException();
        }
    }

    Invader::CompiledTag::CompiledTag(const std::string &path, const std::byte *data, std::size_t size, CacheFileType type) : path(path) {
        if(size < sizeof(HEK::TagFileHeader)) {
            throw OutOfBoundsException();
        }

        auto &header = *reinterpret_cast<const HEK::TagFileHeader *>(data);
        this->tag_class_int = header.tag_class_int;
        compile_tag_fn(*this, data + sizeof(header), size - sizeof(header), this->tag_class_int, type, path);
        this->data_size = this->data.size();
        this->asset_data_size = this->asset_data.size();
    }

    bool CompiledTag::stub() const noexcept {
        return p_stub;
    }

    TagID tag_id_from_index(std::size_t index) noexcept {
        TagID tag;
        tag.id = static_cast<std::uint32_t>((index & 0xFFFF) | ((static_cast<std::uint32_t>(index + 0xE741) << 16) & 0xFFFF0000));
        return tag;
    }
}
