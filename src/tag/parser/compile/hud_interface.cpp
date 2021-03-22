// SPDX-License-Identifier: GPL-3.0-only

#include <invader/tag/parser/parser.hpp>
#include <invader/file/file.hpp>
#include <invader/build/build_workload.hpp>

#include "hud_interface.hpp"

#define CHECK_INTERFACE_BITMAP_SEQ(bitmap_tag, sequence_index, name) { \
    if((!workload.disable_recursion && !workload.disable_error_checking) && (!bitmap_tag.tag_id.is_null())) { \
        std::size_t sequence_count; \
        const BitmapGroupSequence::struct_little *sequences; \
        char bitmap_tag_path[256]; \
        HEK::BitmapType bitmap_type; \
        get_sequence_data(workload, bitmap_tag.tag_id, sequence_count, sequences, bitmap_tag_path, sizeof(bitmap_tag_path), bitmap_type); \
        if(sequence_index >= sequence_count) { \
            REPORT_ERROR_PRINTF(workload, ERROR_TYPE_FATAL_ERROR, tag_index, "Sequence #%zu in %s referenced by %s is out of bounds (>= %zu)", static_cast<std::size_t>(sequence_index), bitmap_tag_path, name, sequence_count); \
            throw InvalidTagDataException(); \
        } \
        else { \
            auto &sequence = sequences[sequence_index]; \
            if(bitmap_type == HEK::BitmapType::BITMAP_TYPE_SPRITES && sequence.sprites.count == 0) { \
                REPORT_ERROR_PRINTF(workload, ERROR_TYPE_FATAL_ERROR, tag_index, "Sequence #%zu in %s referenced in %s has 0 sprites", static_cast<std::size_t>(sequence_index), name, bitmap_tag_path); \
                throw InvalidTagDataException(); \
            } \
            else if(bitmap_type != HEK::BitmapType::BITMAP_TYPE_SPRITES && sequence.bitmap_count == 0) { \
                REPORT_ERROR_PRINTF(workload, ERROR_TYPE_FATAL_ERROR, tag_index, "Sequence #%zu in %s referenced in %s has 0 bitmaps", static_cast<std::size_t>(sequence_index), name, bitmap_tag_path); \
                throw InvalidTagDataException(); \
            } \
        } \
    } \
}
        
#define CHECK_HIGH_RES_SCALING(flags, name) \
if((flags & HEK::HUDInterfaceScalingFlagsFlag::HUD_INTERFACE_SCALING_FLAGS_FLAG_USE_HIGH_RES_SCALE) && workload.get_build_parameters()->details.build_cache_file_engine == HEK::CacheFileEngine::CACHE_FILE_XBOX) { \
    std::string name_copy = name; \
    name_copy[0] = std::toupper(name_copy[0]); \
    REPORT_ERROR_PRINTF(workload, ERROR_TYPE_ERROR, tag_index, "%s has high resolution scaling enabled, but this does not exist on the target engine", name_copy.c_str()); \
}

