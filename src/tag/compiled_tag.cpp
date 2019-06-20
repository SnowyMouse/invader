/*
 * Invader (c) 2018 Kavawuvi
 *
 * This program is free software under the GNU General Public License v3.0 or later. See LICENSE for more information.
 */

// TODO: Use std::filesystem
#ifdef _WIN32
#include <windows.h>
#define PATH_BUFFER_LENGTH MAX_PATH
#else
#include <linux/limits.h>
#define PATH_BUFFER_LENGTH PATH_MAX
#endif

#include <memory>
#include <cstdio>

#include "hek/header.hpp"
#include "hek/class/actor.hpp"
#include "hek/class/actor_variant.hpp"
#include "hek/class/antenna.hpp"
#include "hek/class/biped.hpp"
#include "hek/class/bitmap.hpp"
#include "hek/class/camera_track.hpp"
#include "hek/class/color_table.hpp"
#include "hek/class/contrail.hpp"
#include "hek/class/damage_effect.hpp"
#include "hek/class/decal.hpp"
#include "hek/class/detail_object_collection.hpp"
#include "hek/class/device_control.hpp"
#include "hek/class/device_light_fixture.hpp"
#include "hek/class/device_machine.hpp"
#include "hek/class/dialogue.hpp"
#include "hek/class/effect.hpp"
#include "hek/class/equipment.hpp"
#include "hek/class/flag.hpp"
#include "hek/class/fog.hpp"
#include "hek/class/font.hpp"
#include "hek/class/garbage.hpp"
#include "hek/class/gbxmodel.hpp"
#include "hek/class/globals.hpp"
#include "hek/class/glow.hpp"
#include "hek/class/grenade_hud_interface.hpp"
#include "hek/class/hud_globals.hpp"
#include "hek/class/hud_message_text.hpp"
#include "hek/class/hud_number.hpp"
#include "hek/class/input_device_defaults.hpp"
#include "hek/class/item_collection.hpp"
#include "hek/class/lens_flare.hpp"
#include "hek/class/light.hpp"
#include "hek/class/lightning.hpp"
#include "hek/class/light_volume.hpp"
#include "hek/class/material_effects.hpp"
#include "hek/class/meter.hpp"
#include "hek/class/model_animations.hpp"
#include "hek/class/model_collision_geometry.hpp"
#include "hek/class/multiplayer_scenario_description.hpp"
#include "hek/class/object.hpp"
#include "hek/class/particle.hpp"
#include "hek/class/particle_system.hpp"
#include "hek/class/point_physics.hpp"
#include "hek/class/physics.hpp"
#include "hek/class/projectile.hpp"
#include "hek/class/scenario.hpp"
#include "hek/class/scenario_structure_bsp.hpp"
#include "hek/class/shader_environment.hpp"
#include "hek/class/shader_model.hpp"
#include "hek/class/shader_transparent_chicago.hpp"
#include "hek/class/shader_transparent_chicago_extended.hpp"
#include "hek/class/shader_transparent_glass.hpp"
#include "hek/class/shader_transparent_generic.hpp"
#include "hek/class/shader_transparent_meter.hpp"
#include "hek/class/shader_transparent_plasma.hpp"
#include "hek/class/shader_transparent_water.hpp"
#include "hek/class/sky.hpp"
#include "hek/class/sound.hpp"
#include "hek/class/sound_environment.hpp"
#include "hek/class/sound_looping.hpp"
#include "hek/class/tag_collection.hpp"
#include "hek/class/ui_widget_definition.hpp"
#include "hek/class/unicode_string_list.hpp"
#include "hek/class/unit_hud_interface.hpp"
#include "hek/class/vehicle.hpp"
#include "hek/class/weapon.hpp"
#include "hek/class/weapon_hud_interface.hpp"
#include "hek/class/weather_particle_system.hpp"
#include "hek/class/wind.hpp"
#include "hek/class/virtual_keyboard.hpp"

#include "../error.hpp"

