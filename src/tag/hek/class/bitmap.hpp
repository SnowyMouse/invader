/*
 * Invader (c) 2018 Kavawuvi
 *
 * This program is free software under the GNU General Public License v3.0 or later. See LICENSE for more information.
 */

#ifndef INVADER__TAG__HEK__CLASS__BITMAP_HPP
#define INVADER__TAG__HEK__CLASS__BITMAP_HPP

#include "../../compiled_tag.hpp"
#include "../../../hek/data_type.hpp"
#include "../header.hpp"

namespace Invader::HEK {
    enum BitmapDataType : TagEnum {
        BITMAP_DATA_TYPE_2D_TEXTURE,
        BITMAP_DATA_TYPE_3D_TEXTURE,
        BITMAP_DATA_TYPE_CUBE_MAP,
        BITMAP_DATA_TYPE_WHITE
    };

    enum BitmapDataFormat : TagEnum {
        /** 8-bit alpha */
        BITMAP_FORMAT_A8,

        /** 8-bit luminosity */
        BITMAP_FORMAT_Y8,

        /** 8-bit alpha & luminosity (luminosity = alpha) */
        BITMAP_FORMAT_AY8,

        /** 8-bit alpha, 8-bit luminosity */
        BITMAP_FORMAT_A8Y8,

        /** Unused */
        BITMAP_FORMAT_UNUSED1,

        /** Unused */
        BITMAP_FORMAT_UNUSED2,

        /** 5-bit red, 6-bit green, 5-bit blue */
        BITMAP_FORMAT_R5G6B5,

        /** Unused */
        BITMAP_FORMAT_UNUSED3,

        /** 16-bit ARGB, 1-bit alpha */
        BITMAP_FORMAT_A1R5G5B5,

        /** 16-bit ARGB */
        BITMAP_FORMAT_A4R4G4B4,

        /** 32-bit ARGB */
        BITMAP_FORMAT_X8R8G8B8,

        /** 32-bit ARGB */
        BITMAP_FORMAT_A8R8G8B8,

        /** Unused */
        BITMAP_FORMAT_UNUSED4,

        /** Unused */
        BITMAP_FORMAT_UNUSED5,

        /** DDS, DXT1 compression */
        BITMAP_FORMAT_DXT1,

        /** DDS, DXT3 compression */
        BITMAP_FORMAT_DXT3,

        /** DDS, DXT5 compression */
        BITMAP_FORMAT_DXT5,

        /** 8-bit palette bump */
        BITMAP_FORMAT_P8_BUMP
    };

    const char *bitmap_data_format_name(BitmapDataFormat format);

    enum BitmapType : TagEnum {
        BITMAP_TYPE_2D_TEXTURES,
        BITMAP_TYPE_3D_TEXTURES,
        BITMAP_TYPE_CUBE_MAPS,
        BITMAP_TYPE_SPRITES,
        BITMAP_TYPE_INTERFACE_BITMAPS
    };

    enum BitmapFormat : TagEnum {
        BITMAP_FORMAT_COMPRESSED_WITH_COLOR_KEY_TRANSPARENCY,
        BITMAP_FORMAT_COMPRESSED_WITH_EXPLICIT_ALPHA,
        BITMAP_FORMAT_COMPRESSED_WITH_INTERPOLATED_ALPHA,
        BITMAP_FORMAT_16_BIT_COLOR,
        BITMAP_FORMAT_32_BIT_COLOR,
        BITMAP_FORMAT_MONOCHROME
    };

    enum BitmapUsage : TagEnum {
        BITMAP_USAGE_ALPHA_BLEND,
        BITMAP_USAGE_DEFAULT,
        BITMAP_USAGE_HEIGHT_MAP,
        BITMAP_USAGE_DETAIL_MAP,
        BITMAP_USAGE_LIGHT_MAP,
        BITMAP_USAGE_VECTOR_MAP
    };

