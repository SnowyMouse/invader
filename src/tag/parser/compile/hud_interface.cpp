// SPDX-License-Identifier: GPL-3.0-only

#include <invader/tag/parser/parser.hpp>
#include <invader/file/file.hpp>
#include <invader/build/build_workload.hpp>

#include "hud_interface.hpp"

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

    void get_sequence_data(const Invader::BuildWorkload &workload, const HEK::TagID &tag_id, std::size_t &sequence_count, const BitmapGroupSequence::struct_little *&sequences, char *bitmap_tag_path, std::size_t bitmap_tag_path_size, HEK::BitmapType &bitmap_type) {
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

    void WeaponHUDInterfaceCrosshair::post_compile(Invader::BuildWorkload &workload, std::size_t tag_index, std::size_t, std::size_t struct_offset) {
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

    void WeaponHUDInterfaceMeter::post_compile(Invader::BuildWorkload &workload, std::size_t tag_index, std::size_t, std::size_t struct_offset) {
        // Make sure it's valid
        if(this->sequence_index != NULL_INDEX && (!workload.disable_recursion || !workload.disable_error_checking)) {
            // Figure out what we're getting into
            std::size_t sequence_count;
            const BitmapGroupSequence::struct_little *sequences;
            char bitmap_tag_path[256];
            HEK::BitmapType bitmap_type;
            get_sequence_data(workload, this->meter_bitmap.tag_id, sequence_count, sequences, bitmap_tag_path, sizeof(bitmap_tag_path), bitmap_type);

            if(this->sequence_index >= sequence_count) {
                REPORT_ERROR_PRINTF(workload, ERROR_TYPE_FATAL_ERROR, tag_index, "Sequence #%zu in %s referenced in meter #%zu is out of bounds (>= %zu)", static_cast<std::size_t>(this->sequence_index), bitmap_tag_path, struct_offset / sizeof(WeaponHUDInterfaceMeter::struct_little), sequence_count);
                throw InvalidTagDataException();
            }
            else {
                auto &sequence = sequences[this->sequence_index];
                if(bitmap_type == HEK::BitmapType::BITMAP_TYPE_SPRITES && sequence.sprites.count == 0) {
                    REPORT_ERROR_PRINTF(workload, ERROR_TYPE_FATAL_ERROR, tag_index, "Sequence #%zu in %s referenced in meter #%zu has 0 sprites", static_cast<std::size_t>(this->sequence_index), bitmap_tag_path, struct_offset / sizeof(WeaponHUDInterfaceMeter::struct_little));
                    throw InvalidTagDataException();
                }
                else if(bitmap_type != HEK::BitmapType::BITMAP_TYPE_SPRITES && sequence.bitmap_count == 0) {
                    REPORT_ERROR_PRINTF(workload, ERROR_TYPE_FATAL_ERROR, tag_index, "Sequence #%zu in %s referenced in meter #%zu has 0 bitmaps", static_cast<std::size_t>(this->sequence_index), bitmap_tag_path, struct_offset / sizeof(WeaponHUDInterfaceMeter::struct_little));
                    throw InvalidTagDataException();
                }
            }
        }
    }

    void WeaponHUDInterfaceStaticElement::post_compile(Invader::BuildWorkload &workload, std::size_t tag_index, std::size_t, std::size_t struct_offset) {
        // Make sure it's valid
        if(this->sequence_index != NULL_INDEX && (!workload.disable_recursion || !workload.disable_error_checking)) {
            // Figure out what we're getting into
            std::size_t sequence_count;
            const BitmapGroupSequence::struct_little *sequences;
            char bitmap_tag_path[256];
            HEK::BitmapType bitmap_type;
            get_sequence_data(workload, this->interface_bitmap.tag_id, sequence_count, sequences, bitmap_tag_path, sizeof(bitmap_tag_path), bitmap_type);

            if(this->sequence_index >= sequence_count) {
                REPORT_ERROR_PRINTF(workload, ERROR_TYPE_FATAL_ERROR, tag_index, "Sequence #%zu in %s referenced in static element #%zu is out of bounds (>= %zu)", static_cast<std::size_t>(this->sequence_index), bitmap_tag_path, struct_offset / sizeof(WeaponHUDInterfaceStaticElement::struct_little), sequence_count);
                throw InvalidTagDataException();
            }
            else {
                auto &sequence = sequences[this->sequence_index];
                if(bitmap_type == HEK::BitmapType::BITMAP_TYPE_SPRITES && sequence.sprites.count == 0) {
                    REPORT_ERROR_PRINTF(workload, ERROR_TYPE_FATAL_ERROR, tag_index, "Sequence #%zu in %s referenced in static element #%zu has 0 sprites", static_cast<std::size_t>(this->sequence_index), bitmap_tag_path, struct_offset / sizeof(WeaponHUDInterfaceStaticElement::struct_little));
                    throw InvalidTagDataException();
                }
                else if(bitmap_type != HEK::BitmapType::BITMAP_TYPE_SPRITES && sequence.bitmap_count == 0) {
                    REPORT_ERROR_PRINTF(workload, ERROR_TYPE_FATAL_ERROR, tag_index, "Sequence #%zu in %s referenced in static element #%zu has 0 bitmaps", static_cast<std::size_t>(this->sequence_index), bitmap_tag_path, struct_offset / sizeof(WeaponHUDInterfaceStaticElement::struct_little));
                    throw InvalidTagDataException();
                }
            }
        }
    }

    void WeaponHUDInterfaceOverlayElement::post_compile(Invader::BuildWorkload &workload, std::size_t tag_index, std::size_t, std::size_t struct_offset) {
        // Figure out what we're getting into
        std::size_t sequence_count;
        const BitmapGroupSequence::struct_little *sequences;
        char bitmap_tag_path[256];
        HEK::BitmapType bitmap_type;
        get_sequence_data(workload, this->overlay_bitmap.tag_id, sequence_count, sequences, bitmap_tag_path, sizeof(bitmap_tag_path), bitmap_type);

        // Make sure it's valid
        std::size_t overlay_count = this->overlays.size();
        for(std::size_t i = 0; i < overlay_count; i++) {
            auto &overlay = this->overlays[i];
            if(overlay.sequence_index != NULL_INDEX && (!workload.disable_recursion || !workload.disable_error_checking)) {
                if(overlay.sequence_index >= sequence_count) {
                    REPORT_ERROR_PRINTF(workload, ERROR_TYPE_FATAL_ERROR, tag_index, "Sequence #%zu in %s referenced in overlay #%zu of element #%zu is out of bounds (>= %zu)", static_cast<std::size_t>(overlay.sequence_index), bitmap_tag_path, i, struct_offset / sizeof(WeaponHUDInterfaceOverlayElement::struct_little), sequence_count);
                    throw InvalidTagDataException();
                }
                else {
                    auto &sequence = sequences[overlay.sequence_index];
                    if(bitmap_type == HEK::BitmapType::BITMAP_TYPE_SPRITES && sequence.sprites.count == 0) {
                        REPORT_ERROR_PRINTF(workload, ERROR_TYPE_FATAL_ERROR, tag_index, "Sequence #%zu in %s referenced in overlay #%zu of element #%zu has 0 sprites", static_cast<std::size_t>(overlay.sequence_index), bitmap_tag_path, i, struct_offset / sizeof(WeaponHUDInterfaceOverlayElement::struct_little));
                        throw InvalidTagDataException();
                    }
                    else if(bitmap_type != HEK::BitmapType::BITMAP_TYPE_SPRITES && sequence.bitmap_count == 0) {
                        REPORT_ERROR_PRINTF(workload, ERROR_TYPE_FATAL_ERROR, tag_index, "Sequence #%zu in %s referenced in overlay #%zu of element #%zu has 0 bitmaps", static_cast<std::size_t>(overlay.sequence_index), bitmap_tag_path, i, struct_offset / sizeof(WeaponHUDInterfaceOverlayElement::struct_little));
                        throw InvalidTagDataException();
                    }
                }
            }
        }
    }
}
