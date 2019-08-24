/*
 * Invader (c) 2018 Kavawuvi
 *
 * This program is free software under the GNU General Public License v3.0 or later. See LICENSE for more information.
 */

#ifndef INVADER__BUILD__BUILD_WORKLOAD_HPP
#define INVADER__BUILD__BUILD_WORKLOAD_HPP

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
         * @param always_index_tags always use cached tags
         * @param verbose           output non-error messages to console
         * @param forge_crc         forge the CRC32 of the map
         */
        static std::vector<std::byte> compile_map(
            const char *scenario,
            std::vector<std::string> tags_directories,
            std::string maps_directory,
            const std::vector<std::tuple<HEK::TagClassInt, std::string>> &with_index = std::vector<std::tuple<Invader::HEK::TagClassInt, std::string>>(),
            bool no_indexed_tags = false,
            bool always_index_tags = false,
            bool verbose = false,
            const std::uint32_t *forge_crc = nullptr
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
         * Always index tags when possible
         */
        bool always_index_tags;

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
        std::uint32_t tag_data_address = HEK::CACHE_FILE_PC_BASE_MEMORY_ADDRESS;

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
         * Tag buffer for compiling tags
         */
        std::vector<std::byte> tag_buffer;

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
        std::size_t compile_tag_recursively(const char *path, HEK::TagClassInt tag_class_int);

        /**
         * Build a cache file
         * @param   forge_crc  forge the CRC32 of the map
         * @return  cache file data
         */
        std::vector<std::byte> build_cache_file(const std::uint32_t *forge_crc = nullptr);

        /**
         * Index tags that can be indexed
         * @return  tag data removed
         */
        std::size_t index_tags() noexcept;

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

        /**
         * Fix the encounters in the scenario tag
         */
        void fix_scenario_tag_encounters();

        /**
         * Fix the command lists in the scenario tag
         */
        void fix_scenario_tag_command_lists();

        /**
         * Check if the point is in the bsp
         * @param bsp   bsp index to check
         * @param point point to check
         * @return      true if point is inside the bsp
         */
        bool point_in_bsp(std::uint32_t bsp, const HEK::Point3D<HEK::LittleEndian> &point);

        /**
         * Get the leaf index for the point in BSP
         * @param bsp   bsp index to check
         * @param point point to check
         * @return      leaf index or null if not in BSP
         */
        HEK::FlaggedInt<std::uint32_t> leaf_for_point_in_bsp(std::uint32_t bsp, const HEK::Point3D<HEK::LittleEndian> &point);

        bool intersect_in_bsp(const HEK::Point3D<HEK::LittleEndian> &point_a, const HEK::Point3D<HEK::LittleEndian> &point_b, std::uint32_t bsp, HEK::Point3D<HEK::LittleEndian> &intersection_point, std::uint32_t &surface_index, std::uint32_t &leaf_index);

        /**
         * Get the tag index of the BSP
         * @param  bsp bsp index
         * @return     index of the BSP
         */
        std::size_t get_bsp_tag_index(std::uint32_t bsp);
    };
}
#endif