    enum BitmapSpriteBudgetSize : TagEnum {
        BITMAP_SPRITE_BUDGET_SIZE_32X32,
        BITMAP_SPRITE_BUDGET_SIZE_64X64,
        BITMAP_SPRITE_BUDGET_SIZE_128X128,
        BITMAP_SPRITE_BUDGET_SIZE_256X256,
        BITMAP_SPRITE_BUDGET_SIZE_512X512
    };

    enum BitmapSpriteUsage : TagEnum {
        BITMAP_SPRITE_USAGE_BLEND_ADD_SUBTRACT_MAX,
        BITMAP_SPRITE_USAGE_MULTIPLY_MIN,
        BITMAP_SPRITE_USAGE_DOUBLE_MULTIPLY
    };

    ENDIAN_TEMPLATE(EndianType) struct BitmapGroupSprite {
        EndianType<std::int16_t> bitmap_index;
        PAD(0x2);
        PAD(0x4);
        EndianType<float> left;
        EndianType<float> right;
        EndianType<float> top;
        EndianType<float> bottom;
        Point2D<EndianType> registration_point;

        ENDIAN_TEMPLATE(NewType) operator BitmapGroupSprite<NewType>() const noexcept {
            BitmapGroupSprite<NewType> copy = {};

            COPY_THIS(bitmap_index);
            COPY_THIS(left);
            COPY_THIS(right);
            COPY_THIS(top);
            COPY_THIS(bottom);
            COPY_THIS(registration_point);

            return copy;
        }
    };
    static_assert(sizeof(BitmapGroupSprite<BigEndian>) == 0x20);

    ENDIAN_TEMPLATE(EndianType) struct BitmapGroupSequence {
        TagString name;
        EndianType<std::int16_t> first_bitmap_index;
        EndianType<std::int16_t> bitmap_count;
        PAD(0x10);
        TagReflexive<EndianType, BitmapGroupSprite> sprites;

        ENDIAN_TEMPLATE(NewType) operator BitmapGroupSequence<NewType>() const noexcept {
            BitmapGroupSequence<NewType> copy = {};

            COPY_THIS(name);
            COPY_THIS(first_bitmap_index);
            COPY_THIS(bitmap_count);
            COPY_THIS(sprites);

            return copy;
        }
    };
    static_assert(sizeof(BitmapGroupSequence<BigEndian>) == 0x40);

    struct BitmapDataFlags {
        std::uint16_t power_of_two_dimensions : 1;
        std::uint16_t compressed : 1;
        std::uint16_t palettized : 1;
        std::uint16_t swizzled : 1;
        std::uint16_t linear : 1;
        std::uint16_t v16u16 : 1;
        std::uint16_t unused : 1;
        std::uint16_t make_it_actually_work : 1;
        std::uint16_t external : 1;
    };
    static_assert(sizeof(BitmapDataFlags) == sizeof(std::uint16_t));

    ENDIAN_TEMPLATE(EndianType) struct BitmapData {
        EndianType<TagClassInt> bitmap_class;
        EndianType<std::uint16_t> width;
        EndianType<std::uint16_t> height;
        EndianType<std::uint16_t> depth;
        EndianType<BitmapDataType> type;
        EndianType<BitmapDataFormat> format;
        EndianType<BitmapDataFlags> flags;
        Point2DInt<EndianType> registration_point;
        EndianType<std::uint16_t> mipmap_count;
        PAD(0x2);
        EndianType<std::uint32_t> pixels_offset;
        EndianType<std::uint32_t> pixels_count;
        EndianType<TagID> bitmap_tag_id;
        EndianType<Pointer> pointer;
        PAD(0x4);
        PAD(0x4);

        ENDIAN_TEMPLATE(NewType) operator BitmapData<NewType>() const noexcept {
            BitmapData<NewType> copy = {};

            COPY_THIS(bitmap_class);
            COPY_THIS(width);
            COPY_THIS(height);
            COPY_THIS(depth);
            COPY_THIS(type);
            COPY_THIS(format);
            COPY_THIS(flags);
            COPY_THIS(registration_point);
            COPY_THIS(mipmap_count);
            COPY_THIS(pixels_offset);
            COPY_THIS(pixels_count);
            COPY_THIS(bitmap_tag_id);

            return copy;
        }
    };
    static_assert(sizeof(BitmapData<BigEndian>) == 0x30);

