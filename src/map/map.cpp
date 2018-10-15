/*
 * Invader (c) 2018 Kavawuvi
 *
 * This program is free software under the GNU General Public License v3.0 or later. See LICENSE for more information.
 */

#include "../hek/map.hpp"
#include "../tag/hek/class/bitmap.hpp"
#include "../tag/hek/class/gbxmodel.hpp"
#include "../tag/hek/class/scenario.hpp"
#include "../tag/hek/class/sound.hpp"

#include "map.hpp"

namespace Invader {
    const std::string &Map::name() const {
        return this->p_name;
    }

    const std::string &Map::build() const {
        return this->p_build;
    }

    std::size_t Map::tag_count() const noexcept {
        return this->tags.size();
    }

    Tag &Map::get_tag(std::size_t tag_index) {
        if(tag_index >= this->tags.size()) {
            throw InvalidDependencyException();
        }
        return *this->tags[tag_index];
    }

    const Tag &Map::get_tag(std::size_t tag_index) const {
        return const_cast<Map *>(this)->get_tag(tag_index);
    }

    Tag &Map::get_tag(const HEK::TagDependency<HEK::LittleEndian> &tag_id) {
        return this->get_tag(tag_id.tag_id.read().index);
    }

    const Tag &Map::get_tag(const HEK::TagDependency<HEK::LittleEndian> &tag_id) const {
        return const_cast<Map *>(this)->get_tag(tag_id);
    }

    Tag *Map::get_tag_nullable(const HEK::TagDependency<HEK::LittleEndian> &tag_id) {
        if(tag_id.tag_id.read().id == 0xFFFFFFFF) {
            return nullptr;
        }
        return &this->get_tag(tag_id);
    }

    const Tag *Map::get_tag_nullable(const HEK::TagDependency<HEK::LittleEndian> &tag_id) const {
        return const_cast<Map *>(this)->get_tag_nullable(tag_id);
    }

    Tag &Map::get_scenario_tag() noexcept {
        return this->get_tag(this->scenario_tag_id);
    }

    const Tag &Map::get_scenario_tag() const noexcept {
        return const_cast<Map *>(this)->get_scenario_tag();
    }

    Map::Map(const std::byte *data, std::size_t data_size,
             const std::byte *bitmaps_data, std::size_t bitmaps_data_size,
             const std::byte *loc_data, std::size_t loc_data_size,
             const std::byte *sounds_data, std::size_t sounds_data_size) {
        using namespace HEK;

        if(data_size < sizeof(CacheFileHeader)) {
            throw InvalidMapException();
        }

        const auto &header = *reinterpret_cast<const CacheFileHeader *>(data);

        if(header.file_size > data_size || header.head_literal != CACHE_FILE_HEAD || header.foot_literal != CACHE_FILE_FOOT || header.build.string[31] != 0) {
            throw InvalidMapException();
        }

        this->p_name = header.name.string;
        this->p_build = header.build.string;

        std::size_t tag_data_size = header.tag_data_size;

        if(header.tag_data_offset > data_size || header.tag_data_offset + tag_data_size > data_size || tag_data_size < sizeof(HEK::CacheFileTagDataHeaderPC)) {
            throw InvalidMapException();
        }

        // Load resource maps if provided
        std::vector<Resource> bitmaps;
        std::vector<Resource> loc;
        std::vector<Resource> sounds;
        if(bitmaps_data_size) {
            bitmaps = load_resource_map(bitmaps_data, bitmaps_data_size);
        }
        if(loc_data_size) {
            loc = load_resource_map(loc_data, loc_data_size);
        }
        if(sounds_data_size) {
            sounds = load_resource_map(sounds_data, sounds_data_size);
        }

        const auto tag_data_base_address = CACHE_FILE_PC_BASE_MEMORY_ADDRESS;
        const auto *tag_data = data + header.tag_data_offset;

        this->populate_tag_array(tag_data, tag_data_size, tag_data_base_address, bitmaps, loc, sounds);
        this->load_bsps(data, data_size);
        this->load_asset_data(data, data_size, bitmaps_data, bitmaps_data_size, sounds_data, sounds_data_size);
        this->load_model_data(data, data_size, tag_data);
    }

