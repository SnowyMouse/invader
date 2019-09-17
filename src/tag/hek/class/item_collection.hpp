/*
 * Invader (c) 2018 Kavawuvi
 *
 * This program is free software under the GNU General Public License v3.0. See LICENSE for more information.
 */

#ifndef INVADER__TAG__HEK__CLASS__ITEM_COLLECTION_HPP
#define INVADER__TAG__HEK__CLASS__ITEM_COLLECTION_HPP

#include "../../compiled_tag.hpp"
#include "../../../hek/data_type.hpp"
#include "../header.hpp"

namespace Invader::HEK {
    ENDIAN_TEMPLATE(EndianType) struct ItemCollectionPermutation {
        PAD(0x20);
        EndianType<float> weight;
        TagDependency<EndianType> item; // item
        PAD(0x20);

        ENDIAN_TEMPLATE(NewType) operator ItemCollectionPermutation<NewType>() const noexcept {
            ItemCollectionPermutation<NewType> copy = {};
            COPY_THIS(weight);
            COPY_THIS(item);
            return copy;
        }
    };
    static_assert(sizeof(ItemCollectionPermutation<BigEndian>) == 0x54);

    ENDIAN_TEMPLATE(EndianType) struct ItemCollection {
        TagReflexive<EndianType, ItemCollectionPermutation> permutations;
        EndianType<std::int16_t> spawn_time; // seconds
        PAD(0x2);
        PAD(0x4C);

        ENDIAN_TEMPLATE(NewType) operator ItemCollection<NewType>() const noexcept {
            ItemCollection<NewType> copy = {};
            COPY_THIS(permutations);
            COPY_THIS(spawn_time);
            return copy;
        }
    };
    static_assert(sizeof(ItemCollection<BigEndian>) == 0x5C);

    void compile_item_collection_tag(CompiledTag &compiled, const std::byte *data, std::size_t size);
}
#endif
