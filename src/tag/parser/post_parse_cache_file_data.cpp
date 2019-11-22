// SPDX-License-Identifier: GPL-3.0-only

#include <invader/map/map.hpp>
#include <invader/map/tag.hpp>
#include <invader/tag/parser/parser.hpp>

namespace Invader::Parser {
    void Invader::Parser::ActorVariant::post_parse_cache_file_data(const Invader::Tag &, std::optional<HEK::Pointer>) {
        this->grenade_velocity *= TICK_RATE;
    }

    void Invader::Parser::Bitmap::post_parse_cache_file_data(const Invader::Tag &tag, std::optional<HEK::Pointer>) {
        for(auto &bitmap_data : this->bitmap_data) {
            const std::byte *bitmap_data_ptr;
            bitmap_data_ptr = tag.get_map().get_data_at_offset(bitmap_data.pixels_offset, bitmap_data.pixels_count, bitmap_data.flags.external ? Map::DATA_MAP_BITMAP : Map::DATA_MAP_CACHE);
            bitmap_data.pixels_offset = static_cast<std::size_t>(this->processed_pixel_data.size());
            this->processed_pixel_data.insert(this->processed_pixel_data.end(), bitmap_data_ptr, bitmap_data_ptr + bitmap_data.pixels_count);
            bitmap_data.flags.external = 0;
        }
    }

    void Invader::Parser::Scenario::post_parse_cache_file_data(const Invader::Tag &, std::optional<HEK::Pointer>) {
        auto *script_data = this->script_syntax_data.data();
        auto script_data_size = this->script_syntax_data.size();

        // If we don't have a script node table, give up
        if(script_data_size < sizeof(ScenarioScriptNodeTable::struct_little)) {
            eprintf("scenario tag has an invalid scenario script node table\n");
            throw InvalidTagDataException();
        }

        // Copy the table header
        ScenarioScriptNodeTable::struct_big table = *reinterpret_cast<ScenarioScriptNodeTable::struct_little *>(script_data);
        *reinterpret_cast<ScenarioScriptNodeTable::struct_big *>(script_data) = table;

        // Make sure it's not bullshit
        auto *script_nodes = reinterpret_cast<ScenarioScriptNode::struct_little *>(script_data + sizeof(table));
        auto table_size = table.maximum_count.read();
        std::size_t expected_size = (reinterpret_cast<std::byte *>(script_nodes + table_size) - script_data);
        if(expected_size != script_data_size) {
            eprintf("scenario tag has an invalid scenario script node table (%zu vs %zu)\n", expected_size, script_data_size);
            throw InvalidTagDataException();
        }

        // Copy the rest of the table
        for(std::size_t i = 0; i < table_size; i++) {
            ScenarioScriptNode::struct_big big = script_nodes[i];
            *reinterpret_cast<ScenarioScriptNode::struct_big *>(script_nodes + i) = big;
        }
    }

    void Invader::Parser::GBXModel::post_parse_cache_file_data(const Invader::Tag &tag, std::optional<HEK::Pointer> pointer) {
        for(auto &marker : this->markers) {
            for(auto &instance : marker.instances) {
                // Figure out the region
                std::size_t region_index = instance.region_index;
                std::size_t region_count = this->regions.size();
                if(region_index >= region_count) {
                    eprintf("invalid region index %zu / %zu\n", region_index, region_count);
                    throw OutOfBoundsException();
                }

                // Next, figure out the region permutation
                auto &region = this->regions[region_index];
                std::size_t permutation_count = region.permutations.size();
                std::size_t permutation_index = instance.permutation_index;
                if(permutation_index >= permutation_count) {
                    eprintf("invalid permutation index %zu / %zu for region #%zu\n", permutation_index, permutation_count, region_index);
                    throw OutOfBoundsException();
                }

                // Lastly
                auto &new_marker = region.permutations[permutation_index].markers.emplace_back();
                new_marker.name = marker.name;
                new_marker.node_index = instance.node_index;
                new_marker.rotation = instance.rotation;
                new_marker.translation = instance.translation;
            }
        }
        this->markers.clear();
    }

    void Invader::Parser::GlobalsFallingDamage::post_parse_cache_file_data(const Invader::Tag &, std::optional<HEK::Pointer>) {
        this->maximum_falling_distance = static_cast<float>((this->maximum_falling_distance * this->maximum_falling_distance) / GRAVITY / 2.0f);
        this->harmful_falling_velocity.from = static_cast<float>((this->harmful_falling_velocity.from * this->harmful_falling_velocity.from) / GRAVITY / 2.0f);
        this->harmful_falling_velocity.to = static_cast<float>((this->harmful_falling_velocity.to * this->harmful_falling_velocity.to) / GRAVITY / 2.0f);
    }