    struct BitmapFlags {
        std::uint16_t enable_diffusion_dithering : 1;
        std::uint16_t disable_height_map_compression : 1;
        std::uint16_t uniform_sprite_sequences : 1;
        std::uint16_t filthy_sprite_bug_fix : 1;
        std::uint16_t reserved : 1;
        std::uint16_t reserved_1 : 1;
        std::uint16_t reserved_2 : 1;
        std::uint16_t reserved_3 : 1;
        std::uint16_t reserved_4 : 1;
        std::uint16_t reserved_5 : 1;
        std::uint16_t reserved_6 : 1;
        std::uint16_t reserved_7 : 1;
        std::uint16_t reserved_8 : 1;
        std::uint16_t never_share_resources : 1;
    };
    static_assert(sizeof(BitmapFlags) == sizeof(std::uint16_t));

    ENDIAN_TEMPLATE(EndianType) struct Bitmap {
        // 0x0
        EndianType<BitmapType> type;
        EndianType<BitmapFormat> format;
        EndianType<BitmapUsage> usage;
        EndianType<BitmapFlags> flags;
        EndianType<Fraction> detail_fade_factor;
        EndianType<Fraction> sharpen_amount;

        // 0x10
        EndianType<Fraction> bump_height;
        EndianType<BitmapSpriteBudgetSize> sprite_budget_size;
        EndianType<std::int16_t> sprite_budget_count;
        EndianType<std::int16_t> color_plate_width;
        EndianType<std::int16_t> color_plate_height;

        // 0x1C
        TagDataOffset<EndianType> compressed_color_plate_data;

        // 0x30
        TagDataOffset<EndianType> processed_pixel_data;

        // 0x44
        EndianType<float> blur_filter_size;
        EndianType<float> alpha_bias;
        EndianType<std::int16_t> mipmap_count;
        EndianType<BitmapSpriteUsage> sprite_usage;

        // 0x50
        EndianType<std::uint16_t> sprite_spacing;
        PAD(0x2);
        TagReflexive<EndianType, BitmapGroupSequence> bitmap_group_sequence;
        TagReflexive<EndianType, BitmapData> bitmap_data;

        ENDIAN_TEMPLATE(NewType) operator Bitmap<NewType>() const noexcept {
            Bitmap<NewType> copy = {};

            COPY_THIS(type);
            COPY_THIS(format);
            COPY_THIS(usage);
            COPY_THIS(flags);
            COPY_THIS(detail_fade_factor);
            COPY_THIS(sharpen_amount);
            COPY_THIS(bump_height);
            COPY_THIS(sprite_budget_size);
            COPY_THIS(sprite_budget_count);
            COPY_THIS(color_plate_width);
            COPY_THIS(color_plate_height);
            COPY_THIS(compressed_color_plate_data);
            COPY_THIS(processed_pixel_data);
            COPY_THIS(blur_filter_size);
            COPY_THIS(alpha_bias);
            COPY_THIS(mipmap_count);
            COPY_THIS(sprite_usage);
            COPY_THIS(sprite_spacing);
            COPY_THIS(bitmap_group_sequence);
            COPY_THIS(bitmap_data);

            return copy;
        }
    };
    static_assert(sizeof(Bitmap<BigEndian>) == 0x6C);

    void compile_bitmap_tag(CompiledTag &compiled, const std::byte *data, std::size_t size);
}
#endif