namespace Invader::Parser {
    void WeaponHUDInterface::pre_compile(BuildWorkload &, std::size_t, std::size_t, std::size_t) {
        union {
            std::uint32_t inty;
            HEK::WeaponHUDInterfaceCrosshairTypeFlags flaggy;
        } crosshair_types = {};
        for(auto &c : this->crosshairs) {
            crosshair_types.inty |= (1 << c.crosshair_type);
        }
        this->crosshair_types = crosshair_types.flaggy;

        // Check for zoom flags if we don't have any zoom crosshairs
        if(!(this->crosshair_types & HEK::WeaponHUDInterfaceCrosshairTypeFlagsFlag::WEAPON_HUD_INTERFACE_CROSSHAIR_TYPE_FLAGS_FLAG_ZOOM_LEVEL)) {
            auto &crosshair_types = this->crosshair_types;
            
            // If any zoom flags are set, pretend we do have zoom crosshairs
            for(auto &c : this->crosshairs) {
                auto set_hud_for_zoom_flags = [&c, &crosshair_types]() {
                    for(auto &o : c.crosshair_overlays) {
                        if(
                            (o.flags & HEK::WeaponHUDInterfaceCrosshairOverlayFlagsFlag::WEAPON_HUD_INTERFACE_CROSSHAIR_OVERLAY_FLAGS_FLAG_DONT_SHOW_WHEN_ZOOMED) ||
                            (o.flags & HEK::WeaponHUDInterfaceCrosshairOverlayFlagsFlag::WEAPON_HUD_INTERFACE_CROSSHAIR_OVERLAY_FLAGS_FLAG_SHOW_ONLY_WHEN_ZOOMED) ||
                            (o.flags & HEK::WeaponHUDInterfaceCrosshairOverlayFlagsFlag::WEAPON_HUD_INTERFACE_CROSSHAIR_OVERLAY_FLAGS_FLAG_ONE_ZOOM_LEVEL)
                        ) {
                            crosshair_types |= HEK::WeaponHUDInterfaceCrosshairTypeFlagsFlag::WEAPON_HUD_INTERFACE_CROSSHAIR_TYPE_FLAGS_FLAG_ZOOM_LEVEL;
                            return true;
                        }
                    }
                    return false;
                };
                
                if(set_hud_for_zoom_flags()) {
                    break;
                }
            }
        }
    }

    void get_sequence_data(const BuildWorkload &workload, const HEK::TagID &tag_id, std::size_t &sequence_count, const BitmapGroupSequence::struct_little *&sequences, char *bitmap_tag_path, std::size_t bitmap_tag_path_size, HEK::BitmapType &bitmap_type) {
        if(tag_id.is_null()) {
            sequence_count = 0;
            sequences = nullptr;
            std::snprintf(bitmap_tag_path, bitmap_tag_path_size, "NULL");
        }
        else {
            const auto &bitmap_tag = workload.tags[tag_id.index];
            std::snprintf(bitmap_tag_path, bitmap_tag_path_size, "%s.%s", bitmap_tag.path.c_str(), HEK::tag_fourcc_to_extension(bitmap_tag.tag_fourcc));
            Invader::File::halo_path_to_preferred_path_chars(bitmap_tag_path);
            const auto &bitmap_struct = workload.structs[*bitmap_tag.base_struct];
            const auto &bitmap = *reinterpret_cast<const Bitmap::struct_little *>(bitmap_struct.data.data());
            bitmap_type = bitmap.type;
            sequence_count = bitmap.bitmap_group_sequence.count;
            sequences = sequence_count > 0 ? reinterpret_cast<const Parser::BitmapGroupSequence::struct_little *>(workload.structs[*bitmap_struct.resolve_pointer(&bitmap.bitmap_group_sequence.pointer)].data.data()) : nullptr;
        }
    }