    void Invader::Parser::Glow::post_parse_cache_file_data(const Invader::Tag &, std::optional<HEK::Pointer>) {
        this->attachment_0 = static_cast<HEK::FunctionOut>(this->attachment_0 + 1);
        this->attachment_1 = static_cast<HEK::FunctionOut>(this->attachment_1 + 1);
        this->attachment_2 = static_cast<HEK::FunctionOut>(this->attachment_2 + 1);
        this->attachment_3 = static_cast<HEK::FunctionOut>(this->attachment_3 + 1);
        this->attachment_4 = static_cast<HEK::FunctionOut>(this->attachment_4 + 1);
        this->attachment_5 = static_cast<HEK::FunctionOut>(this->attachment_5 + 1);
    }

    void Invader::Parser::PointPhysics::post_parse_cache_file_data(const Invader::Tag &, std::optional<HEK::Pointer>) {
        this->air_friction /= 10000.0f;
        this->water_friction /= 10000.0f;
    }

    void Invader::Parser::Projectile::post_parse_cache_file_data(const Invader::Tag &, std::optional<HEK::Pointer>) {
        this->initial_velocity *= TICK_RATE;
        this->final_velocity *= TICK_RATE;
    }

    void Invader::Parser::ScenarioCutsceneTitle::post_parse_cache_file_data(const Invader::Tag &, std::optional<HEK::Pointer>) {
        this->fade_in_time *= TICK_RATE;
        this->fade_out_time *= TICK_RATE;
        this->up_time *= TICK_RATE;
    }

    void Invader::Parser::ScenarioStructureBSP::post_parse_cache_file_data(const Invader::Tag &tag, std::optional<HEK::Pointer> pointer) {
        eprintf("unimplemented\n");
        throw std::exception();
    }

    void Invader::Parser::ScenarioStructureBSPMaterial::post_parse_cache_file_data(const Invader::Tag &tag, std::optional<HEK::Pointer> pointer) {
        eprintf("unimplemented\n");
        throw std::exception();
    }

    void Invader::Parser::GBXModelGeometryPart::post_parse_cache_file_data(const Invader::Tag &tag, std::optional<HEK::Pointer> pointer) {
        const auto &part = tag.get_struct_at_pointer<HEK::GBXModelGeometryPart>(*pointer);
        const auto &map = tag.get_map();
        const auto &header = *reinterpret_cast<const HEK::CacheFileTagDataHeaderPC *>(&map.get_tag_data_header());

        // Get model vertices
        std::size_t vertex_count = part.vertex_count.read();
        const auto *vertices = reinterpret_cast<const GBXModelVertexUncompressed::struct_little *>(map.get_data_at_offset(header.model_data_file_offset.read() + part.vertex_offset.read(), sizeof(GBXModelVertexUncompressed::struct_little) * vertex_count));

        for(std::size_t v = 0; v < vertex_count; v++) {
            HEK::GBXModelVertexUncompressed<HEK::BigEndian> vertex_uncompressed = vertices[v];
            HEK::GBXModelVertexCompressed<HEK::BigEndian> vertex_compressed = HEK::compress_vertex(vertex_uncompressed);

            std::size_t data_read;
            this->uncompressed_vertices.emplace_back(GBXModelVertexUncompressed::parse_hek_tag_data(reinterpret_cast<const std::byte *>(&vertex_uncompressed), sizeof(vertex_uncompressed), data_read));
            this->compressed_vertices.emplace_back(GBXModelVertexCompressed::parse_hek_tag_data(reinterpret_cast<const std::byte *>(&vertex_compressed), sizeof(vertex_compressed), data_read));
        }

        // Get model indices
        std::size_t index_count = part.triangle_count.read() + 2;

        std::size_t triangle_count = (index_count) / 3;
        std::size_t triangle_modulo = index_count % 3;
        const auto *indices = reinterpret_cast<const HEK::LittleEndian<HEK::Index> *>(map.get_data_at_offset(header.model_data_file_offset.read() + part.triangle_offset.read() + header.vertex_size.read(), sizeof(std::uint16_t) * index_count));

        for(std::size_t t = 0; t < triangle_count; t++) {
            auto &triangle = this->triangles.emplace_back();
            auto *triangle_indices = indices + t * 3;
            triangle.vertex0_index = triangle_indices[0];
            triangle.vertex1_index = triangle_indices[1];
            triangle.vertex2_index = triangle_indices[2];
        }

        if(triangle_modulo) {
            auto &straggler_triangle = this->triangles.emplace_back();
            auto *triangle_indices = indices + triangle_count * 3;
            straggler_triangle.vertex0_index = triangle_indices[0];
            straggler_triangle.vertex1_index = triangle_modulo > 1 ? triangle_indices[1] : NULL_INDEX;
            straggler_triangle.vertex2_index = NULL_INDEX;
        }
    }

    void Invader::Parser::Sound::post_parse_cache_file_data(const Invader::Tag &tag, std::optional<HEK::Pointer>) {
        this->maximum_bend_per_second = std::pow(this->maximum_bend_per_second, TICK_RATE);
        if(tag.is_indexed()) {
            auto &tag_data = *(reinterpret_cast<const struct_little *>(&tag.get_struct_at_pointer<HEK::SoundPitchRange>(0, 0)) - 1);
            this->compression = tag_data.compression;
            this->encoding = tag_data.encoding;
        }
    }
}