    Map::Map(const std::vector<std::byte> &data,
             const std::vector<std::byte> &bitmaps_data,
             const std::vector<std::byte> &loc_data,
             const std::vector<std::byte> &sounds_data) :
             Map(data.data(), data.size(), bitmaps_data.data(), bitmaps_data.size(), loc_data.data(), loc_data.size(), sounds_data.data(), sounds_data.size()) {}

    Map::Map(const Map &copy) : p_name(copy.p_name), p_build(copy.p_build), scenario_tag_id(copy.scenario_tag_id) {
        for(auto &tag : copy.tags) {
            auto *tag_copy = new Tag(*this);
            this->tags.emplace_back(std::unique_ptr<Tag>(tag_copy));
            tag_copy->p_path = tag->p_path;
            tag_copy->p_tag_class_int = tag->p_tag_class_int;
            tag_copy->p_tag_data = tag->p_tag_data;
            tag_copy->p_asset_data = tag->p_asset_data;
            tag_copy->base_address = tag->base_address;
        }
    }

    void Map::populate_tag_array(const std::byte *tag_data, std::size_t tag_data_size, std::uint32_t tag_data_base_address, const std::vector<Resource> &bitmaps, const std::vector<Resource> &loc, const std::vector<Resource> &sounds) {
        using namespace HEK;
        auto resolve_offset = [&tag_data_size, &tag_data_base_address](std::uint32_t pointer, std::size_t min_size = 0) {
            auto offset = pointer - tag_data_base_address;
            if(offset > tag_data_size || offset + min_size > tag_data_size) {
                throw OutOfBoundsException();
            }
            return offset;
        };

        auto resolve_ptr = [&tag_data, &resolve_offset](std::uint32_t pointer, std::size_t min_size = 0) {
            return tag_data + resolve_offset(pointer, min_size);
        };

        const auto &tag_data_header = *reinterpret_cast<const HEK::CacheFileTagDataHeaderPC *>(tag_data);
        const std::size_t tag_count = tag_data_header.tag_count;

        // Get the scenario tag ID
        this->scenario_tag_id = tag_data_header.scenario_tag.read().index;
        if(this->scenario_tag_id >= tag_count) {
            throw InvalidMapException();
        }

        const auto *tags_ptr = reinterpret_cast<const CacheFileTagDataTag *>(resolve_ptr(tag_data_header.tag_array_address));
        for(std::size_t i = 0; i < tag_count; i++) {
            auto *tag = new Tag(*this);
            this->tags.emplace_back(std::unique_ptr<Tag>(tag));
            const auto &cache_tag = tags_ptr[i];

            // Get the tag path
            auto tag_path_offset = resolve_offset(cache_tag.tag_path, 1);
            const auto *tag_path = reinterpret_cast<const char *>(tag_data + tag_path_offset);
            std::size_t length = 0;
            while(tag_path[length]) {
                length++;
                if(tag_path_offset + length >= tag_data_size) {
                    throw OutOfBoundsException();
                }
            }
            tag->p_path = tag_path;
            tag->p_tag_class_int = cache_tag.primary_class;

            // If indexed, search for it in the provided maps
            if(cache_tag.indexed) {
                std::size_t index = cache_tag.tag_data;
                if(cache_tag.primary_class == TagClassInt::TAG_CLASS_SOUND) {
                    if(sounds.size()) {
                        bool found = false;
                        for(auto &resource : sounds) {
                            if(resource.name == tag->p_path) {
                                tag->p_tag_data = resource.data;
                                found = true;
                                break;
                            }
                        }
                        if(!found) {
                            throw OutOfBoundsException();
                        }
                    }
                }
                else if(cache_tag.primary_class == TagClassInt::TAG_CLASS_BITMAP) {
                    if(bitmaps.size()) {
                        if(index >= bitmaps.size()) {
                            throw OutOfBoundsException();
                        }
                        else {
                            tag->p_tag_data = bitmaps[index].data;
                        }
                    }
                }
                else {
                    if(loc.size()) {
                        if(index >= loc.size()) {
                            throw OutOfBoundsException();
                        }
                        else {
                            tag->p_tag_data = loc[index].data;
                        }
                    }
                }
            }
            else if(cache_tag.primary_class != TagClassInt::TAG_CLASS_SCENARIO_STRUCTURE_BSP) {
                auto tag_data_offset = resolve_offset(cache_tag.tag_data);
                std::size_t size = tag_data_size - tag_data_offset;
                for(std::size_t j = 0; j < tag_count; j++) {
                    if(tags_ptr[j].indexed || tags_ptr[j].primary_class == TagClassInt::TAG_CLASS_SCENARIO_STRUCTURE_BSP) {
                        continue;
                    }
                    auto other_tag_offset = resolve_offset(tags_ptr[j].tag_data);
                    if(other_tag_offset <= tag_data_offset) {
                        continue;
                    }
                    auto difference = other_tag_offset - tag_data_offset;
                    if(size > difference) {
                        size = difference;
                    }
                }
                auto *tag_data_ptr = tag_data + tag_data_offset;
                tag->p_tag_data = std::vector<std::byte>(tag_data_ptr, tag_data_ptr + size);
                tag->base_address = cache_tag.tag_data;
            }
        }
    }