    void WeaponHUDInterfaceCrosshair::post_compile(BuildWorkload &workload, std::size_t tag_index, std::size_t, std::size_t struct_offset) {
        // Figure out what we're getting into
        std::size_t sequence_count;
        const BitmapGroupSequence::struct_little *sequences;
        char bitmap_tag_path[256];
        HEK::BitmapType bitmap_type;
        get_sequence_data(workload, this->crosshair_bitmap.tag_id, sequence_count, sequences, bitmap_tag_path, sizeof(bitmap_tag_path), bitmap_type);

        // Make sure it's valid
        std::size_t overlay_count = this->crosshair_overlays.size();
        if(!workload.disable_recursion || !workload.disable_error_checking) {
            for(std::size_t i = 0; i < overlay_count; i++) {
                auto &overlay = this->crosshair_overlays[i];
                bool not_a_sprite = overlay.flags & HEK::WeaponHUDInterfaceCrosshairOverlayFlagsFlag::WEAPON_HUD_INTERFACE_CROSSHAIR_OVERLAY_FLAGS_FLAG_NOT_A_SPRITE;
                if(not_a_sprite && bitmap_type == HEK::BitmapType::BITMAP_TYPE_SPRITES) {
                    REPORT_ERROR_PRINTF(workload, ERROR_TYPE_FATAL_ERROR, tag_index, "Overlay #%zu of crosshair #%zu is marked as not a sprite, but %s is a sprite sheet", i, struct_offset / sizeof(WeaponHUDInterfaceCrosshair::struct_little), bitmap_tag_path);
                    throw InvalidTagDataException();
                }
                else if(!not_a_sprite && bitmap_type != HEK::BitmapType::BITMAP_TYPE_SPRITES) {
                    REPORT_ERROR_PRINTF(workload, ERROR_TYPE_FATAL_ERROR, tag_index, "Overlay #%zu of crosshair #%zu is not marked as not a sprite, but %s is not a sprite sheet", i, struct_offset / sizeof(WeaponHUDInterfaceCrosshair::struct_little), bitmap_tag_path);
                    throw InvalidTagDataException();
                }

                if(overlay.sequence_index != NULL_INDEX) {
                    if(overlay.sequence_index >= sequence_count) {
                        REPORT_ERROR_PRINTF(workload, ERROR_TYPE_FATAL_ERROR, tag_index, "Sequence #%zu in %s referenced in overlay #%zu of crosshair #%zu is out of bounds (>= %zu)", static_cast<std::size_t>(overlay.sequence_index), bitmap_tag_path, i, struct_offset / sizeof(WeaponHUDInterfaceCrosshair::struct_little), sequence_count);
                    throw InvalidTagDataException();
                    }
                    else {
                        auto &sequence = sequences[overlay.sequence_index];
                        if(not_a_sprite) {
                            if(sequence.bitmap_count == 0) {
                                REPORT_ERROR_PRINTF(workload, ERROR_TYPE_FATAL_ERROR, tag_index, "Sequence #%zu in %s referenced in overlay #%zu of crosshair #%zu has 0 bitmaps", static_cast<std::size_t>(overlay.sequence_index), bitmap_tag_path, i, struct_offset / sizeof(WeaponHUDInterfaceCrosshair::struct_little));
                                throw InvalidTagDataException();
                            }
                        }
                        else {
                            if(sequence.sprites.count == 0) {
                                REPORT_ERROR_PRINTF(workload, ERROR_TYPE_FATAL_ERROR, tag_index, "Sequence #%zu in %s referenced in overlay #%zu of crosshair #%zu has 0 sprites", static_cast<std::size_t>(overlay.sequence_index), bitmap_tag_path, i, struct_offset / sizeof(WeaponHUDInterfaceCrosshair::struct_little));
                                throw InvalidTagDataException();
                            }
                        }
                    }
                }
            }
        }
    }

    void WeaponHUDInterfaceMeter::post_compile(BuildWorkload &workload, std::size_t tag_index, std::size_t, std::size_t struct_offset) {
        // Make sure it's valid
        auto name = std::string("meter #") + std::to_string(struct_offset / sizeof(*this));
        CHECK_INTERFACE_BITMAP_SEQ(this->meter_bitmap, this->sequence_index, name.c_str());
        CHECK_HIGH_RES_SCALING(this->scaling_flags, name.c_str());
    }

    void WeaponHUDInterfaceStaticElement::post_compile(BuildWorkload &workload, std::size_t tag_index, std::size_t, std::size_t struct_offset) {
        // Make sure it's valid
        auto name = std::string("static element #") + std::to_string(struct_offset / sizeof(*this));
        CHECK_INTERFACE_BITMAP_SEQ(this->interface_bitmap, this->sequence_index, name.c_str());
        CHECK_HIGH_RES_SCALING(this->scaling_flags, name.c_str());
    }

    void WeaponHUDInterfaceOverlayElement::post_compile(BuildWorkload &workload, std::size_t tag_index, std::size_t, std::size_t struct_offset) {
        // Make sure it's valid
        std::size_t overlay_count = this->overlays.size();
        for(std::size_t i = 0; i < overlay_count; i++) {
            auto name = std::string("overlay #") + std::to_string(struct_offset / sizeof(*this));
            auto &overlay = this->overlays[i];
            CHECK_INTERFACE_BITMAP_SEQ(this->overlay_bitmap, overlay.sequence_index, name.c_str());
            CHECK_HIGH_RES_SCALING(overlay.scaling_flags, name.c_str());
        }
    }
    
