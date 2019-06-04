/*
 * Invader (c) 2018 Kavawuvi
 *
 * This program is free software under the GNU General Public License v3.0 or later. See LICENSE for more information.
 */

#include "../compile.hpp"

#include "bitmap.hpp"

namespace Invader::HEK {
    void compile_bitmap_tag(CompiledTag &compiled, const std::byte *data, std::size_t size) {
        BEGIN_COMPILE(Bitmap);

        ASSERT_SIZE(tag.compressed_color_plate_data.size);
        INCREMENT_DATA_PTR(tag.compressed_color_plate_data.size);

        auto *processed_data = data;
        ASSERT_SIZE(tag.processed_pixel_data.size);
        compiled.asset_data.insert(compiled.asset_data.begin(), processed_data, processed_data + tag.processed_pixel_data.size);
        INCREMENT_DATA_PTR(tag.processed_pixel_data.size);

        ADD_REFLEXIVE_START(tag.bitmap_group_sequence) {
            ADD_REFLEXIVE(reflexive.sprites);
        } ADD_REFLEXIVE_END

        ADD_REFLEXIVE_START(tag.bitmap_data) {
            reflexive.pointer = -1;
            auto flags = reflexive.flags.read();
            flags.external = 0;
            flags.make_it_actually_work = 1;
            reflexive.flags.write(flags);
        } ADD_REFLEXIVE_END

        FINISH_COMPILE;
    }
}
