// SPDX-License-Identifier: GPL-3.0-only

#ifndef INVADER__MAP__TAG_HPP
#define INVADER__MAP__TAG_HPP

#include <vector>
#include <string>
#include <optional>
#include <cstddef>

#include "../hek/data_type.hpp"

namespace Invader {
    class Map;
    namespace HEK {
        struct CacheFileTagDataTag;
    }

    /**
     * Class for handling tags in compiled maps
     */
    class Tag {
    friend class Map;
    public:
        /**
         * Get the map
         * @return the map
         */
        Map &get_map() noexcept {
            return this->map;
        }

        /**
         * Get the map
         * @return the map
         */
        const Map &get_map() const noexcept {
            return const_cast<Tag *>(this)->get_map();
        }

        /**
         * Get the path of the tag
         * @return path of the tag
         */
        const std::string &get_path() const noexcept {
            return this->path;
        }

        /**
         * Get the class of the tag
         * @return class of the tag
         */
        TagFourCC get_tag_fourcc() const noexcept {
            return this->tag_fourcc;
        }

        /**
         * Get whether this is an indexed tag that is not in the map
         * @return true if this is an indexed tag that is not in the map
         */
        bool is_indexed() const noexcept {
            return this->indexed;
        }
        
        /**
         * Get the resource index. This only applies to indexed Custom Edition maps, and it does not work for sound tags unless resource maps are loaded.
         * @return resource index if applicable
         */
        std::optional<std::size_t> get_resource_index() const noexcept {
            return this->resource_index;
        }

        /**
         * Get the tag index
         * @return tag index
         */
        std::size_t get_tag_index() const noexcept {
            return this->tag_index;
        }

        /**
         * Get whether or not the tag data is available
         * @return true if the tag data is available
         */
        bool data_is_available() const noexcept;
        
        /**
         * Get whether or not the tag is actually a stub (not a real tag)
         * @return true if the tag is a stub
         */
        bool is_stub() const noexcept;

        /**
         * Get the tag data index
         * @return tag data index
         */
        HEK::CacheFileTagDataTag &get_tag_data_index() noexcept;

        /**
         * Get the tag data index
         * @return tag data index
         */
        const HEK::CacheFileTagDataTag &get_tag_data_index() const noexcept;

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
         * Get a pointer to the tag data, optionally guaranteeing that a set amount of bytes is valid.
         * @param  pointer Halo pointer where the data is
         * @param  minimum minimum number of bytes to guarantee to be valid
         * @return         pointer to the data; nullptr if no data and minimum is not set
         * @throws         throw if invalid and minimum is set or if there are fewer bytes valid than minimum
         */
        std::byte *data(HEK::Pointer64 pointer, std::size_t minimum = 0);

        /**
         * Get a const pointer to the tag data, optionally guaranteeing that a set amount of bytes is valid.
         * @param  pointer Halo pointer where the data is
         * @param  minimum minimum number of bytes to guarantee to be valid
         * @return         const pointer to the data; nullptr if no data and minimum is not set
         * @throws         throw if invalid and minimum is set or if there are fewer bytes valid than minimum
         */
        const std::byte *data(HEK::Pointer64 pointer, std::size_t minimum = 0) const {
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
         * Get a reference to the struct at the given offset.
         * @param  pointer Halo pointer where the data is
         * @param  minimum minimum number of bytes to guarantee to be valid
         * @return a reference to the base struct
         * @throws throw if out of bounds
         */
        template <template<template<typename> typename> typename StructType>
        StructType<HEK::LittleEndian> &get_struct_at_pointer(HEK::Pointer64 pointer, std::size_t minimum = sizeof(StructType<HEK::LittleEndian>)) {
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
         * Get a reference to the struct at the given offset.
         * @param  pointer Halo pointer where the data is
         * @param  minimum minimum number of bytes to guarantee to be valid
         * @return a reference to the base struct
         * @throws throw if out of bounds
         */
        template <template<template<typename> typename> typename StructType>
        const StructType<HEK::LittleEndian> &get_struct_at_pointer(HEK::Pointer pointer, std::size_t minimum = sizeof(StructType<HEK::LittleEndian>)) const {
            return const_cast<Tag *>(this)->get_struct_at_pointer<StructType>(pointer, minimum);
        }

        /**
         * Get a reference to the struct at the given offset.
         * @param  pointer Halo pointer where the data is
         * @param  minimum minimum number of bytes to guarantee to be valid
         * @return a reference to the base struct
         * @throws throw if out of bounds
         */
        template <template<template<typename> typename> typename StructType>
        const StructType<HEK::LittleEndian> &get_struct_at_pointer(HEK::Pointer64 pointer, std::size_t minimum = sizeof(StructType<HEK::LittleEndian>)) const {
            return const_cast<Tag *>(this)->get_struct_at_pointer<StructType>(pointer, minimum);
        }

        /**
         * Get a reference to the base struct, throwing an exception if out of bounds.
         * @return a reference to the base struct
         */
        template <template<template<typename> typename> typename StructType>
        const StructType<HEK::LittleEndian> &get_base_struct() const {
            return const_cast<Tag *>(this)->get_base_struct<StructType>();
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
        const StructType<HEK::LittleEndian> *resolve_reflexive(const HEK::TagReflexive<HEK::LittleEndian, StructType> &reflexive) const {
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
         * Resolve the reflexive at the given offset and return a const reference to the desired struct
         * @param reflexive the reflexive
         * @param index     the index
         */
        template <template<template<typename> typename> typename StructType>
        const StructType<HEK::LittleEndian> &get_struct_from_reflexive(const HEK::TagReflexive<HEK::LittleEndian, StructType> &reflexive, std::size_t index) const {
            return const_cast<Tag *>(this)->get_struct_from_reflexive(reflexive, index);
        }

    private:
        /** Map reference */
        Map &map;

        /** Path of tag */
        std::string path;

        /** Class of tag */
        TagFourCC tag_fourcc;

        /** This is indexed and not in the map? */
        bool indexed = false;

        /** Base struct pointer */
        HEK::Pointer base_struct_pointer;

        /** Base struct offset */
        std::size_t base_struct_offset = 0;

        /** Tag data size */
        std::size_t tag_data_size = 0;

        /** Tag data index offset */
        std::size_t tag_data_index_offset = 0;

        /** Tag index in the cache file */
        std::size_t tag_index = 0;

        /** Initialize the tag */
        Tag(Map &map);
        
        /** Resource index */
        std::optional<std::size_t> resource_index;
    };
}
#endif
