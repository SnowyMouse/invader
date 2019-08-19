/*
 * Invader (c) 2018 Kavawuvi
 *
 * This program is free software under the GNU General Public License v3.0 or later. See LICENSE for more information.
 */

#ifndef INVADER__TAG__COMPILED_TAG_HPP
#define INVADER__TAG__COMPILED_TAG_HPP

#include <memory>
#include <string>
#include <vector>
#include <cstddef>

#include "../hek/class_int.hpp"
#include "../hek/data_type.hpp"
#include "../hek/map.hpp"
#include "../hek/endian.hpp"

namespace Invader {
    /**
     * Reference to a tag that is depended upon by another tag
     */
    class CompiledTagDependency {
    public:
        /**
         * Offset to the dependency in the tag data
         */
        std::size_t offset;

        /**
         * Path to the tag
         */
        std::string path;

        /**
         * Type of class
         */
        HEK::TagClassInt tag_class_int;

        /**
         * This is just a tag ID
         */
        bool tag_id_only;
    };

    /**
     * Reference to a particular memory address from within the tag
     */
    class CompiledTagPointer {
    public:
        /**
         * Offset to the pointer in the tag data
         */
        std::size_t offset;

        /**
         * Offset being pointed to in the tag data
         */
        std::size_t offset_pointed;
    };

    /**
     * The resulting data from compiling an HEK tag.
     */
    class CompiledTag {
    friend class BuildWorkload;

    public:
        /**
         * The path of the tag
         */
        std::string path;

        /**
         * The class of the tag
         */
        HEK::TagClassInt tag_class_int;

        /**
         * Raw tag data
         */
        std::vector<std::byte> data;

        /**
         * Raw asset data, such as sound, bitmap, or model data
         */
        std::vector<std::byte> asset_data;

        /**
         * All tags this tag depends upon
         */
        std::vector<CompiledTagDependency> dependencies;

        /**
         * All pointers within this tag
         */
        std::vector<CompiledTagPointer> pointers;

        /**
         * Return if the tag is a stub tag or not. Stub tags are meant to take the place of real tags.
         * @return true if a stub false if not
         */
        bool stub() const noexcept;

        #define INVALID_POINTER ~static_cast<std::size_t>(0)

        /**
         * Convert the pointer at the offset to an offset
         * @param  offset offset to look at
         * @return        offset where the pointer points to or INVALID_POINTER if none
         */
        std::size_t resolve_pointer(std::size_t offset) noexcept;

        /**
         * Convert the pointer at the offset to an offset
         * @param  offset offset to look at
         * @return        offset where the pointer points to or INVALID_POINTER if none
         */
        std::size_t resolve_pointer(HEK::LittleEndian<HEK::Pointer> *offset) noexcept;

        /**
         * Create a stub tag.
         * @param path      path of the tag
         * @param class_int class of the tag
         */
        CompiledTag(const std::string &path, HEK::TagClassInt class_int);

        /**
         * Compile a tag with the given data array
         * @param path path of the tag
         * @param data pointer to the data array
         * @param size length of the data array
         * @param type type of cache file being built
         * @throws     an exception if tag data is invalid
         */
        CompiledTag(const std::string &path, const std::byte *data, std::size_t size, HEK::CacheFileType type = HEK::CacheFileType::CACHE_FILE_MULTIPLAYER);

        /**
         * Compile a tag with the given data array, path, and class
         * @param path      path of the tag
         * @param class_int class of the tag
         * @param data      pointer to the data array
         * @param size      length of the data array
         * @param type      type of cache file being built
         * @throws          an exception if tag data is invalid or if the data does not match the given class
         */
        CompiledTag(const std::string &path, HEK::TagClassInt class_int, const std::byte *data, std::size_t size, HEK::CacheFileType type = HEK::CacheFileType::CACHE_FILE_MULTIPLAYER);
    private:
        /**
         * The tag is a stub tag, taking the place of a real tag.
         */
        bool p_stub = false;

        /**
         * The tag is indexed and is not included in the map file.
         */
        bool indexed = false;

        /**
         * If indexed, this is the index, unless this is a sound tag.
         */
        std::uint32_t index;

        /**
         * Size of data
         */
        std::size_t data_size = 0;

        /**
         * Size of asset data
         */
        std::size_t asset_data_size = 0;
    };

    /**
     * Convert an index to a tag ID.
     * @param  index this is the tag index
     * @return       this is the resulting tag ID
     */
    HEK::TagID tag_id_from_index(std::size_t index) noexcept;
}
#endif
