/*
 * Invader (c) 2018 Kavawuvi
 *
 * This program is free software under the GNU General Public License v3.0 or later. See LICENSE for more information.
 */

#include "header.hpp"

namespace Invader::HEK {
    std::uint16_t TagFileHeader::version_for_tag(TagClassInt tag_class_int) {
        switch(tag_class_int) {
            case TAG_CLASS_ACTOR:
                return 2;
            case TAG_CLASS_MODEL_ANIMATIONS:
                return 4;
            case TAG_CLASS_BIPED:
                return 3;
            case TAG_CLASS_BITMAP:
                return 7;
            case TAG_CLASS_CONTRAIL:
                return 3;
            case TAG_CLASS_EFFECT:
                return 4;
            case TAG_CLASS_EQUIPMENT:
                return 2;
            case TAG_CLASS_ITEM:
                return 2;
            case TAG_CLASS_ITEM_COLLECTION:
                return 0;
            case TAG_CLASS_DAMAGE_EFFECT:
                return 6;
            case TAG_CLASS_LENS_FLARE:
                return 2;
            case TAG_CLASS_LIGHT:
                return 3;
            case TAG_CLASS_SOUND_LOOPING:
                return 3;
            case TAG_CLASS_GBXMODEL:
                return 5;
            case TAG_CLASS_GLOBALS:
                return 3;
            case TAG_CLASS_MODEL:
                return 4;
            case TAG_CLASS_MODEL_COLLISION_GEOMETRY:
                return 10;
            case TAG_CLASS_MULTIPLAYER_SCENARIO_DESCRIPTION:
                return 2;
            case TAG_CLASS_PARTICLE:
                return 2;
            case TAG_CLASS_PARTICLE_SYSTEM:
                return 4;
            case TAG_CLASS_PHYSICS:
                return 4;
            case TAG_CLASS_PLACEHOLDER:
                return 2;
            case TAG_CLASS_PROJECTILE:
                return 5;
            case TAG_CLASS_SCENARIO_STRUCTURE_BSP:
                return 5;
            case TAG_CLASS_SCENARIO:
                return 2;
            case TAG_CLASS_SHADER_ENVIRONMENT:
                return 2;
            case TAG_CLASS_SOUND:
                return 4;
            case TAG_CLASS_SHADER_MODEL:
                return 2;
            case TAG_CLASS_SHADER_TRANSPARENT_WATER:
                return 2;
            case TAG_CLASS_UNIT:
                return 2;
            case TAG_CLASS_VIRTUAL_KEYBOARD:
                return 2;
            case TAG_CLASS_WEAPON:
                return 2;
            case TAG_CLASS_WEAPON_HUD_INTERFACE:
                return 2;
            default:
                return 1;
        }
    }

    TagFileHeader::TagFileHeader(TagClassInt tag_class_int) {
        this->tag_class_int = tag_class_int;
        this->blam = BLAM;
        this->header_size = sizeof(*this);
        this->something_255 = 255;
        this->version = TagFileHeader::version_for_tag(tag_class_int);
    }
}
