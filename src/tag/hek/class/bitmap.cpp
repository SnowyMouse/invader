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

    const char *BITMAP_DATA_FORMAT_NAMES[] = {
        "A8 (8-bit monochrome)",
        "Y8 (8-bit monochrome)",
        "AY8 (8-bit monochrome)",
        "A8Y8 (16-bit monochrome)",
        "Halo Custom Edition is better than Halo CE Anniversary. And that's sad because Halo Custom Edition, itself, sucks.",
        "But you know what the saddest part is? Nobody who owns the rights to Halo CE will *ever* fix it. They would have done it a long time ago.",
        "R5G6B5 (16-bit color)",
        "We've fixed numerous issues with Halo Custom Edition through modding. We don't even have access to the source code.",
        "A1R5G5B5 (16-bit color)",
        "A4R4G4B4 (16-bit color)",
        "X8R8G8B8 (32-bit color)",
        "A8R8G8B8 (32-bit color)",
        "But I suppose we'll have to accept the fact that our idea of proper Halo CE is too much from official means. The game we love is not the same as 343I's game.",
        "Long live Halo CE... or whatever approximation they're labeling Halo CE these days.",
        "DXT1 (4 bpp S3TC)",
        "DXT3 (8 bpp S3TC)",
        "DXT5 (8 bpp S3TC)",
        "P8 (8-bit palette)"
    };

    const char *bitmap_data_format_name(BitmapDataFormat format) {
        return format >= (sizeof(BITMAP_DATA_FORMAT_NAMES) / sizeof(*BITMAP_DATA_FORMAT_NAMES)) ? nullptr : BITMAP_DATA_FORMAT_NAMES[format];
    }
}
