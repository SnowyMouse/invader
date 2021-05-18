// SPDX-License-Identifier: GPL-3.0-only

#include <cstring>
#include <invader/tag/hek/header.hpp>

namespace Invader::HEK {
    std::uint16_t TagFileHeader::version_for_tag(TagFourCC tag_fourcc) {
        switch(tag_fourcc) {
            case TAG_FOURCC_ACTOR:
                return 2;
            case TAG_FOURCC_MODEL_ANIMATIONS:
                return 4;
            case TAG_FOURCC_BIPED:
                return 3;
            case TAG_FOURCC_BITMAP:
                return 7;
            case TAG_FOURCC_CONTRAIL:
                return 3;
            case TAG_FOURCC_EFFECT:
                return 4;
            case TAG_FOURCC_EQUIPMENT:
                return 2;
            case TAG_FOURCC_ITEM:
                return 2;
            case TAG_FOURCC_ITEM_COLLECTION:
                return 0;
            case TAG_FOURCC_DAMAGE_EFFECT:
                return 6;
            case TAG_FOURCC_LENS_FLARE:
                return 2;
            case TAG_FOURCC_LIGHT:
                return 3;
            case TAG_FOURCC_SOUND_LOOPING:
                return 3;
            case TAG_FOURCC_GBXMODEL:
                return 5;
            case TAG_FOURCC_GLOBALS:
                return 3;
            case TAG_FOURCC_MODEL:
                return 4;
            case TAG_FOURCC_MODEL_COLLISION_GEOMETRY:
                return 10;
            case TAG_FOURCC_PARTICLE:
                return 2;
            case TAG_FOURCC_PARTICLE_SYSTEM:
                return 4;
            case TAG_FOURCC_PHYSICS:
                return 4;
            case TAG_FOURCC_PLACEHOLDER:
                return 2;
            case TAG_FOURCC_PREFERENCES_NETWORK_GAME:
                return 2;
            case TAG_FOURCC_PROJECTILE:
                return 5;
            case TAG_FOURCC_SCENARIO_STRUCTURE_BSP:
                return 5;
            case TAG_FOURCC_SCENARIO:
                return 2;
            case TAG_FOURCC_SHADER_ENVIRONMENT:
                return 2;
            case TAG_FOURCC_SOUND:
                return 4;
            case TAG_FOURCC_SHADER_MODEL:
                return 2;
            case TAG_FOURCC_SHADER_TRANSPARENT_WATER:
                return 2;
            case TAG_FOURCC_CAMERA_TRACK:
                return 2;
            case TAG_FOURCC_UNIT:
                return 2;
            case TAG_FOURCC_VIRTUAL_KEYBOARD:
                return 2;
            case TAG_FOURCC_WEAPON:
                return 2;
            case TAG_FOURCC_WEAPON_HUD_INTERFACE:
                return 2;
            default:
                return 1;
        }
    }

    TagFileHeader::TagFileHeader(TagFourCC tag_fourcc) {
        // Clear everything
        std::fill(reinterpret_cast<std::byte *>(this), reinterpret_cast<std::byte *>(this + 1), std::byte());

        // Set values
        this->tag_fourcc = tag_fourcc;
        this->blam = BLAM;
        this->header_size = sizeof(*this);
        this->something_255 = 255;
        this->version = TagFileHeader::version_for_tag(tag_fourcc);
    }

    void TagFileHeader::validate_header(const TagFileHeader *header, std::size_t tag_file_size, std::optional<TagFourCC> expected_tag_class) {
        if(tag_file_size < sizeof(*header)) {
            eprintf_error("Size of buffer is too small to contain a header");
            throw InvalidTagDataHeaderException();
        }

        if(header->blam != BLAM) {
            eprintf_error("Blam literal in header is incorrect");
            throw InvalidTagDataHeaderException();
        }

        auto tag_class = header->tag_fourcc.read();
        auto expected_version = version_for_tag(tag_class);
        if(header->version != expected_version) {
            eprintf_error("Version in header is wrong (%u expected, %u gotten)", expected_version, header->version.read());
            throw InvalidTagDataHeaderException();
        }

        if(expected_tag_class.has_value()) {
            auto expected_tag_class_value = *expected_tag_class;
            if(expected_tag_class_value != tag_class) {
                eprintf_error("Unexpected tag class in header (%s expected, %s gotten)", tag_fourcc_to_extension(expected_tag_class_value), tag_fourcc_to_extension(tag_class));
                throw InvalidTagDataHeaderException();
            }
        }
    }
}
