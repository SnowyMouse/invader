/*
 * Invader (c) 2018 Kavawuvi
 *
 * This program is free software under the GNU General Public License v3.0 or later. See LICENSE for more information.
 */

#ifndef INVADER__TAG__HEK__CLASS__FONT_HPP
#define INVADER__TAG__HEK__CLASS__FONT_HPP

#include "../../compiled_tag.hpp"
#include "../../../hek/data_type.hpp"
#include "../header.hpp"

namespace Invader::HEK {
    ENDIAN_TEMPLATE(EndianType) struct FontCharacterIndex {
        EndianType<std::int16_t> character_index;

        ENDIAN_TEMPLATE(NewType) operator FontCharacterIndex<NewType>() const noexcept {
            FontCharacterIndex<NewType> copy;
            COPY_THIS(character_index);
            return copy;
        }
    };
    static_assert(sizeof(FontCharacterIndex<BigEndian>) == 0x2);

    ENDIAN_TEMPLATE(EndianType) struct FontCharacterTables {
        TagReflexive<EndianType, FontCharacterIndex> character_table;

        ENDIAN_TEMPLATE(NewType) operator FontCharacterTables<NewType>() const noexcept {
            FontCharacterTables<NewType> copy;
            COPY_THIS(character_table);
            return copy;
        }
    };
    static_assert(sizeof(FontCharacterTables<BigEndian>) == 0xC);

    ENDIAN_TEMPLATE(EndianType) struct FontCharacter {
        EndianType<std::int16_t> character;
        EndianType<std::int16_t> character_width;
        EndianType<std::int16_t> bitmap_width;
        EndianType<std::int16_t> bitmap_height;
        EndianType<std::int16_t> bitmap_origin_x;
        EndianType<std::int16_t> bitmap_origin_y;
        EndianType<std::int16_t> hardware_character_index;
        PAD(0x2);
        EndianType<std::int32_t> pixels_offset;

        ENDIAN_TEMPLATE(NewType) operator FontCharacter<NewType>() const noexcept {
            FontCharacter<NewType> copy = {};
            COPY_THIS(character);
            COPY_THIS(character_width);
            COPY_THIS(bitmap_width);
            COPY_THIS(bitmap_height);
            COPY_THIS(bitmap_origin_x);
            COPY_THIS(bitmap_origin_y);
            COPY_THIS(hardware_character_index);
            COPY_THIS(pixels_offset);
            return copy;
        }
    };
    static_assert(sizeof(FontCharacter<BigEndian>) == 0x14);

    ENDIAN_TEMPLATE(EndianType) struct Font {
        EndianType<std::int32_t> flags;
        EndianType<std::int16_t> ascending_height;
        EndianType<std::int16_t> descending_height;
        EndianType<std::int16_t> leading_height;
        EndianType<std::int16_t> leading_width;
        PAD(0x24);
        TagReflexive<EndianType, FontCharacterTables> character_tables;
        TagDependency<EndianType> bold; // font
        TagDependency<EndianType> italic; // font
        TagDependency<EndianType> condense; // font
        TagDependency<EndianType> underline; // font
        TagReflexive<EndianType, FontCharacter> characters;
        TagDataOffset<EndianType> pixels;

        ENDIAN_TEMPLATE(NewType) operator Font<NewType>() const noexcept {
            Font<NewType> copy = {};
            COPY_THIS(flags);
            COPY_THIS(ascending_height);
            COPY_THIS(descending_height);
            COPY_THIS(leading_height);
            COPY_THIS(leading_width);
            COPY_THIS(character_tables);
            COPY_THIS(bold);
            COPY_THIS(italic);
            COPY_THIS(condense);
            COPY_THIS(underline);
            COPY_THIS(characters);
            COPY_THIS(pixels);
            return copy;
        }
    };
    static_assert(sizeof(Font<BigEndian>) == 0x9C);

    void compile_font_tag(CompiledTag &compiled, const std::byte *data, std::size_t size);
}
#endif