    void Map::load_bsps(const std::byte *data, std::size_t data_size) {
        using namespace HEK;

        auto &scenario_tag = this->get_scenario_tag();
        auto &scenario_tag_struct = scenario_tag.get_base_struct<Scenario>();

        std::size_t bsp_count = scenario_tag_struct.structure_bsps.count;
        auto *bsps = scenario_tag.resolve_reflexive(scenario_tag_struct.structure_bsps);
        for(std::size_t b = 0; b < bsp_count; b++) {
            std::size_t bsp_start = bsps[b].bsp_start;
            std::size_t bsp_size = bsps[b].bsp_size;
            if(bsp_start > data_size || bsp_start + bsp_size > data_size) {
                throw OutOfBoundsException();
            }
            auto &bsp_tag = this->get_tag(bsps[b].structure_bsp);
            if(bsp_tag.p_tag_class_int != TagClassInt::TAG_CLASS_SCENARIO_STRUCTURE_BSP) {
                throw InvalidDependencyException();
            }
            bsp_tag.p_tag_data = std::vector<std::byte>(data + bsp_start, data + bsp_start + bsp_size);
            bsp_tag.base_address = bsps[b].bsp_address;
        }
    }

    void Map::load_asset_data(const std::byte *data, std::size_t data_size, const std::byte *bitmaps_data, std::size_t bitmaps_data_size, const std::byte *sounds_data, std::size_t sounds_data_size) {
        using namespace HEK;

        for(std::size_t i = 0; i < this->tags.size(); i++) {
            auto &tag = this->tags[i];
            if(tag->p_tag_data.size() == 0) {
                continue;
            }
            if(tag->p_tag_class_int == TagClassInt::TAG_CLASS_BITMAP) {
                auto &bitmaps_tag = tag->get_base_struct<Bitmap>();
                std::size_t bitmaps_count = bitmaps_tag.bitmap_data.count;
                auto *bitmaps = tag->resolve_reflexive(bitmaps_tag.bitmap_data);
                for(std::size_t b = 0; b < bitmaps_count; b++) {
                    auto &bitmap = bitmaps[b];
                    std::size_t bitmap_offset = bitmap.pixels_offset;
                    std::size_t bitmap_size = bitmap.pixels_count;
                    auto flags = bitmap.flags.read();
                    if(flags.external) {
                        if(bitmap_offset < bitmaps_data_size && bitmap_size + bitmap_offset <= bitmaps_data_size) {
                            bitmap.pixels_offset = static_cast<std::int32_t>(tag->p_asset_data.size());
                            flags.external = 0;
                            bitmap.flags = flags;
                            tag->p_asset_data.insert(tag->p_asset_data.end(), bitmaps_data + bitmap_offset, bitmaps_data + bitmap_offset + bitmap_size);
                        }
                        else if(bitmaps_data_size != 0) {
                            throw OutOfBoundsException();
                        }
                    }
                    else {
                        if(bitmap_offset < data_size && bitmap_offset + bitmap_size <= data_size) {
                            bitmap.pixels_offset = static_cast<std::int32_t>(tag->p_asset_data.size());
                            tag->p_asset_data.insert(tag->p_asset_data.end(), data + bitmap_offset, data + bitmap_offset + bitmap_size);
                        }
                        else {
                            throw OutOfBoundsException();
                        }
                    }
                }
            }
            else if(tag->p_tag_class_int == TagClassInt::TAG_CLASS_SOUND) {
                auto &sounds_tag = tag->get_base_struct<Sound>();
                std::size_t pitch_range_count = sounds_tag.pitch_ranges.count;
                auto *pitch_ranges = tag->resolve_reflexive(sounds_tag.pitch_ranges);
                for(std::size_t p = 0; p < pitch_range_count; p++) {
                    std::size_t permutations_count = pitch_ranges[p].permutations.count;
                    auto *permutations = tag->resolve_reflexive(pitch_ranges[p].permutations);
                    for(std::size_t e = 0; e < permutations_count; e++) {
                        auto &permutation = permutations[e];
                        std::size_t sound_offset = permutation.samples.file_offset;
                        std::size_t sound_size = permutation.samples.size;
                        if(permutation.samples.external) {
                            if(sound_offset < sounds_data_size && sound_size + sound_offset <= sounds_data_size) {
                                permutation.samples.file_offset = static_cast<std::int32_t>(tag->p_asset_data.size());
                                permutation.samples.external = 0;
                                tag->p_asset_data.insert(tag->p_asset_data.end(), sounds_data + sound_offset, sounds_data + sound_offset + sound_size);
                            }
                            else if(sounds_data_size != 0) {
                                throw OutOfBoundsException();
                            }
                        }
                        else {
                            if(sound_offset < data_size && sound_size + sound_offset <= data_size) {
                                permutation.samples.file_offset = static_cast<std::int32_t>(tag->p_asset_data.size());
                                permutation.samples.external = 0;
                                tag->p_asset_data.insert(tag->p_asset_data.end(), data + sound_offset, data + sound_offset + sound_size);
                            }
                            else if(sounds_data_size != 0) {
                                throw OutOfBoundsException();
                            }
                        }
                    }
                }
            }
        }
    }

