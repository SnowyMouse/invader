/*
 * Invader (c) 2018 Kavawuvi
 *
 * This program is free software under the GNU General Public License v3.0 or later. See LICENSE for more information.
 */

#pragma once

#include <vector>
#include <string>
#include "../hek/map.hpp"
#include "../resource/resource_map.hpp"
#include "../tag/compiled_tag.hpp"

namespace Invader {
    /**
     * This class is used for building cache files through the compile_map static function.
     */
    class BuildWorkload {
    public:
        /**
         * Compile a map
         * @param scenario          scenario tag to use
         * @param tags_directories  tags directories to use
         * @param maps_directory    maps directory to use
         * @param with_index        tag index to use
         * @param no_indexed_tags   do not use cached tags
         * @param verbose           output non-error messages to console
         */
        static std::vector<std::byte> compile_map(
            std::string scenario,
            std::vector<std::string> tags_directories,
            std::string maps_directory,
            const std::vector<std::tuple<HEK::TagClassInt, std::string>> &with_index = std::vector<std::tuple<Invader::HEK::TagClassInt, std::string>>(),
            bool no_indexed_tags = false,
            bool verbose = false
        );

    private:
        /**
         * Scenario tag to use
         */
        std::string scenario;

        /**
         * Index of the scenario tag to use
         */
        std::size_t scenario_index;

        /**
         * Array of tag directories to use
         */
        std::vector<std::string> tags_directories;

        /**
         * Cache file type
         */
        HEK::CacheFileType cache_file_type = HEK::CacheFileType::CACHE_FILE_MULTIPLAYER;

        /**
         * Maps directory to use
         */
        std::string maps_directory;

        /**
         * Array of compiled tags
         */
        std::vector<std::unique_ptr<CompiledTag>> compiled_tags;

        /**
         * Number of tags present
         */
        std::size_t tag_count;

        /**
         * Output messages
         */
        bool verbose;

        /**
         * Tag data address to use
         */
        std::uint32_t tag_data_address = 0x40440000;

        /**
         * Bitmaps.map assets
         */
        std::vector<Resource> bitmaps;

        /**
         * Sounds.map assets
         */
        std::vector<Resource> sounds;

        /**
         * Sounds.map assets
         */
        std::vector<Resource> loc;

        /**
         * Compile and load all required tags
         */
        void load_required_tags();

        /**
         * Compile a tag recursively if tag is not present
         * @param  path          path of tag
         * @param  tag_class_int class of tag
         * @return               tag index of compiled tag
         */
        std::size_t compile_tag_recursively(const std::string &path, HEK::TagClassInt tag_class_int);

        /**
         * Build a cache file
         * @return  cache file data
         */
        std::vector<std::byte> build_cache_file();

        /**
         * Index tags that can be indexed
         */
        void index_tags() noexcept;

        /**
         * Get the scenario name
         * @return the scenario name
         */
        std::string get_scenario_name();

        /**
         * Populate the tag array
         * @param tag_data the tag data
         */
        void populate_tag_array(std::vector<std::byte> &tag_data);

        /**
         * Append all tag data to the file
         * @param tag_data tag data to append
         * @param file     file to append tag data to
         */
        void add_tag_data(std::vector<std::byte> &tag_data, std::vector<std::byte> &file);

        /**
         * Append tag_data with data from tag
         * @param  tag_data  tag data to append
         * @param  tag_array pointer to tag array
         * @param  tag       tag index to add
         * @return           offset of tag data
         */
        std::size_t add_tag_data_for_tag(std::vector<std::byte> &tag_data, void *tag_array, std::size_t tag);

        /**
         * Get all bitmap and sound data
         * @param file     file to append data to
         * @param tag_data tag data to use
         */
        void add_bitmap_and_sound_data(std::vector<std::byte> &file, std::vector<std::byte> &tag_data);

        /**
         * Get all model data
         * @param vertices vertices to add
         * @param indices  triangles to add
         * @param tag_data tag data to use
         */
        void add_model_tag_data(std::vector<std::byte> &vertices, std::vector<std::byte> &indices, std::vector<std::byte> &tag_data);
    };
}
