/*
 * Invader (c) 2018 Kavawuvi
 *
 * This program is free software under the GNU General Public License v3.0 or later. See LICENSE for more information.
 */

#ifndef INVADER__MAP__TAG_HPP
#define INVADER__MAP__TAG_HPP

#include <vector>
#include <string>
#include <cstddef>

#include "../hek/data_type.hpp"

namespace Invader {
    class Map;

    /**
     * Class for handling tags in compiled maps
     */
    class Tag {
    friend class Map;
    public:
        /**
         * Get the path of the tag
         * @return path of the tag
         */
        const std::string &path() const noexcept;

        /**
         * Get the class of the tag
         * @return class of the tag
         */
        HEK::TagClassInt tag_class_int() const noexcept;

        /**
         * Get whether this is an indexed tag that is not in the map
         * @return true if this is an indexed tag that is not in the map
         */
        bool is_indexed() const noexcept;

        /**
         * Get a pointer to the tag data, optionally guaranteeing that a set amount of bytes is valid.
         * @param  pointer Halo pointer where the data is
         * @param  minimum minimum number of bytes to guarantee to be valid
         * @return         pointer to the data; nullptr if no data and minimum is not set
         * @throws         throw if invalid and minimum is set or if there are fewer bytes valid than minimum
         */
        std::byte *data(HEK::Pointer pointer, std::size_t minimum = 0);

        /**
         * Get a const pointer to the tag data, optionally guaranteeing that a set amount of bytes is valid.
         * @param  pointer Halo pointer where the data is
         * @param  minimum minimum number of bytes to guarantee to be valid
         * @return         const pointer to the data; nullptr if no data and minimum is not set
         * @throws         throw if invalid and minimum is set or if there are fewer bytes valid than minimum
         */
        const std::byte *data(HEK::Pointer pointer, std::size_t minimum = 0) const {
            return const_cast<Tag *>(this)->data(pointer, minimum);
        }

        /**
         * Get a reference to the struct at the given offset.
         * @param  pointer Halo pointer where the data is
         * @param  minimum minimum number of bytes to guarantee to be valid
         * @return a reference to the base struct
         * @throws throw if out of bounds
         */
        template <template<template<typename> typename> typename StructType>
        StructType<HEK::LittleEndian> &get_struct_at_pointer(HEK::Pointer pointer, std::size_t minimum = sizeof(StructType<HEK::LittleEndian>)) {
            return *reinterpret_cast<StructType<HEK::LittleEndian> *>(this->data(pointer, minimum));
        }

        /**
         * Get a reference to the base struct, throwing an exception if out of bounds.
         * @return a reference to the base struct
         */
        template <template<template<typename> typename> typename StructType>
        StructType<HEK::LittleEndian> &get_base_struct() {
            return get_struct_at_pointer<StructType>(this->base_struct_pointer);
        }

        /**
         * Resolve the reflexive.
         * @param  reflexive the reflexive
         * @return pointer to the first object or nullptr if the reflexive has 0 items
         */
        template <template<template<typename> typename> typename StructType>
        StructType<HEK::LittleEndian> *resolve_reflexive(const HEK::TagReflexive<HEK::LittleEndian, StructType> &reflexive) {
            if(reflexive.count == 0) {
                return nullptr;
            }
            return &get_struct_at_pointer<StructType>(reflexive.pointer.read(), sizeof(StructType<HEK::LittleEndian>) * reflexive.count);
        }

        /**
         * Resolve the reflexive to a const pointer.
         * @param  reflexive the reflexive
         * @return pointer to the first object or nullptr if the reflexive has 0 items
         */
        template <template<template<typename> typename> typename StructType>
        StructType<HEK::LittleEndian> *resolve_reflexive(const HEK::TagReflexive<HEK::LittleEndian, StructType> &reflexive) const {
            return const_cast<Tag *>(this)->resolve_reflexive(reflexive);
        }

        /**
         * Resolve the reflexive at the given offset and return a referenced to the desired struct
         * @param reflexive the reflexive
         * @param index     the index
         */
        template <template<template<typename> typename> typename StructType>
        StructType<HEK::LittleEndian> &get_struct_from_reflexive(const HEK::TagReflexive<HEK::LittleEndian, StructType> &reflexive, std::size_t index) {
            if(reflexive.count <= index) {
                throw OutOfBoundsException();
            }
            return this->resolve_reflexive(reflexive)[index];
        }

        /**
         * Resolve the reflexive at the given offset and return a const referenced to the desired struct
         * @param reflexive the reflexive
         * @param index     the index
         */
        template <template<template<typename> typename> typename StructType>
        StructType<HEK::LittleEndian> &get_struct_from_reflexive(const HEK::TagReflexive<HEK::LittleEndian, StructType> &reflexive, std::size_t index) const {
            return const_cast<Tag *>(this)->get_struct_from_reflexive(reflexive, index);
        }

    private:
        /** Map reference */
        Map &p_map;

        /** Path of tag */
        std::string p_path;

        /** Class of tag */
        HEK::TagClassInt p_tag_class_int;

        /** This is indexed and not in the map? */
        bool indexed = false;

        /** Base struct pointer */
        HEK::Pointer base_struct_pointer;

        /** Base struct offset */
        std::size_t base_struct_offset = 0;

        /** Tag data size */
        std::size_t tag_data_size = 0;

        /** Initialize the tag */
        Tag(Map &map);
    };
}
#endif
