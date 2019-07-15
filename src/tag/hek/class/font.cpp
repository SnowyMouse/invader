/*
 * Invader (c) 2018 Kavawuvi
 *
 * This program is free software under the GNU General Public License v3.0 or later. See LICENSE for more information.
 */

#include "../compile.hpp"
#include "font.hpp"

namespace Invader::HEK {
    void compile_font_tag(CompiledTag &compiled, const std::byte *data, std::size_t size) {
        BEGIN_COMPILE(Font)

        // Skip over existing character tables
        skip_data = true;
        ADD_REFLEXIVE_START(tag.character_tables) {
            ADD_REFLEXIVE(reflexive.character_table)
        } ADD_REFLEXIVE_END
        skip_data = false;

        ADD_DEPENDENCY_ADJUST_SIZES(tag.bold);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.italic);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.condense);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.underline);

        // Begin writing our own character data
        std::int16_t table[65536];
        bool table_used[256] = {};
        std::fill(table, table + 65536, -1);

        // Get character data
        skip_data = true;
        std::vector<FontCharacter<LittleEndian>> characters(tag.characters.count);
        ADD_REFLEXIVE_START(tag.characters) {
            characters[i] = reflexive;
            std::size_t character = static_cast<std::uint16_t>(reflexive.character);
            if(character != 65536) {
                table[character] = static_cast<std::int16_t>(i);
                table_used[character / 256] = true;
            }
            characters[i].hardware_character_index = -1;
        } ADD_REFLEXIVE_END
        skip_data = false;

        // Write character table data
        tag.character_tables.count = 256;
        std::size_t character_tables = compiled.data.size();
        ADD_POINTER_FROM_INT32(tag.character_tables.pointer, character_tables);
        compiled.data.insert(compiled.data.end(), sizeof(FontCharacterTables<LittleEndian>) * 256, std::byte());

        // Make sure character tables point to correct locations
        for(std::size_t i = 0; i < 256; i++) {
            if(!table_used[i]) {
                continue;
            }
            std::size_t offset = character_tables + i * sizeof(FontCharacterTables<LittleEndian>);
            std::size_t offset_to = compiled.data.size();
            add_pointer(compiled, offset + 4, offset_to);
            compiled.data.insert(compiled.data.end(), reinterpret_cast<std::byte *>(table + 256 * i), reinterpret_cast<std::byte *>(table + 256 * (i + 1)));
            reinterpret_cast<FontCharacterTables<LittleEndian> *>(compiled.data.data() + offset)->character_table.count = 256;
        }

        // Write character table
        tag.characters.count = static_cast<Pointer>(characters.size());
        ADD_POINTER_FROM_INT32(tag.characters.pointer, compiled.data.size());
        compiled.data.insert(compiled.data.end(), reinterpret_cast<std::byte *>(characters.data()), reinterpret_cast<std::byte *>(characters.data() + characters.size()));

        // Add pixel data
        ADD_POINTER_FROM_INT32(tag.pixels.pointer, compiled.data.size());
        std::size_t pixels_size = tag.pixels.size;
        compiled.data.insert(compiled.data.end(), data, data + pixels_size);
        INCREMENT_DATA_PTR(pixels_size);

        PAD_32_BIT

        FINISH_COMPILE
    }
}