#include "compiled_tag.hpp"

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

    CompiledTag::CompiledTag(const std::string &path, const std::byte *data, std::size_t size, CacheFileType type) : path(path) {
        if(size < sizeof(HEK::TagFileHeader)) {
            throw OutOfBoundsException();
        }

        auto &header = *reinterpret_cast<const HEK::TagFileHeader *>(data);
        this->tag_class_int = header.tag_class_int;
        switch(header.tag_class_int) {
            case TagClassInt::TAG_CLASS_ACTOR:
                compile_actor_tag(*this, data + sizeof(header), size - sizeof(header));
                break;
            case TagClassInt::TAG_CLASS_ACTOR_VARIANT:
                compile_actor_variant_tag(*this, data + sizeof(header), size - sizeof(header));
                break;
            case TagClassInt::TAG_CLASS_ANTENNA:
                compile_antenna_tag(*this, data + sizeof(header), size - sizeof(header));
                break;
            case TagClassInt::TAG_CLASS_BIPED:
                compile_biped_tag(*this, data + sizeof(header), size - sizeof(header));
                break;
            case TagClassInt::TAG_CLASS_BITMAP:
                compile_bitmap_tag(*this, data + sizeof(header), size - sizeof(header));
                break;
            case TagClassInt::TAG_CLASS_CAMERA_TRACK:
                compile_camera_track_tag(*this, data + sizeof(header), size - sizeof(header));
                break;
            case TagClassInt::TAG_CLASS_CONTRAIL:
                compile_contrail_tag(*this, data + sizeof(header), size - sizeof(header));
                break;
            case TagClassInt::TAG_CLASS_COLOR_TABLE:
                compile_color_table_tag(*this, data + sizeof(header), size - sizeof(header));
                break;
            case TagClassInt::TAG_CLASS_DAMAGE_EFFECT: {
                DamageEffectJasonJones jason_jones = DAMAGE_EFFECT_JASON_JONES_NONE;
                if(type == CACHE_FILE_SINGLEPLAYER) {
                    if(path == "weapons\\pistol\\bullet") {
                        jason_jones = DAMAGE_EFFECT_JASON_JONES_PISTOL_SINGLEPLAYER;
                    }
                }
                compile_damage_effect_tag(*this, data + sizeof(header), size - sizeof(header), jason_jones);
                break;
            }
            case TagClassInt::TAG_CLASS_DECAL:
                compile_decal_tag(*this, data + sizeof(header), size - sizeof(header));
                break;
            case TagClassInt::TAG_CLASS_DETAIL_OBJECT_COLLECTION:
                compile_detail_object_collection_tag(*this, data + sizeof(header), size - sizeof(header));
                break;
            case TagClassInt::TAG_CLASS_DEVICE_CONTROL:
                compile_device_control_tag(*this, data + sizeof(header), size - sizeof(header));
                break;
            case TagClassInt::TAG_CLASS_DEVICE_LIGHT_FIXTURE:
                compile_device_light_fixture_tag(*this, data + sizeof(header), size - sizeof(header));
                break;
            case TagClassInt::TAG_CLASS_DEVICE_MACHINE:
                compile_device_machine_tag(*this, data + sizeof(header), size - sizeof(header));
                break;
            case TagClassInt::TAG_CLASS_DIALOGUE:
                compile_dialogue_tag(*this, data + sizeof(header), size - sizeof(header));
                break;
            case TagClassInt::TAG_CLASS_EFFECT:
                compile_effect_tag(*this, data + sizeof(header), size - sizeof(header));
                break;
            case TagClassInt::TAG_CLASS_EQUIPMENT:
                compile_equipment_tag(*this, data + sizeof(header), size - sizeof(header));
                break;
            case TagClassInt::TAG_CLASS_FLAG:
                compile_flag_tag(*this, data + sizeof(header), size - sizeof(header));
                break;
            case TagClassInt::TAG_CLASS_FOG:
                compile_fog_tag(*this, data + sizeof(header), size - sizeof(header));
                break;
            case TagClassInt::TAG_CLASS_FONT:
                compile_font_tag(*this, data + sizeof(header), size - sizeof(header));
                break;
            case TagClassInt::TAG_CLASS_GARBAGE:
                compile_garbage_tag(*this, data + sizeof(header), size - sizeof(header));
                break;
            case TagClassInt::TAG_CLASS_GBXMODEL:
                compile_gbxmodel_tag(*this, data + sizeof(header), size - sizeof(header));
                break;
            case TagClassInt::TAG_CLASS_GLOBALS:
                compile_globals_tag(*this, data + sizeof(header), size - sizeof(header), type);
                break;
            case TagClassInt::TAG_CLASS_GLOW:
                compile_glow_tag(*this, data + sizeof(header), size - sizeof(header));
                break;
            case TagClassInt::TAG_CLASS_GRENADE_HUD_INTERFACE:
                compile_grenade_hud_interface_tag(*this, data + sizeof(header), size - sizeof(header));
                break;
            case TagClassInt::TAG_CLASS_HUD_GLOBALS:
                compile_hud_globals_tag(*this, data + sizeof(header), size - sizeof(header));
                break;
            case TagClassInt::TAG_CLASS_HUD_MESSAGE_TEXT:
                compile_hud_message_text_tag(*this, data + sizeof(header), size - sizeof(header));
                break;
            case TagClassInt::TAG_CLASS_HUD_NUMBER:
                compile_hud_number_tag(*this, data + sizeof(header), size - sizeof(header));
                break;
            case TagClassInt::TAG_CLASS_INPUT_DEVICE_DEFAULTS:
                compile_input_device_defaults_tag(*this, data + sizeof(header), size - sizeof(header));
                break;
            case TagClassInt::TAG_CLASS_ITEM_COLLECTION:
                compile_item_collection_tag(*this, data + sizeof(header), size - sizeof(header));
                break;
            case TagClassInt::TAG_CLASS_LENS_FLARE:
                compile_lens_flare_tag(*this, data + sizeof(header), size - sizeof(header));
                break;
            case TagClassInt::TAG_CLASS_LIGHT:
                compile_light_tag(*this, data + sizeof(header), size - sizeof(header));
                break;
            case TagClassInt::TAG_CLASS_LIGHTNING:
                compile_lightning_tag(*this, data + sizeof(header), size - sizeof(header));
                break;
            case TagClassInt::TAG_CLASS_LIGHT_VOLUME:
                compile_light_volume_tag(*this, data + sizeof(header), size - sizeof(header));
                break;
            case TagClassInt::TAG_CLASS_MATERIAL_EFFECTS:
                compile_material_effects_tag(*this, data + sizeof(header), size - sizeof(header));
                break;
            case TagClassInt::TAG_CLASS_METER:
                compile_meter_tag(*this, data + sizeof(header), size - sizeof(header));
                break;
            case TagClassInt::TAG_CLASS_MODEL_ANIMATIONS:
                compile_model_animations_tag(*this, data + sizeof(header), size - sizeof(header));
                break;
            case TagClassInt::TAG_CLASS_MODEL_COLLISION_GEOMETRY:
                compile_model_collision_geometry_tag(*this, data + sizeof(header), size - sizeof(header));
                break;
            case TagClassInt::TAG_CLASS_MULTIPLAYER_SCENARIO_DESCRIPTION:
                compile_multiplayer_scenario_description_tag(*this, data + sizeof(header), size - sizeof(header));
                break;
            case TagClassInt::TAG_CLASS_PARTICLE:
                compile_particle_tag(*this, data + sizeof(header), size - sizeof(header));
                break;
            case TagClassInt::TAG_CLASS_PARTICLE_SYSTEM:
                compile_particle_system_tag(*this, data + sizeof(header), size - sizeof(header));
                break;
            case TagClassInt::TAG_CLASS_PLACEHOLDER:
                compile_object_tag(*this, data + sizeof(header), size - sizeof(header), ObjectType::OBJECT_TYPE_PLACEHOLDER);
                break;
            case TagClassInt::TAG_CLASS_PROJECTILE:
                compile_projectile_tag(*this, data + sizeof(header), size - sizeof(header));
                break;
            case TagClassInt::TAG_CLASS_POINT_PHYSICS:
                compile_point_physics_tag(*this, data + sizeof(header), size - sizeof(header));
                break;
            case TagClassInt::TAG_CLASS_PHYSICS:
                compile_physics_tag(*this, data + sizeof(header), size - sizeof(header));
                break;
            case TagClassInt::TAG_CLASS_SCENARIO:
                compile_scenario_tag(*this, data + sizeof(header), size - sizeof(header));
                break;
            case TagClassInt::TAG_CLASS_SCENARIO_STRUCTURE_BSP:
                compile_scenario_structure_bsp_tag(*this, data + sizeof(header), size - sizeof(header));
                break;
            case TagClassInt::TAG_CLASS_SCENERY:
                compile_object_tag(*this, data + sizeof(header), size - sizeof(header), ObjectType::OBJECT_TYPE_SCENERY);
                break;
            case TagClassInt::TAG_CLASS_SHADER_ENVIRONMENT:
                compile_shader_environment_tag(*this, data + sizeof(header), size - sizeof(header));
                break;
            case TagClassInt::TAG_CLASS_SHADER_MODEL:
                compile_shader_model_tag(*this, data + sizeof(header), size - sizeof(header));
                break;
            case TagClassInt::TAG_CLASS_SHADER_TRANSPARENT_CHICAGO:
                compile_shader_transparent_chicago_tag(*this, data + sizeof(header), size - sizeof(header));
                break;
            case TagClassInt::TAG_CLASS_SHADER_TRANSPARENT_CHICAGO_EXTENDED:
                compile_shader_transparent_chicago_extended_tag(*this, data + sizeof(header), size - sizeof(header));
                break;
            case TagClassInt::TAG_CLASS_SHADER_TRANSPARENT_GENERIC:
                compile_shader_transparent_generic_tag(*this, data + sizeof(header), size - sizeof(header));
                break;
            case TagClassInt::TAG_CLASS_SHADER_TRANSPARENT_GLASS:
                compile_shader_transparent_glass_tag(*this, data + sizeof(header), size - sizeof(header));
                break;
            case TagClassInt::TAG_CLASS_SHADER_TRANSPARENT_METER:
                compile_shader_transparent_meter_tag(*this, data + sizeof(header), size - sizeof(header));
                break;
            case TagClassInt::TAG_CLASS_SHADER_TRANSPARENT_PLASMA:
                compile_shader_transparent_plasma_tag(*this, data + sizeof(header), size - sizeof(header));
                break;
            case TagClassInt::TAG_CLASS_SHADER_TRANSPARENT_WATER:
                compile_shader_transparent_water_tag(*this, data + sizeof(header), size - sizeof(header));
                break;
            case TagClassInt::TAG_CLASS_SKY:
                compile_sky_tag(*this, data + sizeof(header), size - sizeof(header));
                break;
            case TagClassInt::TAG_CLASS_SOUND:
                compile_sound_tag(*this, data + sizeof(header), size - sizeof(header));
                break;
            case TagClassInt::TAG_CLASS_SOUND_ENVIRONMENT:
                compile_sound_environment_tag(*this, data + sizeof(header), size - sizeof(header));
                break;
            case TagClassInt::TAG_CLASS_SOUND_LOOPING:
                compile_sound_looping_tag(*this, data + sizeof(header), size - sizeof(header));
                break;
            case TagClassInt::TAG_CLASS_SOUND_SCENERY:
                compile_object_tag(*this, data + sizeof(header), size - sizeof(header), ObjectType::OBJECT_TYPE_SOUND_SCENERY);
                break;
            case TagClassInt::TAG_CLASS_STRING_LIST:
                compile_unicode_string_list_tag(*this, data + sizeof(header), size - sizeof(header));
                break;
            case TagClassInt::TAG_CLASS_TAG_COLLECTION:
                compile_tag_collection_tag(*this, data + sizeof(header), size - sizeof(header));
                break;
            case TagClassInt::TAG_CLASS_UI_WIDGET_COLLECTION:
                compile_tag_collection_tag(*this, data + sizeof(header), size - sizeof(header));
                break;
            case TagClassInt::TAG_CLASS_UI_WIDGET_DEFINITION:
                compile_ui_widget_definition_tag(*this, data + sizeof(header), size - sizeof(header));
                break;
            case TagClassInt::TAG_CLASS_UNICODE_STRING_LIST:
                compile_unicode_string_list_tag(*this, data + sizeof(header), size - sizeof(header));
                break;
            case TagClassInt::TAG_CLASS_UNIT_HUD_INTERFACE:
                compile_unit_hud_interface_tag(*this, data + sizeof(header), size - sizeof(header));
                break;
            case TagClassInt::TAG_CLASS_VEHICLE:
                compile_vehicle_tag(*this, data + sizeof(header), size - sizeof(header));
                break;
            case TagClassInt::TAG_CLASS_VIRTUAL_KEYBOARD:
                compile_virtual_keyboard_tag(*this, data + sizeof(header), size - sizeof(header));
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
                compile_weapon_tag(*this, data + sizeof(header), size - sizeof(header), jason_jones);
                break;
            }
            case TagClassInt::TAG_CLASS_WEAPON_HUD_INTERFACE:
                compile_weapon_hud_interface_tag(*this, data + sizeof(header), size - sizeof(header));
                break;
            case TagClassInt::TAG_CLASS_WEATHER_PARTICLE_SYSTEM:
                compile_weather_particle_system_tag(*this, data + sizeof(header), size - sizeof(header));
                break;
            case TagClassInt::TAG_CLASS_WIND:
                compile_wind_tag(*this, data + sizeof(header), size - sizeof(header));
                break;
            default:
                throw UnknownTagClassException();
        }
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
