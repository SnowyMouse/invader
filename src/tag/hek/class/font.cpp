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
        ADD_REFLEXIVE_START(tag.character_tables) {
            ADD_REFLEXIVE(reflexive.character_table)
        } ADD_REFLEXIVE_END
        ADD_DEPENDENCY_ADJUST_SIZES(tag.bold);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.italic);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.condense);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.underline);
        ADD_REFLEXIVE(tag.characters);
        ADD_POINTER_FROM_INT32(tag.pixels.pointer, compiled.data.size());
        std::size_t pixels_size = tag.pixels.size;
        compiled.data.insert(compiled.data.end(), data, data + pixels_size);
        INCREMENT_DATA_PTR(pixels_size);
        FINISH_COMPILE
    }
}