    void Map::load_model_data(const std::byte *data, std::size_t data_size, const std::byte *tag_data) {
        using namespace HEK;

        const auto &tag_data_header = *reinterpret_cast<const HEK::CacheFileTagDataHeaderPC *>(tag_data);

        std::size_t model_data_offset = tag_data_header.model_data_file_offset;
        std::size_t model_data_size = tag_data_header.model_data_size;

        if(model_data_offset > data_size || model_data_offset + model_data_size > data_size) {
            throw OutOfBoundsException();
        }

        const auto *vertices = data + model_data_offset;

        std::size_t model_data_index_offset = tag_data_header.vertex_size;
        if(model_data_index_offset > model_data_size) {
            throw OutOfBoundsException();
        }
        std::size_t indices_size = model_data_size - model_data_index_offset;

        const auto *indices = vertices + model_data_index_offset;

        for(std::size_t t = 0; t < this->tags.size(); t++) {
            auto &tag = this->tags[t];
            if(tag->p_tag_class_int != TagClassInt::TAG_CLASS_GBXMODEL) {
                continue;
            }
            auto &model_tag = tag->get_base_struct<GBXModel>();
            std::size_t geometry_count = model_tag.geometries.count;
            auto *geometries = tag->resolve_reflexive(model_tag.geometries);
            for(std::size_t g = 0; g < geometry_count; g++) {
                auto &geometry = geometries[g];
                std::size_t part_count = geometry.parts.count;
                auto *parts = tag->resolve_reflexive(geometry.parts);
                for(std::size_t p = 0; p < part_count; p++) {
                    auto &part = parts[p];

                    // Add vertices
                    std::size_t vertex_offset = part.vertex_offset;
                    std::size_t vertex_size = sizeof(GBXModelVertexUncompressed<LittleEndian>) * part.vertex_count;
                    if(vertex_offset > model_data_index_offset || vertex_offset + vertex_size > model_data_index_offset) {
                        throw OutOfBoundsException();
                    }
                    auto *vertex_data = vertices + vertex_offset;
                    part.vertex_offset = static_cast<std::uint32_t>(tag->p_asset_data.size());
                    tag->p_asset_data.insert(tag->p_asset_data.end(), vertex_data, vertex_data + vertex_size);

                    // Add indices
                    std::size_t index_offset = part.triangle_offset;
                    std::size_t index_size = 2 * (part.triangle_count + 2);
                    if(index_offset > indices_size || index_offset + index_size > indices_size) {
                        throw OutOfBoundsException();
                    }
                    auto *index_data = indices + index_offset;
                    part.triangle_offset = static_cast<std::uint32_t>(tag->p_asset_data.size());
                    part.triangle_offset_2 = static_cast<std::uint32_t>(tag->p_asset_data.size());
                    tag->p_asset_data.insert(tag->p_asset_data.end(), index_data, index_data + index_size);
                }
            }
        }
    }
}
