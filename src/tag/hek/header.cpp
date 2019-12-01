// SPDX-License-Identifier: GPL-3.0-only

#include <cstring>
#include <invader/tag/hek/header.hpp>

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
            case TAG_CLASS_CAMERA_TRACK:
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
        // Clear everything
        std::fill(reinterpret_cast<std::byte *>(this), reinterpret_cast<std::byte *>(this + 1), std::byte());

        // Set values
        #ifndef INVADER_EXTRACT_HIDDEN_VALUES
        this->tag_class_int = tag_class_int;
        this->blam = BLAM;
        this->header_size = sizeof(*this);
        this->something_255 = 255;
        this->version = TagFileHeader::version_for_tag(tag_class_int);
        #endif
    }

    void TagFileHeader::validate_header(const TagFileHeader *header, std::size_t tag_file_size, bool verbose, std::optional<TagClassInt> expected_tag_class) {
        if(tag_file_size < sizeof(*header)) {
            if(verbose) {
                eprintf_error("Size of buffer is too small to contain a header");
            }
            throw InvalidTagDataHeaderException();
        }

        if(header->blam != BLAM) {
            if(verbose) {
                eprintf_error("Blam literal in header is incorrect");
            }
            throw InvalidTagDataHeaderException();
        }

        auto tag_class = header->tag_class_int.read();
        auto expected_version = version_for_tag(tag_class);
        if(header->version != expected_version) {
            if(verbose) {
                eprintf_error("Version in header is wrong (%u expected, %u gotten)", expected_version, header->version.read());
            }
            throw InvalidTagDataHeaderException();
        }

        if(expected_tag_class.has_value()) {
            auto expected_tag_class_value = *expected_tag_class;
            if(expected_tag_class_value != tag_class) {
                if(verbose) {
                    eprintf_error("Unexpected tag class in header (%s expected, %s gotten)", tag_class_to_extension(expected_tag_class_value), tag_class_to_extension(tag_class));
                }
                throw InvalidTagDataHeaderException();
            }
        }
    }
}