    void UnitHUDInterface::post_compile(BuildWorkload &workload, std::size_t tag_index, std::size_t, std::size_t) {
        // Bounds check these values
        CHECK_INTERFACE_BITMAP_SEQ(this->hud_background_interface_bitmap, this->hud_background_sequence_index, "HUD background");
        CHECK_INTERFACE_BITMAP_SEQ(this->shield_panel_background_interface_bitmap, this->shield_panel_background_sequence_index, "shield panel background");
        CHECK_INTERFACE_BITMAP_SEQ(this->shield_panel_meter_meter_bitmap, this->shield_panel_meter_sequence_index, "shield panel meter");
        CHECK_INTERFACE_BITMAP_SEQ(this->health_panel_background_interface_bitmap, this->shield_panel_background_sequence_index, "health panel background");
        CHECK_INTERFACE_BITMAP_SEQ(this->health_panel_meter_meter_bitmap, this->health_panel_meter_sequence_index, "health panel meter");
        CHECK_INTERFACE_BITMAP_SEQ(this->motion_sensor_background_interface_bitmap, this->motion_sensor_background_sequence_index, "motion sensor background");
        CHECK_INTERFACE_BITMAP_SEQ(this->motion_sensor_foreground_interface_bitmap, this->motion_sensor_foreground_sequence_index, "motion sensor foreground");
        
        // Check these memes
        CHECK_HIGH_RES_SCALING(this->hud_background_scaling_flags, "HUD background");
        CHECK_HIGH_RES_SCALING(this->shield_panel_background_scaling_flags, "shield panel background");
        CHECK_HIGH_RES_SCALING(this->shield_panel_meter_scaling_flags, "shield panel meter");
        CHECK_HIGH_RES_SCALING(this->health_panel_background_scaling_flags, "health panel background");
        CHECK_HIGH_RES_SCALING(this->health_panel_meter_scaling_flags, "health panel meter");
        CHECK_HIGH_RES_SCALING(this->motion_sensor_background_scaling_flags, "motion sensor background");
        CHECK_HIGH_RES_SCALING(this->motion_sensor_foreground_scaling_flags, "motion sensor foreground");
        
        // Bounds check the overlays
        auto overlay_count = this->overlays.size();
        for(std::size_t i = 0; i < overlay_count; i++) {
            char overlay_name[256];
            std::snprintf(overlay_name, sizeof(overlay_name), "overlay #%zu", i);
            auto &overlay = this->overlays[i];
            CHECK_INTERFACE_BITMAP_SEQ(overlay.interface_bitmap, overlay.sequence_index, overlay_name);
            CHECK_HIGH_RES_SCALING(overlay.scaling_flags, overlay_name);
        }
        
        // Bounds check the meters
        auto meter_count = this->meters.size();
        for(std::size_t i = 0; i < meter_count; i++) {
            char meter_name[256];
            std::snprintf(meter_name, sizeof(meter_name), "meter #%zu", i);
            auto meter_bg_name = std::string(meter_name) + " background";
            auto meter_fg_name = std::string(meter_name) + " meter";
            
            auto &meter = this->meters[i];
            CHECK_INTERFACE_BITMAP_SEQ(meter.background_interface_bitmap, meter.background_sequence_index, meter_bg_name.c_str());
            CHECK_INTERFACE_BITMAP_SEQ(meter.meter_meter_bitmap, meter.meter_sequence_index, meter_fg_name.c_str());
            CHECK_HIGH_RES_SCALING(meter.background_scaling_flags, meter_bg_name.c_str());
            CHECK_HIGH_RES_SCALING(meter.meter_scaling_flags, meter_fg_name.c_str());
        }
    }
}
