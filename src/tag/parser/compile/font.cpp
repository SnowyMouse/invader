// SPDX-License-Identifier: GPL-3.0-only

#include <invader/tag/parser/parser.hpp>
#include <invader/build/build_workload.hpp>

namespace Invader::Parser {
    void Font::pre_compile(BuildWorkload &workload, std::size_t tag_index, std::size_t, std::size_t) {
        // Clear the character tables
        this->character_tables.clear();

        // Resize the character table table to 256
        static constexpr std::uint16_t CHARACTER_TABLE_COUNT = 256;
        static constexpr std::uint16_t CHARACTER_TABLE_SIZE = 256;
        this->character_tables.resize(CHARACTER_TABLE_COUNT);

        // Figure out how many characters we can address
        const std::size_t CHARACTER_SIZE = this->characters.size();
        const std::uint16_t CHARACTER_COUNT = CHARACTER_SIZE > (UINT16_MAX - 1) ? (UINT16_MAX - 1) : CHARACTER_SIZE;
        if(CHARACTER_SIZE >= UINT16_MAX) {
            REPORT_ERROR_PRINTF(workload, ERROR_TYPE_WARNING, tag_index, "Character count exceeds %zu, so %zu characters will be unused", static_cast<std::size_t>(UINT16_MAX - 1), CHARACTER_SIZE - UINT16_MAX)
        }

        // Go through it all
        for(std::uint16_t i = 0; i < CHARACTER_COUNT; i++) {
            auto character_index = this->characters[i].character;
            std::size_t table = (character_index & 0xFF00) >> 8;
            std::size_t index = (character_index & 0xFF);
            auto &ctable = this->character_tables[table];
            if(ctable.character_table.size() == 0) {
                ctable.character_table.resize(CHARACTER_TABLE_SIZE);

                for(std::uint16_t c = 0; c < CHARACTER_TABLE_SIZE; c++) {
                    // Now we need to look for the character; set to null for now
                    ctable.character_table[c].character_index = NULL_INDEX;
                }
            }
            this->character_tables[table].character_table[index].character_index = i;
        }
    }
}
