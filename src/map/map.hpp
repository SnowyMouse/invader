/*
 * Invader (c) 2018 Kavawuvi
 *
 * This program is free software under the GNU General Public License v3.0 or later. See LICENSE for more information.
 */

#pragma once

#include <string>
#include <vector>
#include <cstddef>
#include <memory>

#include "../resource/resource_map.hpp"
#include "tag.hpp"

namespace Invader {
    /**
     * Class for handling compiled maps
     */
    class Map {
    friend class Tag;
    public:
        /**
         * Get the name of the map
         * @return reference to the name of the map
         */
        const std::string &name() const;

        /**
         * Get the build of the map
         * @return reference to the build of the map
         */
        const std::string &build() const;

        /**
         * Get the tag count
         * @return the tag count
         */
        std::size_t tag_count() const noexcept;

        /**
         * Get a reference to a tag with its tag index
         * @param  tag_index tag index
         * @return           reference to the tag
         * @throws           throw if tag index is out of bounds
         */
        Tag &get_tag(std::size_t tag_index);

        /**
         * Get a const reference to a tag with its tag index
         * @param  tag_index tag index
         * @return           reference to the tag
         * @throws           throw if tag index is out of bounds
         */
        const Tag &get_tag(std::size_t tag_index) const;

        /**
         * Get a reference to a tag with its tag index
         * @param  tag_index tag index
         * @return           reference to the tag
         * @throws           throw if tag index is out of bounds
         */
        Tag &get_tag(const HEK::TagDependency<HEK::LittleEndian> &tag_index);

        /**
         * Get a const reference to a tag with its tag index
         * @param  tag_index tag index
         * @return           reference to the tag
         * @throws           throw if tag index is out of bounds
         */
        const Tag &get_tag(const HEK::TagDependency<HEK::LittleEndian> &tag_index) const;

        /**
         * Get a pointer to a tag with a tag dependency
         * @param  tag_id tag index
         * @return        tag if found or nullptr if not
         * @throws        throw if tag index is out of bounds
         */
        Tag *get_tag_nullable(const HEK::TagDependency<HEK::LittleEndian> &tag_id);

        /**
         * Get a const pointer to a tag with a tag dependency
         * @param  tag_id tag index
         * @return        tag if found or nullptr if not
         * @throws        throw if tag index is out of bounds
         */
        const Tag *get_tag_nullable(const HEK::TagDependency<HEK::LittleEndian> &tag_id) const;

        /**
         * Get a reference to the scenario tag
         * @return           reference to the tag
         */
        Tag &get_scenario_tag() noexcept;

        /**
         * Get a const reference to the scenario tag
         * @return           reference to the tag
         */
        const Tag &get_scenario_tag() const noexcept;

        /**
         * Constructor using a pointer to the map data and the size of the map, optionally including pointers and sizes for bitmaps.map, loc.map, and sounds.map
         * @param data              pointer to map data
         * @param data_size         length of map data
         * @param bitmaps_data      pointer to bitmaps data
         * @param bitmaps_data_size length of bitmaps data
         * @param loc_data          pointer to loc data
         * @param loc_data_size     length of loc data
         * @param sounds_data       pointer to sounds data
         * @param sounds_data_size  length of sounds data
         */
        Map(const std::byte *data, std::size_t data_size,
            const std::byte *bitmaps_data = nullptr, std::size_t bitmaps_data_size = 0,
            const std::byte *loc_data = nullptr, std::size_t loc_data_size = 0,
            const std::byte *sounds_data = nullptr, std::size_t sounds_data_size = 0);

        /**
         * Constructor using a vector for the map data, optionally including vectors for bitmaps, loc, and sounds
         * @param data         vector containing all map data
         * @param bitmaps_data vector containing all bitmaps data
         * @param loc_data     vector containing all loc data
         * @param sounds_data  vector containing all sounds data
         */
        Map(const std::vector<std::byte> &data,
            const std::vector<std::byte> &bitmaps_data = std::vector<std::byte>(),
            const std::vector<std::byte> &loc_data = std::vector<std::byte>(),
            const std::vector<std::byte> &sounds_data = std::vector<std::byte>());

        /**
         * Copy constructor for Map
         */
        Map(const Map &copy);
    private:
        /** Name of map */
        std::string p_name;

        /** Build of map */
        std::string p_build;

        /** Tag array */
        std::vector<std::unique_ptr<Tag>> tags;

        /** Scenario tag ID */
        std::size_t scenario_tag_id;

        /** Populate the tag array */
        void populate_tag_array(const std::byte *tag_data, std::size_t tag_data_size, std::uint32_t tag_data_base_address, const std::vector<Resource> &bitmaps, const std::vector<Resource> &loc, const std::vector<Resource> &sounds);

        /** Load all BSP tags */
        void load_bsps(const std::byte *data, std::size_t data_size);

        /** Load all asset data */
        void load_asset_data(const std::byte *data, std::size_t data_size, const std::byte *bitmaps_data, std::size_t bitmaps_data_size, const std::byte *sounds_data, std::size_t sounds_data_size);

        /** Load model data */
        void load_model_data(const std::byte *data, std::size_t data_size, const std::byte *tag_data);
    };
}
